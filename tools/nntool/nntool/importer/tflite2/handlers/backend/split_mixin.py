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

from nntool.graph.types import NNEdge, SplitNode
from nntool.importer.common.provisional_dim import ProvisionalDim

from ..backend_handler import BackendHandler


class SplitMixin(object):
    @classmethod
    def _common(cls, node, **kwargs):
        all_nodes = kwargs['all_nodes']
        G = kwargs['G']
        axis = kwargs['axis']
        splits = kwargs.get('splits')
        opts = kwargs['opts']
        input_idx = kwargs.get('input_idx', 0)
        num_splits = kwargs.get('num_splits')
        # eliminates a silly split / unpack that does dothing - seen in dtln1.tflite
        if splits is None and num_splits == 1 and len(node.output) == 1:
            all_nodes[node.output[0]] = all_nodes[node.input[0]]
            return None

        inputs = [all_nodes[inp] for inp in node.input]
        x = inputs[input_idx]
        x_shape = x[2].shape
        if axis and axis < 0:
            axis = axis + len(x_shape)
        act_slices, pout_shapes, axis = SplitNode.get_splits(
            x_shape, axis, splits=splits,
            num_splits=num_splits)
        out_shapes = [BackendHandler.remove_unspecified_dim(shape) for shape in pout_shapes]
        params = SplitNode(node.name, act_slices=act_slices, out_shapes=out_shapes, axis=axis)

        if opts.get('load_quantization'):
            G.quantization[params.name] = BackendHandler.load_tf_quantization([node.input[0]], node.output)

        G.add_edge(NNEdge(from_node=x[0], to_node=params, from_idx=x[1], to_idx=0))

        for idx, tensor in enumerate(node.output):
            all_nodes[tensor] = (params, idx, ProvisionalDim(pout_shapes[idx]))
        return params
