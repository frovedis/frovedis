#!/usr/bin/env python

import numpy as np
from frovedis.exrpc.server import *
from frovedis.matrix.dense import FrovedisBlockcyclicMatrix
from frovedis.matrix.wrapper import PBLAS
import sys

# initializing the Frovedis server
argvs = sys.argv
argc = len(argvs)
if (argc < 2):
    print ('Please give frovedis_server calling command as the first argument \n(e.g. "mpirun -np 2 -x /opt/nec/nosupport/frovedis/ve/bin/frovedis_server")')
    quit()
FrovedisServer.initialize(argvs[1])

# sample numpy matrices creation
m1 = np.matrix([[1],[2],[3],[4]], dtype=np.float64) # 4x1
m2 = np.matrix([[5],[6],[7],[8]], dtype=np.float64) # 4x1
m3 = np.matrix([[1,0,0,0],[0,1,0,0],[0,0,1,0],[0,0,0,1]],
               dtype=np.float64) # 4x4: eye(I)
m4 = np.matrix([[1,2,3,4],[5,6,7,8],[8,7,6,5],[4,3,2,1]],
               dtype=np.float64) # 4x4

# Creating Frovedis server side blockcyclic matrics from numpy matrices
bcm1 = FrovedisBlockcyclicMatrix(m1) # blockcyclic vector (x)
bcm2 = FrovedisBlockcyclicMatrix(m2) # blockcyclic vector (y)
bcm3 = FrovedisBlockcyclicMatrix(m3) # blockcyclic matrix (m)
bcm4 = FrovedisBlockcyclicMatrix(m4) # blockcyclic matrix (n)

# --- swap demo ---
PBLAS.swap(bcm1,bcm2)
print ("after swap (x <=> y):")
bcm1.get_rowmajor_view()
bcm2.get_rowmajor_view()

# --- scal demo ---
PBLAS.scal(bcm1,2)
print ("after scal (x = 2x):")
bcm1.get_rowmajor_view()

# --- axpy demo ---
PBLAS.axpy(bcm1,bcm2,2)
print ("after axpy (y = 2x + y):")
bcm1.get_rowmajor_view()
bcm2.get_rowmajor_view()

# --- copy demo ---
PBLAS.copy(bcm1,bcm2)
print ("after copy (y = x):")
bcm1.get_rowmajor_view()
bcm2.get_rowmajor_view()

# --- dot demo ---
r1 = PBLAS.dot(bcm1,bcm2) # dot product on transformed blockcyclic vectors
print ("x.y = " + str(r1))
# short-cut version (blockcyclic matrix creation and deletion will happen automatically)
r2 = PBLAS.dot(m1,m2) # dot product on original numpy data
print ("x.y = " + str(r2))

# --- nrm2 demo ---
r = PBLAS.nrm2(bcm1)
print ("norm(x) = " + str(r))
r = PBLAS.nrm2(bcm2)
print ("norm(y) = " + str(r))

# --- gemv (matrix-vector multiplication) demo ---
print ("m*x: ")
gemv_ret = PBLAS.gemv(bcm3,bcm1)
gemv_ret.get_rowmajor_view()
gemv_ret.release()

# --- ger (vector-vector multiplication) demo ---
print ("xT*y: ")
ger_ret = PBLAS.ger(bcm1,bcm2) # (4x4) = (4x1) * (1x4)
ger_ret.get_rowmajor_view()
ger_ret.release()

# --- gemm (matrix-matrix multiplication) demo ---
print ("m*n: ")
gemm_ret = PBLAS.gemm(bcm3,bcm4)
gemm_ret.get_rowmajor_view()
gemm_ret.release()

# --- geadd (matrix-matrix addition) demo ---
print ("n = m + n: ")
PBLAS.geadd(bcm3,bcm4)
bcm4.get_rowmajor_view()

# Releasing Frovedis side blockcyclic matrices
bcm1.release()
bcm2.release()
bcm3.release()
bcm4.release()

# Shutting down the Frovedis server
FrovedisServer.shut_down()

