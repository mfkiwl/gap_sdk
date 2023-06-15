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

import math
from typing import Sequence, Tuple

import numpy as np
from bfloat16 import bfloat16
from nntool.expressions.symbolic.float_quantization.float_qrec import FloatQRec
from nntool.utils.exp_17_15 import exp_fp_17_15
from nntool.utils.np_erf import erf_lut
from nntool.utils.pow_sqrt import (arctan_17_15alt, logn_17_15, pow_17_15,
                            rsqrt_16_16, sqrt_17_15, square_17_15)
from nntool.utils.sigmoid_tanh_lut import sigmoid_lut, tanh_lut
from nntool.utils.sin_cos import fpcos, fpsin

from ..basic import (Abs, Add, ATan, Cast, Clip, ClipBits, Cos, Div, Erf, Exp,
                     GapAbs, GapMax, GapMin, Log, LShift, Max, Min, Mul, NoOP,
                     Norm, Pow, RSqrt, Sigmoid, Sin, Sqrt, Sub, TanH)
from ..function import Function
from ..quantization_base import qhandler
from ..symbol import (Constant, QuantizedConstant, QuantizedValue, Rational,
                      Symbol, SymbolStats, Variable, c_headers, environment,
                      nargs)
from .q15_scale_float import Q15ScaleFloat
from .q15_scale_q_rec import Q15ScaleQRec
from .q15_scaled_quantization import Q15ScaledQuantization
from .scale_quantized import ScaleQuantized


@qhandler("Q15Scale", Constant, Rational)
class BasicConstantQuant(Q15ScaledQuantization):

    @classmethod
    def _quantize(cls,
                  sym: Symbol, sym_ctrl:
                  SymbolStats,
                  qrec: Q15ScaleQRec = None,
                  **kwargs) -> Tuple[Symbol, Q15ScaleQRec]:
        if isinstance(sym, QuantizedConstant):
            max_val = np.max(sym.value)
            min_val = np.max(sym.value)
            return (sym, Q15ScaleQRec(sym.dtype, 1, 0, min_val=min_val, max_val=max_val))
        if sym.is_zero(qrec):
            return (sym, Q15ScaleQRec(sym.dtype, 1, 15, min_val=0, max_val=0))
        if len(sym.value) == 1:
            max_val = np.max(sym.value)
            min_val = np.max(sym.value)
            return (QuantizedConstant(1), Q15ScaleQRec(np.int32, sym.value[0], 0, min_val=min_val, max_val=max_val))
        # iinfo = np.iinfo(qdtype)
        # if np.all(sym.value == np.floor(sym.value)) and np.all(np.abs(sym.value) < iinfo.max):
        #     return (QuantizedConstant(sym.value.astype(qdtype)), Q15ScaleQRec(qdtype, 1, 0))
        max_val = sym_ctrl.get_max(sym)
        # scale constants to Q15 by default
        qdtype = qrec.dtype if qrec and qrec.dtype else np.int16
        q = cls.get_maxq_from_dtype(qdtype)
        qrec = Q15ScaleQRec.inherit(
            qrec, np.int32, max_val, q, max_val=max_val, min_val=-max_val)
        return (QuantizedConstant(qrec.quantize_and_clip(sym.value)), qrec)


@qhandler("Q15Scale", Variable)
class BasicVariableQuant(Q15ScaledQuantization):

    @classmethod
    def _quantize(cls,
                  sym: Symbol,
                  sym_ctrl: SymbolStats,
                  qrec: Q15ScaleQRec = None,
                  **kwargs) -> Tuple[Symbol, Q15ScaleQRec]:

        quantize_inputs = kwargs.get('quantize_inputs', None)
        prequantized_variables = kwargs.get('prequantized_variables', {})
        qtypes = kwargs.get('qtypes', {})

        # if the symbol is in prequantized_variables it is probably
        # an output from a previous function so already has a quant record
        if sym.name in prequantized_variables:
            sym.qrec = prequantized_variables[sym.name]
            return (sym, prequantized_variables[sym.name])

        # see if an nntool quantizer qtype is available
        if not qrec and qtypes and sym.name in qtypes:
            in_range = sym_ctrl.get_range(sym)
            qtype = qtypes[sym.name]
            if in_range is None:
                if qtype.max_val is not None and qtype.min_val is None:
                    in_range = (qtype.min_val, qtype.max_val)
                else:
                    in_range = (qtype.min, qtype.max)
            osym, qrec = cls.qrec_from_qtype(sym, qtypes[sym.name], in_range)
            if qrec:
                osym.qrec = qrec
                if isinstance(qrec, Q15ScaleQRec) and qrec.zero_point != 0:
                    osym = Sub(Cast(osym, dtype=np.int32), QuantizedConstant(
                        qrec.zero_point), dtype=np.int32, tag=True)
                    qrec = Q15ScaleQRec.override(
                        qrec, dtype=np.int32, zero_point=0)
                return (osym, qrec)

        # figure out the quantization from the maximum value recorded
        max_val = sym_ctrl.get_max(sym)
        assert max_val is not None, f"stats must be present on all variables. {sym.name} is missing."
        qdtype = qrec.dtype if qrec and qrec.dtype else np.int8
        q = cls.get_maxq_from_dtype(qdtype)
        qrec = Q15ScaleQRec.inherit(
            qrec, np.int8, max_val, q, max_val=max_val, min_val=-max_val, zero_point=0)
        # tag the variable with its quantization
        sym.qrec = qrec
        if quantize_inputs:
            return (Q15ScaleFloat(sym, to_qrec=qrec), qrec)
        return (sym, qrec)

    @classmethod
    def qrec_from_qtype(cls, sym, qtype, in_range):
        if qtype.dtype in [np.int8, np.uint8, np.int16, np.uint16]:
            if len(qtype.scale) > 1:
                return sym, None
            qrec = Q15ScaleQRec.from_qtype(qtype)
            # max_val, min_val, bitlen = Q15ScaleQRec.dtype_zp_to_min_max(
            #     qtype.dtype, qtype.scale[0], qtype.zero_point[0])
            # qrec = Q15ScaleQRec(qtype.dtype, max_val, bitlen,
            #                     max_val=max_val, min_val=min_val,
            #                     zero_point=qtype.zero_point[0])
            sym.qrec = qrec
            return sym, qrec
        if qtype.dtype in [np.float16, bfloat16, np.float32]:
            qrec = FloatQRec(dtype=qtype.dtype)
            sym.qrec = qrec
            max_val = np.max(np.maximum(
                np.abs(in_range[0]), np.abs(in_range[1])))
            return (
                Cast(
                    Add(
                        Mul(
                            sym,
                            Constant(
                                np.atleast_1d(math.pow(2, 15) /
                                              max_val).astype(qtype.dtype),
                                dtype=qtype.dtype),
                            dtype=qtype.dtype),
                        Constant([0.5], dtype=qtype.dtype),
                        dtype=qtype.dtype
                    ),
                    dtype=np.int32),
                Q15ScaleQRec(np.int32, max_val, 15,
                             max_val=max_val, min_val=-max_val,
                             zero_point=0))
        raise NotImplementedError(
            "don't know how to convert input type to Q15 quantization")


@qhandler("Q15Scale", QuantizedValue)
class BasicQuantizedQ15QuantizedValue(Q15ScaledQuantization):

    @classmethod
    def _quantize(cls,
                  sym: Symbol,
                  sym_ctrl: SymbolStats,
                  qrec: Q15ScaleQRec = None,
                  **kwargs) -> Tuple[Symbol, Q15ScaleQRec]:
        return (sym.contents[0], sym.qrec)


@qhandler("Q15Scale", NoOP)
class BasicQuantizedQ15QuantizedNoOP(Q15ScaledQuantization):

    @classmethod
    def _quantize(cls,
                  sym: Symbol,
                  sym_ctrl: SymbolStats,
                  qrec: Q15ScaleQRec = None,
                  **kwargs) -> Tuple[Symbol, Q15ScaleQRec]:
        in_syms, in_qrecs = zip(*[cls.quantize(inner_sym, sym_ctrl, **kwargs)
                                  for inner_sym in sym.contents])
        return NoOP(in_syms[0]), in_qrecs[0]


class BasicFunctionQuant(Q15ScaledQuantization):

    @classmethod
    def _quantize(cls,
                  sym: Symbol,
                  sym_ctrl: SymbolStats,
                  qrec: Q15ScaleQRec = None,
                  **kwargs) -> Tuple[Symbol, Q15ScaleQRec]:
        raise NotImplementedError()

    @staticmethod
    def cast_symbols(in_syms, qrecs, dtype=np.int32):
        return zip(*[
            (
                ScaleQuantized(
                    sym,
                    from_qrec=qrec,
                    to_qrec=Q15ScaleQRec.override(qrec, dtype=dtype, zero_point=0)),
                Q15ScaleQRec.override(qrec, dtype=dtype, zero_point=0)
            )
            if qrec.dtype != dtype else (sym, qrec)
            for sym, qrec in zip(in_syms, qrecs)])


def find_range(sym_ctrl: SymbolStats, sym: Symbol, qrecs: Sequence[Q15ScaleQRec]):
    sym_range = sym_ctrl.get_range(sym)
    if sym_range:
        return sym_range
    assert all(
        qrec.min_val is not None and qrec.max_val is not None for qrec in qrecs), 'all values must be set'
    val_range = np.array([
        sym.call_with_constants(qrecs[0].min_val, qrecs[1].min_val),
        sym.call_with_constants(qrecs[0].max_val, qrecs[1].min_val),
        sym.call_with_constants(qrecs[0].min_val, qrecs[1].max_val),
        sym.call_with_constants(qrecs[0].max_val, qrecs[1].max_val),
    ])
    return np.min(val_range), np.max(val_range)


@qhandler("Q15Scale", Add, Sub, Min, Max)
class BasicEqualizeQ15Quant(BasicFunctionQuant):

    @classmethod
    def _quantize(cls,
                  sym: Symbol,
                  sym_ctrl: SymbolStats,
                  qrec: Q15ScaleQRec = None,
                  **kwargs) -> Tuple[Symbol, Q15ScaleQRec]:
        sym_cls = sym.__class__
        if sym_cls == Min:
            sym_cls = GapMin
        elif sym_cls == Max:
            sym_cls = GapMax

        in_syms, in_qrecs = zip(*[cls.quantize(inner_sym, sym_ctrl, **kwargs)
                                  for inner_sym in sym.contents])

        min_val, max_val = find_range(sym_ctrl, sym, in_qrecs)

        in_syms, in_qrecs = cls.cast_symbols(in_syms, in_qrecs)
        # scale to the larger Q but not more than Q15
        calc_qrec = None
        scale_to = None

        if in_syms[0].is_zero(in_qrecs[0]):
            return in_syms[1], in_qrecs[1]
        if in_syms[1].is_zero(in_qrecs[1]):
            return in_syms[0], in_qrecs[0]

        # equalize quantization of incoming arguments
        if in_qrecs[0].scaledq != in_qrecs[1].scaledq:
            # if one argument is constant then scale it. It will be reduced to
            # something that fits
            if in_syms[0].is_constant and not in_syms[1].is_constant:
                scale_to = 1
            elif in_syms[1].is_constant and not in_syms[0].is_constant:
                scale_to = 0
            elif in_qrecs[0].scaledq < in_qrecs[1].scaledq: # reduce precision of 0
                spare_bits = 15 - in_qrecs[1].q
                if math.ceil(math.log2(in_qrecs[1].scaledq/in_qrecs[0].scaledq)) < spare_bits:
                    in_syms = (in_syms[0], LShift(in_syms[1], QuantizedConstant(spare_bits), dtype=np.int32))
                    in_qrecs = (in_qrecs[0], Q15ScaleQRec.override(in_qrecs[1], q=15))
                scale_to = 1
            elif in_qrecs[0].scaledq > in_qrecs[1].scaledq: # reduce precision of 1
                spare_bits = 15 - in_qrecs[0].q
                if math.ceil(math.log2(in_qrecs[0].scaledq/in_qrecs[1].scaledq)) < spare_bits:
                    in_syms = (LShift(in_syms[0], QuantizedConstant(spare_bits), dtype=np.int32), in_syms[1])
                    in_qrecs = (Q15ScaleQRec.override(in_qrecs[0], q=15), in_qrecs[1])
                scale_to = 0

            if scale_to == 0:
                in_syms = [
                    in_syms[0],
                    ScaleQuantized(
                        in_syms[1],
                        from_qrec=in_qrecs[1],
                        to_qrec=in_qrecs[0],
                        tag=True,
                        comment=f"{sym.name} scale arg 1 to 0 - {in_qrecs[1]} -> {in_qrecs[0]}")]
                calc_qrec = Q15ScaleQRec.override(in_qrecs[0], dtype=np.int32)
            elif scale_to == 1:
                in_syms = [
                    ScaleQuantized(
                        in_syms[0],
                        from_qrec=in_qrecs[0],
                        to_qrec=in_qrecs[1],
                        tag=True,
                        comment=f"{sym.name} scale arg 0 to 1 - {in_qrecs[0]} -> {in_qrecs[1]}"),
                    in_syms[1]]
                calc_qrec = Q15ScaleQRec.override(in_qrecs[1], dtype=np.int32)
        else:
            calc_qrec = in_qrecs[0]

        out_sym = sym_cls(*in_syms, dtype=np.int32, qrec=calc_qrec)

        iinfo = np.iinfo(np.int16)
        # check if we need to scale to make sure that result fits in Q15
        if calc_qrec.quantize(min_val) < iinfo.min or calc_qrec.quantize(max_val) > iinfo.max:
            log_scale = np.log2(calc_qrec.scale)
            # check if our scale is a power of 2
            if log_scale == np.abs(log_scale):
                # we just need to norm with one extra bit for overflow
                norm = int(log_scale+1)
                out_qrec = Q15ScaleQRec.override(calc_qrec, q=calc_qrec.q-norm)
                out_sym = Norm(
                    out_sym,
                    QuantizedConstant(norm),
                    tag=True,
                    dtype=np.int32,
                    comment=f'{sym.name} norm pow2 result {norm}')
            else:
                # scale is not a power of 2 so we need to do a proper scaling
                # TODO - Should we look here to see if we can scale close to a power of 2?
                out_qrec = Q15ScaleQRec.from_range(np.int32, 15, min_val, max_val)
                out_sym = ScaleQuantized(
                    out_sym,
                    from_qrec=calc_qrec,
                    to_qrec=out_qrec,
                    tag=True,
                    comment=f'{sym.name} scale result to output - {calc_qrec} -> {out_qrec}')
        else:
            out_qrec = calc_qrec

        return out_sym, out_qrec

@qhandler("Q15Scale", Mul)
class BasicMulQ15Quant(BasicFunctionQuant):

    @classmethod
    def _quantize(cls,
                  sym: Symbol,
                  sym_ctrl: SymbolStats,
                  qrec: Q15ScaleQRec = None,
                  **kwargs) -> Tuple[Symbol, Q15ScaleQRec]:

        in_syms, in_qrecs = zip(*[cls.quantize(inner_sym, sym_ctrl, **kwargs)
                                  for inner_sym in sym.contents])
        sym_cls = sym.__class__
        # eliminate multiply by zero
        if any(in_sym.is_zero(in_qrec) for in_sym, in_qrec in zip(in_syms, in_qrecs)):
            return (QuantizedConstant(0), Q15ScaleQRec(np.int32, 1, 15))
        # TODO - This caused regression issues that I did not have time to track down
        # # eliminate multiply by one
        # sym_is_one = [idx for idx, sym in enumerate(in_syms) if sym.is_one]
        # if sym_is_one:
        #     idx = sym_is_one[0] if len(sym_is_one) == 1 else 1
        #     idx = 0 if idx == 1 else 1
        #     return (in_syms[idx], in_qrecs[idx])
        in_syms, in_qrecs = cls.cast_symbols(in_syms, in_qrecs)
        prod_scale = in_qrecs[0].scale * in_qrecs[1].scale
        prod_q = in_qrecs[0].q + in_qrecs[1].q
        calc_qrec = Q15ScaleQRec(np.int32, prod_scale, prod_q, max_val=prod_scale, min_val=-prod_scale)
        if prod_q > 15:
            out_qrec = Q15ScaleQRec.override(calc_qrec, q=15)
            qsym = Norm(
                sym_cls(*in_syms, dtype=np.int32),
                QuantizedConstant(prod_q - 15),
                dtype=np.int32,
                tag=True,
                comment=f'normalize input to Q15 - {prod_q - 15}'
            )
        else:
            out_qrec = calc_qrec
            qsym = sym_cls(*in_syms)
        return (qsym, out_qrec)


@qhandler("Q15Scale", Div)
class BasicDivQ15Quant(BasicFunctionQuant):

    @classmethod
    def _quantize(cls,
                  sym: Symbol,
                  sym_ctrl: SymbolStats,
                  qrec: Q15ScaleQRec = None,
                  **kwargs) -> Tuple[Symbol, Q15ScaleQRec]:

        in_syms, in_qrecs = zip(*[cls.quantize(inner_sym, sym_ctrl, **kwargs)
                                  for inner_sym in sym.contents])

        if isinstance(in_syms[1], Constant) and np.prod(in_syms[1].shape) == 1:
            value = 1 / in_qrecs[1].dequantize(in_syms[1].value)
            # wrap the incoming value in a QuantizedValue object so that it will
            # return the current quantization
            return BasicMulQ15Quant.quantize(
                Mul(
                    QuantizedValue(
                        in_syms[0], qrec=in_qrecs[0], name=in_syms[0].name),
                    Constant(
                        value, name=in_syms[1].name, shape=in_syms[1].shape),
                    name=sym.name,
                    dtype=np.int32),
                sym_ctrl)

        # LHS and RHS must be Q15 maximum
        # Then move both to the highest Q and shift lhs by that Q
        (lhs, rhs), (lhs_qrec, rhs_qrec) = cls.cast_symbols(in_syms, in_qrecs)
        lhs_adjust = 15 - lhs_qrec.q if lhs_qrec.q > 15 else 0
        rhs_adjust = 15 - rhs_qrec.q if rhs_qrec.q > 15 else 0

        if (lhs_qrec.q + lhs_adjust) < (rhs_qrec.q + rhs_adjust):
            lhs_adjust += (rhs_qrec.q + rhs_adjust) - (lhs_qrec.q + lhs_adjust)
        elif (lhs_qrec.q + lhs_adjust) > (rhs_qrec.q + rhs_adjust):
            rhs_adjust += (lhs_qrec.q + lhs_adjust) - (rhs_qrec.q + rhs_adjust)

        res_q = (lhs_qrec.q + lhs_adjust)
        lhs_adjust += res_q

        if lhs_adjust < 0:
            lhs = Norm(lhs, QuantizedConstant(-lhs_adjust), dtype=np.int32)
        elif lhs_adjust > 0:
            lhs = LShift(lhs, QuantizedConstant(lhs_adjust), dtype=np.int32)
        if rhs_adjust < 0:
            rhs = Norm(rhs, QuantizedConstant(-rhs_adjust), dtype=np.int32)
        elif rhs_adjust > 0:
            rhs = LShift(rhs, QuantizedConstant(rhs_adjust), dtype=np.int32)

        prod_scale = in_qrecs[0].scale / in_qrecs[1].scale
        calc_qrec = Q15ScaleQRec(
            np.int32, prod_scale, res_q, max_val=prod_scale, min_val=-prod_scale)
        # No scaling necessary because the scale will be correct and the q must be 15 or less
        return (Div(lhs, rhs, is_floor=True), calc_qrec)


@nargs(1)
@c_headers('"math_funcs.h"')
@environment({
    'sqrt_17_15': sqrt_17_15,
})
class Sqrt1715(Function):

    def _impl(self, *args, **kwargs):
        return sqrt_17_15(np.array(args[0]))

    def _py_expr(self, *args, **kwargs):
        return "sqrt_17_15(np.array(%s))" % (args[0],)

    def _c_expr(self, *args, **kwargs):
        return "sqrt_17_15(%s)" % (args[0],)


@qhandler("Q15Scale", Sqrt)
class BasicQ17Q15SqrtQuant(BasicFunctionQuant):

    @classmethod
    def _quantize(cls,
                  sym: Symbol,
                  sym_ctrl: SymbolStats,
                  qrec: Q15ScaleQRec = None,
                  **kwargs) -> Tuple[Symbol, Q15ScaleQRec]:

        in_syms, in_qrecs = zip(*[cls.quantize(inner_sym, sym_ctrl, **kwargs)
                                  for inner_sym in sym.contents])
        in_qrec = in_qrecs[0]
        new_scale = np.sqrt(in_qrec.scale)
        in_syms, in_qrecs = cls.cast_symbols(in_syms, in_qrecs)
        in_sym = in_syms[0]
        if in_qrec.q < 15:
            in_sym = LShift(in_sym, QuantizedConstant(
                15 - in_qrec.q), dtype=in_sym.dtype)
        elif in_qrec.q > 15:
            in_sym = Norm(in_sym, QuantizedConstant(
                in_qrec.q - 15), dtype=in_sym.dtype)

        out_qrec = Q15ScaleQRec(np.int32, new_scale, 15)
        return (Cast(Sqrt1715(in_sym, dtype=np.uint32), dtype=np.int32, tag=True), out_qrec)


@nargs(1)
@c_headers('"math_funcs.h"')
@environment({
    'rsqrt_16_16': rsqrt_16_16,
})
class RSqrt1616(Function):

    def _impl(self, *args, **kwargs):
        return rsqrt_16_16(np.array(args[0]))

    def _py_expr(self, *args, **kwargs):
        return "rsqrt_16_16(np.array(%s))" % (args[0],)

    def _c_expr(self, *args, **kwargs):
        return "rsqrt_16_16(%s)" % (args[0],)


@qhandler("Q15Scale", RSqrt)
class BasicQ16Q16RSqrtQuant(BasicFunctionQuant):

    @classmethod
    def _quantize(cls,
                  sym: Symbol,
                  sym_ctrl: SymbolStats,
                  qrec: Q15ScaleQRec = None,
                  **kwargs) -> Tuple[Symbol, Q15ScaleQRec]:

        in_syms, in_qrecs = zip(*[cls.quantize(inner_sym, sym_ctrl, **kwargs)
                                  for inner_sym in sym.contents])
        in_qrec = in_qrecs[0]
        # normalize scale to Q16
        old_scale = in_qrec.scale
        old_scale *= pow(2, 16 - in_qrec.q)
        # calculate new scale
        new_scale = 1/np.sqrt(old_scale)
        # new scale may be very small versus max value - normalise back
        norm = np.log2(sym_ctrl.stats[sym.name]['max']/new_scale)
        if norm > 0:
            norm = int(np.ceil(norm))
            new_scale *= np.power(2, norm)
            # add 1 since output is in Q16 and we want to get back to Q15
            norm = norm + 1
        else:
            norm = 1
        in_syms, in_qrecs = cls.cast_symbols(in_syms, in_qrecs)
        in_sym = in_syms[0]

        out_qrec = Q15ScaleQRec(np.int32, new_scale, 15)
        return (Cast(Norm(RSqrt1616(in_sym, dtype=np.uint32), QuantizedConstant(norm), dtype=np.uint32), dtype=np.int32, tag=True), out_qrec)


@nargs(1)
@environment({
    'logn_17_15': logn_17_15,
})
@c_headers('"math_funcs.h"')
class Log1715(Function):

    def _impl(self, *args, **kwargs):
        return logn_17_15(np.array(args[0]))

    def _py_expr(self, *args, **kwargs):
        return "logn_17_15(np.array(%s))" % (args[0],)

    def _c_expr(self, *args, **kwargs):
        return "logn_17_15(%s)" % (args[0],)


@qhandler("Q15Scale", Log)
class BasicQ17Q15LogQuant(BasicFunctionQuant):

    @classmethod
    def _quantize(cls,
                  sym: Symbol,
                  sym_ctrl: SymbolStats,
                  qrec: Q15ScaleQRec = None,
                  **kwargs) -> Tuple[Symbol, Q15ScaleQRec]:

        in_syms, in_qrecs = zip(*[cls.quantize(inner_sym, sym_ctrl, **kwargs)
                                  for inner_sym in sym.contents])
        in_qrec = in_qrecs[0]
        log_off = np.log(in_qrec.scale)
        in_syms, in_qrecs = cls.cast_symbols(in_syms, in_qrecs)
        in_sym = in_syms[0]
        if in_qrec.q < 15:
            in_sym = LShift(in_sym, QuantizedConstant(
                15 - in_qrec.q), dtype=np.int32)
        elif in_qrec.q > 15:
            in_sym = Norm(in_sym, QuantizedConstant(
                in_qrec.q - 15), dtype=np.int32)

        # log(Qx * scale) = log(Qx) + log(scale)
        max_val = sym_ctrl.get_max(sym)
        out_qrec = Q15ScaleQRec(np.int32, max_val, 15)
        calc_qrec = Q15ScaleQRec(np.int32, 1, 15)
        qlog_off = calc_qrec.quantize_and_clip(log_off)
        # logn_17_15(1) is -340695 which uses 19 bits. The offset is added before scaling so calculate
        # the number of bits at minimum left after the addition and fit the scaling in that
        max_bits = math.ceil(math.log2(math.fabs(-340695 + qlog_off))) + 2
        return (
            ScaleQuantized(Add(Log1715(in_sym, dtype=np.int32), QuantizedConstant(
                qlog_off), dtype=np.int32), from_qrec=calc_qrec, to_qrec=out_qrec, num_bits=31-max_bits, tag=True),
            out_qrec)


@nargs(2)
@environment({
    'pow_17_15': pow_17_15,
})
@c_headers('"math_funcs.h"')
class Pow1715(Function):

    def _impl(self, *args, **kwargs):
        return pow_17_15(np.array(args[0]), np.array(args[1]))

    def _py_expr(self, *args, **kwargs):
        return "pow_17_15(np.array(%s), np.array(%s))" % (args[0], args[1])

    def _c_expr(self, *args, **kwargs):
        return "pow_17_15(%s, %s)" % (args[0], args[1])


@nargs(1)
@environment({
    'square_17_15': square_17_15,
})
@c_headers('"math_funcs.h"')
class Square1715(Function):

    def _impl(self, *args, **kwargs):
        return square_17_15(np.array(args[0]))

    def _py_expr(self, *args, **kwargs):
        return "square_17_15(np.array(%s))" % (args[0],)

    def _c_expr(self, *args, **kwargs):
        return "square_17_15(%s)" % (args[0],)


@qhandler("Q15Scale", Pow)
class BasicQ17Q15PowQuant(BasicFunctionQuant):

    @classmethod
    def _quantize(cls,
                  sym: Symbol,
                  sym_ctrl: SymbolStats,
                  qrec: Q15ScaleQRec = None,
                  **kwargs) -> Tuple[Symbol, Q15ScaleQRec]:

        in_syms, in_qrecs = zip(*[cls.quantize(inner_sym, sym_ctrl, **kwargs)
                                  for inner_sym in sym.contents])
        rhs = in_syms[1].resolve()
        if not isinstance(rhs, Constant):
            raise NotImplementedError(
                "power is currently only supported with fractional constants, 2, 1, & 0")

        val = np.round(in_qrecs[1].dequantize(rhs.value), decimals=4)
        lhs, lhs_qrec = cls.cast_symbols(in_syms[:1:], in_qrecs[:1:])
        lhs, lhs_qrec = lhs[0], lhs_qrec[0]
        if lhs_qrec.q < 15:
            lhs = LShift(lhs, QuantizedConstant(
                15 - lhs_qrec.q, dtype=np.int8), dtype=np.int32)
        elif lhs_qrec.q > 15:
            lhs = Norm(lhs, QuantizedConstant(
                lhs_qrec.q - 15, dtype=np.int8), dtype=np.int32)

        if val == 2:
            out_qrec = Q15ScaleQRec(np.int32, np.power(lhs_qrec.scale, 2), 15)
            return (Cast(Square1715(lhs, dtype=np.int32), dtype=np.int32, tag=True), out_qrec)
        if val == -2:
            out_qrec = Q15ScaleQRec(
                np.int32, 1/np.power(lhs_qrec.scale, 2), 15)
            return (Div(QuantizedConstant(1 << 30), Cast(Square1715(lhs, dtype=np.int32), dtype=np.int32), dtype=np.int32), out_qrec)
        if val == 1:
            out_qrec = Q15ScaleQRec(np.int32, lhs_qrec.scale, 15)
            return (lhs, out_qrec)
        if val == 0:
            out_qrec = Q15ScaleQRec(np.int32, 1, 0)
            return (QuantizedConstant(1), out_qrec)
        if val > 0 and val < 1:
            out_scale = np.power(lhs_qrec.scale, val).item()
            out_qrec = Q15ScaleQRec(
                np.uint32, out_scale, 15)
            qval = int(math.floor(val * math.pow(2, 15) + 0.5))
            return (
                Cast(
                    Pow1715(
                        lhs,
                        QuantizedConstant(qval),
                        dtype=np.int32),
                    dtype=np.int32,
                    comment=f'{sym.name} POW on scale {lhs_qrec.scale:.3f} -> {out_scale:.3f}',
                    tag=True),
                out_qrec)
        raise NotImplementedError(
            "power is currently only supported with fractional constants, 2, 1, & 0")


@nargs(1)
@environment({
    'exp_fp_17_15': exp_fp_17_15,
})
@c_headers('"math_funcs.h"')
class Exp1715(Function):

    def _impl(self, *args, **kwargs):
        return exp_fp_17_15(np.array(args[0]))

    def _py_expr(self, *args, **kwargs):
        return "exp_fp_17_15(np.array(%s))" % (args[0],)

    def _c_expr(self, *args, **kwargs):
        return "exp_17_15(%s)" % (args[0],)


@qhandler("Q15Scale", Exp)
class BasicQ17Q15ExpQuant(BasicFunctionQuant):

    @classmethod
    def _quantize(cls,
                  sym: Symbol,
                  sym_ctrl: SymbolStats,
                  qrec: Q15ScaleQRec = None,
                  **kwargs) -> Tuple[Symbol, Q15ScaleQRec]:

        in_syms, in_qrecs = zip(*[cls.quantize(inner_sym, sym_ctrl, **kwargs)
                                  for inner_sym in sym.contents])

        # e^xy = e^x^y so have to be in scale 1
        calc_qrec = Q15ScaleQRec(np.int32, 1, 15)
        in_syms, in_qrecs = cls.cast_symbols(in_syms, in_qrecs)
        arg = ScaleQuantized(
            in_syms[0], from_qrec=in_qrecs[0], to_qrec=calc_qrec)
        max_val = sym_ctrl.get_max(sym)
        out_qrec = Q15ScaleQRec(np.int32, max_val, 15)

        return (ScaleQuantized(Cast(Exp1715(arg, dtype=np.uint32), dtype=np.int32),
                               from_qrec=calc_qrec,
                               to_qrec=out_qrec, tag=True),
                out_qrec)


@nargs(1)
@environment({
    'arctan_17_15alt': arctan_17_15alt,
})
@c_headers('"math_funcs.h"')
class Arctan1715(Function):

    def _impl(self, *args, **kwargs):
        return arctan_17_15alt(np.array(args[0]))

    def _py_expr(self, *args, **kwargs):
        return "arctan_17_15alt(np.array(%s))" % (args[0],)

    def _c_expr(self, *args, **kwargs):
        return "arctan_17_15(%s)" % (args[0],)


@qhandler("Q15Scale", ATan)
class BasicQ17Q15ArctanQuant(BasicFunctionQuant):

    @classmethod
    def _quantize(cls,
                  sym: Symbol,
                  sym_ctrl: SymbolStats,
                  qrec: Q15ScaleQRec = None,
                  **kwargs) -> Tuple[Symbol, Q15ScaleQRec]:

        in_syms, in_qrecs = zip(*[cls.quantize(inner_sym, sym_ctrl, **kwargs)
                                  for inner_sym in sym.contents])

        calc_qrec = Q15ScaleQRec(np.int32, 1, 15)
        in_syms, in_qrecs = cls.cast_symbols(in_syms, in_qrecs)
        lhs = ScaleQuantized(
            in_syms[0], from_qrec=in_qrecs[0], to_qrec=calc_qrec)
        return (Arctan1715(lhs, tag=True), calc_qrec)


@nargs(1)
@environment({
    'fpcos': fpcos,
})
@c_headers('"math_funcs.h"')
class Cos_Q15(Function):

    def _impl(self, *args, **kwargs):
        return fpcos(np.array(args[0]))

    def _py_expr(self, *args, **kwargs):
        return "fpcos(np.array(%s))" % (args[0],)

    def _c_expr(self, *args, **kwargs):
        return "fpcos(%s)" % (args[0],)


@nargs(1)
@environment({
    'fpsin': fpsin,
})
@c_headers('"math_funcs.h"')
class Sin_Q15(Function):

    def _impl(self, *args, **kwargs):
        return fpsin(np.array(args[0]))

    def _py_expr(self, *args, **kwargs):
        return "fpsin(np.array(%s))" % (args[0],)

    def _c_expr(self, *args, **kwargs):
        return "fpsin(%s)" % (args[0],)


@qhandler("Q15Scale", Cos, Sin)
class BasicQ15_Q4_12SinCosQuant(BasicFunctionQuant):

    @classmethod
    def _quantize(cls,
                  sym: Symbol,
                  sym_ctrl: SymbolStats,
                  qrec: Q15ScaleQRec = None,
                  **kwargs) -> Tuple[Symbol, Q15ScaleQRec]:

        in_syms, in_qrecs = zip(*[cls.quantize(inner_sym, sym_ctrl, **kwargs)
                                  for inner_sym in sym.contents])
        # scale to Q15 * 2 * pi and clip to -/+ short int
        calc_qrec = Q15ScaleQRec(np.int32, 2 * math.pi, 15)
        # output is Q12 * 1
        out_qrec = Q15ScaleQRec(np.int16, 1, 12)
        in_syms, in_qrecs = cls.cast_symbols(in_syms, in_qrecs)
        lhs = Cast(
            Clip(
                ScaleQuantized(
                    in_syms[0],
                    from_qrec=in_qrecs[0],
                    to_qrec=calc_qrec),
                dtype=calc_qrec.dtype,
                clip_dtype=np.int16),
            dtype=np.int16,
            tag=True,
            comment=f'{sym.name} scale and clip input - {in_qrecs[0]} -> {calc_qrec}')
        if isinstance(sym, Cos):
            qsym = Cos_Q15
        else:
            qsym = Sin_Q15
        return (qsym(lhs), out_qrec)


@nargs(1)
@environment({
    'tanh_lut': tanh_lut,
})
@c_headers('"CNN_BasicKernels_SQ8.h"')
class TanHLUT(Function):

    def _impl(self, *args, **kwargs):
        return tanh_lut(np.array(args[0]))

    def _py_expr(self, *args, **kwargs):
        return "tanh_lut(np.array(%s))" % (args[0],)

    def _c_expr(self, *args, **kwargs):
        return "Tanh(%s)" % (args[0],)


@nargs(1)
@environment({
    'sigmoid_lut': sigmoid_lut,
})
@c_headers('"CNN_BasicKernels_SQ8.h"')
class SigmoidLUT(Function):

    def _impl(self, *args, **kwargs):
        return sigmoid_lut(np.array(args[0]))

    def _py_expr(self, *args, **kwargs):
        return "sigmoid_lut(np.array(%s))" % (args[0],)

    def _c_expr(self, *args, **kwargs):
        return "Sigmoid(%s)" % (args[0],)


@qhandler("Q15Scale", TanH, Sigmoid)
class BasicQ12Q15TanHSigmoidQuant(BasicFunctionQuant):

    @classmethod
    def _quantize(cls,
                  sym: Symbol,
                  sym_ctrl: SymbolStats,
                  qrec: Q15ScaleQRec = None,
                  **kwargs) -> Tuple[Symbol, Q15ScaleQRec]:

        in_syms, in_qrecs = zip(*[cls.quantize(inner_sym, sym_ctrl, **kwargs)
                                  for inner_sym in sym.contents])
        # scale to Q12
        func = 'tanh' if isinstance(sym, TanH) else 'sigmoid'
        calc_qrec = Q15ScaleQRec(np.int32, 1, 12)
        # output is Q15 * 1
        out_qrec = Q15ScaleQRec(
            np.int32, 1, 15, min_val=-1.0 if func == 'tanh' else 0.0, max_val=1.0)
        in_syms, in_qrecs = cls.cast_symbols(in_syms, in_qrecs)
        lhs = ScaleQuantized(
            in_syms[0], from_qrec=in_qrecs[0], to_qrec=calc_qrec)
        if in_qrecs[0].max_abs > pow(2, 4):
            lhs = ClipBits(lhs, QuantizedConstant(
                16, dtype=np.int8), clip_signed=True, dtype=lhs.dtype)
        if isinstance(sym, TanH):
            qsym = TanHLUT
        else:
            qsym = SigmoidLUT
        return (qsym(lhs), out_qrec)


@qhandler("Q15Scale", Abs)
class BasicQ17Q15NoChangeQuant(BasicFunctionQuant):

    @classmethod
    def _quantize(cls,
                  sym: Symbol,
                  sym_ctrl: SymbolStats,
                  qrec: Q15ScaleQRec = None,
                  **kwargs) -> Tuple[Symbol, Q15ScaleQRec]:

        in_syms, in_qrecs = zip(*[cls.quantize(inner_sym, sym_ctrl, **kwargs)
                                  for inner_sym in sym.contents])
        sym_cls = sym.__class__
        if isinstance(sym, Abs):
            sym_cls = GapAbs

        return (sym_cls(*in_syms), in_qrecs[0])

@nargs(1)
@environment({
    'erf_lut': erf_lut,
})
@c_headers('"CNN_BasicKernels_SQ8.h"')
class ErfLUT(Function):

    def _impl(self, *args, **kwargs):
        return erf_lut(np.array(args[0]))

    def _py_expr(self, *args, **kwargs):
        return "erf_lut(np.array(%s))" % (args[0],)

    def _c_expr(self, *args, **kwargs):
        return "Erf(%s)" % (args[0],)


@qhandler("Q15Scale", Erf)
class BasicQ12Q15ErfQuant(BasicFunctionQuant):

    @classmethod
    def _quantize(cls,
                  sym: Symbol,
                  sym_ctrl: SymbolStats,
                  qrec: Q15ScaleQRec = None,
                  **kwargs) -> Tuple[Symbol, Q15ScaleQRec]:

        in_syms, in_qrecs = zip(*[cls.quantize(inner_sym, sym_ctrl, **kwargs)
                                  for inner_sym in sym.contents])
        # scale to Q13
        calc_qrec = Q15ScaleQRec(np.int32, 1, 13)

        # output is Q15 * 1
        out_qrec = Q15ScaleQRec(
            np.int32, 1, 15, min_val=-1.0, max_val=1.0)
        in_syms, in_qrecs = cls.cast_symbols(in_syms, in_qrecs)
        lhs = ScaleQuantized(
            in_syms[0], from_qrec=in_qrecs[0], to_qrec=calc_qrec)
        if in_qrecs[0].max_abs > pow(2, 3):
            lhs = ClipBits(lhs, QuantizedConstant(
                8, dtype=np.int8), clip_signed=True, dtype=lhs.dtype)
        return (ErfLUT(lhs), out_qrec)

