#!/usr/bin/env python

from frovedis.exrpc.server import FrovedisServer
from frovedis.matrix.crs import FrovedisCRSMatrix
from frovedis.matrix.dvector import FrovedisDvector
from frovedis.mllib.linear_model import *
import numpy as np
import sys

# initializing the Frovedis server
argvs = sys.argv
argc = len(argvs)
if (argc < 2):
    print ('Please give frovedis_server calling command as the first argument \n(e.g. "mpirun -np 2 -x /opt/nec/nosupport/frovedis/ve/bin/frovedis_server")')
    quit()
FrovedisServer.initialize(argvs[1])

mat = FrovedisCRSMatrix(dtype=np.float64).load("./input/libSVMFile.txt")
lbl = [1,0,1,1,1,0,1,1]

# fitting input matrix and label on logistic regression object
lr = LogisticRegression(solver='lbfgs', C=1.0, verbose=0).fit(mat,lbl)

# predicting on loaded model
print("predicting on lbfgs logistic regression model: ")
print(lr.predict(mat))
print("score: %.2f " % (lr.score(mat, lbl)))

# fitting input matrix and label on logistic regression object
lr = LogisticRegression(solver='sag', C=1.0, verbose=0).fit(mat,lbl)

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

lr.release()

FrovedisServer.shut_down()
