#!/usr/bin/env python

from frovedis.exrpc.server import FrovedisServer
from frovedis.matrix.crs import FrovedisCRSMatrix
from frovedis.matrix.dvector import FrovedisDoubleDvector
from frovedis.mllib.naive_bayes import MultinomialNB
from frovedis.mllib.naive_bayes import BernoulliNB
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

#mat = np.random.randint(5, size=(6, 100))
#lbl = np.array([1, 2, 1, 1, 2, 1])
mat = pd.DataFrame([[1, 0, 1, 0, 0, 1, 0],
                    [0, 1, 0, 1, 0, 1, 0],
                    [0, 1, 0, 0, 1, 0, 1],
                    [1, 0, 0, 1, 0, 1, 0]],dtype=np.float64)
lbl = np.array([0, 1, 1, 0],dtype=np.float64)

nbm = MultinomialNB(alpha=1.0).fit(mat,lbl)
nbm.debug_print()
print("predicting on nbm multinomial classifier model: ")
nbmc = nbm.predict(mat)
print("Accuracy of model: ", nbm.score(mat,lbl))

nbm2 = BernoulliNB(alpha=1.0).fit(mat,lbl)
nbm2.debug_print()
print("predicting on nbm bernoulli classifier model: ")
nbbc = nbm2.predict(mat) 
print("Accuracy of model: ", nbm2.score(mat,lbl))

from sklearn.naive_bayes import MultinomialNB
clb = MultinomialNB(alpha=1.0).fit(mat,lbl)

from sklearn.naive_bayes import BernoulliNB
clf = BernoulliNB(alpha=1.0).fit(mat,lbl)

if (clf.predict(mat) == nbbc).all() and (clb.predict(mat)== nbmc).all():
  print("Status: Passed")
else:
  print("Status: Failed")

nbm.release()
nbm2.release()
FrovedisServer.shut_down()

