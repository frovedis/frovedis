#!/usr/bin/env python

import sys

import numpy as np
np.set_printoptions(threshold=5)

#from sklearn.tree import DecisionTreeClassifier, DecisionTreeRegressor
from frovedis.mllib.tree import DecisionTreeClassifier, DecisionTreeRegressor

# initializing the Frovedis server
argvs = sys.argv
argc = len(argvs)
if (argc < 2):
    print ('Please give frovedis_server calling command as the first argument \n(e.g. "mpirun -np 2 /opt/nec/frovedis/ve/bin/frovedis_server")')
    quit()

from frovedis.exrpc.server import FrovedisServer
FrovedisServer.initialize(argvs[1])

# classification data
from sklearn.datasets import load_breast_cancer
mat, lbl = load_breast_cancer(return_X_y=True)

# fitting input matrix and label on DecisionTree Classifier object
dtc = DecisionTreeClassifier(criterion='gini', max_depth=5)
dtc.fit(mat,lbl)
#dtc.debug_print()

# predicting on train model
print("predicting on DecisionTree classifier model: ")
print(dtc.predict(mat))
print("predicting probability on DecisionTree classifier model: ")
print (dtc.predict_proba(mat))
print("prediction accuracy: %.4f" % (dtc.score(mat, lbl)))

# regression data
from sklearn.datasets import load_boston
mat, lbl = load_boston(return_X_y=True)

# fitting input matrix and label on DecisionTree Regressor object
dtr = DecisionTreeRegressor(criterion='mse', max_depth=5)
dtr.fit(mat, lbl)
#dtr.debug_print()

# predicting on train model
print("predicting on  DecisionTree Regressor model: ")
print(dtr.predict(mat))
print("prediction score: %.4f" % (dtr.score(mat, lbl)))

#clean-up
#dtc.release()
#dtr.release()
FrovedisServer.shut_down()
