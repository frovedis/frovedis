#!/usr/bin/env python

import sys
import numpy as np
import pandas as pd
from frovedis.exrpc.server import FrovedisServer
from frovedis.matrix.crs import FrovedisCRSMatrix
from frovedis.mllib.naive_bayes import MultinomialNB
from frovedis.mllib.naive_bayes import BernoulliNB

#Objective: Run without error

# initializing the Frovedis server
argvs = sys.argv
argc = len(argvs)
if (argc < 2):
    print 'Please give frovedis_server calling command as the first argument \n(e.g. "mpirun -np 2 -x /opt/nec/nosupport/frovedis/ve/bin/frovedis_server")'
    quit()
FrovedisServer.initialize(argvs[1])

mat = FrovedisCRSMatrix(dtype=np.float64).load("./input/multi.txt")
lbl = np.array([1.0,0.0,1.0,1.0,1.0,0.0,1.0,1.0])
nbm = MultinomialNB(alpha=1.0,fit_prior=True,class_prior=None,verbose=0).fit(mat,lbl)
nbm.debug_print()
print("predicting on nbm multinomial classifier model: ")
mnb = nbm.predict(mat)
print(mnb)
print("Accuracy of model: ", nbm.score(mat,lbl))
ret1 = (mnb == lbl).all()

mat = FrovedisCRSMatrix(dtype=np.float64).load("./input/bern.txt")
mat.debug_print()
lbl = np.array([1.0,0.0,1.0,1.0,1.0,0.0,1.0,1.0])
nbm2 = BernoulliNB(alpha=1.0,fit_prior=True,class_prior=None,binarize=0.0,verbose= 0).fit(mat,lbl)
nbm2.debug_print()
print("predicting on nbm bernoulli classifier model: ")
bnb =  nbm2.predict(mat)
print(bnb)
print("Accuracy of model", nbm2.score(mat,lbl))
ret2 = (bnb == lbl).all()

if ret1 and ret2: print("Status: Passed")
else: print("Status: Failed")

nbm.release()
nbm2.release()
FrovedisServer.shut_down()
