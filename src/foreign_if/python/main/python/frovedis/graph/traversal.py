""" traversal.py """

import time
import numpy as np
import networkx as nx
from ..exrpc.server import FrovedisServer
from ..exrpc import rpclib
from .graph import Graph
from .g_validate import *

def generate_traversal_path(result, source):
    ret = {}
    destid = result['destids']
    pred = result['predecessors']
    dest_to_pred = dict(zip(destid, pred))

    for k in destid:
        if k == source: # currently only possible for source node 
              ret[k] = [source]
        else:
            path = []
            pred = dest_to_pred[k]
            while pred != source:
                if pred in ret:
                    #print("matched for %d" % (pred))
                    break
                path.append(pred)
                pred = dest_to_pred[pred]
            path.reverse()
            if pred == source: # not found in ret
                ret[k] = [source] + path + [k]
            else:
                ret[k] = ret[pred] + path + [k]
        yield {k: ret[k]}

def bfs_impl(G, source, depth_limit=None, opt_level=1, hyb_threshold=0.4,
             verbose=0):
    """
     --- main wrapper function which invokes server side bfs implementation ---
    """
    G, inp_movable, depth = validate_bfs(G, source, depth_limit)
    (host, port) = FrovedisServer.getServerInstance()
    stime = time.time()
    res = rpclib.call_frovedis_bfs(host, port, G.get(), \
                 source, opt_level, hyb_threshold, depth)
    excpt = rpclib.check_server_exception()
    if excpt["status"]:
        raise RuntimeError(excpt["info"])
    if inp_movable:
        G.release()
    etime = time.time()
    if verbose: 
        print("bfs computation time: %.3f sec." % (etime - stime))
    return res

def bfs(G, source, depth_limit=None, opt_level=1, hyb_threshold=0.4, \
        verbose=0):
    """
    DESC:   Computes bfs traversal path of a graph
    PARAM:  G: frovedis/networkx graph object 
            source: int (1 ~ G.num_vertices) 
            opt_level: int (0, 1 or 2) (default: 1)
            hyb_threshold: double (0.0 ~ 1.0) (default: 0.4)
            depth_limit: long (default: long max)
    RETURN: A dictionary with keys as destination node-id 
            and values as correspondingb traversal path from the source.
            In case any node in input graph not reachable from the source,
            it would not be included in the resultant dictionary.
    """
    bfsres = bfs_impl(G, source, depth_limit=depth_limit, 
                      opt_level=opt_level, hyb_threshold=hyb_threshold,
                      verbose=verbose)
    stime = time.time()
    ret = generate_traversal_path(bfsres, source)
    etime = time.time()
    if verbose:
        print("bfs res conversion time: %.3f sec." % (etime - stime))
    return ret

def descendants_at_distance(G, source, distance, opt_level=1, \
                            hyb_threshold=0.4, verbose=0):
    """
     --- wrapper for networkx descendants_at_distance ---
    """
    G, inp_movable, depth = validate_bfs(G, source, distance)
    (host, port) = FrovedisServer.getServerInstance()
    stime = time.time()
    res = rpclib.bfs_descendants_at_distance(host, port, G.get(), source, \
                                             opt_level, hyb_threshold, \
                                             distance)
    excpt = rpclib.check_server_exception()
    if excpt["status"]:
        raise RuntimeError(excpt["info"])
    if inp_movable:
        G.release()
    etime = time.time()
    if verbose: 
        print("bfs descendants_at_distance computation time: %.3f " + \
              "sec." % (etime - stime))
    return set(res)

def bfs_edges(G, source, reverse=False, depth_limit=None, sort_neighbors=None,
              opt_level=1, hyb_threshold=0.4, verbose=0):
    """
     --- wrapper for networkx bfs_edges ---
    """
    # TODO: confirm functionality of reverse and sort_neighbors 
    if reverse:
        raise NotImplementedError("reverse = True is currently " + \
                                  "not supported!")
    if sort_neighbors is not None:
        raise NotImplementedError("sort_neighbors is currently " + \
                                  "not supported!")
    bfsres = bfs_impl(G, source, depth_limit=depth_limit, 
                      opt_level=opt_level, hyb_threshold=hyb_threshold, 
                      verbose=verbose)
    dest = bfsres["destids"]
    pred = bfsres["predecessors"]
    for i in range(1, len(dest)): #starting from 1 (ignoring self-edge)
        yield (pred[i], dest[i])

def bfs_tree(G, source, reverse=False, depth_limit=None, sort_neighbors=None,
             opt_level=1, hyb_threshold=0.4, verbose=0):
    """
     --- wrapper for networkx bfs_tree ---
    """
    ret = nx.DiGraph()
    ret.add_node(source)
    edges = bfs_edges(G, source, reverse=reverse, 
                      depth_limit=depth_limit,
                      sort_neighbors=sort_neighbors,
                      opt_level=opt_level, hyb_threshold=hyb_threshold,
                      verbose=verbose)
    ret.add_edges_from(edges)
    return ret

def bfs_predecessors(G, source, depth_limit=None, sort_neighbors=None,
                     opt_level=1, hyb_threshold=0.4, verbose=0):
    """
     --- wrapper for networkx bfs_predecessors ---
    """
    for src, dst in bfs_edges(
        G, source, depth_limit=depth_limit, sort_neighbors=sort_neighbors,
        opt_level=opt_level, hyb_threshold=hyb_threshold, verbose=verbose):
        yield (dst, src)

def bfs_successors(G, source, depth_limit=None, sort_neighbors=None,
                   opt_level=1, hyb_threshold=0.4, verbose=0):
    """
     --- wrapper for networkx bfs_successors ---
    """
    parent = source
    children = []
    for p, c in bfs_edges(
        G, source, depth_limit=depth_limit, sort_neighbors=sort_neighbors,
        opt_level=opt_level, hyb_threshold=hyb_threshold, verbose=verbose):
        if p == parent:
            children.append(c)
            continue
        yield (parent, children)
        children = [c]
        parent = p
    yield (parent, children)    

def single_source_shortest_path(G, source, return_distance=False, verbose=0):
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
    G, inp_movable = validate_traversal(G, source)
    (host, port) = FrovedisServer.getServerInstance()
    stime = time.time()
    res = rpclib.call_frovedis_sssp(host, port, G.get(), source)
    excpt = rpclib.check_server_exception()
    if excpt["status"]:
        raise RuntimeError(excpt["info"])
    if inp_movable:
        G.release()
    etime = time.time()
    if verbose:
        print("sssp computation time: %.3f sec." % (etime - stime))

    stime = time.time()
    x = generate_traversal_path(res, source)
    if return_distance:
        destid = res['destids']
        dist = res['distances']
        y = dict(zip(destid, dist))
        ret = (x, y)
    else:
        ret = x
    etime = time.time()
    if verbose:
        print("sssp res conversion time: %.3f sec." % (etime - stime))
    return ret

