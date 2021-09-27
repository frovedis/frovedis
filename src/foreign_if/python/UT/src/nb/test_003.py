#!/usr/bin/env python

from frovedis.exrpc.server import FrovedisServer
from frovedis.matrix.crs import FrovedisCRSMatrix
from frovedis.matrix.dvector import FrovedisDoubleDvector
from frovedis.mllib.naive_bayes import MultinomialNB
from frovedis.mllib.naive_bayes import BernoulliNB
import sys
import numpy as np
import pandas as pd

# Objective : When data contains special characters 

# initializing the Frovedis server
argvs = sys.argv
argc = len(argvs)
if (argc < 2):
    print 'Please give frovedis_server calling command as the first argument \n(e.g. "mpirun -np 2 /opt/nec/frovedis/ve/bin/frovedis_server")'
    quit()
FrovedisServer.initialize(argvs[1])

mat = pd.read_csv("./input/train_1.csv")
lbl = np.array([1, 2, 3, 4, 5, 6,7,8],dtype=np.float64)

try:
  # fitting input matrix and label on linear nbm object
  nbm = MultinomialNB(alpha=1, fit_prior=True, class_prior=None, verbose = 0).fit(mat,lbl)
except TypeError, e:
  print("Status: Passed")
else:
  print("Status: Failed")
  nbm.release()

FrovedisServer.shut_down()

