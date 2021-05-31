""" ConnectedComponents.py """

import sys
import time
import numpy as np
import networkx as nx
from ..exrpc.server import FrovedisServer
from ..exrpc import rpclib
from .graph import Graph
from .g_validate import validate_graph

def connected_components(G, opt_level=2, hyb_threshold=0.4,
                         verbose=0,
                         print_summary=False,
                         print_limit=5):
    """
    DESC:   Computes connected components of a graph
    PARAM:  G: frovedis/networkx graph object
            opt_level: int (0, 1 or 2) (default: 2)
            hyb_threshold: double (0.0 ~ 1.0) (default: 0.4)
            verbose: int (0 or 1) (default: 0)
            print_summary: whether to print summary of connected components
            print_limit: the maximum number of nodes info to be printed
    RETURN: A dictionary with keys as root-nodeid for each component,
            and values as list of pairs of nodeid with its distance
            from root of the component to which the node belongs
    """
    G, inp_movable = validate_graph(G)
    nvert = G.num_vertices
    (host, port) = FrovedisServer.getServerInstance()
    # graph.py: node data is loaded as int64
    nodes_dist = np.empty(nvert, dtype=np.int64) # actually levels (I-type)
    nodes_in_which_cc = np.empty(nvert, dtype=np.int64)

    stime = time.time()
    root_with_cc_count = rpclib.call_frovedis_cc(host, port,\
                                    G.get(), nodes_in_which_cc,\
                                    nodes_dist, nvert,\
                                    opt_level, hyb_threshold)
    excpt = rpclib.check_server_exception()
    if excpt["status"]:
        raise RuntimeError(excpt["info"])
    if inp_movable:
        G.release()
    etime = time.time()
    if verbose:
        print("cc computation time: %.3f sec." % (etime - stime))

    stime = time.time()
    if print_summary:
        show_cc_summary(root_with_cc_count, nodes_in_which_cc, \
                        nodes_dist, nvert, print_limit)
    ret = {root_with_cc_count[i] : [] \
           for i in range(0, len(root_with_cc_count), 2)}
    for i in range(0, nvert):
        cc_root = nodes_in_which_cc[i]
        if cc_root != sys.maxsize:
            ret[cc_root].append(i + 1)
    etime = time.time()
    if verbose:
        print("cc res conversion time: %.3f sec." % (etime - stime))
    for i in sorted(ret.values()):
        yield set(i)

def show_cc_summary(root_with_cc_count,
                    nodes_in_which_cc, nodes_dist,
                    num_vertices, print_limit):
    """Connected Component Summary"""
    num_cc = len(root_with_cc_count) // 2
    print("Number of connected components: %d" % num_cc)

    num_cc_printed = min(print_limit, num_cc)
    print("Root with its count of nodes in each connected component: " \
          + "(root_id:count)")
    for i in range(0, 2 * num_cc_printed, 2):
        print("%d:%d  " % (root_with_cc_count[i], \
                           root_with_cc_count[i + 1]))
    if num_cc > print_limit:
        print("...")

    print("Nodes in which cc: (node_id:root_id)")
    count = 1
    for i in range(num_vertices):
        if nodes_in_which_cc[i] != sys.maxsize:
            print("%d:%d  " % (i+1, nodes_in_which_cc[i]))
            count = count + 1
        if count > print_limit:
            print("...")
            break

    print("Nodes dist: (node:level_from_root)")
    count = 1
    for i in range(num_vertices):
        if nodes_dist[i] != sys.maxsize:
            print("%d:%d  " % (i+1, nodes_dist[i]))
            count = count + 1
        if count > print_limit:
            print("...")
            break
