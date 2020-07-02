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
n_clusters = 2
acm = AgglomerativeClustering(n_clusters=n_clusters)

# fitting the training matrix on agglomerative clustering object
acm.fit(train_mat)
print('\nconstructed dendogram of diemnsion: (%d, 4)' % (train_mat.numRows()-1))
acm.debug_print() # prints dendogram 

# printing clustered labels
print('\ncomputed labels for %d clusters: %s' % (n_clusters, str(acm.labels_)))

# saving the trained model
acm.save("./out/MyAcmClusteringModel")
acm.release() # freeing model object from memory of Frovedis Server

# loading the same trained model
acm.load("./out/MyAcmClusteringModel",dtype=np.float64)

# printing relabeling for new n_clusters 
# on loaded model (on previously fitted matrix)
n_clusters = 3
new_labels = acm.predict(n_clusters)
print('\nrecomputed labels for %d clusters: %s' % (n_clusters, str(new_labels)))

acm.release()
FrovedisServer.shut_down()
