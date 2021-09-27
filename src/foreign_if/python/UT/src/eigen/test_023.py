#!/usr/bin/env python

import sys
import numpy as np
from frovedis.exrpc.server import FrovedisServer
from frovedis.linalg import eigsh

desc = "Testing eigsh() for when parameter - sigma is not correct: "

# initializing the Frovedis server
argvs = sys.argv
argc = len(argvs)
if argc < 2:
    print ('Please give frovedis_server calling command as the first argument \n'
           '(e.g. "mpirun -np 2 /opt/nec/frovedis/ve/bin/frovedis_server")')
    quit()
FrovedisServer.initialize(argvs[1])

# sample numpy array square symmetric dense data (6x6)
mat = np.asarray([[2.,-1.,0.,0.,-1.,0.], [-1.,3.,-1.,0.,-1.,0.],
                  [0.,-1.,2.,-1.,0.,0.], [0.,0.,-1.,3.,-1.,-1],
                  [-1.,-1.,0.,-1.,3.,0.], [0.,0.,0.,-1.,0.,1.]])

try:
    eigen_vals, eigen_vecs = eigsh(mat, k = 3, sigma = "1.0")
    print(desc, "Failed")
except:
    print(desc, "Passed")
    
FrovedisServer.shut_down()