#!/usr/bin/env python

import sys
import numpy as np
from frovedis.exrpc.server import FrovedisServer

#from sklearn.cluster import DBSCAN
from frovedis.mllib.cluster import DBSCAN

# initializing the Frovedis server
argvs = sys.argv
argc = len(argvs)
if (argc < 2):
    print ('Please give frovedis_server calling command as the first argument \n(e.g. "mpirun -np 2 /opt/nec/frovedis/ve/bin/frovedis_server")')
    quit()
FrovedisServer.initialize(argvs[1])

# creating dbscan object
dbm = DBSCAN(eps=5, min_samples=2)

# fitting the training matrix on dbscan object
tmat = np.loadtxt("./input/knn_data.txt")
print ("lables: ", dbm.fit_predict(tmat))

dbm.release()
FrovedisServer.shut_down()
