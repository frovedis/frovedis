#!/usr/bin/env python

import sys
import numpy as np
from frovedis.exrpc.server import FrovedisServer
from frovedis.matrix.crs import FrovedisCRSMatrix
from frovedis.linalg import eigsh

desc = "Testing eigsh() for FrovedisCRSMatrix: "

# initializing the Frovedis server
argvs = sys.argv
argc = len(argvs)
if argc < 2:
    print ('Please give frovedis_server calling command as the first argument \n'
           '(e.g. "mpirun -np 2 -x /opt/nec/nosupport/frovedis/ve/bin/frovedis_server")')
    quit()
FrovedisServer.initialize(argvs[1])

# sample numpy array square symmetric dense data (6x6)
mat = np.asarray([[2.,-1.,0.,0.,-1.,0.], [-1.,3.,-1.,0.,-1.,0.],
                  [0.,-1.,2.,-1.,0.,0.], [0.,0.,-1.,3.,-1.,-1],
                  [-1.,-1.,0.,-1.,3.,0.], [0.,0.,0.,-1.,0.,1.]])

mat = FrovedisCRSMatrix(mat)
try:
    eigen_vals, eigen_vecs = eigsh(mat, k = 3)
    print(desc, "Passed")
except:
    print(desc, "Failed")
    
FrovedisServer.shut_down()