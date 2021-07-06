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

from quantization.qtype import QType
import numpy as np

from graph.types.activations import HSigmoidActivationParameters, SigmoidActivationParameters

from ..backend_handler import BackendHandler
from ..handler import tflite_op
from .math_mixin import BasicMathMixin


@tflite_op("LOGISTIC")
class Logistic(BasicMathMixin, BackendHandler):

    @classmethod
    def _common(cls, node, **kwargs):
        if kwargs['opts'].get('load_quantization') and kwargs['opts'].get('use_lut_sigmoid'):
            kwargs['in_qs'] = [QType.from_min_max_sq(-8, 8, dtype=np.int8)]
        params_class = SigmoidActivationParameters if kwargs['opts'].get('use_lut_sigmoid') else HSigmoidActivationParameters
        return super(Logistic, cls)._common(
            node,
            params_class=params_class,
            **kwargs)

    @classmethod
    def version_1(cls, node, **kwargs):
        return cls._common(node, **kwargs)
