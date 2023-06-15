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

from copy import deepcopy
import logging
from nntool.graph.manipulations.dump_refs import DumpSubgraphRef

from nntool.graph.types.misc import PadNode
from nntool.graph.types.pooling import PoolingNodeBase
from nntool.graph.types.tensor_arithmetic import BroadcastableNodeBase
from nntool.utils.graph import GraphView

from ..dim import Dim
from .base import (FilterNodeBase, NNEdge, NNNodeRef, NodeOptions, NNNodeBase,
                   SensitiveToOrderMixin, SingleInputAndOutputMixin, cls_op_name)

LOG = logging.getLogger(__name__)


def insert_ext(l, elem, idx):
    if idx >= len(l):
        l.extend([None]*(idx + 1 - len(l)))
    l[idx] = elem


class FusionInputOutputNodeBase(NNNodeBase):

    def __init__(self, *args, idx=0, dims=None, **kwargs):
        super().__init__(*args, **kwargs)
        if isinstance(dims, (list, tuple)):
            dims = Dim.unnamed(dims)
        self.dims = dims
        self.idx = idx

    @property
    def attrs(self) -> dict:
        super_attrs = super().attrs
        super_attrs.update({k: getattr(self, k) for k in [
            "idx", "dims"
        ] if getattr(self, k)})
        return super_attrs

    @property
    def can_equalize(self):
        return False

    def get_output_size(self, in_dims):
        self.dims = in_dims[0].clone()
        return [self.dims]

    def get_parameter_size(self):
        return 0

    def __str__(self):
        return ""


@cls_op_name('fusion_input')
class FusionInputNode(FusionInputOutputNodeBase):
    def __call__(self, graph):
        if not isinstance(graph, GraphView):
            raise ValueError('expecting GraphView as parameter')
        return NNNodeRef(graph, self, 0)


@cls_op_name('fusion_output')
class FusionOutputNode(FusionInputOutputNodeBase):
    pass


class FusionNodeBase(NNNodeBase):
    fusion_op_name = "!!NOT SET!!"
    quantize_internals = True

    def __init__(self, name, *args, fusion_type=None, subgraph: GraphView=None,
                 input_mapping=None,
                 output_mapping=None,
                 in_dims=None, out_dims=None,
                 inout_set=False, **kwargs):
        # output_mapping [(int_node1, 1), (int_node2, 0), ....]
        # input mapping list of list inputs [[(int_node1, 1), (int_node2, 0)], ....]
        super(FusionNodeBase, self).__init__(name, *args, **kwargs)
        # subgraph = deepcopy(subgraph)
        self._subgraph = subgraph
        if not inout_set:
            in_hints = kwargs.get('in_dims_hint')
            out_hints = kwargs.get('out_dims_hint')
            nodes = self.contained_nodes()
            if input_mapping is None:
                input_mapping = [[(nodes[0], 0)]]

            for from_idx, node_list in enumerate(input_mapping):
                hint = [in_hints[from_idx]] if in_hints and len(
                    in_hints) > from_idx else None
                dims = in_dims[from_idx] if in_dims and len(
                    in_dims) > from_idx else None
                input_node = FusionInputNode("%s_in_%s" % (name, from_idx),
                                                   idx=from_idx,
                                                   dims=dims,
                                                   in_dims_hint=hint, out_dims_hint=hint)
                for node, to_idx in node_list:
                    subgraph.add_edge(NNEdge(input_node, node, to_idx=to_idx))

            if output_mapping is None:
                output_mapping = [(nodes[-1], 0)]

            for to_idx, (node, from_idx) in enumerate(output_mapping):
                hint = [out_hints[to_idx]] if out_hints and len(
                    out_hints) > to_idx else None
                output_node = FusionOutputNode("%s_out_%s" % (name, to_idx),
                                                     idx=to_idx,
                                                     in_dims_hint=hint, out_dims_hint=hint)
                subgraph.add_edge(NNEdge(node, output_node, from_idx=from_idx))

        inputs_by_idx = sorted(list({node.idx: node for node in subgraph.nodes()
                                     if isinstance(node, FusionInputNode)}.values()),
                               key=lambda x: x.idx)
        outputs_by_idx = sorted([node for node in subgraph.nodes()
                                 if isinstance(node, FusionOutputNode)],
                                key=lambda x: x.idx)

        if not self.in_dims_hint:
            self.in_dims_hint = [None]*len(inputs_by_idx)
            for idx, node in enumerate(inputs_by_idx):
                edge = subgraph.out_edges(node.name)[0]
                if edge.to_node.in_dims_hint:
                    self.in_dims_hint[idx] = edge.to_node.in_dims_hint[edge.to_idx]
            if all(hint is None for hint in self.in_dims_hint):
                self.in_dims_hint = None
        if in_dims is not None:
            self.in_dims = in_dims
        if not self.out_dims_hint:
            self.out_dims_hint = [None]*len(outputs_by_idx)
            for idx, node in enumerate(outputs_by_idx):
                edge = subgraph.in_edges(node.name)[0]
                if edge.to_node.out_dims_hint:
                    self.out_dims_hint[idx] = edge.from_node.out_dims_hint[edge.from_idx]
            if all(hint is None for hint in self.out_dims_hint):
                self.out_dims_hint = None
        if out_dims is not None:
            self.out_dims = out_dims

        self.fusion_type = fusion_type

    @property
    def _nargs(self):
        return len(self._subgraph.nodes(node_classes=FusionInputNode))

    @property
    def attrs(self) -> dict:
        super_attrs = super().attrs
        super_attrs['subgraph'] = DumpSubgraphRef(self.name)
        super_attrs['inout_set'] = True
        super_attrs.update({k: getattr(self, k) for k in [
            "fusion_type"
        ] if getattr(self, k)})
        return super_attrs


    @staticmethod
    def get_mapping_from_edges(edges):
        mappings = {}
        for from_idx, edge in enumerate(edges):
            node_mappings = mappings.get(edge.to_node)
            if not node_mappings:
                node_mappings = {}
                mappings[edge.to_node] = node_mappings
            node_mappings[from_idx] = edge.to_idx
        return mappings

    @property
    def op_name(self):
        if self.fusion_type:
            return self.fusion_op_name + '_' + self.fusion_type
        return self.fusion_op_name

    @property
    def subgraph(self) -> GraphView:
        return self._subgraph

    def contained_nodes(self):
        return [node for node in self.subgraph.topological_sort()
                if not isinstance(node, FusionInputOutputNodeBase)]

    def get_contained_node(self, name):
        return next((n for n in self.contained_nodes() if n.name == name), None)

    @property
    def can_equalize(self):
        return all([param.can_equalize for param in self.contained_nodes()])

    @staticmethod
    def convert_input_mapping(input_mapping):
        orig = []
        for node, edges in input_mapping.items():
            for from_idx, to_idx in edges.items():
                if from_idx >= len(orig) or not isinstance(orig[from_idx], list):
                    inps = []
                    insert_ext(orig, inps, from_idx)
                else:
                    inps = orig[from_idx]
                inps.append((node, to_idx))
        return orig

    @staticmethod
    def convert_output_mapping(output_mapping):
        orig = []
        for node, edges in output_mapping.items():
            for from_idx, to_idx in edges.items():
                insert_ext(orig, (node, from_idx), to_idx)
        return orig

    def get_parameter_size(self):
        return 0

    def get_output_size(self, in_dims):
        node_out_dims = []
        for node in self.subgraph.topological_sort():
            if isinstance(node, FusionInputNode):
                node_in_dims = [self.clone_dim_with_hint(
                    in_dims[node.idx], node.idx)]
            else:
                node_in_dims = []
                for edge in self.subgraph.in_edges(node.name):
                    insert_ext(
                        node_in_dims, edge.from_node.out_dims[edge.from_idx], edge.to_idx)
            node.in_dims = [
                dim.clone() if dim else None for dim in node_in_dims]
            out_dims = node.get_output_size(node.in_dims)
            node.out_dims = out_dims
            if isinstance(node, FusionOutputNode):
                insert_ext(node_out_dims, out_dims[0], node.idx)
        return node_out_dims

    @staticmethod
    def dont_quantize_internals(deco_cls):
        setattr(deco_cls, 'quantize_internals', False)
        return deco_cls

    def __str__(self):
        return "{}".format(", ".join([str(node).strip() for node in self.contained_nodes()]))


@cls_op_name('matrix_scale')
class MatScaleFusionNode(FusionNodeBase, SensitiveToOrderMixin):
    fusion_op_name = "matscale"

    def __init__(self, *args, activation=None, **kwargs):
        self.activation = activation
        super(MatScaleFusionNode, self).__init__(*args, **kwargs)

    def get_output_size(self, in_dims):
        return [Dim.broadcast(in_dims)]


class FilterFusionNodeBase(FusionNodeBase, SingleInputAndOutputMixin):

    def _init_at_options(self):
        if self._at_options is None:
            self._at_options = NodeOptions(None)
        self._at_options.extend(
            *[node.at_options for node in self.contained_nodes()])

    @property
    def at_options(self):
        self._init_at_options()
        return self._at_options

    @at_options.setter
    def gen_ctrl(self, val):
        self._init_at_options()
        self._at_options = val

    def contained_filters(self):
        return [x for x in self.contained_nodes() if isinstance(x, FilterNodeBase)]

    @property
    def graph_label(self):
        label = [self.name]
        for node in self.contained_nodes():
            inner_label = node.graph_label[1::]
            if isinstance(node, PoolingNodeBase):
                inner_label = [f'{node.pool_type} pool {node.filter_dim}']
            else:
                inner_label = node.graph_label[1::]
            label.extend(inner_label)
        return label

    def get_parameter_size(self):
        return sum([node.get_parameter_size() for node in self.contained_nodes()])

    def __str__(self):
        return "{} {}".format(", ".join([str(node).strip() for node in self.contained_nodes()]), self.gen_ctrl or "")

    def compute_load(self):
        return sum([load if load else 0 for load in [node.compute_load()
                                                     for node in self.contained_nodes()]])


@cls_op_name('conv_fusion')
class ConvFusionNode(FilterFusionNodeBase, SensitiveToOrderMixin):

    fusion_op_name = "conv_fusion"


@cls_op_name('conv_batchnorm_fusion')
class ConvBatchNormFusionNode(FilterFusionNodeBase, SensitiveToOrderMixin):

    fusion_op_name = "conv_batchnorm_fusion"


@cls_op_name('linear_fusion')
class LinearFusionNode(FilterFusionNodeBase):

    fusion_op_name = "linear_fusion"


@cls_op_name('padded_add_fusion')
class PaddedAddFusionNode(FusionNodeBase, SensitiveToOrderMixin):
    fusion_op_name = "padded_add_fusion"

    def get_pad_node(self):
        return [node for node in self.contained_nodes() if isinstance(node, PadNode)][0]

    @property
    def padding_axis(self):
        pad_node = self.get_pad_node()
        padding_axis = [idx for idx, pad in enumerate(pad_node.padding) if sum(pad)][0]
        return padding_axis

    @padding_axis.setter
    def padding_axis(self, value):
        pad_node = self.get_pad_node()
        old_padding_axis = [idx for idx, pad in enumerate(pad_node.padding) if sum(pad)][0]
        new_pad = list(deepcopy(pad_node.padding))
        pad_val = pad_node.padding[old_padding_axis]
        new_pad.remove(pad_val)
        new_pad.insert(value, pad_val)
        pad_node.padding = tuple(new_pad)

    def _init_at_options(self):
        if self._at_options is None:
            self._at_options = NodeOptions(None)
        self._at_options.extend(
            *[node.at_options for node in self.contained_nodes()])

    @property
    def at_options(self):
        self._init_at_options()
        return self._at_options

    @at_options.setter
    def gen_ctrl(self, val):
        self._init_at_options()
        self._at_options = val

    def padding_dim(self):
        pad_node = [node for node in self.contained_nodes(
        ) if isinstance(node, PadNode)]
        return pad_node[0].padding

    def get_parameter_size(self):
        return 0

    def __str__(self):
        return "{} {}".format(", ".join([str(node).strip() for node in self.contained_nodes()]), self.gen_ctrl or "")

    def compute_load(self):
        return sum([load if load else 0 for load in [node.compute_load()
                                                     for node in self.contained_nodes()]])


@cls_op_name('matmulop_fusion')
class MatMulOpFusionNode(FusionNodeBase, SingleInputAndOutputMixin, SensitiveToOrderMixin):
    '''Fusion of operators. At present restricted to single input and output but
    this could be removed perhaps'''

    fusion_op_name = "matmulop_fusion"

    def _init_at_options(self):
        if self._at_options is None:
            self._at_options = NodeOptions(None)
        self._at_options.extend(
            *[node.at_options for node in self.contained_nodes()])

    def get_output_size(self, in_dims):
        out_dims = super(MatMulOpFusionNode,
                         self).get_output_size(in_dims)
        return out_dims

    @property
    def graph_label(self):
        label = [self.name]
        for node in self.contained_nodes():
            inner_label = node.graph_label[1::]
            if inner_label:
                label.extend(inner_label)
            else:
                label.append(node.CLS_OP_NAME)
        return label

    @property
    def at_options(self):
        self._init_at_options()
        return self._at_options

    @at_options.setter
    def gen_ctrl(self, val):
        self._init_at_options()
        self._at_options = val

    def get_parameter_size(self):
        return sum([node.get_parameter_size() for node in self.contained_nodes()])

    def __str__(self):
        return "{} {}".format(", ".join([str(node).strip() for node in self.contained_nodes()]), self.gen_ctrl or "")

    def compute_load(self):
        return sum([load if load else 0 for load in [node.compute_load()
                                                     for node in self.contained_nodes()]])


class ActivationFusionNodeBase(FusionNodeBase):

    @property
    def graph_label(self):
        label = [self.name]
        for node in self.contained_nodes():
            inner_label = node.graph_label[1::]
            label.extend(inner_label)
        return label


@cls_op_name('activation_fusion')
class ActivationFusionNode(ActivationFusionNodeBase, SensitiveToOrderMixin):
    fusion_op_name = "activation_fusion"


@cls_op_name('broadcastable_activation_fusion')
class BroadcastableActivationFusionNode(ActivationFusionNodeBase, BroadcastableNodeBase):
    fusion_op_name = "broadcastable_activation_fusion"


dont_quantize_internals = FusionNodeBase.dont_quantize_internals
