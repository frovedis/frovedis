#!/usr/bin/env python

import sys
import numpy as np
np.set_printoptions(threshold=5)

#from sklearn.naive_bayes import MultinomialNB
#from sklearn.naive_bayes import BernoulliNB
from frovedis.mllib.naive_bayes import MultinomialNB
from frovedis.mllib.naive_bayes import BernoulliNB

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

mnb = MultinomialNB(alpha=1.0).fit(mat,lbl)
pred = mnb.predict(mat)
print("prediction on multinomial classifier model: ") 
print(pred)
print("prediction accuracy: %.4f" % (mnb.score(mat,lbl)))

bnb = BernoulliNB(alpha=1.0).fit(mat,lbl)
pred = bnb.predict(mat)
print("prediction on bernoulli classifier model: ") 
print(pred)
print("prediction accuracy: %.4f" % (bnb.score(mat,lbl)))

# Clean-up
FrovedisServer.shut_down()
