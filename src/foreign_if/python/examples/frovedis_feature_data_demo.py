#!/usr/bin/env python

import sys
import numpy as np
np.set_printoptions(threshold=5)

from scipy.sparse import csr_matrix
from frovedis.matrix.ml_data import FrovedisFeatureData

# initializing the Frovedis server
argvs = sys.argv
argc = len(argvs)
if (argc < 2):
    print ('Please give frovedis_server calling command as the first argument \n(e.g. "mpirun -np 2 -x /opt/nec/nosupport/frovedis/ve/bin/frovedis_server")')
    quit()

from frovedis.exrpc.server import FrovedisServer
FrovedisServer.initialize(argvs[1])

# --- sparse data --- 
data = np.array([1, 2, 3, 4, 5, 6])
indices = np.array([0, 2, 2, 0, 1, 2])
indptr = np.array([0, 2, 3, 6])
X = csr_matrix((data, indices, indptr),
               dtype=np.float64,
               shape=(3, 3))

mat = np.matrix([[0, 0, 0, 0],
                 [0, 1, 1, 1],
                 [1, 0, 1, 0],
                 [1, 1, 1, 0],
                 [1, 1, 1, 1]])
mat = csr_matrix(mat)

data = FrovedisFeatureData(mat)
data.debug_print()
data.get().debug_print()
print(data.is_dense())

data = FrovedisFeatureData(mat, dense_kind='rowmajor', densify=True) # idensify sparse data
data.debug_print()
data.get().debug_print()
print(data.is_dense())

data.release()
data.debug_print() # no display, data has been released
FrovedisServer.shut_down()
