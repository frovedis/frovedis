#!/usr/bin/env python

import sys
import numpy as np
from frovedis.exrpc.server import FrovedisServer
from frovedis.mllib.preprocessing import StandardScaler
from scipy.sparse import csr_matrix
from scipy import sparse

desc =  " Test for explicitly setting 'var_' attribute " #sparse csr_matrix

# initializing the Frovedis server
argvs = sys.argv
argc = len(argvs)
if (argc < 2):
    print ('Please give frovedis_server calling command as the first argument \n(e.g. "mpirun -np 2 /opt/nec/frovedis/ve/bin/frovedis_server")')
    quit()
FrovedisServer.initialize(argvs[1])


#dense data
train_mat = np.loadtxt("./input/gmm_data.txt")
sparseMatrix = sparse.csr_matrix(train_mat)

# creating Standard Scaler object
ss = StandardScaler(True, False, True, False, 0)

# fitting the training matrix on Standard Scaler object
ss.fit(sparseMatrix)

try:
    ss.var_ = [32.75, 3.5]
    print(desc, "Failed")
except:
    print(desc, "Passed")

FrovedisServer.shut_down()
