#!/usr/bin/env python

import sys
import numpy as np
np.set_printoptions(threshold=5)

#from sklearn.svm import LinearSVR
from frovedis.mllib.svm import LinearSVR

# initializing the Frovedis server
argvs = sys.argv
argc = len(argvs)
if (argc < 2):
    print ('Please give frovedis_server calling command as the first argument \n(e.g. "mpirun -np 2 /opt/nec/frovedis/ve/bin/frovedis_server")')
    quit()

from frovedis.exrpc.server import FrovedisServer
FrovedisServer.initialize(argvs[1])

# regression data
from sklearn.datasets import load_boston
mat, lbl = load_boston(return_X_y=True)

# fitting input matrix and label on svm regression object
svr = LinearSVR(loss='epsilon_insensitive', epsilon = 0.1,
                max_iter=1000, penalty='l2').fit(mat, lbl)

# predicting on loaded model
print("predicting on trained model: ")
print(svr.predict(mat))
print("prediction score: %.2f" % svr.score(mat, lbl))

# saving the model
svr.save("./out/SVRModel")

# loading the same model
svr.load("./out/SVRModel")

# debug_print
svr.debug_print()
svr.release()

FrovedisServer.shut_down()

