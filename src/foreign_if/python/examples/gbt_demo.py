#!/usr/bin/env python

import sys
import numpy as np
import pandas as pd
from frovedis.exrpc.server import FrovedisServer

from frovedis.mllib.ensemble import GradientBoostingClassifier
from frovedis.mllib.ensemble import GradientBoostingRegressor
#from sklearn.ensemble import GradientBoostingClassifier
#from sklearn.ensemble import GradientBoostingRegressor

# initializing the Frovedis server
argvs = sys.argv
argc = len(argvs)
if (argc < 2):
    print ('Please give frovedis_server calling command as the first argument \n(e.g. "mpirun -np 2 -x /opt/nec/nosupport/frovedis/ve/bin/frovedis_server")')
    quit()
FrovedisServer.initialize(argvs[1])


mat = np.array([[10, 0, 1, 0, 0, 1, 0],
                [0, 1, 0, 1, 0, 1, 0],
                [0, 1, 0, 0, 1, 0, 1],
                [1, 0, 0, 1, 0, 1, 0]])
lbl = np.array([1, 0, 1, 0])

gbt = GradientBoostingClassifier(n_estimators=2)
print("fitting GradientBoostingClassifier on the data")
gbt = gbt.fit(mat, lbl)

print("predict output for GradientBoostingClassifier:")
pred = gbt.predict(mat)
print(pred)

print("score for GradientBoostingClassifier: %.3f" % gbt.score(mat, lbl))

gbt.save("./out/gbt1")
gbt.release()
gbt.load("./out/gbt1")
gbt.debug_print()
gbt.release()

gbt2 = GradientBoostingRegressor(n_estimators=2)
print("fitting GradientBoostingRegressor on the data")
lbl2 = np.array([1.2,0.3,1.1,1.9])
gbt2 = gbt2.fit(mat, lbl2)

print("predict output for GradientBoostingRegressor:")
pred2 = gbt2.predict(mat)
print(pred2)

print("score for GradientBoostingRegressor: %.3f" % gbt2.score(mat, lbl2))

gbt2.save("./out/gbt2")
gbt2.release()
gbt2.load("./out/gbt2")
gbt2.debug_print()
gbt2.release()

FrovedisServer.shut_down()
