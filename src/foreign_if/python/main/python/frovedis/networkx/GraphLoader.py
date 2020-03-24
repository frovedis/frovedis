"""GraphLoader.py"""

import numpy as np
from scipy.sparse import coo_matrix
import networkx as nx
from .graph import Graph

def read_edgelist(path, comments='#', delimiter=None, create_using=None,\
        nodetype=None, data=True, edgetype=None, encoding='utf-8'):
    """
    DESC: Reads edgelist data from persistent storage.
    PARAMS:    path : file or string
                      File or filename to read. If a file is
                      provided, it must be opened in 'rb' mode. Filenames
                      ending in .gz or .bz2 will be uncompressed.
               comments : string, optional
                      The character used to indicate the start of a comment.
               delimiter : string, optional
                      The string used to separate values.  The default is
                      whitespace.
               create_using : NetworkX graph constructor, optional
                              (default=nx.Graph)
                      Graph type to create. If graph instance, then cleared
                      before populated.
               nodetype : int, float, str, Python type, optional
                      Convert node data from strings to specified type
               data : bool or list of (label,type) tuples
                      Tuples specifying dictionary key names and types for
                      edge data
               edgetype : int, float, str, Python type, optional OBSOLETE
                      Convert edge data from strings to specified type and
                      use as 'weight'
               encoding: string, optional
                      Specify which encoding to use when reading file.
    """
    #nx_graph = nx.read_edgelist(path, comments, delimiter, create_using, \
    #                nodetype, data, edgetype, encoding)
    #return Graph(nx_graph=nx_graph)
    mat = np.loadtxt(fname=path, comments=comments, \
                     delimiter=delimiter, dtype=np.int64)
    rowid = mat[:, 0] - 1
    colid = mat[:, 1] - 1
    maxid = max(rowid.max(), colid.max())
    num_vertices = maxid + 1
    num_edges = mat.shape[0]
    shape = (num_vertices, num_vertices)
    if (isinstance(create_using, nx.classes.digraph.DiGraph)):
        data = np.ones(num_edges)
        coo = coo_matrix((data, (rowid, colid)), shape=shape)
    else:
        data = np.ones(num_edges*2)
        rowid_ = np.concatenate((rowid, colid))
        colid_ = np.concatenate((colid, rowid))
        coo = coo_matrix((data, (rowid_, colid_)), shape=shape)
    smat = coo.tocsr()
    return Graph(smat)

