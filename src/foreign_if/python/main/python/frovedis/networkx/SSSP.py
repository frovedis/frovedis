""" SSSP.py """

import numpy as np
import networkx as nx
from .graph import Graph

from ..exrpc.server import FrovedisServer
from ..exrpc import rpclib

def single_source_shortest_path(G, source):
    """
    DESC: Calls frovedis sssp
    PARAM:  Frovedis graph -> G
            source int
    RETURN: dict of lists
    """
    if isinstance(G, nx.classes.graph.Graph):
        G = Graph(nx_graph=G) #convert to frov graph
        inp_movable = True
    else:
        inp_movable = False

    if source < 1 and source > G.num_vertices:
        raise ValueError("source %d is not found in the given graph." % source)
    dist_lst = np.empty(G.num_vertices, dtype=np.int32)
    pred_lst = np.empty(G.num_vertices, dtype=np.int64)
    (host, port) = FrovedisServer.getServerInstance()
    #long, int, long, int, long
    rpclib.call_frovedis_sssp(host, port,\
            G.get(), dist_lst, pred_lst, G.num_vertices, source-1)
    excpt = rpclib.check_server_exception()
    if excpt["status"]:
        raise RuntimeError(excpt["info"])
    sssp_ret = {}
    srcid = source -1
    for i in range(0, G.num_vertices):
        if i == srcid:
            sssp_ret[i+1] = [source]
        elif i == pred_lst[i]: 
            pass # skip for those nodes which can not be reached from source 
        else:    # for all nodes which can be reached from source
            dist = dist_lst[i]
            sssp_ret[i+1] = [0] * (dist + 1) # +1 for storing the destination itself
            pred = pred_lst[i]
            cur_idx = dist - 1
            while(pred != srcid):
                sssp_ret[i+1][cur_idx] = pred + 1
                pred = pred_lst[pred]
                cur_idx = cur_idx - 1
            sssp_ret[i+1][0] = source   #loop terminates when it finds source
            sssp_ret[i+1][dist] = i + 1 #storing the destination at end
    if(inp_movable): 
        G.release()
    return sssp_ret
