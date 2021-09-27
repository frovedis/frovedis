#!/usr/bin/env python

from frovedis.exrpc.server import FrovedisServer
from frovedis.matrix.crs import FrovedisCRSMatrix
from frovedis.matrix.dvector import FrovedisDoubleDvector
from frovedis.mllib.naive_bayes import MultinomialNB
from frovedis.mllib.naive_bayes import BernoulliNB
import sys
import numpy as np
import pandas as pd

# Objective : When alpha numeric label pass

# initializing the Frovedis server
argvs = sys.argv
argc = len(argvs)
if (argc < 2):
    print 'Please give frovedis_server calling command as the first argument \n(e.g. "mpirun -np 2 /opt/nec/frovedis/ve/bin/frovedis_server")'
    quit()
FrovedisServer.initialize(argvs[1])

mat = pd.DataFrame([[1, 0, 1, 0, 0, 1, 0],
                    [0, 1, 0, 1, 0, 1, 0],
                    [0, 1, 0, 0, 1, 0, 1],
                    [1, 0, 0, 1, 0, 1, 0]],dtype=np.float64)
lbl = np.array([0, "a", 1, 0])

try:
  nbm = MultinomialNB(alpha=1).fit(mat,lbl)
except ValueError, e:
  print("Status: Passed")
else:
  print("Status: Failed")

FrovedisServer.shut_down()

