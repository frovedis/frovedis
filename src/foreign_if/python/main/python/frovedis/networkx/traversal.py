""" traversal.py """

import numpy as np
import networkx as nx
from .graph import Graph

from ..exrpc.server import FrovedisServer
from ..exrpc import rpclib

def generate_traversal_path(pred_list, source, nvert):
    ret = {}
    for i in range(1, nvert + 1):
        if i == source:
            ret[i] = [source]
        elif i == pred_list[i - 1]:
            pass # skip for those nodes which can not be reached from source
        else:    # for all nodes which can be reached from source
            path = []
            pred = pred_list[i - 1]
            while pred != source:
                if pred in ret:
                    #print("matched for %d" % (pred))
                    break
                path.append(pred)
                pred = pred_list[pred - 1]
            # loop terminates when it finds source
            # adding source at the begining of the path
            # and the destination at the end of the path
            path.reverse()
            if pred == source: # not found in ret
                ret[i] = [source] + path + [i]
            else:
                ret[i] = ret[pred] + path + [i]
    return ret

def bfs(G, source, opt_level=1, hyb_threshold=0.4):
    """
    DESC:   Computes bfs traversal path of a graph
    PARAM:  G: frovedis/networkx graph object 
            source: int (1 ~ G.num_vertices) 
            opt_level: int (0, 1 or 2) (default: 1)
            hyb_threshold: double (0.0 ~ 1.0) (default: 0.4)
    RETURN: A dictionary with keys as destination node-id 
            and values as correspondingb traversal path from the source.
            In case any node in input graph not reachable from the source,
            it would not be included in the resultant dictionary.
    """
    if isinstance(G, nx.classes.graph.Graph):
        G = Graph(nx_graph=G) #convert to frov graph
        inp_movable = True
    else:
        inp_movable = False

    if source < 1 or source > G.num_vertices:
        raise ValueError("source %d is not found in the given graph." % source)

    # graph.py: node data is loaded as int64
    dist_lst = np.empty(G.num_vertices, dtype=np.int64) # actually levels (I-type)
    pred_lst = np.empty(G.num_vertices, dtype=np.int64) # (I-type)

    nvert = G.num_vertices
    (host, port) = FrovedisServer.getServerInstance()
    import time
    stime = time.time()
    rpclib.call_frovedis_bfs(host, port, G.get(), \
                             dist_lst, pred_lst,  \
                             nvert,               \
                             source, opt_level, hyb_threshold)
    excpt = rpclib.check_server_exception()
    if excpt["status"]:
        raise RuntimeError(excpt["info"])
    etime = time.time()
    print("bfs computation time: %.3f sec." % (etime - stime))
    if inp_movable:
        G.release()
    import time
    stime = time.time()
    ret = generate_traversal_path(pred_lst, source, nvert)
    etime = time.time()
    print("bfs res conversion time: %.3f sec." % (etime - stime))
    return ret
    
def single_source_shortest_path(G, source, return_distance=False):
    """
    DESC:   Computes single source shortest path of a graph
    PARAM:  G: frovedis/networkx graph object 
            source: int (1 ~ G.num_vertices) 
            return_distance: bool (whether to return distance information)
    RETURN: If return_distance is True, then it returns 
            a tuple of dictionaries (x, y). 
            Where keys in "x" are the destination node-ids and 
            values are the corresponding travsersal path from the source node.
            And keys in "y" are the destination node-ids and
            values are the corresponding shortest distance from the source node.

            If return_distance is False, then it returns only
            the traversal path, "x".

            In case a node in input graph is not reachable from the source, 
            it would not be included in the results.
    """
    if isinstance(G, nx.classes.graph.Graph):
        G = Graph(nx_graph=G) #convert to frov graph
        inp_movable = True
    else:
        inp_movable = False

    if source < 1 or source > G.num_vertices:
        raise ValueError("source %d is not found in the given graph." % source)

    # graph.py: edge data is loaded as float64
    dist_lst = np.empty(G.num_vertices, dtype=np.float64) 
    # graph.py: node data is loaded as int64
    pred_lst = np.empty(G.num_vertices, dtype=np.int64)   

    nvert = G.num_vertices
    (host, port) = FrovedisServer.getServerInstance()
    import time
    stime = time.time()
    rpclib.call_frovedis_sssp(host, port,\
            G.get(), dist_lst, pred_lst, nvert, source)
    excpt = rpclib.check_server_exception()
    if excpt["status"]:
        raise RuntimeError(excpt["info"])
    etime = time.time()
    print("sssp computation time: %.3f sec." % (etime - stime))
    if inp_movable:
        G.release()
    stime = time.time()
    x = generate_traversal_path(pred_lst, source, nvert)
    if return_distance:
        y = {}
        for i in range(0, nvert):
            if i + 1 == source:
                y[i + 1] = 0
            elif pred_lst[i] != i + 1: #if reachable from source
                y[i + 1] = dist_lst[i]
        ret = (x, y)
    else:
        ret = x
    etime = time.time()
    print("sssp res conversion time: %.3f sec." % (etime - stime))
    return ret

