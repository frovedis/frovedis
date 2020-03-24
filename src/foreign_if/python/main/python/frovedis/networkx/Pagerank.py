""" Pagerank.py """

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
    copy_gr = None
    if isinstance(G, nx.classes.graph.Graph):
        copy_gr = Graph(nx_graph=G) #convert to frov graph
    elif isinstance(G, Graph):
        copy_gr = G.copy()
    else:
        raise TypeError("Requires networkx graph or frovedis graph, but provided type is: ", type(G))
    (host, port) = FrovedisServer.getServerInstance()
    result = rpclib.call_frovedis_pagerank(host, port,\
                copy_gr.get(), tol, alpha, max_iter)
    excpt = rpclib.check_server_exception()
    if excpt["status"]:
        raise RuntimeError(excpt["info"])
    # TODO: vertices ID based on input graph
    verts = {}
    v_id = 1
    for i in result:
        verts[v_id] = i
        v_id = v_id + 1
    copy_gr.release()
    return verts
