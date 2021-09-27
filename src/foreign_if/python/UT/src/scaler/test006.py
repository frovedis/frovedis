#!/usr/bin/env python
import sys
import numpy as np
from frovedis.exrpc.server import FrovedisServer
from frovedis.mllib.preprocessing import StandardScaler

desc =  " Testing for explicitly setting 'variance' attribute on Dense data(numpy.array) "

# initializing the Frovedis server
argvs = sys.argv
argc = len(argvs)
if (argc < 2):
    print ('Please give frovedis_server calling command as the first argument \n(e.g. "mpirun -np 2 /opt/nec/frovedis/ve/bin/frovedis_server")')
    quit()
FrovedisServer.initialize(argvs[1])

#dense data
train_mat = np.loadtxt("./input/gmm_data.txt")

# creating Standard Scaler object
ss = StandardScaler(True, True, True, False, 0)
# fitting the training matrix on Standard Scaler object
ss.fit(train_mat)
ss.transform(train_mat)


try:
    ss.var_ = [32.75, 3.5]
    print(desc, "Failed")
except:
    print(desc, "Passed")

FrovedisServer.shut_down()
