#!/usr/bin/env python

import sys
import numpy as np
np.set_printoptions(threshold=5)

#from sklearn.linear_model import LogisticRegression 
from frovedis.mllib.linear_model import LogisticRegression 

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

# fitting input matrix and label on logistic regression object
lr = LogisticRegression(solver='lbfgs').fit(mat,lbl)

# predicting on loaded model
print("predicting on lbfgs logistic regression model: ")
print(lr.predict(mat))
print("score: %.2f " % (lr.score(mat, lbl)))

# fitting input matrix and label on logistic regression object
lr = LogisticRegression(solver='sag').fit(mat,lbl)

# predicting on loaded model
print("predicting on sag logistic regression model: ")
print(lr.predict(mat))
print("score: %.2f " % (lr.score(mat, lbl)))

print("attributes in fitted model: ")
print("coef: ")
print(lr.coef_)

print("intercept: ")
print(lr.intercept_)

print("classes: ")
print(lr.classes_)

'''
# saving the model
lr.save("./out/LRModel")

# loading the same model
lr.load("./out/LRModel")
print("attributes in loaded model: ")
print("coef: ")
print(lr.coef_)

print("intercept: ")
print(lr.intercept_)

print("classes: ")
print(lr.classes_)
'''
#lr.release()
FrovedisServer.shut_down()
