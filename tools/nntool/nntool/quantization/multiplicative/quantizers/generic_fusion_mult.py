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
from nntool.graph.types import (ActivationFusionNodeBase, MatMulOpFusionNode,
                         MatScaleFusionNode, PaddedAddFusionNode)
from nntool.quantization.new_qrec import QRec
from nntool.quantization.qtype_constraint import MatchAll
from nntool.quantization.unified_quantization_handler import (fusion_handler,
                                                       in_qs_constraint,
                                                       out_qs_constraint,
                                                       params_type)

from ..mult_quantization_handler import MultQuantizionHandler

class GenericFusionMultBase(MultQuantizionHandler):
    @classmethod
    def _quantize(cls, params, in_qs, stats, **kwargs):
        if 'out_qs' in kwargs and kwargs['out_qs'] is not None:
            return QRec.scaled(in_qs=in_qs, out_qs=kwargs['out_qs'])
        fusion_G = params.subgraph
        fusion_outputs = sorted(fusion_G.outputs(), key=lambda x: x.idx)
        G = kwargs['G']
        qset = kwargs['qset']
        out_qs = []
        for foutp in fusion_outputs:
            in_edge = fusion_G.in_edges(foutp)[0]
            out_qs.append(qset[(params.name, in_edge.from_node.name)].out_qs[in_edge.from_idx])
        return QRec.scaled(in_qs=in_qs, out_qs=out_qs)


@params_type(ActivationFusionNodeBase, MatScaleFusionNode, PaddedAddFusionNode)
@in_qs_constraint(MatchAll({'dtype': np.int8}))
#@out_qs_constraint(MatchAll({'dtype': np.int8}))
@fusion_handler
class GenericFusionMult(GenericFusionMultBase):
    pass

@params_type(ActivationFusionNodeBase, MatScaleFusionNode, PaddedAddFusionNode)
@in_qs_constraint(MatchAll({'dtype': np.uint8}))
#@out_qs_constraint(MatchAll({'dtype': np.uint8}))
@fusion_handler
class GenericFusionMultU8(GenericFusionMultBase):
    pass

@params_type(ActivationFusionNodeBase, MatScaleFusionNode, PaddedAddFusionNode)
@in_qs_constraint(MatchAll({'dtype': np.uint16}))
#@out_qs_constraint(MatchAll({'dtype': np.uint16}))
@fusion_handler
class GenericFusionMultU16(GenericFusionMultBase):
    pass

@params_type(MatMulOpFusionNode)
@in_qs_constraint(({'dtype': {np.int8, np.int16, np.uint8, np.uint16}}))
#@out_qs_constraint(MatchAll({'dtype': np.int8}))
@fusion_handler
class GenericFusionMatMult(GenericFusionMultBase):
    pass
