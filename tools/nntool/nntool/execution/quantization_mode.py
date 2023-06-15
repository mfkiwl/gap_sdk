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

from typing import Union, Optional, Tuple
from nntool.graph.types import NNNodeBase


class QuantizationMode():
    def __init__(self, qlevel: str = "all", qstep: Optional[Union[int, Union[str, Tuple]]] = None, dequantize=False):
        self._qlevel = qlevel
        self._qstep = qstep
        self._dequantize = dequantize

    @classmethod
    def all(cls):
        return cls()

    @classmethod
    def step_all(cls):
        return cls(qlevel="step_all")

    @classmethod
    def all_float_quantize_dequantize(cls):
        return cls(qlevel="float_q_deq")

    @classmethod
    def all_dequantize(cls):
        return cls(dequantize=True)

    @classmethod
    def none(cls):
        return cls(qlevel="none")

    @classmethod
    def step(cls, qstep: Union[NNNodeBase, int]):
        return cls(qlevel="step", qstep=qstep)

    def get_quantized(self, node: NNNodeBase, step_idx: int):
        if self._qlevel in ("none", "float_q_deq"):
            return False
        if self._qlevel == "all":
            return True
        if isinstance(self._qstep, NNNodeBase):
            return node == self._qstep
        return step_idx == self._qstep

    @property
    def qlevel(self):
        return self._qlevel

    @property
    def qstep(self):
        return self._qstep

    @property
    def is_float_q_deq(self):
        return self._qlevel == "float_q_deq"

    @property
    def is_step(self):
        return self._qlevel == "step"

    @property
    def is_step_all(self):
        return self._qlevel == "step_all"

    @property
    def is_all(self):
        return self._qlevel == "all"

    @property
    def is_none(self):
        return self._qlevel == "none"

    @property
    def dequantize(self):
        return (self.is_step or self.is_all) and self._dequantize

    def __eq__(self, o: object) -> bool:
        if isinstance(o, QuantizationMode):
            return (self.qlevel == o.qlevel and self.qstep == o.qstep and
                    self.dequantize == o.dequantize)
        return False

    def __str__(self):
        if self.is_none or self.is_all:
            return self._qlevel
        if isinstance(self._qstep, NNNodeBase):
            return "node {}".format(self._qstep.name)
        return "step # {}".format(self._qstep)
