#!/usr/bin/env python

import sys
import numpy as np
from frovedis.exrpc.server import FrovedisServer
from frovedis.mllib.cluster import DBSCAN

# initializing the Frovedis server
argvs = sys.argv
argc = len(argvs)
if (argc < 2):
    print ('Please give frovedis_server calling command as the first argument \n(e.g. "mpirun -np 2 -x /opt/nec/nosupport/frovedis/ve/bin/frovedis_server")')
    quit()
FrovedisServer.initialize(argvs[1])

# creating dbscan object
dbm = DBSCAN(eps=3, metric='euclidean', verbose=0)

# fitting the training matrix on dbscan object
tmat = np.loadtxt("./input/knn_data.txt")
tmat = np.asarray(tmat, dtype=np.float64)
dbm.fit(tmat)

lbl = dbm.labels_
print ("fit_predict: ", dbm.fit_predict(tmat))
print ("Labels: ", lbl)

dbm.release()
FrovedisServer.shut_down()
