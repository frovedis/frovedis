#!/usr/bin/env python

import sys

import numpy as np
np.set_printoptions(threshold=5)

#from sklearn.svm import SVC
from frovedis.mllib.svm import SVC

# initializing the Frovedis server
argvs = sys.argv
argc = len(argvs)
if (argc < 2):
    print ('Please give frovedis_server calling command as the first argument \n(e.g. "mpirun -np 2 -x /opt/nec/nosupport/frovedis/ve/bin/frovedis_server")')
    quit()

from frovedis.exrpc.server import FrovedisServer
FrovedisServer.initialize(argvs[1])

# classification data
from sklearn.datasets import load_breast_cancer
mat, lbl = load_breast_cancer(return_X_y=True)

# fitting input matrix and label on svm kernel object
#svc = SVC(C=1.0, kernel='linear', degree=3, probability=True)
svc = SVC(C=1.0, kernel='rbf', degree=3, probability=True)
svc.fit(mat, lbl)

# predicting on loaded model
print("predicting on trained model: ")
print(svc.predict(mat))

print("predict_proba on trained model: ")
print(svc.predict_proba(mat))

print("support_vectors_ on trained model: ")
print(svc.support_vectors_)

print("support_ on trained model: ")
print(svc.support_)

# saving the model
svc.save("./out/SVCModel")

# loading the same model
svc.load("./out/SVCModel")
print("predicting on loaded model: ")
print(svc.predict(mat))

svc.release()
FrovedisServer.shut_down()
