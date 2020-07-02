#!/usr/bin/env python

from frovedis.exrpc.server import FrovedisServer
import numpy as np
import sys

# initializing the Frovedis server
argvs = sys.argv
argc = len(argvs)
if (argc < 2):
    print ('Please give frovedis_server calling command as the first argument \n(e.g. "mpirun -np 2 -x /opt/nec/nosupport/frovedis/ve/bin/frovedis_server")')
    quit()
FrovedisServer.initialize(argvs[1])


from sklearn.datasets import load_boston
mat, label = load_boston(return_X_y=True)


from frovedis.mllib.linear_model import LinearRegression
lr = LinearRegression().fit(mat, label)

print("predicting on trained model: ")
print(lr.predict(mat[:10]))
print("score: %.2f" % lr.score(mat[:10], label[:10]))
print("coef: ")
print(lr.coef_)

# when solver = lapack, following outputs can also be obtained
#print("singular: ")
#print(lr.singular_)
#print("rank: %d" % lr.rank_)

# saving the model
lr.save("./out/LNRModel")

# loading the same model
lr.load("./out/LNRModel")

# debug_print
lr.debug_print()

lr.release()
FrovedisServer.shut_down()
