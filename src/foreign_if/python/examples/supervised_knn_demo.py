#!/usr/bin/env python

import sys
import numpy as np
from frovedis.exrpc.server import FrovedisServer
from frovedis.mllib.neighbors import KNeighborsClassifier
from frovedis.mllib.neighbors import KNeighborsRegressor

# initializing the Frovedis server
argvs = sys.argv
argc = len(argvs)
if (argc < 2):
    print ('Please give frovedis_server calling command as the first argument \n(e.g. "mpirun -np 2 /opt/nec/frovedis/ve/bin/frovedis_server")')
    quit()
FrovedisServer.initialize(argvs[1])

samples = np.loadtxt("./input/knn_data.txt", dtype=np.float64)
lbl = [10, 10, 10, 20, 10, 20]

# fitting input matrix and label on KNeighbors Classifier object
knc = KNeighborsClassifier(n_neighbors=3, algorithm='brute', metric='euclidean')
knc.fit(samples, lbl)

dist, ind = knc.kneighbors(samples)
print(dist)
print(ind)

graph = knc.kneighbors_graph(samples, mode="distance")
print(graph)

print("predicting on classifier model: ")
print (knc.predict(samples))
print("prediction probability on classifier model: ")
proba = knc.predict_proba(samples)
print(proba)
print("Accuracy: ")
print(knc.score(samples, lbl))

# fitting input matrix and label on KNeighbors Regressor object
lbl2 = [1.2,0.3,1.1,1.9,1.7,0.5]
knr = KNeighborsRegressor(n_neighbors=3, algorithm='brute', metric='euclidean')
knr.fit(samples, lbl2)

#dist, ind = knr.kneighbors(samples)
#print(dist)
#print(ind)

#graph = knr.kneighbors_graph(samples, mode="distance")
#print(graph)

print("predicting on regressor model: ")
print (knr.predict(samples))
print("RMSE: ")
print(knr.score(samples, lbl2))

# clean-up
knc.release()
knr.release()
FrovedisServer.shut_down()
