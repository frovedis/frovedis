#!/usr/bin/env python

import numpy as np
from scipy.sparse import csr_matrix
from frovedis.exrpc.server import *
from frovedis.matrix.crs import FrovedisCRSMatrix 
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

# Creating Frovedis server side crs matrix from scipy data
# "mat" can be any sparse matrix or array-like python object
fmat = FrovedisCRSMatrix(mat)

# Viewing the created matrix (for debugging)
fmat.debug_print()

# Saving the created matrix 
fmat.save("./out/crs_3x3")

# asCRS demo 
asmat = FrovedisCRSMatrix.asCRS(fmat) # no constructor (returns self) + no destructor
asmat.debug_print()
asmat = FrovedisCRSMatrix.asCRS(mat)  # constructor (creates and returns) + no destructor
asmat.debug_print()
asmat = FrovedisCRSMatrix.asCRS(mat)  # constructor (creates and returns) + destructor for above object
asmat.debug_print()

# Releasing the matrix from Frovedis server
fmat.release()
asmat.release()

# Shutting down the Frovedis server
FrovedisServer.shut_down()

