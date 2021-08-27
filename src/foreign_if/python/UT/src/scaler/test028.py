#!/usr/bin/env python

import sys
import numpy as np
from frovedis.exrpc.server import FrovedisServer
from frovedis.mllib.preprocessing import StandardScaler

desc = "Test with_mean and with_std is None"

# initializing the Frovedis server
argvs = sys.argv
argc = len(argvs)
if (argc < 2):
    print ('Please give frovedis_server calling command as the first argument \n(e.g. "mpirun -np 2 -x /opt/nec/nosupport/frovedis/ve/bin/frovedis_server")')
    quit()
FrovedisServer.initialize(argvs[1])


#dense data
train_mat = np.loadtxt("./input/gmm_data.txt")

try:
    ss = StandardScaler(True, None, None, False)
    ss.fit(train_mat)
    trans_mat = ss.transform(train_mat)
    inverse_trans_mat = ss.inverse_transform(trans_mat)
    print(ss.mean_)
    print(ss.var_)
    print(desc, "Passed")
except:
    print(desc, "Failed")

FrovedisServer.shut_down()

