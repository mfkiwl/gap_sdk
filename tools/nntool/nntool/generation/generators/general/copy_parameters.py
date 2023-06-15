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

from nntool.generation.at_types.gen_ctrl import GenCtrl
from nntool.generation.generators.general.autotiler_kernel import NewAutoTilerKernel
from nntool.generation.generators.generator_base import GeneratorBase, paramstype
from nntool.generation.generators.helpers.in_out_bindings_mixin import \
    InOutBindingsMixin
from nntool.graph.types import StridedSliceNode, RepeatNode, CopyNode

LOG = logging.getLogger(__name__)

@paramstype(CopyNode, StridedSliceNode, RepeatNode)
class GenCopyNode(GeneratorBase, InOutBindingsMixin):
    @classmethod
    def globals_generator(cls, gen, node, qrec, pnode, fnode) -> bool:
        return True

    @classmethod
    def bindings_generator(cls, gen, node, qrec, in_eparams, out_eparams, cname) -> bool:
        cls.set_in_out_bindings(
            gen, in_eparams, out_eparams, cname, node, qrec)
        return True

    @classmethod
    def kernel_generator(cls, gen, node, qrec, in_eparams, out_eparams, cname) -> bool:
        del in_eparams, out_eparams
        if isinstance(node, RepeatNode):
            gen.kernels.append(RepeatKernel(cname, node, qrec))
            return True

        if isinstance(node, StridedSliceNode):
            if not all([act_slice[2] == 1 for act_slice in node.act_slice]):
                raise ValueError(
                    f"Cannot generate strided slice layer {node.name} with a copy if stride != 1")
            if not all([act_slice[0] == 0 for act_slice in node.act_slice]):
                raise ValueError(
                    f"Cannot generate strided slice layer {node.name} with a copy if start != 0")
        gen.kernels.append(CopyKernel(cname, node, qrec))
        return True

class CopyKernelBase(NewAutoTilerKernel):

    def __init__(self, cname, params, qrec, gen_ctrl=None):
        if gen_ctrl is None:
            gen_ctrl = GenCtrl(None, cname=cname)
        else:
            gen_ctrl.cname = cname

        if qrec.out_qs[0].is_floating:
            gen_ctrl.float_dump = 1

        attrs = {
            'size': params.out_dims[0].size(),
            'feature_size': (qrec.out_qs[0].dtype_bits//8)
        }
        if isinstance(params, RepeatNode):
            attrs['n_repeat'] = params.n_repeats

        # other attributes
        extra_attrs = {
            'cname': cname,
            'node_name': params.name
        }

        super().__init__(attrs, extra_attrs, gen_ctrl=gen_ctrl)


class RepeatKernel(CopyKernelBase):
    CALL_TEMPLATE = '''
// generator for {node_name}
CNN_Repeat("{cname}", {gen_ctrl}, {size}, {feature_size}, {n_repeat});
'''

class CopyKernel(CopyKernelBase):
    CALL_TEMPLATE = '''
// generator for {node_name}
CNN_Copy("{cname}", {gen_ctrl}, {size}, {feature_size});
'''
