#!/usr/bin/env python

# This test code only covers FrovedisBlockcyclicMatrix APIs.
# Other two matrices (FrovedisRowmajorMatrix, FrovedisColmajorMatrix)
# also have similar APIs (except rich set of overloaded operators)

import numpy as np
from frovedis.exrpc.server import *
from frovedis.matrix.dense import *
import sys

# initializing the Frovedis server
argvs = sys.argv
argc = len(argvs)
if (argc < 2):
    print 'Please give frovedis_server calling command as the first argument \n(e.g. "mpirun -np 2 -x /opt/nec/nosupport/frovedis/ve/bin/frovedis_server")'
    quit()
FrovedisServer.initialize(argvs[1])

# sample 4x4 numpy matrix creation
mat = np.matrix([[1,2,3,4],[5,6,7,8],[8,7,6,5],[4,3,2,1]],
                 dtype=np.float64)

# Creating Frovedis server side blockcyclic matrix from numpy matrix
print("creating blockcyclic matrix from numpy matrix: ")
fmat = FrovedisBlockcyclicMatrix(mat) # 'mat' can be any array-like object or file name
fmat.debug_print() #view for debugging

# Saving the matrix in text format
print("saving matrix in text file: ./out/numpy_4x4")
fmat.save("./out/numpy_4x4")

# Loading the saved text file
print("loading matrix from text file: ./out/numpy_4x4")
fmat.load("./out/numpy_4x4")

# Viewing the loaded matrix
print("printing rowmajor view of the loaded matrix:")
fmat.get_rowmajor_view()

# getting back the numpy matrix
mat2 = fmat.to_numpy_matrix()
print ("Frovedis Blockcyclic Matrix => Numpy 2D Matrix")
print mat2

# Deepcopying matrix
cmat = FrovedisBlockcyclicMatrix(fmat) # copy constructor
print("deepcopied matrix view: ") # cmat|fmat are two different objects having same values
fmat.get_rowmajor_view()
cmat.get_rowmajor_view()
print("releasing deepcopied matrix: ")
cmat.release()  
print("printing source matrix after releasing copied matrix to check deep copy effect: ")
cmat.get_rowmajor_view() # No view, since cmat is released
fmat.get_rowmajor_view() # cmat is released | fmat is still there in memory

# Verifying corner cases: Operation on empty (non-existent) matrix
dmat = FrovedisBlockcyclicMatrix() #empty matrix - no matrix is created on Frovedis server
copy_dmat = FrovedisBlockcyclicMatrix(dmat) #copying empty matrix (no effect)
dmat.debug_print() #no effect
dmat.save("./out/empty") #no effect
dmat.release() #no effect
#dmat.to_frovedis_rowmatrix() #exception: Empty matrix.
#dmat.transpose() #exception: Empty input matrix.

# Checking invalid copy
rmat = FrovedisRowmajorMatrix() #empty rowmajor matrix
#bmat = FrovedisBlockcyclicMatrix(rmat) #exception: conversion not possible (Incompatible type)

# Adding with self
print("m + m: ")
rmat = fmat + fmat
rmat.get_rowmajor_view()

# Subtracting from self
print("m - m: ")
rmat = fmat - fmat
rmat.get_rowmajor_view()

# Multiplyig with self
print("m * m: ")
rmat = fmat * fmat
rmat.get_rowmajor_view()

# Transposing the loaded matrix
print("trans(m): ")
rmat = ~fmat # fmat.transpose()
rmat.get_rowmajor_view()
rmat.release()

# Saving the loaded matrix in binary format
print("saving matrix in binary file: ./out/numpy_4x4_bin")
fmat.save_binary("./out/numpy_4x4_bin")

# Loading the saved binary file
print("loading matrix from  binary file: ./out/numpy_4x4_bin")
fmat.load_binary("./out/numpy_4x4_bin")

# Viewing the loaded matrix 
print("printing rowmajor view of the loaded matrix:")
fmat.get_rowmajor_view()

# asBCM demo 
asmat = FrovedisBlockcyclicMatrix.asBCM(fmat) # no constructor (returns self) + no destructor
asmat.debug_print()
asmat = FrovedisBlockcyclicMatrix.asBCM(mat)  # constructor (creates and returns) + no destructor
asmat.debug_print()
asmat = FrovedisBlockcyclicMatrix.asBCM(mat)  # constructor (creates and returns) + destructor for above object
asmat.debug_print()

# Releasing the matrix from Frovedis server
fmat.release()
asmat.release()

# sample blockcyclic matrix from numpy matrix
lwr_triangle = FrovedisBlockcyclicMatrix(np.matrix([[1,0,0],[1,1,0],[1,1,1]],
                                                 dtype=np.float64))
# sample expressions
# no need to bother about releasing (rvalue) intermediate results of these expressions 
# they will be auto-released after the result is used
eye = lwr_triangle * lwr_triangle.inv()
upr_triangle = lwr_triangle * lwr_triangle.inv() * (~lwr_triangle)
expr = lwr_triangle * upr_triangle

print("lwr_triangle: ")
lwr_triangle.get_rowmajor_view()
print("upr_triangle: ")
upr_triangle.get_rowmajor_view()
print("expr: ")
expr.get_rowmajor_view()
print("eye: ")
eye.get_rowmajor_view()

# releasing these matrices, since they wouldn't be automatically released
# when their destructor will be called, Frovedis server will already be shut-down
lwr_triangle.release() 
upr_triangle.release() 
expr.release()
eye.release()

# about auto-release of Frovedis side memory, without explicit call to release()
b1 = FrovedisBlockcyclicMatrix() # constructor 
b1 = FrovedisBlockcyclicMatrix() # destructor of above b1 (auto-released) + constructor
b1 = FrovedisBlockcyclicMatrix() # destructor of above b1 (auto-released) + constructor
b2 = FrovedisBlockcyclicMatrix() # constructor
b3 = FrovedisBlockcyclicMatrix() # constructor

# Shutting down the Frovedis Server
FrovedisServer.shut_down()

# destructor of b1, b2 and b3 (auto-release not possible, since FrovedisServer is down)
# but they are anyhow released, while shutting down the Frovedis server.
