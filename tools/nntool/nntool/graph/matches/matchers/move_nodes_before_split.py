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

from nntool.graph.types import NNEdge, TransposeNode, ReshapeNode, SplitNode, StridedSliceNode
from nntool.graph.types.base import ComparableNodeMixin
from nntool.quantization.quantizer.new_quantizer import NewQuantizer
from nntool.utils.graph import GraphView

from ..matcher import (Matcher, description, groups, match_name,
                       needs_valid_dimension, run_before)

LOG = logging.getLogger(__name__)


@groups('*')
@match_name("move_nodes_before_split")
@description("Tries to move idenical nodes on all branches of a split before the split")
@needs_valid_dimension(True)
@run_before('fuse_gap_convs', 'fuse_gap_linear', 'fuse_gap_pool', 'fuse_op_activation_scale8')
class MoveNodesBeforeSplit(Matcher):
    @staticmethod
    def moveable_same_operation_edges(G, node):
        out_edges = G.out_edges(node.name)
        if not out_edges:
            return None
        first_edge = out_edges[0]
        first_node = first_edge.to_node
        if not isinstance(first_node, ComparableNodeMixin):
            return None
        if len(first_node.in_dims) > 1 or len(first_node.out_dims) > 1:
            return None
        if first_node.in_dims[0] != first_node.out_dims[0]:
            return None
        # actually transpose can be moved and reshape and sss maybe but the split needs to be modified
        if isinstance(first_node, (TransposeNode, ReshapeNode, StridedSliceNode)):
            return None
        for edge in out_edges[1::]:
            if not isinstance(edge.to_node, ComparableNodeMixin):
                return None
            if not first_edge.to_node.is_same_operation_as(G, edge.to_node):
                return None
            if len(G.indexed_out_edges(edge.to_node.name)) > 1:
                return None
        return out_edges

    def _match(self, G: GraphView, **kwargs):
        has_modified_graph = False
        for node in G.nodes(node_classes=SplitNode):
            same_op_edges = self.moveable_same_operation_edges(G, node)
            if not same_op_edges:
                continue
            has_modified_graph = True
            in_edges = G.in_edges(node.name)
            assert len(in_edges) == 1
            # sort by name to ensure that operation is repeatable
            same_op_edges.sort(key=lambda x: x.to_node.name)
            keep_node = same_op_edges[0].to_node
            LOG.info(
                'split node %s has duplicate operations on its out edges', node.name)
            LOG.info('moving %s before split node %s', keep_node.name, node.name)
            for edge in G.out_edges(node.name):
                node_out_edges = G.out_edges(edge.to_node.name)
                G.remove(edge.to_node)
                if edge.to_node != keep_node:
                    LOG.info('deleting duplicate node %s', edge.to_node.name)
                    if G.quantization:
                        nid = edge.to_node.name
                        if nid in G.quantization:
                            del G.quantization[nid]
                for out_edge in node_out_edges:
                    G.add_edge(NNEdge(from_node=node, from_idx=edge.from_idx,
                                      to_node=out_edge.to_node, to_idx=out_edge.to_idx))
            G.insert_node_at_edge(keep_node, in_edges[0], edge_class=NNEdge)
            if G.quantization:
                quantizer = NewQuantizer.from_quantized_graph(G)
                quantizer.quantize()



        return has_modified_graph
