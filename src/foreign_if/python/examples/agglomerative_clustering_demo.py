#!/usr/bin/env python

import sys
import numpy as np
from frovedis.exrpc.server import FrovedisServer
from frovedis.matrix.dense import FrovedisRowmajorMatrix
from frovedis.mllib.cluster import AgglomerativeClustering

# initializing the Frovedis server
argvs = sys.argv
argc = len(argvs)
if (argc < 2):
    print ('Please give frovedis_server calling command as the first argument \n(e.g. "mpirun -np 2 -x /opt/nec/nosupport/frovedis/ve/bin/frovedis_server")')
    quit()
FrovedisServer.initialize(argvs[1])

train_mat = np.loadtxt("./input/spectral_data.txt")

# creating spectral agglomerative object
n_clusters = 2
acm = AgglomerativeClustering(n_clusters=n_clusters)

# fitting the training matrix on agglomerative clustering object
acm.fit(train_mat)
acm.debug_print() # prints dendrogram 

# printing clustered labels
print('computed labels for %d clusters: %s' % (n_clusters, str(acm.labels_)))
print('children:')
print(acm.children_)
print('distances:')
print(acm.distances_)
print('no. of connected components: %d'% (acm.n_connected_components_))
print('no. of leaves: %d'% (acm.n_leaves_))

# saving the trained model
acm.save("./out/MyAcmClusteringModel")
acm.release() # freeing model object from memory of Frovedis Server

# loading the same trained model
acm.load("./out/MyAcmClusteringModel",dtype=np.float64)

# printing relabeling for new n_clusters 
# on loaded model (on previously fitted matrix)
n_clusters = 3
new_labels = acm.reassign(n_clusters)
print('\nrecomputed labels for %d clusters: %s' % (n_clusters, str(new_labels)))

acm.release()
FrovedisServer.shut_down()
