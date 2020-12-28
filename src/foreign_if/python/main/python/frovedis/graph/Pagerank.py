""" Pagerank.py """

import sys
import time
import networkx as nx
from ..exrpc.server import FrovedisServer
from ..exrpc import rpclib
from .graph import Graph
from .g_validate import validate_graph 

def pagerank(G, alpha=0.85, personalization=None, max_iter=100, tol=1.0e-6, \
        nstart=None, weight='weight', dangling=None, verbose=0):
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
    G, inp_movable = validate_graph(G)
    (host, port) = FrovedisServer.getServerInstance()
    result = rpclib.call_frovedis_pagerank(host, port,\
                G.get(), tol, alpha, max_iter, verbose)
    excpt = rpclib.check_server_exception()
    if excpt["status"]:
        raise RuntimeError(excpt["info"])
    if inp_movable:
        G.release()
    stime = time.time()
    ret = dict(zip(result['nodeid'], result['rank']))
    etime = time.time()
    if verbose:
        print("pagerank result dictionary conversion time: %.3f sec." % (etime - stime))
    return ret
