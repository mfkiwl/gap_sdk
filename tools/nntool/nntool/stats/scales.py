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

from nntool.graph.types import Conv2DNode, LinearNode

class Scales():

    @staticmethod
    def conv2d_scale_output(node, scale, weights=None, biases=None):
        assert node.groups == 1 or node.tf_depthwise,\
            "this needs checking for grouped convs"
        for out_c in range(node.filter_dim.out_c):
            weights[node.filter_dim.srange(out_c=out_c)] /= scale[out_c]
            if biases is not None:
                biases[out_c] /= scale[out_c]
        return weights, biases

    @staticmethod
    def conv2d_scale_input(node, scale, weights=None):
        if node.tf_depthwise:
            for in_c in range(node.in_dims[0].c):
                in_c_start = in_c * node.multiplier
                weights[node.filter_dim.srange(out_c=\
                    [in_c_start, in_c_start + node.multiplier, 1])] *= scale[in_c]
        else:
            assert node.groups == 1,\
                "this needs checking for grouped convs"
            for in_c in range(node.in_dims[0].c):
                weights[node.filter_dim.srange(in_c=in_c)] *= scale[in_c]
        return weights

    @staticmethod
    def linear_scale_output(node, scale, weights=None, biases=None):
        if weights is None:
            weights = node.weights
        if biases is None and node.has_bias:
            biases = node.biases
        filt = node.filter_dim.get_filter_dims()
        for out_c in range(filt.out_c):
            weights[node.filter_dim.srange(out_c=out_c)] /= scale[out_c]
            if biases is not None:
                biases[out_c] /= scale[out_c]

        return weights, biases

    @staticmethod
    def linear_scale_input(node, scale, weights=None):
        if weights is None:
            weights = node.weights
        in_c_weights = weights.reshape(node.filter_dim)
        for in_c in range(node.in_dims[0].c):
            in_c_weights[node.filter_dim.srange(in_c=in_c, use_order=node.filter_dim.order)] *= scale[in_c]
        return weights

    @classmethod
    def scale_input(cls, G, node, scale, weights=None):
        if not isinstance(node, (Conv2DNode, LinearNode)):
            raise ValueError()
        if weights is None:
            weights = G.indexed_in_edges(node.name)[1].from_node.dqvalue.copy()
        if isinstance(node, Conv2DNode):
            return cls.conv2d_scale_input(node, scale, weights)
        if isinstance(node, LinearNode):
            return cls.linear_scale_input(node, scale, weights)
        raise ValueError()

    @classmethod
    def scale_output(cls, G, node, scale, weights=None, biases=None):
        if not isinstance(node, (Conv2DNode, LinearNode)):
            raise ValueError()
        if weights is None:
            in_edges = G.indexed_in_edges(node.name)
            weights = in_edges[1].from_node.dqvalue.copy()
            biases = in_edges[2].from_node.dqvalue.copy()
        if isinstance(node, Conv2DNode):
            return cls.conv2d_scale_output(node, scale, weights, biases)
        if isinstance(node, LinearNode):
            return cls.linear_scale_output(node, scale, weights, biases)
        raise ValueError()
