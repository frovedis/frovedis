#!/usr/bin/env python

import sys
import numpy as np
from frovedis.mllib.neighbors import NearestNeighbors
from frovedis.exrpc.server import FrovedisServer

# initializing the Frovedis server
argvs = sys.argv
argc = len(argvs)
if (argc < 2):
    print ('Please give frovedis_server calling command as the first argument \n(e.g. "mpirun -np 2 -x /opt/nec/nosupport/frovedis/ve/bin/frovedis_server")')
    quit()
FrovedisServer.initialize(argvs[1])

samples = np.loadtxt("./input/knn_data.txt", dtype=np.float64)
knn = NearestNeighbors(n_neighbors=3, radius=2.0, 
                       algorithm='brute', metric='euclidean')
knn.fit(samples)

distances, indices = knn.kneighbors()
print(distances)
print(indices)

graph = knn.kneighbors_graph(samples)
print(graph)

dist, ind = knn.radius_neighbors()
for i in range(len(dist)):
  print(dist[i])
  print(ind[i])

rad_graph = knn.radius_neighbors_graph()
print(rad_graph)

FrovedisServer.shut_down()
