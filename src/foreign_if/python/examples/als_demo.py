#!/usr/bin/env python

import sys
from frovedis.exrpc.server import FrovedisServer
from frovedis.matrix.crs import FrovedisCRSMatrix
from frovedis.mllib.recommendation import ALS

# initializing the Frovedis server
argvs = sys.argv
argc = len(argvs)
if (argc < 2):
    print ('Please give frovedis_server calling command as the first argument \n(e.g. "mpirun -np 2 -x /opt/nec/nosupport/frovedis/ve/bin/frovedis_server")')
    quit()
FrovedisServer.initialize(argvs[1])

#mat = FrovedisCRSMatrix().load("./input/libSVMFile.txt")
from sklearn.datasets import load_breast_cancer
mat, lbl = load_breast_cancer(return_X_y=True)

# fitting the input matrix on a ALS object
als = ALS(rank=4).fit(mat)

# ids (userId or prodId)  start with 0
print (als.predict([(1,1), (0,1), (2,3), (3,1)]))

# saving the model
als.save("./out/MyMFModel")

# loading the same model
als.load("./out/MyMFModel")

# recommend 2 users for 2nd product
print (als.recommend_users(1,2))

# recommend 2 products for 2nd user
print (als.recommend_products(1,2))

als.release()
FrovedisServer.shut_down()
