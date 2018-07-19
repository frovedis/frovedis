#!/usr/bin/env python

from frovedis.exrpc.server import FrovedisServer
from frovedis.matrix.sparse import FrovedisCRSMatrix
from frovedis.matrix.dvector import FrovedisDoubleDvector
from frovedis.mllib.linear_model import *
import sys

# initializing the Frovedis server
argvs = sys.argv
argc = len(argvs)
if (argc < 2):
    print 'Please give frovedis_server calling command as the first argument \n(e.g. "mpirun -np 2 -x /opt/nec/nosupport/frovedis/ve/bin/frovedis_server")'
    quit()
FrovedisServer.initialize(argvs[1])

mat = FrovedisCRSMatrix().load("./input/libSVMFile.txt")
lbl = FrovedisDoubleDvector([1,0,1,1,1,0,1,1])

# fitting input matrix and label on logistic regression object
lr = LogisticRegression(solver='lbfgs', verbose=0).fit(mat,lbl)

# predicting on loaded model
print("predicting on lbfgs logistic regression model: ")
print lr.predict(mat)

# fitting input matrix and label on logistic regression object
lr = LogisticRegression(solver='sag', verbose=0).fit(mat,lbl)

# predicting on loaded model
print("predicting on sag logistic regression model: ")
print lr.predict(mat)

# saving the model
lr.save("./out/LRModel")

# loading the same model
lr.load("./out/LRModel")

# debug_print
lr.debug_print()
lr.release()

FrovedisServer.shut_down()
