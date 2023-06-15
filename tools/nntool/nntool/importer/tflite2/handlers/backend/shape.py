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
from nntool.graph.dim import Dim
from nntool.graph.types import ConstantInputNode
from nntool.importer.common.provisional_dim import ProvisionalDim
from nntool.importer.tflite2.common.tflite_node import TFLiteNode

from ..backend_handler import BackendHandler
from ..handler import tflite_op, partial_support, ps_description


@tflite_op("SHAPE")
@partial_support(True)
@ps_description('Shape is always reduced to a constant since tensors must always have a known shape in nntool')
class Shape(BackendHandler):

    @classmethod
    def _common(cls, node: TFLiteNode, **kwargs):
        all_nodes = kwargs['all_nodes']
        G = kwargs['G']

        inputs = [all_nodes[t] for t in node.input]
        x = inputs[0]
        x_shape = x[2].shape
        node.input[0].used = True
        params = ConstantInputNode(node.name,
                                         dims=Dim.unnamed([len(x_shape)]),
                                         value=np.array([1 if dim is None else dim for dim in x_shape]))
        all_nodes[node.output[0]] = (params, 0, ProvisionalDim([len(x_shape)]))
        return params

    @classmethod
    def version_1(cls, node: TFLiteNode, **kwargs):
        return cls._common(node, **kwargs)
