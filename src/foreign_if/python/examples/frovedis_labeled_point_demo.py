#!/usr/bin/env python

import sys
import numpy as np
from scipy.sparse import csr_matrix
from frovedis.matrix.ml_data import FrovedisLabeledPoint
from frovedis.exrpc.server import FrovedisServer

# initializing the Frovedis server
argvs = sys.argv
argc = len(argvs)
if (argc < 2):
    print ('Please give frovedis_server calling command as the first argument \n(e.g. "mpirun -np 2 -x /opt/nec/nosupport/frovedis/ve/bin/frovedis_server")')
    quit()
FrovedisServer.initialize(argvs[1])

# --- sparse data --- 
data = np.array([1, 2, 3, 4, 5, 6])
indices = np.array([0, 2, 2, 0, 1, 2])
indptr = np.array([0, 2, 3, 6])
X = csr_matrix((data, indices, indptr),
               dtype=np.float64,
               shape=(3, 3))

y = [1, 2, 3] # label
#data = FrovedisLabeledPoint(X,y) # abnormal case (type mismatch)

y = np.asarray([1, 2],dtype=np.float64) # label
#data = FrovedisLabeledPoint(X,y) # abnormal case (size mismatch)

y = np.asarray([1, 2, 3],dtype=np.float64) # label
data = FrovedisLabeledPoint(X,y) # okay sparse case
data.debug_print()
data.is_dense()

# --- dense data ---
X = np.asmatrix([[1,0,2], [0,0,3], [4,5,6]], dtype=np.float64)
data = FrovedisLabeledPoint(X,y) # okay dense case

# --- FrovedisLabeledPoint functionalities ---
data.debug_print()
data.numRows()
data.numCols()
data.get()
data.get_dtype()
data.get_itype()
data.is_dense()

data.release()
data.debug_print() # no display, data has been released

