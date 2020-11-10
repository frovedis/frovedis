#!/usr/bin/env python

import sys
import numpy as np
from frovedis.exrpc.server import FrovedisServer
from frovedis.mllib.svm import SVC

# initializing the Frovedis server
argvs = sys.argv
argc = len(argvs)
if (argc < 2):
    print ('Please give frovedis_server calling command as the first argument \n(e.g. "mpirun -np 2 -x /opt/nec/nosupport/frovedis/ve/bin/frovedis_server")')
    quit()
FrovedisServer.initialize(argvs[1])

#from sklearn.datasets import load_boston
#mat, lbl = load_boston(return_X_y=True)

mat = np.array([[10, 0, 1, 0, 0, 1, 0],
                    [0, 1, 0, 1, 0, 1, 0],
                    [0, 1, 0, 0, 1, 0, 1],
                    [1, 0, 0, 1, 0, 1, 0]],dtype=np.float64)
lbl = np.array([1, -1, 1, -1],dtype=np.float64)


# fitting input matrix and label on svm regression object
svc = SVC(C=1.0, kernel='rbf', degree=3, gamma='scale', coef0=0.0,
          shrinking=True, probability=True, tol=0.001, cache_size=128,
          class_weight=None, verbose=False, max_iter=-1).fit(mat,lbl)

# predicting on loaded model
print("predicting on trained model: ")
print(svc.predict(mat[:10]))

print("support_vectors_ on trained model: ")
print(svc.support_vectors_)
print("support_ on trained model: ")
print(svc.support_)
print("predict_proba on trained model: ")
print(svc.predict_proba(mat[:10]))

# saving the model
svc.save("./out/SVCModel")

# loading the same model
svc2 = svc.load("./out/SVCModel")
print(svc2.predict(mat))

svc.release()

FrovedisServer.shut_down()

