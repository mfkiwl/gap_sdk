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

import numpy as np
from nntool.graph.types import ConcatNode, ConstantInputNode, NNEdge
from nntool.importer.common.provisional_dim import ProvisionalDim
from nntool.importer.onnx.common import logger

from nntool.importer.common.constant_mixin import ConstantMixin

def print_small(tensor):
    if tensor.size < 6:
        return str(tensor)
    return ""

class ConcatMixin(ConstantMixin):

    @classmethod
    def gen_concat(cls, node, inputs, axis, **kwargs):
        all_nodes = kwargs['all_nodes']
        G = kwargs['G']
        valid_name = kwargs['valid_name']
        inputs = [all_nodes[inp] for inp in node.input if all_nodes[inp][2].shape]
        input_shapes = [inp[2].shape for inp in inputs]
        axis_sum = sum(shape[axis] for shape in input_shapes)
        axis = axis if axis >= 0 else len(input_shapes[0]) + axis
        output_shape = [axis_sum if idx == axis else dim for idx, dim in enumerate(input_shapes[0])]
        pout_dim = ProvisionalDim(output_shape)
        none_dims = sum([1 if dim is None else 0 for dim in output_shape[:axis:]])
        if all(cls.is_constant(inp) for inp in inputs):
            value = np.concatenate([cls.get_constant(inp) for inp in inputs], axis=axis)
            logger.info(f"reducing {valid_name} to a constant {print_small(value)}")
            params = ConstantInputNode(valid_name, value=value)
        else:
            params = ConcatNode(valid_name, axis=axis - none_dims)
            for idx, inp in enumerate(inputs):
                G.add_edge(NNEdge(from_node=inp[0], to_node=params, from_idx=inp[1], to_idx=idx))
        all_nodes[node.output[0]] = (params, 0, pout_dim, inputs[0][3])
        return params
