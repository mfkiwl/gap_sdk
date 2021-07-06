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

from cmd2 import Cmd2ArgumentParser, with_argparser
from interpreter.nntool_shell_base import NNToolShellBase

class AdjustCommand(NNToolShellBase):
    # ADJUST COMMAND
    parser_adjust = Cmd2ArgumentParser()
    parser_adjust.add_argument('-n', '--no_postprocess',
                               action='store_true', help='Don\'t try to eliminate transposes')
    parser_adjust.add_argument('-o', '--one_cycle',
                               action='store_true', help='Do one cycle of post processing')
    @with_argparser(parser_adjust)
    def do_adjust(self, args):
        """
Adjust activation and parameter tensors to match AutoTiler order.
Must be run before generating code."""
        self._check_graph()
        self.G.adjust_order(postprocess=not args.no_postprocess, one_cycle=args.one_cycle)
        self.G.add_dimensions()
        