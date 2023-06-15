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
from typing import Mapping, Optional, Sequence, Union, Tuple

import numpy as np
from nntool.graph.types import (ActivationFusionNodeBase,
                         ConstantInputNode, FilterFusionNodeBase,
                         FusionInputNode, FusionOutputNode,
                         InputNode, MatMulOpFusionNode,
                         PaddedAddFusionNode, NNNodeBase)
from nntool.execution.kernels.kernel_executer import KernelExecuter
from nntool.quantization.new_qrec import QRec
from nntool.utils.graph import Graph
from nntool.execution.execution_progress import ExecutionProgress
from nntool.execution.quantization_mode import QuantizationMode

LOG = logging.getLogger(__name__)


class GraphExecuter():
    def __init__(self,
                 G: Graph,
                 qrecs: Optional[Mapping[Union[str, Tuple], QRec]] = None):
        self._G = G
        self._qrecs = qrecs

    @staticmethod
    def collect_outputs(G, saved_outputs, node):
        # collect outputs from previous nodes
        # InputNode is already set above
        if isinstance(node, (InputNode, FusionInputNode, ConstantInputNode)):
            inputs = None
        else:
            inputs = [None]*len(node.in_dims)
            for edge in G.in_edges(node.name):
                inputs[edge.to_idx] = saved_outputs[edge.from_node][edge.from_idx]
        return inputs

    @staticmethod
    def save_output(saved_outputs, node, outputs):
        saved_outputs[node] = outputs

    def execute_qnoq_iterator(self,
                              in_tensors,
                              step_idx_limit=None,
                              silent=True,
                              yield_fusions=True,
                              parent_node=None,
                              parent_step_idx=None,
                              saved_outputs=None,
                              G=None):

        if not silent:
            LOG.debug("execute quantization comparison")
            ExecutionProgress.start()
        if G is None:
            G = self._G
            saved_outputs = {}

        inputs = sorted([node for node in G.inputs() if isinstance(node, (InputNode, FusionInputNode))], key=lambda node: node.name)
        inputs.extend(node for node in G.inputs() if isinstance(node, ConstantInputNode))
        for node in G.topological_sort(inputs):
            step_idx = node.step_idx
            if step_idx_limit is not None and step_idx > step_idx_limit:
                break

            if not silent:
                ExecutionProgress.progress(step_idx, node.name)

            output = self.collect_outputs(G, saved_outputs, node)
            if parent_node:
                nid = (parent_node.name, node.name)
            else:
                nid = node.name

            if isinstance(node, (FusionInputNode, FusionOutputNode)):
                qrec = None
            else:
                qrec = self._qrecs[nid]

            if isinstance(node, (FilterFusionNodeBase, ActivationFusionNodeBase, PaddedAddFusionNode)):
                for (f_step_idx, f_pnode, f_output, f_details, f_qoutput, f_qdetails, f_node) in self.execute_qnoq_iterator(
                    output,
                    yield_fusions=yield_fusions,
                    silent=silent,
                    parent_node=node,
                    parent_step_idx=step_idx,
                    saved_outputs=saved_outputs,
                    G=node.subgraph
                ):
                    if yield_fusions and not isinstance(f_node, (FusionInputNode, FusionOutputNode)):
                        yield f_step_idx, f_pnode, f_output, f_details, f_qoutput, f_qdetails, f_node

                f_outputs = node.subgraph.outputs()
                num_outputs = max(f_out.idx for f_out in f_outputs) + 1

                output = [None]*num_outputs
                for f_out in f_outputs:
                    output[f_out.idx] = saved_outputs[f_out][0]
                qoutput = []
            else:
                if isinstance(node, (InputNode, ConstantInputNode)):
                    details = {}
                    output = KernelExecuter.execute(node, in_tensors,
                                                    None,
                                                    details=details)
                    qdetails = {}
                    qoutput = KernelExecuter.execute(
                        node, in_tensors, qrec, details=qdetails)
                else:
                    qoutput = []
                    for val_idx, val in enumerate(output):
                        qoutput.append(qrec.in_qs[val_idx].quantize(val))
                    details = {}
                    output = KernelExecuter.execute(node, output,
                                                    None,
                                                    details=details)
                    qdetails = {}
                    qoutput = KernelExecuter.execute(
                        node, qoutput, qrec, details=qdetails)

                qoutput = [qrec.out_qs[i].dequantize(
                    out) for i, out in enumerate(qoutput)]

            yield step_idx, node, output, details, qoutput, qdetails, None
            self.save_output(saved_outputs, node, output)

        if not silent:
            ExecutionProgress.end()

    def execute_iterator(self,
                         in_tensors: Sequence[np.ndarray],
                         step_idx_limit: Optional[int] = None,
                         start_node: Optional[NNNodeBase] = None,
                         qmode: Optional[QuantizationMode] = None,
                         yield_fusions=True,
                         yield_details=True,
                         only_yield_step=False,
                         record_inputs: Optional[Mapping] = None,
                         silent=True,
                         parent_node=None,
                         parent_step_idx=None,
                         saved_outputs=None,
                         G=None):
        if qmode is None:
            qmode = QuantizationMode.none()

        if G is None:
            G = self._G
            saved_outputs = {}

        if not silent:
            LOG.debug("execute uncached: quantization mode %s", qmode)
            ExecutionProgress.start()

        inputs = sorted([node for node in G.inputs() if isinstance(node, (InputNode, FusionInputNode))], key=lambda node: node.name)
        inputs.extend(node for node in G.inputs() if isinstance(node, ConstantInputNode))
        for node in G.topological_sort(inputs):
            step_idx = node.step_idx
            if step_idx_limit is not None and step_idx > step_idx_limit:
                break

            if start_node and start_node != node:
                continue

            # collect outputs from previous nodes
            # InputNode is already set above
            output_tensors = self.collect_outputs(G, saved_outputs, node)

            if not silent:
                ExecutionProgress.progress(step_idx, node.name)
            if parent_node:
                nid = (parent_node.name, node.name)
            else:
                nid = node.name
            if record_inputs is not None:
                if output_tensors is None:
                    record_inputs[nid] = output_tensors
                else:
                    record_inputs[nid] = [np.copy(output_tensor)
                                          for output_tensor in output_tensors]
            if isinstance(node, (FusionInputNode, FusionOutputNode)):
                qrec = None
            else:
                if self._qrecs and qmode.get_quantized(node, step_idx):
                    if nid not in self._qrecs:
                        LOG.warning(
                            "no quantization parameters on %s", node.name)
                        qrec = None
                    else:
                        qrec = self._qrecs[nid]
                    if qmode.is_step and output_tensors:
                        output_tensors = [qrec.in_qs[i].quantize(
                            output_tensor) for i, output_tensor in enumerate(output_tensors)]
                else:
                    qrec = None

            details = {} if yield_details and (
                not only_yield_step or step_idx == step_idx_limit) else None
            if isinstance(node, (FilterFusionNodeBase, ActivationFusionNodeBase, PaddedAddFusionNode, MatMulOpFusionNode)):

                for f_step_idx, f_pnode, f_node, f_output_tensors, f_details in self.execute_iterator(
                        output_tensors,
                        qmode=qmode,
                        yield_fusions=yield_fusions,
                        yield_details=yield_details,
                        silent=True,
                        parent_node=node,
                        parent_step_idx=step_idx,
                        saved_outputs=saved_outputs,
                        G=node.subgraph
                ):
                    if yield_fusions and not isinstance(f_node, (FusionInputNode, FusionOutputNode)):
                        yield f_step_idx, f_pnode, f_node, f_output_tensors, f_details
                f_outputs = node.subgraph.outputs()
                num_outputs = max(f_output.idx for f_output in f_outputs) + 1
                output_tensors = [None]*num_outputs
                for f_output in f_outputs:
                    output_tensors[f_output.idx] = saved_outputs[f_output][0]

            elif isinstance(node, (InputNode, FusionInputNode)):
                output_tensors = KernelExecuter.execute(
                    node, in_tensors, qrec, details)
            else:
                output_tensors = KernelExecuter.execute(
                    node, output_tensors, qrec, details)

            if qmode.dequantize and qrec:
                qoutput_tensors = [qrec.out_qs[i].dequantize(
                    output_tensor) for i, output_tensor in enumerate(output_tensors)]
                if parent_node:
                    yield parent_step_idx, parent_node, node, qoutput_tensors, details
                elif not only_yield_step or step_idx == step_idx_limit:
                    yield step_idx, node, None, qoutput_tensors, details
                if qmode.is_step and qmode.get_quantized(node, step_idx):
                    output_tensors = qoutput_tensors
            elif qmode.is_float_q_deq and qrec:
                if qmode.is_step and qmode.get_quantized(node, step_idx):
                    output_tensors = [qrec.out_qs[i].dequantize(
                        output_tensor) for i, output_tensor in enumerate(output_tensors)]
                qoutput_tensors = [qrec.out_qs[i].dequantize(qrec.out_qs[i].quantize(
                    output_tensor)) for i, output_tensor in enumerate(output_tensors)]
                if parent_node:
                    yield parent_step_idx, parent_node, node, qoutput_tensors, details
                elif not only_yield_step or step_idx == step_idx_limit:
                    yield step_idx, node, None, qoutput_tensors, details
            else:
                if qmode.is_step and qmode.get_quantized(node, step_idx) and qrec:
                    output_tensors = [qrec.out_qs[i].dequantize(
                        output_tensor) for i, output_tensor in enumerate(output_tensors)]
                if parent_node:
                    yield parent_step_idx, parent_node, node, output_tensors, details
                elif not only_yield_step or step_idx == step_idx_limit:
                    yield step_idx, node, None, output_tensors, details

            self.save_output(saved_outputs, node, output_tensors)

        if not silent:
            ExecutionProgress.end()

    def execute_qnoq(self,
                     in_tensors: Sequence[np.ndarray],
                     step_idx_limit=None,
                     all_details=None,
                     yield_fusions=False,
                     silent=True):
        outputs = []
        if yield_fusions:
            fusion_outputs = []
            if all_details is not None:
                fusion_details = []
            else:
                fusion_details = None
        else:
            fusion_details = None
            fusion_outputs = None

        for _, _, _, _, qoutput, qdetails, fnode in self.execute_qnoq_iterator(in_tensors,
                                                                               step_idx_limit=step_idx_limit,
                                                                               silent=silent):
            if yield_fusions:
                if fnode:
                    fusion_outputs.append([output_tensor.copy()
                                           for output_tensor in qoutput])
                    if all_details is not None:
                        fusion_details.append(qdetails)
                else:
                    outputs.append({
                        'outputs': outputs.append([output_tensor.copy() for output_tensor in qoutput]),
                        'fusion_outputs': fusion_outputs.copy(),
                    })
                    fusion_outputs.clear()
                    if all_details is not None:
                        all_details.append({
                            'details': qdetails,
                            'fusion_details': fusion_details.copy()
                        })
                        fusion_details.clear()
            elif fnode is None:
                outputs.append([output_tensor.copy()
                                for output_tensor in qoutput])
                if all_details is not None:
                    all_details.append(qdetails)
        return outputs

    def execute(self,
                in_tensors: Sequence[np.ndarray],
                step_idx_limit=None,
                only_yield_step=False,
                qmode: QuantizationMode = None,
                all_details=None,
                yield_fusions=False,
                append_fusion_output=False,
                silent=True):

        if qmode is None:
            qmode = QuantizationMode.none()

        if qmode.is_step_all:
            iterator = [(qoutput, qdetails, fnode)
                        for _, _, _, _, qoutput, qdetails, fnode
                        in self.execute_qnoq_iterator(in_tensors,
                                                      yield_fusions=yield_fusions or append_fusion_output,
                                                      step_idx_limit=step_idx_limit,
                                                      silent=silent)]
        else:
            iterator = [(output_tensors, details, fnode)
                        for _, _, fnode, output_tensors, details
                        in self.execute_iterator(in_tensors, step_idx_limit=step_idx_limit,
                                                 qmode=qmode,
                                                 yield_fusions=yield_fusions or append_fusion_output,
                                                 only_yield_step=only_yield_step,
                                                 yield_details=all_details is not None,
                                                 silent=silent)]

        outputs = []
        if yield_fusions:
            fusion_outputs = []
            if all_details is not None:
                fusion_details = []
            else:
                fusion_details = None
        else:
            fusion_details = None
            fusion_outputs = None

        for output_tensors, details, fnode in iterator:
            if append_fusion_output and fnode:
                outputs.append([output_tensor.copy()
                                for output_tensor in output_tensors])
                if all_details is not None:
                    all_details.append(details)
            elif not append_fusion_output and yield_fusions:
                if fnode:
                    fusion_outputs.append([output_tensor.copy()
                                           for output_tensor in output_tensors])
                    if all_details is not None:
                        fusion_details.append(details)
                else:
                    outputs.append({
                        'outputs': outputs.append([output_tensor.copy() for output_tensor in output_tensors]),
                        'fusion_outputs': fusion_outputs.copy(),
                    })
                    fusion_outputs.clear()
                    if all_details is not None:
                        all_details.append({
                            'details': details,
                            'fusion_details': fusion_details.copy()
                        })
                        fusion_details.clear()
            else:
                outputs.append([output_tensor.copy()
                                for output_tensor in output_tensors])
                if all_details is not None:
                    all_details.append(details)
        return outputs
