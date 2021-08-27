#!/usr/bin/env python
import sys
import numpy as np
from frovedis.exrpc.server import FrovedisServer
from frovedis.mllib.preprocessing import StandardScaler

desc = "Testing StandardScaler inverse_transform(), with fit. " #Dense data

# initializing the Frovedis server
argvs = sys.argv
argc = len(argvs)
if (argc < 2):
    print ('Please give frovedis_server calling command as the first argument \n(e.g. "mpirun -np 2 -x /opt/nec/nosupport/frovedis/ve/bin/frovedis_server")')
    quit()
FrovedisServer.initialize(argvs[1])

#dense data
train_mat = np.loadtxt("./input/gmm_data.txt")

# creating Standard Scaler object
ss = StandardScaler(True, True, True, False, 0)

# fitting the training matrix on Standard Scaler object
ss.fit(train_mat)

trans_mat = ss.transform(train_mat)

try:   
    inverse_mat = ss.inverse_transform(trans_mat)
    print(desc, "Passed")
except:
    print(desc, "Failed")

FrovedisServer.shut_down()

