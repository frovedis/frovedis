#!/usr/bin/env python

import sys
import numpy as np
from frovedis.exrpc.server import FrovedisServer 
from frovedis.matrix.dense import FrovedisBlockcyclicMatrix
from frovedis.matrix.wrapper import SCALAPACK

# initializing the Frovedis server
argvs = sys.argv
argc = len(argvs)
if (argc < 2):
    print ('Please give frovedis_server calling command as the first argument \n(e.g. "mpirun -np 2 -x /opt/nec/nosupport/frovedis/ve/bin/frovedis_server")')
    quit()
FrovedisServer.initialize(argvs[1])

# Loading Frovedis server side blockcyclic matrix from text file
bcm1 = FrovedisBlockcyclicMatrix(dtype=np.float64)
print("loading blockcyclic matrix from text file: ./input/mat_3x3")
bcm1.load("./input/mat_3x3")
bcm1.get_rowmajor_view()

# --- getrf (LU factorization) demo ---
rf = SCALAPACK.getrf(bcm1)
print("getrf info: " + str(rf.stat()))

# --- getri (matrix inversion using LU factor) demo ---
stat = SCALAPACK.getri(bcm1,rf.ipiv())
print("getri info: " + str(stat))

# Saving inverse matrix in text format
print("saving result to text file: ./out/inv_mat_3x3")
bcm1.save("./out/inv_mat_3x3") #result can be saved in text file

# Releasing IPIV vector and LU factored matrix from Frovedis server
rf.release()
bcm1.release()

# Loading new text file containing 4x4 input data for computing SVD
print("loading blockcyclic matrix from text file: ./input/svd_input_4x4")
bcm1.load("./input/svd_input_4x4")
bcm1.get_rowmajor_view()

# --- gesvd (computing SVD) demo ---
svd = SCALAPACK.gesvd(bcm1)
print("gesvd info: " + str(svd.stat()))

# Saving SVD results (singular values and singular vectors) in text format
print("saving svd results: ")
# sfile name is mandatory... ufile/vfile can be None (default), if not required to be saved
svd.save("./out/svd_output_sfile","./out/svd_output_ufile","./out/svd_output_vfile")

# getting the result back at pythob side
print("printing the results (umat, svec, vmat) at python client:")
(umat,svec,vmat) = svd.to_numpy_results()
print (umat)
print (svec)
print (vmat)

# Releasing input matrix and resultant matrices
print("releasing frovedis side svd input and result data")
bcm1.release()
svd.release()

# Loading the same results from saved text files
print("loading the same svd results from saved files as blockcyclic matrix (umat/vmat):")
svd.load("./out/svd_output_sfile","./out/svd_output_ufile","./out/svd_output_vfile")
print("printing the loaded svd results (at Frovedis server): ")
svd.debug_print()
svd.release()

# sfile name is mandatory... ufile/vfile can be None (default), if not required to be loaded
print("loading only svec and umat (as colmajor matrix) from saved files:")
svd.load("./out/svd_output_sfile", "./out/svd_output_ufile", mtype='C')
print("printing partially loaded svd results (at Frovedis server): ")
svd.debug_print()
svd.release()

# Shutting down the Frovedis server
FrovedisServer.shut_down()

