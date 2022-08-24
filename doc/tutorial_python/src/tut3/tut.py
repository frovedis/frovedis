import os
from sklearn.datasets import load_breast_cancer

from frovedis.exrpc.server import FrovedisServer # frovedis
from frovedis.mllib.linear_model import LogisticRegression # frovedis
#from sklearn.linear_model import LogisticRegression # sklearn

X, y = load_breast_cancer(return_X_y=True)

FrovedisServer.initialize("mpirun -np 4 {}".format(os.environ['FROVEDIS_SERVER'])) # frovedis
clf = LogisticRegression(random_state=0).fit(X, y)
score = clf.score(X, y)
FrovedisServer.shut_down() # frovedis

print("score: {}".format(score))
