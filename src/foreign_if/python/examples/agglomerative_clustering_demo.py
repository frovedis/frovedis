#!/usr/bin/env python

from frovedis.exrpc.server import FrovedisServer
from frovedis.matrix.dense import FrovedisRowmajorMatrix
from frovedis.mllib.cluster import AgglomerativeClustering
import sys
import numpy as np

# initializing the Frovedis server
argvs = sys.argv
argc = len(argvs)
if (argc < 2):
    print ('Please give frovedis_server calling command as the first argument \n(e.g. "mpirun -np 2 -x /opt/nec/nosupport/frovedis/ve/bin/frovedis_server")')
    quit()
FrovedisServer.initialize(argvs[1])

train_mat = FrovedisRowmajorMatrix(dtype=np.float64).load("./input/spectral_data.txt")

# creating spectral agglomerative object
acm = AgglomerativeClustering(n_clusters=2, verbose=0)

# fitting the training matrix on agglomerative clustering object
acm.fit(train_mat)
acm.debug_print()

# saving the trained model
acm.save("./out/MyAcmClusteringModel")
acm.release()

# loading the same trained model
acm.load("./out/MyAcmClusteringModel",dtype=np.float64)
acm.debug_print()

acm.release()
FrovedisServer.shut_down()
