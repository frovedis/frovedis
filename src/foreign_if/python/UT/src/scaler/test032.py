#!/usr/bin/env python

import sys
import numpy as np
from frovedis.exrpc.server import FrovedisServer
from frovedis.mllib.preprocessing import StandardScaler
from frovedis.matrix.dense import FrovedisColmajorMatrix

desc = "Testing StandardScaler inverse_transform(), without fit. " #sparse csr_matrix

# initializing the Frovedis server
argvs = sys.argv
argc = len(argvs)
if (argc < 2):
    print ('Please give frovedis_server calling command as the first argument \n(e.g. "mpirun -np 2 /opt/nec/frovedis/ve/bin/frovedis_server")')
    quit()
FrovedisServer.initialize(argvs[1])

#dense data
train_mat = np.loadtxt("./input/gmm_data.txt")
mat = FrovedisColmajorMatrix(train_mat)

# creating Standard Scaler object
ss = StandardScaler(True, True, True, False, 0)


try:
    trans_mat = ss.transform(mat)
    inverse_tran_mat = ss.inverse_transform(trans_mat)
    print(desc, "Failed")
except:
    print(desc, "Passed")

FrovedisServer.shut_down()
