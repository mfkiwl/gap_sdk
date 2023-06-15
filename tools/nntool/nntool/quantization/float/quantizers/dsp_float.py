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
from bfloat16 import bfloat16
from nntool.graph.types import RFFT2DPreprocessingNode, MFCCPreprocessingNode
from nntool.quantization.float.float_quantization_handler import \
    FloatQuantizionHandler
from nntool.quantization.new_qrec import QRec
from nntool.quantization.qtype import QType
from nntool.quantization.qtype_constraint import MatchAll
from nntool.quantization.unified_quantization_handler import (in_qs_constraint,
                                                       out_qs_constraint,
                                                       params_type, needs_stats)


@params_type(RFFT2DPreprocessingNode)
@in_qs_constraint(MatchAll({'dtype': set([np.float32, np.float16, bfloat16])}))
@out_qs_constraint(MatchAll({'dtype': set([np.float32, np.float16, bfloat16])}))
@needs_stats(False)
class RFFT2DFloat(FloatQuantizionHandler):
    @classmethod
    def _quantize(cls, params, in_qs, stats, **kwargs):
        force_out_qs, dtype = cls.get_float_opts(**kwargs)
        if force_out_qs and all(qtype.dtype != dtype for qtype in force_out_qs if qtype is not None):
            return None
        return QRec.float(in_qs=[QType(dtype=dtype), QType(dtype=dtype), QType(dtype=dtype), QType(dtype=np.int16), QType(dtype=dtype)],
                          out_qs=[QType(dtype=dtype)],
                          float_dtype=dtype)

@params_type(MFCCPreprocessingNode)
@in_qs_constraint(MatchAll({'dtype': set([np.float32, np.float16, bfloat16])}))
@out_qs_constraint(MatchAll({'dtype': set([np.float32, np.float16, bfloat16])}))
@needs_stats(False)
class MFCCFloat(FloatQuantizionHandler):
    @classmethod
    def _quantize(cls, params, in_qs, stats, **kwargs):
        force_out_qs, dtype = cls.get_float_opts(**kwargs)
        if force_out_qs and all(qtype.dtype != dtype for qtype in force_out_qs if qtype is not None):
            return None
        return QRec.float(in_qs=[QType(dtype=dtype), QType(dtype=dtype), QType(dtype=dtype), QType(dtype=np.int16), QType(dtype=dtype), QType(dtype=np.int16), QType(dtype=dtype), QType(dtype=dtype)],
                          out_qs=[QType(dtype=dtype)],
                          float_dtype=dtype)
