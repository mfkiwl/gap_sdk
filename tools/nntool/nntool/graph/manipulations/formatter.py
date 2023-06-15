# Copyright (C) 2022  GreenWaves Technologies, SAS

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

from nntool.graph.types import ImageFormatNode, NNEdge, TransposeNode
from nntool.quantization.qtype import QType


def insert_formatter(G, input_node, formatter, normalizer):
    format_node = ImageFormatNode(input_node.name + "_formatter",
                                        norm_func=normalizer.upper(),
                                        format_change=formatter.upper())
    out_edges = G.out_edges(input_node.name)

    # dims updated to reflect formatter
    if format_node.output_channels is not None and format_node.input_channels is not None:
        out_dim = input_node.get_output_size(None)[0]
        if formatter.upper() in ("BW8", "BW16"):
            assert format_node.input_channels == 1
            in_dim = out_dim.clone()
            format_node.out_dims_hint = input_node.out_dims_hint
            format_node.in_dims_hint = input_node.out_dims_hint
            input_node.dims = in_dim
            for out_edge in out_edges:
                G.remove_edge(out_edge)
        else:
            if not out_dim.is_named or out_dim.c != format_node.output_channels:
                raise ValueError(
                    "current graph input is not named or does not match formatter output channels")
            if formatter.upper() in ("RGB16", "BW16") and normalizer.upper() != "OUT_INT16":
                raise ValueError(
                    "rgb16 and bw16 formatters must have out_int16 as normalization function")
            in_dim = out_dim.clone()
            in_dim.c = format_node.input_channels
            in_dim.impose_order(("h", "w", "c"))
            format_node.in_dims_hint = [["h", "w", "c"]]
            input_node.dims = in_dim
            if input_node.fixed_order:
                new_out_edges = []
                for out_edge in out_edges:
                    if isinstance(out_edge.to_node, TransposeNode):
                        trans_node = out_edge.to_node
                        transpose_edges = G.out_edges(trans_node.name)
                        new_out_edges.extend(transpose_edges)
                        G.remove(trans_node)
                        if G.quantization:
                            nid = trans_node.name
                            if nid in G.quantization:
                                del G.quantization[trans_node.name]
                    else:
                        new_out_edges.append(out_edge)
                out_edges = new_out_edges
            else:
                input_node.fixed_order = True
                for out_edge in out_edges:
                    G.remove_edge(out_edge)
            format_node.out_dims_hint = [["c", "h", "w"]] * len(out_edges)
            input_node.out_dims_hint = [["h", "w", "c"]]
            G.node_options[input_node.name] = input_node.at_options
    # qrec updated to reflect formatter
    input_qrec = G.quantization and G.quantization.get(input_node.name)
    if input_qrec and format_node.input_dtype and format_node.output_dtype:
        formatter_qrec = G.quantization.get(format_node.name)
        if not formatter_qrec:
            if input_qrec.out_qs[0].dtype != format_node.output_dtype:
                raise ValueError(
                    "current graph input output quantization does not match formatter output")
            formatter_qrec = deepcopy(input_qrec)
            formatter_qrec.out_qs[0] = deepcopy(formatter_qrec.out_qs[0])
            if formatter_qrec.ktype.startswith('scaled'):
                formatter_in_q = QType(
                    scale=1, zero_point=0, dtype=format_node.input_dtype)
            elif formatter_qrec.ktype.startswith('symmetric'):
                formatter_in_q = QType(q=0, dtype=format_node.input_dtype)
            else:
                raise NotImplementedError("quantization has unknown type")
            if len(formatter_qrec.in_qs) > 0:
                formatter_qrec.in_qs[0] = formatter_in_q
                input_qrec.in_qs[0] = formatter_in_q
            else:
                formatter_qrec.in_qs.append(formatter_in_q)
                input_qrec.in_qs.append(formatter_in_q)
            input_qrec.out_qs[0] = formatter_in_q
        G.quantization[format_node.name] = formatter_qrec

    G.add_node(format_node)
    G.add_edge(NNEdge(input_node, format_node))
    for out_edge in out_edges:
        G.add_edge(NNEdge(format_node, out_edge.to_node, to_idx=out_edge.to_idx))


def remove_formatter(G, fmt_node):
    input_edges = G.in_edges(fmt_node.name)
    assert len(input_edges) == 1, "formatter node should only have one input"
    input_node = input_edges[0].from_node
    fmt_edges = G.out_edges(fmt_node.name)
    fmt_qrec = G.quantization and G.quantization.get(fmt_node.name)
    G.remove(fmt_node)

    input_node.dims = fmt_node.out_dims[0]
    input_node.out_dims_hint = fmt_node.out_dims_hint
    for fmt_edge in fmt_edges:
        G.add_edge(NNEdge(input_node, fmt_edge.to_node, to_idx=fmt_edge.to_idx))
    if fmt_qrec:
        input_qrec = G.quantization[input_node.name]
        input_qrec.out_qs = fmt_qrec.out_qs
        input_qrec.in_qs = fmt_qrec.out_qs
        G.quantization.remove_node(fmt_node)
