#!/usr/bin/env python

import sys
import numpy as np
from frovedis.matrix.dense import FrovedisRowmajorMatrix
from frovedis.exrpc.server import FrovedisServer
from frovedis.mllib.linear_model import SGDClassifier
#from sklearn.decomposition import PCA

# initializing the Frovedis server
argvs = sys.argv
argc = len(argvs)
if (argc < 2):
    print ('Please give frovedis_server calling command as the first argument \n(e.g. "mpirun -np 2 -x /opt/nec/nosupport/frovedis/ve/bin/frovedis_server")')
    quit()
FrovedisServer.initialize(argvs[1])

X = np.array([[-1, -1], [-2, -1], [1, 1], [2, 1]])
Y = np.array([1, 1, 2, 2])
clf = SGDClassifier(max_iter=1000, tol=1e-3, loss='hinge')
#clf = SGDClassifier(max_iter=1000, tol=1e-3, loss='log')
clf.fit(X, Y)

print(clf.predict([[-0.8, -1]]))

clf.save("sgd_classifier1")


FrovedisServer.shut_down()
