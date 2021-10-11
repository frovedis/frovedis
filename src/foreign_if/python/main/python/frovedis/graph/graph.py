"""
Graph class
"""

import os
import numpy as np
import networkx as nx
from scipy.sparse import issparse, csr_matrix

from ..exrpc import rpclib
from ..exrpc.server import FrovedisServer
from ..matrix.crs import FrovedisCRSMatrix

# re-implementation of networkx.to_scipy_sparse_matrix()
# TODO: result confirmation
def to_scipy_sparse_matrix(nx_graph, format='csr'):
    '''
    # rows are assumed to be in sorted order 1, 2, ..., num_vertices
    # hence extraction is not required
    stime = time.time()
    rows = nx_graph.adj.keys()
    print("rows: {0}".format(time.time() - stime) + " [sec]")
    '''

    #stime = time.time()
    edges = [list(i.keys()) for i in nx_graph.adj.values()]
    #print("edges: {0}".format(time.time() - stime) + " [sec]")

    #stime = time.time()
    num_edges = sum([len(i) for i in edges])
    num_vertices = len(nx_graph.adj)
    data = np.ones(num_edges) # assumes, graph weight = 1.0
    indices = np.zeros(num_edges)
    indptr = np.zeros(num_vertices + 1)
    #print("allocation: {0}".format(time.time() - stime) + " [sec]")

    #stime = time.time()
    ctr = 0
    for i in range(len(edges)): #consider-using-enumerate
        vec = edges[i]
        for j in range(len(vec)): #consider-using-enumerate
            indices[ctr + j] = vec[j] - 1
        ctr = ctr + len(vec)
        indptr[i + 1] = ctr
    #print("flatten indices: {0}".format(time.time() - stime) + " [sec]")

    '''
    # not required, assuming frovedis graph edges have weight = 1.0 always
    stime = time.time()
    d = [i.values() for i in nx_graph.adj.values()]
    print("data: {0}".format(time.time() - stime) + " [sec]")

    stime = time.time()
    ctr = 0
    for i in range(len(d)):
        vec = d[i]
        for j in range(len(vec)):
            try:
                data[ctr + j] = vec[j]['weight']
            except KeyError:
                data[ctr + j] = 1.0
        ctr = ctr + len(vec)
    print("flatten data: {0}".format(time.time() - stime) + " [sec]")
    '''

    return csr_matrix((data, indices, indptr), dtype=np.float64, \
                      shape=(num_vertices, num_vertices))

class Graph(object):
    """
    Graph class for frovedis
    """
    def __init__(self, nx_graph=None): #TODO: Update name of nx_graph
        """
        DESC: Graph constructor
        PARAM: nx_graph
        """
        self.fdata = None
        self.num_edges = None
        self.num_vertices = None
        if issparse(nx_graph):  # any sparse matrix
            mat = nx_graph.tocsr()
            self.load_csr(mat)
        elif isinstance(nx_graph, nx.classes.graph.Graph):
            self.load(nx_graph)
        elif nx_graph is not None:
            raise ValueError("Graph: Supported types are networkx graph or scipy sparse matrices!")

    def load(self, nx_graph):
        """
        DESC: load a networkx graph to create a frovedis graph
        PARAM: nx_graph
        RETURN: self
        """
        self.release()
        self.num_edges = nx_graph.number_of_edges()
        self.num_vertices = nx_graph.number_of_nodes()
        #import time
        #t1 = time.time()
        #TODO: use reimplemented version after result correctness
        order = sorted(list(nx_graph.nodes()))
        nx_smat = nx.to_scipy_sparse_matrix(nx_graph, format='csr', nodelist=order)
        #print("Graph.py -> nx.to_scipy_sparse_matrix: ", time.time() - t1)
        # by default, edge data is loaded as float64
        # and node data is loaded as int64
        #TODO: support loading data as same dtype/itype in input nx-graph
        smat = FrovedisCRSMatrix(mat=nx_smat, dtype=np.float64, itype=np.int64)
        (host, port) = FrovedisServer.getServerInstance()
        self.fdata = rpclib.set_graph_data(host, port, smat.get())
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        return self

    def load_csr(self, smat):
        """
        DESC: loads Frovedis graph from a scipy csr_matrix
        PARAM: any sparse matrix
        RETURN: self
        """
        self.release()
        self.num_edges = len(smat.data)
        self.num_vertices = smat.shape[0]
        # by default, edge data is loaded as float64
        # and node data is loaded as int64
        #TODO: support loading data as same dtype/itype in input matrix
        fsmat = FrovedisCRSMatrix(mat=smat, dtype=np.float64, itype=np.int64)
        (host, port) = FrovedisServer.getServerInstance()
        self.fdata = rpclib.set_graph_data(host, port, fsmat.get())
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        return self

    def debug_print(self):
        """
        DESC: Print number of edges and vertices in graph
        PARAM: None
        RETURN: None
        """
        if self.fdata != None: #Consider using 'is not'
            print("Num of edges: ", self.num_edges)
            print("Num of vertices: ", self.num_vertices)
            (host, port) = FrovedisServer.getServerInstance()
            rpclib.show_graph_py(host, port, self.get())
            excpt = rpclib.check_server_exception()
            if excpt["status"]:
                raise RuntimeError(excpt["info"])

    def release(self):
        """
        DESC: Release the graph resources
        PARAM: None
        RETURN: None
        """
        if self.fdata != None: #Consider using 'is not'
            (host, port) = FrovedisServer.getServerInstance()
            rpclib.release_graph_py(host, port, self.get())
            excpt = rpclib.check_server_exception()
            if excpt["status"]:
                raise RuntimeError(excpt["info"])
            self.fdata = None
            self.num_edges = None
            self.num_vertices = None

    def clear(self):
        """
        DESC: Wrapper for release
        PARAM: None
        RETURN: None
        """
        self.release()

    def number_of_edges(self):
        """
        DESC: Returns number of edges
        PARAM: None
        RETURN: Long
        """
        return self.num_edges

    def number_of_nodes(self):
        """
        DESC: Returns number of nodes
        PARAM: None
        RETURN: Long
        """
        return self.num_vertices

    def save(self, fname):
        """
        DESC: Saves graph to persistent storage.
        PARAM: string-> file path
        RETURN: None
        """
        if self.fdata != None: #Consider using 'is not'
            if os.path.exists(fname):
                raise ValueError(\
                    "another graph object with %s name already exists!" % fname)
            (host, port) = FrovedisServer.getServerInstance()
            rpclib.save_graph_py(host, port, self.get(), fname.encode('ascii'))
            excpt = rpclib.check_server_exception()
            if excpt["status"]:
                raise RuntimeError(excpt["info"])

    def load_text(self, fname):
        """
        DESC: Loads graph from persistent storage.
        PARAM: string-> file path
        RETURN: None
        """
        if not os.path.exists(fname):
            raise ValueError(\
                "the graph object with name %s does not exist!" % fname)
        self.release()
        (host, port) = FrovedisServer.getServerInstance()
        dummy_graph = \
            rpclib.load_graph_from_text_file(host, port, fname.encode('ascii'))
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        self.fdata = dummy_graph['dptr']
        self.num_edges = dummy_graph['nEdges']
        self.num_vertices = dummy_graph['nNodes']
        return self

    def to_networkx_graph(self):
        """
        DESC: Convert from frovedis graph to networkx graph.
        PARAM: None
        Return: Networkx graph
        """
        (host, port) = FrovedisServer.getServerInstance()
        dmat = \
            rpclib.get_graph_data(host, port, self.get())
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        # TODO: support other types
        fmat = FrovedisCRSMatrix(dmat, dtype=np.float64, itype=np.int64)
        smat = fmat.to_scipy_matrix()
        return nx.from_scipy_sparse_matrix(smat)

    def copy(self):
        """
        DESC: Create a copy of graph data(used in pagerank)
        PARAM: None
        RETURN: Frovedis graph
        """
        (host, port) = FrovedisServer.getServerInstance()
        gptr = \
            rpclib.copy_graph_py(host, port, self.get())
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        c_graph = Graph()
        c_graph.fdata = gptr
        c_graph.num_edges = self.num_edges
        c_graph.num_vertices = self.num_vertices
        return c_graph

    def get(self):
        """
        DESC: Fetches fdata
        PARAM: None
        Return: Long
        """
        return self.fdata

    def __del__(self):
        if FrovedisServer.isUP():
            self.release()
