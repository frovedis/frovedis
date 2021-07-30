#!/usr/bin/env python

import sys
from frovedis.exrpc.server import FrovedisServer
from frovedis.linalg import eigsh
from scipy.sparse import csr_matrix

desc = "Testing eigsh() for csr_matrix and which = 'SM': "

# initializing the Frovedis server
argvs = sys.argv
argc = len(argvs)
if argc < 2:
    print ('Please give frovedis_server calling command as the first argument \n'
           '(e.g. "mpirun -np 2 -x /opt/nec/nosupport/frovedis/ve/bin/frovedis_server")')
    quit()
FrovedisServer.initialize(argvs[1])

# sample square symmetric sparse matrix (6x6)
mat = csr_matrix([[ 2.,-1., 0., 0.,-1., 0.], [-1., 3.,-1., 0.,-1., 0.], 
                  [ 0.,-1., 2.,-1., 0., 0.], [ 0., 0.,-1., 3.,-1.,-1], 
                  [-1.,-1., 0.,-1., 3., 0.], [ 0., 0., 0.,-1., 0., 1.]])

try:
    eigen_vals, eigen_vecs = eigsh(mat, k = 3, which = 'SM')
    print(desc, "Passed")
except:
    print(desc, "Failed")
    
FrovedisServer.shut_down()