#!/usr/bin/env python

import sys
import numpy as np
from frovedis.exrpc.server import FrovedisServer
from frovedis.linalg import eigsh

# initializing the Frovedis server
argvs = sys.argv
argc = len(argvs)
if argc < 2:
    print ('Please give frovedis_server calling command as the first argument \n'
           '(e.g. "mpirun -np 2 /opt/nec/frovedis/ve/bin/frovedis_server")')
    quit()
FrovedisServer.initialize(argvs[1])

# ---------- dense square symmetric matrix -----------
mat = np.asarray([[2.,-1.,0.,0.,-1.,0.], [-1.,3.,-1.,0.,-1.,0.], [0.,-1.,2.,-1.,0.,0.],
                  [0.,0.,-1.,3.,-1.,-1], [-1.,-1.,0.,-1.,3.,0], [0.,0.,0.,-1.,0.,1.]])


#----------- frovedis eigsh() ------
from frovedis.linalg import eigsh as f_eigsh
eigen_vals, eigen_vecs = f_eigsh(mat, k = 3, maxiter = 60)

print("frovedis computed eigen values:\n")
print(eigen_vals)

print("\nfrovedis computed eigen vectors:\n")
print(eigen_vecs)
 

#----------- scipy eigsh() --------
from scipy.sparse.linalg import eigsh as s_eigsh
eigen_vals, eigen_vecs = s_eigsh(mat, k = 3, maxiter = 60)

print("\nscipy computed eigen values:\n")
print(eigen_vals)

 
print("\nscipy computed eigen vectors:\n")
print(eigen_vecs)

FrovedisServer.shut_down()
