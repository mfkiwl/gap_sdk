# Copyright (C) 2020  GreenWaves Technologies, SAS

# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU Affero General Public License as
# published by the Free Software Foundation, either version 3 of the
# License, or (at your option) any later version.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Affero General Public License for more details.

# You should have received a copy of the GNU Affero General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

import argparse
import logging
from pathlib import Path

import numpy as np
from cmd2 import Cmd, Cmd2ArgumentParser, with_argparser
from nntool.interpreter.commands.qtune import load_options
from nntool.interpreter.nntool_shell_base import NNToolShellBase
from nntool.quantization.handlers_helpers import (add_options_to_parser,
                                           get_options_from_args)
from nntool.quantization.quantizer.new_quantizer import NewQuantizer
from nntool.utils.stats_funcs import STATS_BITS

from nntool.graph.types.constant_input import ConstantInputNode
from nntool.stats.activation_ranges_collector import ActivationRangesCollector

QUANTIZATION_SCHEMES = ['SQ8', 'POW2', 'FLOAT']

LOG = logging.getLogger(__name__)


class FquantCommand(NNToolShellBase):
    # FQUANT COMMAND
    parser_fquant = Cmd2ArgumentParser()
    parser_fquant.add_argument('-f',
                               '--force_width',
                               choices=STATS_BITS, type=int, default=0,
                               help='force all layers to this bit-width in case of POW2 scheme, ' +
                               'SQ8 will automatically force 8-bits')
    parser_fquant.add_argument('-s', '--scheme',
                               type=str, choices=QUANTIZATION_SCHEMES, default='SQ8',
                               help='quantize with scaling factors (TFlite quantization-like) [default] or POW2')
    parser_fquant.add_argument('--uniform',
                               type=float, default=0.0,
                               help='Use uniform distribution for input with the specified max value')
    parser_fquant.add_argument('--normal',
                               type=float, default=0.2,
                               help='Use normal distribution for input with the specified scale value')
    parser_fquant.add_argument('--num_inference',
                               type=int, default=1,
                               help='How many inferences')
    parser_fquant.add_argument('--seed',
                               type=int, default=0,
                               help='numpy random seed, default not set and inputs change every time')
    parser_fquant.add_argument('--json',
                               completer_method=Cmd.path_complete,
                               help='json file file containing saved quantization options using qtunesave command')
    add_options_to_parser(parser_fquant)

    @with_argparser(parser_fquant)
    def do_fquant(self, args: argparse.Namespace):
        """
Attempt to calculate a fake quantization for graph using random tensors and parameters.
This is intended to allow code generation for performance testing even if no real
weights and input data are avalaible."""
        self._check_graph()
        graph_options = get_options_from_args(args)
        node_options = {}
        if args.json:
            json_path = Path(args.json)
            if not json_path.exists() or not json_path.is_file():
                self.perror(f'{json_path} does not exist or is not a file')
                return
            saved_graph_options, node_options = load_options(json_path)
            saved_graph_options.update(graph_options)
            graph_options = saved_graph_options

        state = ConstantInputNode.save_compression_state(self.G)
        try:
            if self.replaying_history and self.history_stats:
                astats = self.history_stats
            else:
                if args.seed:
                    np.random.seed(args.seed)
                ConstantInputNode.fake(self.G, True)
                stats_collector = ActivationRangesCollector()
                for _ in range(args.num_inference):
                    if args.uniform:
                        input_tensors = [np.random.uniform(-args.uniform, args.uniform, inp.dims.shape)
                                        for inp in self.G.input_nodes()]
                    else:
                        input_tensors = [np.random.normal(0, args.normal, inp.dims.shape)
                                        for inp in self.G.input_nodes()]
                    stats_collector.collect_stats(self.G, input_tensors)
                astats = stats_collector.stats
                self._record_stats(astats)
                ConstantInputNode.fake(self.G, False)

            if args.force_width:
                graph_options['bits'] = args.force_width

            quantizer = NewQuantizer(self.G)
            quantizer.schemes.append(args.scheme)
            quantizer.set_stats(
                current_stats=astats,
                current_graph_options=graph_options,
                current_node_options=node_options)
            quantizer.quantize()
            LOG.info("Quantization set. Use qshow command to see it.")
        finally:
            ConstantInputNode.restore_compression_state(self.G, state)
