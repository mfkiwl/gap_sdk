
# Copyright (C) 2020  GreenWaves TechnoLOGies, SAS

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

import os

from graph.constant_store import ConstantStore
from graph.dim import Dim
from graph.nngraph import NNGraph
from graph.types import ConstantInputParameters
from graph.types.base import NNEdge
from importer.tflite2.common.tflite_graph import TFLiteGraph
from importer.tflite2.tflite_schema_head.Model import Model
from quantization.multiplicative.mult_quantization import \
    MultConstantQuantizationRecord
from quantization.quantization_set import QuantizationSet
from utils.add_sys_path import add_sys_path
from utils.node_id import NodeId

from ..common.provisional_dim import ProvisionalDim
from ..importer_base import ImporterBase
from .common import LOG, check, get_unique_suffix
from .common.handler_helper import get_all_backend_handlers
from .fix_split_in_edges import fix_split_in_edges
from .remove_concats import remove_concats

# pylint: disable=E1101


class TFLiteImporter(ImporterBase):
    DEFAULT_OPTS = {
        'rescale_perchannel': True
    }

    def __init__(self, *args, **kwargs) -> None:
        super(TFLiteImporter, self).__init__(*args, **kwargs)
        self._name_cache = {}
        self._provisional_outputs = None

    @property
    def provisional_outputs(self):
        return self._provisional_outputs

    def create_graph(self, filename, opts):
        opts = dict(self.DEFAULT_OPTS, **opts)
        self._name_cache = {}
        add_sys_path(os.path.dirname(__file__))
        buf = open(filename, "rb").read()
        model = Model.GetRootAsModel(buf, 0)
        LOG.info("Importing TFLITE model version %s", model.Version())
        check(model.Version() == 3, "Only support version 3 graphs at present")
        if model.SubgraphsLength() > 1:
            LOG.warning("nntool only supports one subgraph. There may be errors loading this graph.")
        G = NNGraph(model=model, filename=filename, name=opts.get('name'),
                    constant_store=ConstantStore())
        if opts.get('load_quantization'):
            G.quantization = QuantizationSet()
            G.has_quantized_parameters = True
            G.graph_identity.quantization_type = 'SQ8'

        self._import_tflite_graph(G, TFLiteGraph.from_model(model, 0), opts)

        fix_split_in_edges(G)
        G.add_dimensions()
        remove_concats(G)

        return G

    def _import_tflite_graph(self, G: NNGraph, graph: TFLiteGraph, opts: dict):
        handlers = self._get_handlers(graph.model_version)
        all_nodes = {}
        constants = self._get_all_constants(
            G, graph.tensors, load_quantization=opts.get('load_quantization'))
        all_nodes.update(constants)
        inputs = self._get_input_nodes(
            G, graph.input, load_quantization=opts.get('load_quantization'))
        all_nodes.update(inputs)
        self._provisional_outputs = self._get_output_nodes(
            G, graph.output, load_quantization=opts.get('load_quantization'))
        self._import_nodes(G, graph, handlers, all_nodes, self._provisional_outputs, opts)
        # propagate_hints(G)
        return G

    @staticmethod
    def _validate_name(name):
        def replace_all(text, bad_chars):
            for i in bad_chars:
                text = text.replace(i, '_')
            return text
        new_name = replace_all(name, [":", "/"])
        if new_name != name:
            new_name += "_" + get_unique_suffix()
        return new_name

    @staticmethod
    def _get_dim_from_shape(tf_shape):
        return Dim.unnamed([d.dim_value for d in tf_shape.dim
                            if (d.dim_value > 0 and d.dim_param == "")])

    def _get_handlers(self, graph_version):
        return get_all_backend_handlers(graph_version)

    @staticmethod
    def _load_quantization(qrecs, node_recs):
        for tensor in node_recs:
            qtype = tensor.qtype
            if qtype:
                qrecs[NodeId(node_recs[tensor][0])] = MultConstantQuantizationRecord(
                    in_qs=[qtype], out_qs=[qtype])

    def _get_all_constants(self, G, tensors, load_quantization=False):
        node_recs = {
            tensor: (
                ConstantInputParameters(
                    self._validate_name(tensor.name),
                    dims=Dim.unnamed(tensor.shape),
                    value=tensor.value),
                0,
                ProvisionalDim.from_tflite_shape(tensor.shape)
            )
            for tensor in tensors if tensor.is_constant
        }
        if load_quantization:
            self._load_quantization(G.quantization, node_recs)

        return node_recs

    def _get_input_nodes(self, G, inputs, load_quantization=False):
        prov_dims = {
            idx: ProvisionalDim.from_tflite_shape(inp.shape, check_for_batch=0)
            for idx, inp in enumerate(inputs)
        }
        hints = {
            idx: (['h', 'w', 'c'] if (len(dim.shape) == 4 and
                                      (dim.shape[-1] == 1 or dim.shape[-1] == 3))
                  else None)
            for idx, dim in prov_dims.items()
        }
        node_recs = {
            inp: (
                G.add_input(Dim.unnamed(prov_dims[idx].known_shape).apply_naming_hints(hints[idx]),
                            in_dim_hint=[hints[idx]] if hints[idx] else None,
                            out_dim_hint=[hints[idx]] if hints[idx] else None),
                0,
                prov_dims[idx]
            )
            for idx, inp in enumerate(inputs)
        }
        if load_quantization:
            self._load_quantization(G.quantization, node_recs)
        return node_recs

    def _get_output_nodes(self, G, outputs, load_quantization=False):
        node_recs = {
            outp: (
                G.add_output(),
                0,
                ProvisionalDim.from_tflite_shape(outp.shape)
            )
            for outp in outputs
        }
        if load_quantization:
            self._load_quantization(G.quantization, node_recs)
        return node_recs

    def _import_nodes(self, G, graph, handlers, all_nodes, outputs, opts):
        for node in graph.nodes:
            handler = handlers.get(node.op_name, None)
            if not handler:
                raise ValueError("no handler found for %s" % node.op_type)
            if node.is_custom and handler:
                handler = handler.get(node.custom_op_name, None)
                if not handler:
                    raise ValueError("no handler found for custom operation %s" % node.custom_op_name)

            params = handler.handle(node, all_nodes=all_nodes, G=G, opts=opts, importer=self)
            if params is None:
                continue
            for idx, out_tensor in enumerate(node.output):
                output = outputs.get(out_tensor)
                if not output:
                    continue
                G.add_edge(NNEdge(from_node=params,
                                  to_node=output[0], from_idx=idx, to_idx=output[1]))
