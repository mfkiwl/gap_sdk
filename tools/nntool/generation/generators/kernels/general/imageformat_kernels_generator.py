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

from generation.code_block import CodeBlock
from generation.generator_decorators import generation_function
from graph.types import ImageFormatParameters

from ..autotiler_kernel import AutotilerKernel

LOG = logging.getLogger("nntool." + __name__)

KOP_NORM = {"RGB565_RGB888": "KOP_NORM_RGB565",
              "RGB888": "KOP_NORM_RGB888",
              "RGB16": "KOP_NORM_RGB16",
              "BW8": "KOP_NORM_BW",
              "BW16": "KOP_NORM_BW16"}


def gen_at_imageformat(code_block, name, in_dim, do_offset, kop_norm):
    if in_dim.has_key('w') and in_dim.has_key('h'):
        code_block.write('CNN_Norm("{}", {}, {}, {}, {});',
                         name, in_dim.w, in_dim.h, do_offset and "1" or "0", kop_norm)
    else:
        if len(in_dim.shape) > 2:
            LOG.warning(f"Input Dim has no hints -> we are assuming HxWxC order in this case -> {in_dim.shape[0]}x{in_dim.shape[1]}x{in_dim.shape[2]}, for .onnx graphs may not be the case")
        code_block.write('CNN_Norm("{}", {}, {}, {}, {});',
                         name, in_dim.shape[1], in_dim.shape[0], do_offset and "1" or "0", kop_norm)


@generation_function("kernels", (ImageFormatParameters, ))
def imageformat_kernels_generator(gen, node, qrec, in_eparams, out_eparams, cname):
    del in_eparams, out_eparams, qrec
    gen.kernels.append(ImageFormatKernel(cname, node))
    return True


class ImageFormatKernel(AutotilerKernel):
    def __init__(self, cname, params):
        self.in_dim = params.in_dims[0]
        self.cname = cname
        self.node_name = params.name
        assert params.format_change in ("RGB565_RGB888", "RGB888",
                                        "RGB16", "BW8", "BW16"), "unknown format change"
        assert params.norm_func in ("OFFSET_INT8", "SHIFT_INT8",
                                    "OUT_INT16"), "unknown normalization"
        self.in_format = params.format_change
        self.do_offset = params.norm_func == "OFFSET_INT8"

    def code(self, code_block=None):
        if code_block is None:
            code_block = CodeBlock()

        code_block.comment("generator for {}", self.node_name)

        gen_at_imageformat(code_block, self.cname, self.in_dim,
                           self.do_offset, KOP_NORM[self.in_format])
        return code_block
