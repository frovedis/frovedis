#!/usr/bin/env python

from frovedis.exrpc.server import FrovedisServer
from frovedis.mllib.tree import DecisionTreeClassifier
from frovedis.mllib.tree import DecisionTreeRegressor
from sklearn import tree
import sys
import numpy as np
import pandas as pd

#Objective :  Check and compare prediction of Frovedis with scikit-learn

# initializing the Frovedis server
argvs = sys.argv
argc = len(argvs)
if (argc < 2):
    print 'Please give frovedis_server calling command as the first argument \n(e.g. "mpirun -np 2 /opt/nec/frovedis/ve/bin/frovedis_server")'
    quit()
FrovedisServer.initialize(argvs[1])

mat = np.asmatrix([[1.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.0],
                   [0.0, 1.0, 0.0, 1.0, 0.0, 1.0, 0.0],
                   [0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 1.0],
                   [1.0, 0.0, 0.0, 1.0, 0.0, 1.0, 0.0]], dtype=np.float64)
lbl = np.array([0.0, 1.0, 1.0, 0.0], dtype=np.float64)

# fitting input matrix and label on DecisionTree Classifier object
dtc1 = DecisionTreeClassifier(criterion='gini', splitter='best', max_depth=None,
                 min_samples_split=2, min_samples_leaf=1, min_weight_fraction_leaf=0.0,
                 max_features=None, random_state=None, max_leaf_nodes=1,
                 min_impurity_decrease=0.0,
                 class_weight=None, presort=False, verbose = 0)
dtc = dtc1.fit(mat,lbl)
dtc.debug_print()

# predicting on train model
print("predicting on DecisionTree classifier model: ")
prec = dtc.predict(mat)
print prec
print("Accuracy score for predicted DecisionTree Classifier model")
print dtc.score(mat,lbl)


# fitting input matrix and label on DecisionTree Regressor object
dtr1 = DecisionTreeRegressor(criterion='mse', splitter='best',
                 max_depth=None, min_samples_split=2, min_samples_leaf=1,
                 min_weight_fraction_leaf=0.0, max_features=None, random_state=None,
                 max_leaf_nodes=1, min_impurity_decrease=0.0, min_impurity_split=None,
                 class_weight=None, presort=False, verbose = 0)
lbl1 = np.array([1.2,0.3,1.1,1.9],dtype=np.float64)
dtr = dtr1.fit(mat,lbl1)
dtr.debug_print()

# predicting on train model
print("predicting on  DecisionTree Regressor model: ")
prer = dtr.predict(mat)
print prer
print("Root mean square for predicted DecisionTree Regressor model")
print dtr.score(mat,lbl1)


# fitting input matrix and label from scikit-learn
clf = tree.DecisionTreeClassifier()
clf = clf.fit(mat, lbl)

clr = tree.DecisionTreeRegressor()
clr = clr.fit(mat, lbl1)

if (clf.predict(mat) == prec).all() and (clr.predict(mat)== prer).all():
  print("Status: Passed")
else:
  print("Status: Failed")

#clean-up
dtc.release()
dtr.release()
FrovedisServer.shut_down()

