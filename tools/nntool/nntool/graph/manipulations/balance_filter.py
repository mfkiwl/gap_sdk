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

import numpy as np

from nntool.graph.types import MultiplicativeBiasNodeBase
from nntool.utils.stats_funcs import astats

LOG = logging.getLogger(__name__)


def balance_filter(weights, biases, pnode, precision_threshold=None,
                   small_value_threshold=0.0000000001, fnode=None, G=None):
    node = pnode if fnode is None else fnode
    channel_dim = node.filter_dim.get_order_idx('out_c')
    stats = astats(weights, channel_dim=channel_dim, channel_details=True)
    if precision_threshold:
        if not any(stat['avg_prec'] < precision_threshold for stat in stats['channel_stats']):
            LOG.debug("layer %s doesn't have any weights below precision threshold", node.name)
        return False, None, None

    LOG.info("balancing weights of layer %s", node.name)
    if node.has_mul_bias and node.mul_biases is not None:
        mul_bias = node.mul_biases
    else:
        mul_bias = np.ones(node.filter_dim.out_c, dtype=np.float32)

    scale = np.array([max(abs(stat['max']), abs(stat['min']))
                      for stat in stats['channel_stats']])
    # don't balance channels that are effectively zero
    threshold_idx = scale < small_value_threshold
    scale[threshold_idx] = 1
    weights_shape = [1 if idx != channel_dim else size
                     for idx, size in enumerate(weights.shape)]
    weights /= scale.reshape(weights_shape)
    biases /= scale
    mul_bias *= scale
    node.has_mul_bias = True
    node.mul_biases = mul_bias
    return True, weights, biases

def balance_all_filters(G, precision_threshold=0.20):
    for _, node, _, fnode in G.nodes_iterator(yield_fusions=True):
        anode = node if fnode is None else fnode
        if isinstance(anode, MultiplicativeBiasNodeBase):
            balance_filter_with_constants(G, node, precision_threshold, fnode)

def balance_filter_with_constants(G, node, precision_threshold, fnode=None):
    in_edges = G.indexed_in_edges(node.name)[1]
    weights_node = in_edges[1].from_node
    biases_node = in_edges[1].from_node
    weights = weights_node.dqvalue.copy()
    biases = biases_node.dqvalue.copy()
    modified, weights, biases = balance_filter(
        weights, biases,
        node, precision_threshold=precision_threshold,
        fnode=fnode, G=G)
    if modified:
        weights_node.dqvalue = weights
        biases_node.dqvalue = biases
