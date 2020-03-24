#!/usr/bin/env python

from frovedis.exrpc.server import FrovedisServer
from frovedis.ensemble import GradientBoostingClassifier
from frovedis.ensemble import GradientBoostingRegressor

import sys
import numpy as np
import pandas as pd

# initializing the Frovedis server
argvs = sys.argv
argc = len(argvs)
if (argc < 2):
    print ('Please give frovedis_server calling command as the first argument \n(e.g. "mpirun -np 2 -x /opt/nec/nosupport/frovedis/ve/bin/frovedis_server")')
    quit()
FrovedisServer.initialize(argvs[1])


X = np.array([[0, 0], [1, 1]], dtype= np.float64)
Y = np.array([0, 1])

gbt = GradientBoostingClassifier(n_estimators=2)

print("fitting GradientBoostingClassifier on the data")
gbt = gbt.fit(X,Y)


x_test = np.array([[2., 2.]], dtype=np.float64)
pred = gbt.predict(x_test)

print("predict output for GradientBoostingClassifier:")
print(pred)

print("score for GradientBoostingClassifier:")
print(gbt.score(x_test, [1]))

gbt.save("./out/gbt1")
gbt.release()
gbt.load("./out/gbt1")
gbt.debug_print()
gbt.release()


X2 = np.array([[0, 1], [2, 1]], dtype= np.float64)
Y2 = np.array([7.8, 4.6])

gbt2 = GradientBoostingRegressor(n_estimators=2)
print("fitting GradientBoostingRegressor on the data")
gbt2 = gbt2.fit(X2,Y2)

x_test2 = np.array([[1., 2.]], dtype=np.float64)
pred2 = gbt2.predict(x_test2)

print("predict output for GradientBoostingRegressor:")
print(pred2)


print("score for GradientBoostingRegressor:")
print(gbt2.score(x_test, [1]))

gbt2.save("./out/gbt2")
gbt2.release()
gbt2.load("./out/gbt2")
gbt2.debug_print()
gbt2.release()

FrovedisServer.shut_down()

