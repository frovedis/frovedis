""" SSSP.py """

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

    if source < 1:
        raise ValueError("Source should be greater than or equal to 1.")
    dist_lst = [] # = [int(0)] * G.num_vertices
    pred_lst = [] #= [long(0)] * G.num_vertices
    (host, port) = FrovedisServer.getServerInstance()
    #long, int, long, int, long
    rpclib.call_frovedis_sssp(host, port,\
            G.get(), dist_lst, pred_lst, G.num_vertices, source-1)
    excpt = rpclib.check_server_exception()
    if excpt["status"]:
        raise RuntimeError(excpt["info"])
    sssp_ret = {i:[] for i in range(1, G.num_vertices+1)}
    for i in range(G.num_vertices):
        ctr = 0
        for dist in dist_lst:
            if dist == i:
                if ctr != source-1:
                    #adding 1 in ctr to convert from 0's value to 1's values
                    sssp_ret[ctr+1].extend(sssp_ret[pred_lst[ctr]+1])
                    sssp_ret[ctr+1].append(ctr+1)
                else:
                    sssp_ret[ctr+1].append(ctr+1)
            ctr = ctr + 1
    if(inp_movable): 
        G.release()
    return sssp_ret
