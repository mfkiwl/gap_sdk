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

from nntool.graph.dim import Dim
from nntool.graph.types.base import NNEdge
from nntool.graph.types.lstm import LSTMNode
from nntool.importer.common.provisional_dim import ProvisionalDim
from nntool.importer.tflite2.tflite_schema_head.UnidirectionalSequenceLSTMOptions import \
    UnidirectionalSequenceLSTMOptions

from ..backend_handler import BackendHandler
from ..handler import tflite_op


@tflite_op("UNIDIRECTIONAL_SEQUENCE_LSTM")
class UnidirectionalSequenceLSTM(BackendHandler):

    @classmethod
    def _common(cls, node, **kwargs):
        node_opts = node.get_options(UnidirectionalSequenceLSTMOptions)
        G = kwargs['G']
        opts = kwargs['opts']
        all_nodes = kwargs['all_nodes']

        inputs = [all_nodes.get(t) for t in node.input]
        if node_opts:
            time_major = node_opts.TimeMajor()
            cell_clip = node_opts.CellClip()
            proj_clip = node_opts.ProjClip()
            activation = cls.TF_ACTIVATIONS[node_opts.FusedActivationFunction()]
        else:
            time_major = False
            cell_clip = 0.0
            proj_clip = 0.0
            activation = "tanh"

        x = cls.remove_known_batch_dimension(G, inputs[0], node, batch_axis=1 if time_major else 0)
        x_shape = x[2].shape

        max_time = int(x_shape[0 if time_major else 1])
        n_cells = int(node.input[2].shape[0])
        n_inputs = int(x_shape[2])
        pout_dims = ProvisionalDim([x_shape[0], x_shape[1], n_cells])
        params = LSTMNode(node.name,
                                in_dims_hint=[['sz', 'c']],
                                out_dims_hint=[['sz', 'c']],
                                cell_clip=cell_clip,
                                proj_clip=proj_clip,
                                n_input_cells=max_time,
                                n_cells=max_time,  # TF says max_time - we say cells
                                n_inputs=n_inputs,  # Input will be n_input_cells, n_inputs
                                n_output_cells=max_time,  # Output will be n_output_cells, n_states
                                n_states=n_cells,  # TF says cells - we say states
                                activation=activation)

        constant_nodes = cls.get_all_const_inputs(
            G,
            all_nodes,
            opts,
            node,
            params,
            exclude=[0],
            names=["%s_%s" % (in_name, node.name)
                   for in_name in LSTMNode.INPUT_NAMES],
            short_names=LSTMNode.INPUT_NAMES,
            load_quantization_if_present=True,
            skip_empty_tensors=False)

        # trim batch dimension from state values
        for state_node_name in ['i_state', 'c_state']:
            state_node = constant_nodes[LSTMNode.INPUT_NAMES.index(
                state_node_name)]
            state_node.value = state_node.value[0]
            state_node.dims = Dim(
                list(state_node.value.shape), is_ordered=True)
            # set by default as allocated
            state_node.at_options.allocate = True
            state_node.is_constant = False
            # reset state after each invocation
            state_node.always_copy = True
            # add a single reset
            state_node.reset_name = "Reset"

        if opts.get('load_quantization'):
            G.quantization[params.name] = cls.load_tf_quantization(node.input, node.output)

        G.add_edge(
            NNEdge(from_node=x[0], to_node=params, from_idx=x[1], to_idx=0))
        all_nodes[node.output[0]] = (params, 0, pout_dims)
        return params

    @classmethod
    def version_1(cls, node, **kwargs):
        return cls._common(node, **kwargs)

    @classmethod
    def version_4(cls, node, **kwargs):
        return cls._common(node, **kwargs)
