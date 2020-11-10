""" Pagerank.py """

import sys
import networkx as nx
from ..exrpc.server import FrovedisServer
from ..exrpc import rpclib
from .graph import Graph

def pagerank(G, alpha=0.85, personalization=None, max_iter=100, tol=1.0e-6, \
        nstart=None, weight='weight', dangling=None):
    """
    DESC: Calls frovedis pagerank
    PARAM:  Frovedis graph -> G
            float -> alpha -> 0.85
            dict -> personalization -> None
            int -> max_iter -> 100
            double -> tol -> 1.0e-6
            dict -> nstart -> None
            string -> weight -> weight
            dict -> dangling -> None
    RETURN: dict of ranks
    """
    if isinstance(G, nx.classes.graph.Graph):
        frov_gr = Graph(nx_graph=G)
        movable = True
    elif isinstance(G, Graph):
        frov_gr = G
        movable = False
    else:
        raise TypeError("Requires networkx graph or frovedis graph, but provided type is: ", type(G))
    (host, port) = FrovedisServer.getServerInstance()
    result = rpclib.call_frovedis_pagerank(host, port,\
                frov_gr.get(), tol, alpha, max_iter)
    excpt = rpclib.check_server_exception()
    if excpt["status"]:
        raise RuntimeError(excpt["info"])
    if movable:
        frov_gr.release()
    verts = {}
    for i in range(len(result)):
        if result[i] != sys.float_info.max:
            verts[i+1] = result[i]
    return verts
