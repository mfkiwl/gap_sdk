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
from nntool.graph.dim import Dim, FcFilterDim
from nntool.graph.types import (ConstantInputNode, LinearNode,
                         MatMulTransposedNode, NNEdge, TransposeNode)
from nntool.graph.types.tensor_shape import ReshapeNode
from nntool.quantization.new_qrec import QRec
from nntool.quantization.qtype import QType

from .promote_linear_mixin import PromoteLinearMixin


class MatMulMixin(PromoteLinearMixin):

    @classmethod
    def _handle(cls, node, quantized=False, **kwargs):
        all_nodes = kwargs['all_nodes']
        G = kwargs['G']
        valid_name = kwargs['valid_name']
        inputs = [all_nodes[inp] for inp in node.input]
        x = inputs[0]
        x_shape = cls._get_real_dim(x[2].shape)
        y_idx = 3 if quantized else 1
        y = inputs[y_idx]
        y_shape = cls._get_real_dim(y[2].shape)

        if quantized:
            qrecs = kwargs['qrecs']
            x_zp = cls.get_constant(inputs[2])
            x_scale = cls.get_constant(inputs[1])
            if len(x_scale) > 1:
                raise NotImplementedError('QMatMul scales must be scalar')
            x_qtype = QType(dtype=x_zp.dtype, scale=x_scale, zero_point=x_zp)
            y_zp = cls.get_constant(inputs[5])
            y_scale = cls.get_constant(inputs[4])
            if len(y_scale) > 1:
                raise NotImplementedError('QMatMul scales must be scalar')
            y_qtype = QType(
                dtype=y_zp.dtype, scale=y_scale,
                zero_point=y_zp)
            o_zp = cls.get_constant(inputs[7])
            o_scale = cls.get_constant(inputs[6])
            o_qtype = QType(dtype=o_zp.dtype, scale=o_scale, zero_point=o_zp)
        else:
            o_qtype = None


        if cls.is_linear(y, x_shape, y_shape):
            filt_dim = FcFilterDim(y_shape[1], np.prod(x_shape))
            weights = np.transpose(cls.get_constant(y), [1, 0])
            weights_params = ConstantInputNode(f'{valid_name}_weights',
                                                     dims=Dim.unnamed(
                                                         [y_shape[1], np.prod(x_shape)]),
                                                     value=weights)
            cls.record_constant_qrec(y, weights_params, **kwargs)
            params = LinearNode(valid_name, filter_dim=filt_dim, has_bias=True,
                                #   in_dims_hint=[
                                #       ['c'], ['out_c', 'in_c'], ['out_c']],
                                  in_dims_hint=[
                                      None, ['out_c', 'in_c'], ['out_c']],
                                  out_dims_hint=[['c']])

            out_dims = params.get_output_size([Dim.unnamed(x_shape)])
            biases_params = ConstantInputNode(f'{valid_name}_biases', dims=Dim.unnamed([y_shape[1]]),
                                                    value=np.zeros((y_shape[1]), dtype=np.float32))
            G.add_edge(NNEdge(from_node=weights_params,
                              to_node=params, to_idx=1))
            G.add_edge(NNEdge(from_node=biases_params,
                              to_node=params, to_idx=2))
            if quantized:
                weights_params.qtype = y_qtype
                qrecs[params.name] = QRec.scaled(
                    in_qs=[x_qtype, y_qtype, None],
                    out_qs=[o_qtype],
                )
        else:
            params = MatMulTransposedNode(valid_name)
            trans_shape = [i for i in range(len(y_shape))]
            temp = trans_shape[-1]
            trans_shape[-1] = trans_shape[-2]
            trans_shape[-2] = temp
            trans2 = TransposeNode(f'{valid_name}_tin2', transpose=tuple(trans_shape))
            out_dims = params.get_output_size(
                [Dim.unnamed(x_shape), Dim.unnamed(y_shape[:-2] + y_shape[-2:][::-1])])
            bias_dim = [out_dims[0].shape[-1]]
            # if len(y_shape) > 2:
            #     bias_dim = [np.prod(y_shape[:-2]), out_dims[0].shape[-1]]
            G.add_edge(
                NNEdge(from_node=y[0], to_node=trans2, from_idx=y[1], to_idx=0))
            G.add_edge(
                NNEdge(from_node=trans2, to_node=params, from_idx=0, to_idx=1))
            biases_params = ConstantInputNode(
                f'{valid_name}_biases',
                dims=Dim.unnamed(bias_dim),
                value=np.zeros(tuple(bias_dim),
                dtype=np.float32)
            )
            G.add_edge(NNEdge(from_node=biases_params,
                              to_node=params, to_idx=2))

            if quantized:
                qrecs[trans2.name] = QRec.scaled(
                    in_qs=[y_qtype],
                    out_qs=[y_qtype],
                )
                qrecs[params.name] = QRec.scaled(
                    in_qs=[x_qtype, y_qtype],
                    out_qs=[o_qtype],
                )

        G.add_edge(
            NNEdge(from_node=x[0], to_node=params, from_idx=x[1], to_idx=0))
        if cls.is_linear(y, x_shape, y_shape) and len(x[2].known_shape) != len(out_dims[0].shape):
            n_ones = [1] * (len(x[2].known_shape) - len(out_dims[0].shape))
            reshape = ReshapeNode(f'{valid_name}_reshape_out',
                                  old_shape=out_dims[0],
                                  shape=Dim.unnamed(n_ones + out_dims[0].shape))
            G.add_edge(
                NNEdge(from_node=params, to_node=reshape, from_idx=x[1], to_idx=0))
            params = reshape
            out_dims = [Dim.unnamed(n_ones + out_dims[0].shape)]

        pout_dims = x[2].infer_mapping(out_dims[0].shape)
        all_nodes[node.output[0]] = (params, 0, pout_dims, o_qtype)
        return params
