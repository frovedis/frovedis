#!/usr/bin/env python

from frovedis.exrpc.server import FrovedisServer
from frovedis.mllib.naive_bayes import MultinomialNB, BernoulliNB
import sys
import numpy as np
import pandas as pd

# Objective: testing with dataframe matrix and numpy label 

# initializing the Frovedis server
argvs = sys.argv
argc = len(argvs)
if (argc < 2):
    print 'Please give frovedis_server calling command as the first argument \n(e.g. "mpirun -np 2 /opt/nec/frovedis/ve/bin/frovedis_server")'
    quit()
FrovedisServer.initialize(argvs[1])

mat = pd.DataFrame([[1.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.0],
                    [0.0, 1.0, 0.0, 1.0, 0.0, 1.0, 0.0],
                    [0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 1.0],
                    [1.0, 0.0, 0.0, 1.0, 0.0, 1.0, 0.0]])
lbl = np.array([0.0, 1.0, 1.0, 0.0])

# one-liner prediction
ret1 = MultinomialNB(alpha=1.0).fit(mat,lbl).predict(mat)
ret2 = BernoulliNB(alpha=1.0).fit(mat,lbl).predict(mat)
if (ret1 == lbl).all() and (ret2 == lbl).all(): 
  print("Status: Passed")
else: 
  print("Status: Failed")

FrovedisServer.shut_down()
