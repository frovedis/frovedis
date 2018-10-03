#!/usr/bin/env python

from frovedis.exrpc.server import FrovedisServer
from frovedis.matrix.crs import FrovedisCRSMatrix
from frovedis.mllib.cluster import KMeans
import sys
import numpy as np

# initializing the Frovedis server
argvs = sys.argv
argc = len(argvs)
if (argc < 2):
    print ('Please give frovedis_server calling command as the first argument \n(e.g. "mpirun -np 2 -x /opt/nec/nosupport/frovedis/ve/bin/frovedis_server")')
    quit()
FrovedisServer.initialize(argvs[1])

train_mat = FrovedisCRSMatrix(dtype=np.float64).load("./input/kmeans_data.txt")
test_mat = FrovedisCRSMatrix(dtype=np.float64).load("./input/kmeans_tdata.txt")

# creating KMeans object
kmeans = KMeans(n_clusters=2, verbose=0)

# fitting the training matrix on KMEans object
kmeans.fit(train_mat)
kmeans.debug_print()

# saving the trained model
kmeans.save("./out/MyKMeansModel")
kmeans.release()

# loading the same trained model
kmeans.load("./out/MyKMeansModel",dtype=np.float64)

# predicting with test data on loaded model
print kmeans.predict(test_mat)

kmeans.release()
FrovedisServer.shut_down()
