#!/usr/bin/env python

import sys
import numpy as np
from frovedis.exrpc.server import FrovedisServer
from frovedis.mllib.ensemble import RandomForestClassifier
from frovedis.mllib.ensemble import RandomForestRegressor

#Objective: Run without error

# initializing the Frovedis server
argvs = sys.argv
argc = len(argvs)
if (argc < 2):
    print ('Please give frovedis_server calling command as the first argument \n(e.g. "mpirun -np 2 /opt/nec/frovedis/ve/bin/frovedis_server")')
    quit()
FrovedisServer.initialize(argvs[1])

mat = np.array([[10, 0, 1, 0, 0, 1, 0],
                [0, 1, 0, 1, 0, 1, 0],
                [0, 1, 0, 0, 1, 0, 1],
                [1, 0, 0, 1, 0, 1, 0]], dtype=np.float64)
lbl = np.array([100, 500, 100, 600], dtype=np.float64) 

# fitting input matrix and label on DecisionTree Classifier object
rfc1 = RandomForestClassifier(criterion='gini', max_depth=4,
                              min_samples_split=2, min_samples_leaf=1, min_weight_fraction_leaf=0.0, 
                              max_leaf_nodes=32, random_state=324,
                              min_impurity_decrease=0.0, class_weight=2, verbose = 0, n_estimators=10)
rfc = rfc1.fit(mat,lbl)
#rfc.save('./rfmodel')
#rfc.debug_print()

# predicting on train model
print("predicting on RandomForest classifier model: ")
rfcm = rfc.predict(mat)
print (rfcm)
print("Accuracy score for predicted RandomForest Classifier model")
print (rfc.score(mat,lbl))

# # fitting input matrix and label on RandomForest Regressor object
rfr1 = RandomForestRegressor(criterion='mae', max_depth=5, min_samples_split=2, min_samples_leaf=1,
                             min_weight_fraction_leaf=0.0, max_features='auto', random_state = None,
                             max_leaf_nodes=1, min_impurity_decrease=0.0, min_impurity_split=None,
                             verbose = 0, n_estimators =10)
lbl1 = np.array([1.2,0.3,1.1,1.9])
rfr = rfr1.fit(mat,lbl)
#rfr.debug_print()

# predicting on train model
print("predicting on RandomForest Regressor model: ")
rfrm =  rfr.predict(mat)
print (rfrm)
print("Root mean square for predicted RandomForest Regressor model")
print (rfr.score(mat,lbl))

#clean-up
rfc1.release()
rfr1.release()
FrovedisServer.shut_down()

