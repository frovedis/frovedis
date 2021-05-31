""" g_validate.py """

import numpy as np
import networkx as nx
from .graph import Graph

def validate_graph(G):
    """
     --- graph input error checker ---
    """
    if isinstance(G, nx.classes.graph.Graph):
        G = Graph(nx_graph=G) #convert to frov graph
        inp_movable = True
    elif isinstance(G, Graph):
        inp_movable = False
    else:
        raise TypeError("Requires networkx graph or frovedis graph, "  +\
                        "but provided type is: ", type(G))
    return G, inp_movable

def validate_traversal(G, source):
    """
     --- traversal input error checker ---
    """
    G, inp_movable = validate_graph(G)
    if source < 1 or source > G.num_vertices:
        raise ValueError("source %d is not found in the given graph." % source)
    return G, inp_movable

def validate_bfs(G, source, depth_limit=None):
    """
     --- bfs input error checker ---
    """
    G, inp_movable = validate_traversal(G, source)
    if depth_limit is None:
        depth = np.iinfo(np.int64).max
    else:
        depth = depth_limit
    if depth < 0:
        raise ValueError("depth limit %d should be a positive integer." \
                         % depth_limit)
    return G, inp_movable, depth
