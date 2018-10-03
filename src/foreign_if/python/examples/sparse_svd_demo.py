#!/usr/bin/env python

import numpy as np
from scipy.sparse import csr_matrix
from frovedis.exrpc.server import *
from frovedis.matrix.crs import FrovedisCRSMatrix 
from frovedis.matrix.wrapper import ARPACK
#from python.matrix.results import GesvdResult
import sys

# initializing the Frovedis server
argvs = sys.argv
argc = len(argvs)
if (argc < 2):
    print ('Please give frovedis_server calling command as the first argument \n(e.g. "mpirun -np 2 -x /opt/nec/nosupport/frovedis/ve/bin/frovedis_server")')
    quit()
FrovedisServer.initialize(argvs[1])

# sample 3x3 scipy csr matrix creation
data = np.array([1, 2, 3, 4, 5, 6])
indices = np.array([0, 2, 2, 0, 1, 2])
indptr = np.array([0, 2, 3, 6])
mat = csr_matrix((data, indices, indptr),
                 dtype=np.float64,
                 shape=(3, 3))

# Calling Arpack routine to compute SVD on (scipy) sparse matrix
svd = ARPACK.computeSVD(mat,2) # short-cut
# Viewing gesvd results (for debugging)
svd.debug_print()

# Creating Frovedis server side CRS matrix from scipy data
fmat = FrovedisCRSMatrix(mat)
# Calling Arpack routine to compute SVD on Frovedis CRS  matrix
svd = ARPACK.computeSVD(fmat,2)
# Viewing gesvd results (for debugging)
svd.debug_print()

# releasing matrix and results
fmat.release()
svd.release()

# Shutting down the Frovedis server
FrovedisServer.shut_down()

