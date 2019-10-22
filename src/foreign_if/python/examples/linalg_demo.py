#!/usr/bin/env python

import numpy as np
from frovedis.exrpc.server import FrovedisServer
from frovedis.matrix.dense import FrovedisBlockcyclicMatrix
import sys

# initializing the Frovedis server
argvs = sys.argv
argc = len(argvs)
if (argc < 2):
    print ('Please give frovedis_server calling command as the first argument \n(e.g. "mpirun -np 2 -x /opt/nec/nosupport/frovedis/ve/bin/frovedis_server")')
    quit()
FrovedisServer.initialize(argvs[1])

mat = np.asmatrix([[2,1,1], [3,2,1], [2,1,2]], dtype=np.float64)
bmat = FrovedisBlockcyclicMatrix(mat)

print("input matrix:")
print(mat)

print("--- demo of frovedis wrapper with input as numpy matrix ---")
from frovedis.linalg import *
frov_rf = dgetrf(mat)
print("lu factor: ")
print(frov_rf[0])
frov_ri = dgetri(frov_rf[0], frov_rf[1])
print("inverse: ")
print(frov_ri[0])

print("--- demo of frovedis wrapper with input as FrovedisBlockcyclicMatrix ---")
frov_rf = dgetrf(bmat)
print("lu factor: ")
print(frov_rf[0].to_numpy_matrix())
frov_ri = dgetri(frov_rf[0], frov_rf[1])
print("inverse: ")
print(frov_ri[0].to_numpy_matrix())

print("--- demo of native scipy.linalg routines ---")
from scipy.linalg import lapack
lpck_rf = lapack.dgetrf(mat)
print("lu factor: ")
print(lpck_rf[0])
lpck_ri = lapack.dgetri(lpck_rf[0], lpck_rf[1])
print("inverse: ")
print(lpck_ri[0])

