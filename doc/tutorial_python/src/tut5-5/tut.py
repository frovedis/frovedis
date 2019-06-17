import os
import numpy as np
from frovedis.exrpc.server import FrovedisServer
from frovedis.matrix.dense import FrovedisBlockcyclicMatrix
from frovedis.matrix.wrapper import SCALAPACK

FrovedisServer.initialize("mpirun -np 2 {}".format(os.environ['FROVEDIS_SERVER']))

# Loading Frovedis server side blockcyclic matrix from text file
bcm = FrovedisBlockcyclicMatrix(dtype=np.float64)
print("loading blockcyclic matrix from text file: ./input")
bcm.load("./input")
m = bcm.to_numpy_matrix()
print (m)

# --- getrf (LU factorization) ---
rf = SCALAPACK.getrf(bcm)

# --- getri (matrix inversion using LU factor) ---
SCALAPACK.getri(bcm,rf.ipiv())
print("inverse of the matrix")
print (bcm.to_numpy_matrix())

# Releasing IPIV vector and LU factored matrix from Frovedis server
rf.release()
bcm.release()

# use the same matrix
bcm = FrovedisBlockcyclicMatrix(m)
x = np.matrix([[1],[2],[3]], dtype=np.float64)
bcx = FrovedisBlockcyclicMatrix(x)

# --- gesv (solve Ax = b) ---
print("solve Ax = b")
SCALAPACK.gesv(bcm,bcx)
print (bcx.to_numpy_matrix())

# use the same data
bcm = FrovedisBlockcyclicMatrix(m)
svd = SCALAPACK.gesvd(bcm)

# getting the result back at pythob side
print("printing the results (umat, svec, vmat)")
(umat,svec,vmat) = svd.to_numpy_results()
print (umat)
print (svec)
print (vmat)

# Releasing input matrix and resultant matrices
bcm.release()
svd.release()

# Shutting down the Frovedis server
FrovedisServer.shut_down()

