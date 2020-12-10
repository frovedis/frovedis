#!/usr/bin/env python

import sys
import numpy as np
from frovedis.exrpc.server import FrovedisServer

# initializing the Frovedis server
argvs = sys.argv
argc = len(argvs)
if (argc < 2):
    print ('Please give frovedis_server calling command as the first argument \n(e.g. "mpirun -np 2 -x /opt/nec/nosupport/frovedis/ve/bin/frovedis_server")')
    quit()
FrovedisServer.initialize(argvs[1])

from sklearn.model_selection import GridSearchCV
def grid_search_cv(obj, param, mat, lab):
    grid = GridSearchCV(obj, param, refit=True)
    grid_result = grid.fit(mat, lab)
    print('Best Score: ', grid_result.best_score_)
    print('Best Params: ', grid_result.best_params_)
    plbl =  grid.predict(mat)
    print('Frovedis accuracy_score with best C: ', grid.score(mat,lab))

from frovedis.matrix.crs import FrovedisCRSMatrix
mat = FrovedisCRSMatrix(dtype=np.float64).load("./input/libSVMFile.txt").to_scipy_matrix()
lbl = np.array([1,0,1,1,1,0,1,1])

from frovedis.mllib.linear_model import *
lr = LogisticRegression(solver='sag')
param = { 'C': [0.001,0.0001] }
grid_search_cv(lr, param, mat, lbl)

lr.release()
FrovedisServer.shut_down()
