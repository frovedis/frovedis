#!/usr/bin/env python

from frovedis.exrpc.server import FrovedisServer
from frovedis.mllib.naive_bayes import MultinomialNB
import sys
import numpy as np
import pandas as pd
import os

# Objective: testing of save API of multinomial model

# initializing the Frovedis server
argvs = sys.argv
argc = len(argvs)
if (argc < 2):
    print 'Please give frovedis_server calling command as the first argument \n(e.g. "mpirun -np 2 -x /opt/nec/nosupport/frovedis/ve/bin/frovedis_server")'
    quit()
FrovedisServer.initialize(argvs[1])

mat = pd.DataFrame([[1, 0, 1, 0, 0, 1, 0],
                    [0, 1, 0, 1, 0, 1, 0],
                    [0, 1, 0, 0, 1, 0, 1],
                    [1, 0, 0, 1, 0, 1, 0]],dtype=np.float64)
lbl = np.array([0, 1, 1, 0],dtype=np.float64)

# fitting input matrix and label on linear nbm object
nbm = MultinomialNB(alpha=1.0).fit(mat,lbl)
os.system("rm -rf ./out/MNBModel") #deleting if any
nbm.save("./out/MNBModel")
try:
   fh = open("./out/MNBModel/label", "rb")
except IOError:
   print "Failed"
else:
   print "Passed"
   fh.close()

os.system("rm -rf ./out/MNBModel") 
nbm.release()
FrovedisServer.shut_down()

