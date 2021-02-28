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
from graph.dim import Dim, FcFilterDim
from graph.types import FcParameters, NNEdge
from importer.common.provisional_dim import ProvisionalDim
from utils.sparse_list import SparseList

from ..backend_handler import BackendHandler
from ..handler import onnx_op, partial_support, ps_description
from .promote_linear_mixin import PromoteLinearMixin


@onnx_op("Gemm")
@partial_support(True)
@ps_description("Supports only operations mapped onto autotiler linear kernel at present")
class Gemm(PromoteLinearMixin, BackendHandler):

    @classmethod
    def _common(cls, node, **kwargs):
        all_nodes = kwargs['all_nodes']
        G = kwargs['G']
        valid_name = kwargs['valid_name']

        trans_a = node.attrs.get('transA', 0)
        trans_b = node.attrs.get('transB', 0)
        alpha = node.attrs.get('alpha', 1.0)
        beta = node.attrs.get('beta', 1.0)

        inputs = [all_nodes[inp] for inp in node.input]

        x = inputs[0]
        x_shape = x[2].shape
        y = inputs[1]
        y_shape = y[2].shape

        real_x_shape = cls._get_real_dim(x_shape)
        real_y_shape = cls._get_real_dim(y_shape)

        real_x_shape = [real_x_shape[1], real_x_shape[0]] if len(
            real_x_shape) == 2 and trans_a else real_x_shape
        real_y_shape = [real_y_shape[1], real_y_shape[0]] if len(
            real_y_shape) == 2 and trans_b else real_y_shape

        if not cls.is_linear(y, real_x_shape, real_y_shape) or trans_a:
            raise ValueError(
                "GEMM is only currently supported for operations that map onto a linear kernel")

        if len(inputs) > 2:
            has_bias = True
            biases = cls.get_constant(inputs[2])
        else:
            biases = None
            has_bias = False

        filt_dim = FcFilterDim(real_y_shape[1], real_x_shape[0])
        weights = cls.get_constant(y) * alpha
        if not trans_b:
            weights = np.transpose(weights, [1, 0])
        params = FcParameters(valid_name, filt=filt_dim, has_bias=has_bias,
                              in_dims_hint=SparseList([['c']]),
                              out_dims_hint=SparseList([['c']]),
                              constant_store=G.constant_store)
        params.weights = weights
        params.biases = biases * beta
        out_dims = params.get_output_size([Dim.unnamed(real_x_shape)])
        G.add_edge(NNEdge(from_node=x[0], to_node=params, from_idx=x[1], to_idx=0))
        if isinstance(x[2], ProvisionalDim):
            out_dim = x[2].infer_mapping(out_dims[0].shape)
        else:
            out_dim = out_dims[0]
        all_nodes[node.output[0]] = (params, 0, out_dim)
        return params

    @classmethod
    def version_1(cls, node, **kwargs):
        return cls._common(node, **kwargs)

    @classmethod
    def version_6(cls, node, **kwargs):
        return cls._common(node, **kwargs)

    @classmethod
    def version_7(cls, node, **kwargs):
        return cls._common(node, **kwargs)

    @classmethod
    def version_9(cls, node, **kwargs):
        return cls._common(node, **kwargs)

    @classmethod
    def version_11(cls, node, **kwargs):
        return cls._common(node, **kwargs)
