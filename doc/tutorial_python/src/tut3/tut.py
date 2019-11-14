import os
import numpy as np
from sklearn.datasets import load_breast_cancer

from frovedis.exrpc.server import FrovedisServer # frovedis
from frovedis.mllib.linear_model import LogisticRegression # frovedis
#from sklearn.linear_model import LogisticRegression # sklearn

X, y = load_breast_cancer(return_X_y=True)

C = 10.0
max_iter=10000
solver = "sag"

FrovedisServer.initialize("mpirun -np 4 {}".format(os.environ['FROVEDIS_SERVER'])) # frovedis
clf = LogisticRegression(random_state=0, solver=solver, C=C, max_iter=max_iter).fit(X, y)
y_pred = clf.predict(X)
score = 1.0 * sum(y == y_pred) / len(y)
FrovedisServer.shut_down() # frovedis

print("score: {}".format(score))
