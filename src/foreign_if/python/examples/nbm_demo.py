#!/usr/bin/env python

import sys
import numpy as np
from frovedis.exrpc.server import FrovedisServer
from frovedis.matrix.crs import FrovedisCRSMatrix
from frovedis.mllib.naive_bayes import MultinomialNB
from frovedis.mllib.naive_bayes import BernoulliNB

# initializing the Frovedis server
argvs = sys.argv
argc = len(argvs)
if (argc < 2):
    print ('Please give frovedis_server calling command as the first argument \n(e.g. "mpirun -np 2 -x /opt/nec/nosupport/frovedis/ve/bin/frovedis_server")')
    quit()
FrovedisServer.initialize(argvs[1])

# Demo of Multinomial Naive Bayes
mat = FrovedisCRSMatrix(dtype=np.float64).load("./input/multi.txt")
lbl = np.array([1.0,0.0,1.0,1.0,1.0,0.0,1.0,1.0])
nbm1 = MultinomialNB(alpha=1.0).fit(mat,lbl)
print("\nmultinomial model: ")
nbm1.debug_print()

mnb = nbm1.predict(mat)
print("prediction on multinomial classifier model: ", mnb)
print("accuracy of model: ", nbm1.score(mat,lbl))

# Demo of Bernoulli Naive Bayes
mat = FrovedisCRSMatrix(dtype=np.float64).load("./input/bern.txt")
lbl = np.array([1.0,0.0,1.0,1.0,1.0,0.0,1.0,1.0])
nbm2 = BernoulliNB(alpha=1.0).fit(mat,lbl)
print("\nbernoulli model: ")
nbm2.debug_print()

bnb =  nbm2.predict(mat)
print("prediction on bernoulli classifier model: ", bnb)
print("accuracy of model", nbm2.score(mat,lbl))

# Clean-up
nbm1.release()
nbm2.release()
FrovedisServer.shut_down()

