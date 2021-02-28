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

import logging

from graph.types import ConcatParameters

from ..adjust_base import AdjusterBase, handles

LOG = logging.getLogger("nntool." + __name__)

@handles(ConcatParameters)
class ConcatAdjuster(AdjusterBase):
    def adjust(self, G, node):
        # if the concat axis is already 0 nothing to do
        if node.axis == 0:
            return False
        LOG.info("concat %s: moving axis %s to 0 and inserting transposes", node.name, node.axis)
        trans = self.move_axis_to_zero_trans(node.axis, node.out_dims[0].shape)
        self.apply_input_trans(node, trans)
        node.axis = 0
        self.apply_output_trans(node, self.invert(trans), index=0)
        return True
