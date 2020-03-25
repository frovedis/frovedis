""" ConnectedComponents.py """

import numpy as np
import networkx as nx
from .graph import Graph

from ..exrpc.server import FrovedisServer
from ..exrpc import rpclib

def connected_components(G, print_summary=False):
    """
    DESC:   Computes connected components of a graph
    PARAM:  Frovedis graph object
    RETURN: A dictionary with keys as root-nodeid for each component,
            and values as list of pairs of nodeid with its distance 
            from root of the component to which the node belongs
    """
    if isinstance(G, nx.classes.graph.Graph):
        G = Graph(nx_graph=G) #convert to frov graph
        inp_movable = True
    else:
        inp_movable = False

    (host, port) = FrovedisServer.getServerInstance()
    nodes_dist = np.empty(G.num_vertices, dtype=np.int32)
    nodes_in_which_cc = np.empty(G.num_vertices, dtype=np.int64)
    num_nodes_in_each_cc = rpclib.call_frovedis_bfs(host, port,\
                                    G.get(), nodes_in_which_cc,\
                                    nodes_dist, G.num_vertices)
    excpt = rpclib.check_server_exception()
    if excpt["status"]:
        raise RuntimeError(excpt["info"])
    if(print_summary):
        num_cc = len(num_nodes_in_each_cc)
        print("Number of Connected Components: %d" % num_cc)
        num_cc_printed = 20
        if num_cc < num_cc_printed:
            num_cc_printed = num_cc
        print("Number of nodes in each connected component: (printing the first %d) " % num_cc_printed)
        for i in range(num_cc_printed):
            print("%d:%d  " % (i, num_nodes_in_each_cc[i]))
        print("Nodes in which cc: ")
        for i in range(G.num_vertices):
            print("%d:%d  " % (i, nodes_in_which_cc[i]))
        print("Nodes dist: ")
        for i in range(G.num_vertices):
            print("%d:%d  " % (i, nodes_dist[i]))
    ret = {i+1 : [] for i in np.unique(nodes_in_which_cc)}
    for i in range(G.num_vertices):
        cc_root = nodes_in_which_cc[i] + 1
        pair = (i+1, nodes_dist[i])
        ret[cc_root].append(pair)
    if(inp_movable):
        G.release()
    return ret
