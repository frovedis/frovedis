import os
import numpy as np
from frovedis.exrpc.server import FrovedisServer
from frovedis.matrix.dense import FrovedisBlockcyclicMatrix
from frovedis.matrix.wrapper import PBLAS

FrovedisServer.initialize("mpirun -np 2 {}".format(os.environ['FROVEDIS_SERVER']))

# numpy matrices creation
x = np.matrix([[1],[2],[3],[4]], dtype=np.float64) # 4x1
y = np.matrix([[5],[6],[7],[8]], dtype=np.float64) # 4x1
m = np.matrix([[1,0,0,0],[0,1,0,0],[0,0,1,0],[0,0,0,1]],
               dtype=np.float64) # 4x4: eye(I)
n = np.matrix([[1,2,3,4],[5,6,7,8],[8,7,6,5],[4,3,2,1]],
               dtype=np.float64) # 4x4

# Creating Frovedis server side blockcyclic matrics from numpy matrices
bcx = FrovedisBlockcyclicMatrix(x) # blockcyclic vector (x)
bcy = FrovedisBlockcyclicMatrix(y) # blockcyclic vector (y)
bcm = FrovedisBlockcyclicMatrix(m) # blockcyclic matrix (m)
bcn = FrovedisBlockcyclicMatrix(n) # blockcyclic matrix (n)

# --- print original data
print ("x:")
print (x)
print ("y:")
print (y)
print ("m:")
print (m)
print ("m:")
print (n)

# --- swap ---
PBLAS.swap(bcx,bcy)
print ("after swap (x <=> y):")
print ("x = ")
bcx.debug_print()
print (bcx.to_numpy_matrix())
print ("y = ")
print (bcy.to_numpy_matrix())

# --- scal ---
PBLAS.scal(bcx,2)
print ("after scal (x = 2x), x = ")
print (bcx.to_numpy_matrix())

# --- axpy ---
PBLAS.axpy(bcx,bcy,2)
print ("after axpy (y = 2x + y), y = ")
print (bcy.to_numpy_matrix())

# --- copy ---
PBLAS.copy(bcx,bcy)
print ("after copy (y = x), y = ")
print (bcy.to_numpy_matrix())

# --- dot ---
r1 = PBLAS.dot(bcx,bcy) # dot product on transformed blockcyclic vectors
# this is also OK; creates blockcyclic matrix internally
# r1 = PBLAS.dot(x,y) 
print ("x.y = " + str(r1))

# --- nrm2 ---
r = PBLAS.nrm2(bcx)
print ("norm(x) = " + str(r))

# --- gemv (matrix-vector multiplication) ---
print ("m*x =  ")
gemv_ret = PBLAS.gemv(bcm,bcx)
print (gemv_ret.to_numpy_matrix())
gemv_ret.release()

# --- ger (vector-vector multiplication) ---
print ("x*yT =  ")
ger_ret = PBLAS.ger(bcx,bcy) # (4x4) = (4x1) * (1x4)
print (ger_ret.to_numpy_matrix())
ger_ret.release()

# --- gemm (matrix-matrix multiplication) ---
print ("m*n =  ")
gemm_ret = PBLAS.gemm(bcm,bcn)
print (gemm_ret.to_numpy_matrix())
gemm_ret.release()

# --- geadd (matrix-matrix addition) ---
print ("n = m + n")
PBLAS.geadd(bcm,bcn)
print (bcn.to_numpy_matrix())

# Releasing Frovedis side blockcyclic matrices
bcx.release()
bcy.release()
bcm.release()
bcn.release()

# Shutting down the Frovedis server
FrovedisServer.shut_down()
