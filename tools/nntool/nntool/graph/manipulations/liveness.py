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

from typing import Mapping, Sequence

from nntool.graph.types.input_output import InputNNNodeBase, InputNode, OutputNode

def calculate_liveness(G, steps: Sequence[Mapping]) -> Mapping[str, Mapping]:
    liveness = {}
    for i, step in enumerate(steps):
        node = step['node']
        # initialize the steps arrays for the analysis below
        step['start'] = []
        step['end'] = []
        # input nodes create tensors
        if isinstance(node, InputNNNodeBase):
            edges = G.out_edges(node.name)
            if edges:
                assert all(edge.from_idx == 0 for edge in edges), "inputs should create a single tensor"
                eparams = edges[0].params
                liveness[eparams.name] = {
                    'start': i, 'end': i,
                    'is_input':True, 'is_output': False, 'dims': eparams.dims
                }
        else:
            # check what we consume
            for edge in G.in_edges(node.name):
                eparams = edge.params
                live = liveness.get(eparams.name)
                assert live is not None, "Inputs to node must have already been created"
                if live['end'] < i:
                    live['end'] = i
                if isinstance(node, OutputNode):
                    live['is_output'] = True
            # check what we create
            for edge in G.out_edges(node.name):
                eparams = edge.params
                live = liveness.get(eparams.name)
                if live is None:
                    live = {
                        'start': i, 'end': i, 'is_input':False,
                        'is_output': False, 'dims': eparams.dims,
                    }
                    liveness[eparams.name] = live

    # update the tensors created in each step
    for edge_name, live in liveness.items():
        steps[live['start']]['start'].append(edge_name)
        steps[live['end']]['end'].append(edge_name)
    return liveness
