"""GraphLoader.py"""

import numpy as np
from scipy.sparse import coo_matrix
import networkx as nx
from .graph import Graph

def custom_read_edgelist(path, comments='#', delimiter=' ', \
                         create_using=None, \
                         nodetype=np.int64, data=True, \
                         edgetype=np.int64, encoding='utf-8'):
    """
    DESC: Customized read_edgelist() to construct graph adjacency matrix
          in the form of scipy csr matrix directly from input file.
    PARAMS:    Same as in networkx.read_edgelist().
               nodetype, data, edgetype, encoding are not used.
    """
    # checking number of columns in input file
    import csv
    fstr = open(path, 'r')
    reader = csv.reader(fstr, delimiter=delimiter)
    sample = next(reader)
    while(sample[0][0] == comments): #skipping comments
      sample = next(reader)
    fstr.close()

    ncol = len(sample)
    if ncol == 2:
      names = ['src', 'dst']
    elif ncol == 3:
      names = ['src', 'dst', 'wgt']
    else: 
      msg = "read_edgelist: Expected 2 or 3 columns in input file!\n"
      msg = msg + str(ncol) + " column detected in first row: " + str(sample)
      msg = msg + "\nPlease ensure if the specified delimiter '"
      msg = msg + delimiter + "' is correct!"
      raise ValueError(msg)

    # loading data by excluding duplicate rows
    import pandas as pd
    df = pd.read_csv(path, sep=delimiter, comment=comments, \
                     names = names, dtype = edgetype)

    # converting to numpy array
    mat = df.drop_duplicates().values
    num_edges = mat.shape[0]

    # checking whether data is 0-based or 1-based
    tarr = mat[:, :2].flatten()
    import sys
    if sys.version_info[0] < 3:
      min_id = long(tarr.min())
      max_id = long(tarr.max())
    else:
      min_id = int(tarr.min())
      max_id = int(tarr.max())

    if min_id == 0:
      rowid = mat[:, 0]
      colid = mat[:, 1]
      num_vertices = max_id + 1
    else:
      rowid = mat[:, 0] - 1
      colid = mat[:, 1] - 1
      num_vertices = max_id

    # extracting edge weight information (if available)
    if ncol == 3: 
      data = mat[:, 2]
    else:
      data = np.ones(num_edges)

    # constructing sparse matrix structure
    data = np.asarray(data, dtype = edgetype)
    rowid = np.asarray(rowid, dtype = nodetype)
    colid = np.asarray(colid, dtype = nodetype)
    shape = (num_vertices, num_vertices)
    if (isinstance(create_using, nx.classes.digraph.DiGraph)):
        coo = coo_matrix((data, (rowid, colid)), shape=shape)
    else:
        data_ = np.concatenate((data, data))
        rowid_ = np.concatenate((rowid, colid))
        colid_ = np.concatenate((colid, rowid))
        coo = coo_matrix((data_, (rowid_, colid_)), shape=shape)
    return coo.tocsr()

def read_edgelist(path, comments='#', delimiter=' ', \
                  create_using=None,\
                  nodetype=np.int64, data=True, \
                  edgetype=np.int64, encoding='utf-8'):
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
    #                            nodetype, data, edgetype, encoding)
    #return Graph(nx_graph=nx_graph)
    smat = custom_read_edgelist(path, comments, delimiter, create_using, \
                                nodetype, data, edgetype, encoding)
    return Graph(nx_graph=smat)

