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
from abc import ABC, abstractmethod

import numpy as np
from graph.dim import Dim

LOG = logging.getLogger("nntool." + __name__)
LOGL = LOG.info


class Action(ABC):
    def __init__(self, node) -> None:
        self.node = node

    @abstractmethod
    def _execute(self, node):
        pass

    def execute(self):
        self._execute(self.node)


class DebugActionBase(Action):
    def __init__(self, node, message=None) -> None:
        super(DebugActionBase, self).__init__(node)
        if message is None:
            message = ""
        self.message = message

    def _execute(self, node):
        LOGL("%s", str(self))


class StartAction(DebugActionBase):
    def __str__(self) -> str:
        return "Start Action (%s): %s" % (self.message, self.node.name)


class StartActionUp(StartAction):
    def __init__(self, node) -> None:
        super(StartActionUp, self).__init__(node, "up")


class StartActionDown(StartAction):
    def __init__(self, node) -> None:
        super(StartActionDown, self).__init__(node, "down")


class EndAction(DebugActionBase):
    def __str__(self) -> str:
        return "End Action (%s): %s" % (self.message, self.node.name)


class EndActionUp(EndAction):
    def __init__(self, node) -> None:
        super(EndActionUp, self).__init__(node, "up")


class EndActionDown(EndAction):
    def __init__(self, node) -> None:
        super(EndActionDown, self).__init__(node, "down")


class SetHintAction(Action):
    def __init__(self, node, direction, idx, val=None, transpose=None, duplicate=False) -> None:
        super(SetHintAction, self).__init__(node)
        self.direction = direction
        self.idx = idx
        if val is None:
            hints_name = self.direction + '_dims_hint'
            hints = getattr(self.node, hints_name)
            val = hints and hints[idx]
        if val is not None:
            if transpose:
                val = [val[idx] for idx in transpose]
            else:
                val = val.copy()
        self.val = val
        self.duplicate = duplicate

    @staticmethod
    def _set_hint(node, direction, idx, val):
        hints_name = direction + '_dims_hint'
        hints = getattr(node, hints_name)
        if hints is None:
            dims_name = direction + '_dims'
            dims = getattr(node, dims_name)
            hints = [None] * len(dims)
            setattr(node, hints_name, hints)
        hints[idx] = val

    def _execute(self, node):
        if self.val is None:
            return
        LOGL("%s", str(self))
        self._set_hint(node, self.direction, self.idx, self.val)
        if self.duplicate:
            self._set_hint(node, "in" if self.direction == "out" else "in", self.idx, self.val)

    def __str__(self) -> str:
        return "%s set hint %s[%s] to %s%s" % (self.node.name, self.direction, self.idx,
                                               self.val, " and coresponding" if self.duplicate else "")


class SetReshapeAction(Action):
    def __init__(self, node, in_shape=None, out_shape=None) -> None:
        super(SetReshapeAction, self).__init__(node)
        self.in_shape = in_shape.clone() if in_shape is not None else None
        self.out_shape = out_shape.clone() if out_shape is not None else None

    def _execute(self, node):
        LOGL("%s", str(self))
        if self.in_shape is not None:
            node.old_shape = self.in_shape
        if self.out_shape is not None:
            node.shape = self.out_shape

    def __str__(self) -> str:
        return "%s set reshape in %s out %s" % (self.node.name, self.in_shape, self.out_shape)


class TransposeSlidedSlice(Action):
    def __init__(self, node, transpose) -> None:
        super(TransposeSlidedSlice, self).__init__(node)
        self.transpose = tuple(transpose)

    def _execute(self, node):
        LOGL("%s", str(self))
        node.act_slice = [node.act_slice[idx] for idx in self.transpose]
        node.out_shape = [node.out_shape[idx] for idx in self.transpose]

    def __str__(self) -> str:
        return "%s transpose slided slice parameters with %s" % (self.node.name, self.transpose)


class TransposePad(Action):
    def __init__(self, node, transpose) -> None:
        super(TransposePad, self).__init__(node)
        self.transpose = tuple(transpose)

    def _execute(self, node):
        LOGL("%s", str(self))
        node.padding = [node.padding[idx] for idx in self.transpose]
        node.pad_vals = [node.pad_vals[idx] for idx in self.transpose]

    def __str__(self) -> str:
        return "%s transpose pad parameters with %s" % (self.node.name, self.transpose)


class TransposeReverse(Action):
    def __init__(self, node, transpose) -> None:
        super(TransposeReverse, self).__init__(node)
        self.transpose = tuple(transpose)

    def _execute(self, node):
        LOGL("%s", str(self))
        node.axis = self.transpose[node.axis]

    def __str__(self) -> str:
        return "%s transpose reverse parameters with %s" % (self.node.name, self.transpose)


class TransposeInputBase(Action):
    def __init__(self, node, transpose) -> None:
        super(TransposeInputBase, self).__init__(node)
        self.transpose = tuple(transpose)

    @abstractmethod
    def _execute(self, node):
        pass

    @classmethod
    def from_history(cls, node, history, transpose=None):
        if transpose is not None:
            return cls(node, transpose)
        # Find the first entry in the transpose history that actually has a transpose
        first_valid_entry = next(iter([entry
                                       for rec in history
                                       for entry in rec[1::] if entry[0]]))
        # The shapes in the transpose history are cloned to the reorder above
        # will not affect them.
        return cls(node, first_valid_entry[0])


class ReorderInputDims(TransposeInputBase):
    def _execute(self, node):
        LOGL("%s", str(self))
        node.dims.transpose(self.transpose)
        if node.out_dims_hint:
            node.out_dims_hint[0] = [node.out_dims_hint[0][idx] for idx in self.transpose]
            node.in_dims_hint = node.out_dims_hint

    def __str__(self) -> str:
        return "%s input dims with %s" % (self.node.name, self.transpose)


class ReorderConstantInput(TransposeInputBase):
    def _execute(self, node):
        LOGL("%s", str(self))
        node.value = np.transpose(node.value, self.transpose)
        node.dims = Dim.unnamed(node.value.shape)

    def __str__(self) -> str:
        return "%s reorder constant input to %s" % (self.node.name, self.transpose)


class DeleteTranspose(Action):
    def __init__(self, node, direction, idx) -> None:
        super(DeleteTranspose, self).__init__(node)
        self.direction = direction
        self.idx = idx

    def _execute(self, node):
        LOGL("%s", str(self))
        transpose_name = "transpose_" + self.direction
        trans = getattr(node, transpose_name)
        trans[self.idx] = None
        if all(t is None for t in trans):
            setattr(node, transpose_name, None)

    def __str__(self) -> str:
        return "%s delete transpose %s[%s]" % (self.node.name, self.direction, self.idx)


class SetTranspose(Action):
    def __init__(self, node, direction, idx, transpose) -> None:
        super(SetTranspose, self).__init__(node)
        self.direction = direction
        self.idx = idx
        self.transpose = tuple(transpose)

    def __move_transpose(self, from_dir, to_dir):
        pass

    def _execute(self, node):
        LOGL("%s", str(self))
        transpose_name = "transpose_" + self.direction
        trans = getattr(node, transpose_name)
        if trans is None:
            dims_name = self.direction + '_dims'
            dims = getattr(self.node, dims_name)
            trans = [None] * len(dims)
            setattr(self.node, transpose_name, trans)
        trans[self.idx] = self.transpose
        if all(t is not None and tuple(trans[0]) == tuple(t) for t in trans):
            other_dir = "in" if self.direction == "out" else "out"
            LOGL("moving transpose from %s to %s", self.direction, other_dir)
            other_transpose_name = "transpose_" + other_dir
            assert getattr(node, other_transpose_name) is None
            setattr(node, other_transpose_name, [trans[0]])
            setattr(node, transpose_name, None)
            self.__move_transpose(self.direction, other_dir)

    def __str__(self) -> str:
        return "%s set transpose %s[%s] to %s" % (self.node.name, self.direction, self.idx, self.transpose)


class ReorderLinear(Action):
    def __init__(self, node, direction, transpose, shape, qrec=None) -> None:
        super(ReorderLinear, self).__init__(node)
        self.direction = direction
        self.shape = shape.clone()
        self.transpose = tuple(transpose)
        self.qrec = qrec

    @classmethod
    def out_from_history(cls, node, history, qrec):
        # Find the first entry in the transpose history that actually has a transpose
        first_valid_entry = next(iter([entry
                                       for rec in history
                                       for entry in rec[1::] if entry[0]]))
        # The shapes in the transpose history are cloned to the reorder above
        # will not affect them.
        return cls(node, "out", first_valid_entry[0], first_valid_entry[1], qrec=qrec)

    def _execute(self, node):
        LOGL("%s", str(self))
        if self.direction == "in":
            node.transpose_filter_in(self.transpose, self.shape)
            return
        node.transpose_filter_out(self.transpose, self.shape, self.qrec)

    def __str__(self) -> str:
        return "reorder linear layer %s %s with shape %s transposed %s" % (self.node.name, self.direction,
                                                                           self.shape, self.transpose)
