#!/usr/bin/env python

from frovedis.exrpc.server import FrovedisServer
from frovedis.matrix.sparse import FrovedisCRSMatrix
from frovedis.matrix.dvector import FrovedisDoubleDvector
from frovedis.mllib.svm import LinearSVC
import sys

# initializing the Frovedis server
argvs = sys.argv
argc = len(argvs)
if (argc < 2):
    print 'Please give frovedis_server calling command as the first argument \n(e.g. "mpirun -np 2 -x /opt/nec/nosupport/frovedis/ve/bin/frovedis_server")'
    quit()
FrovedisServer.initialize(argvs[1])

mat = FrovedisCRSMatrix().load("./input/libSVMFile.txt")
lbl = FrovedisDoubleDvector([1,0,1,1,1,0,1,1])

# fitting input matrix and label on linear svm object
svm = LinearSVC(solver='lbfgs',verbose=0).fit(mat,lbl)

# predicting on loaded model
print("predicting on lbfgs svm regression model: ")
print svm.predict(mat)

# fitting input matrix and label on linear svm object
svm = LinearSVC(solver='sag',verbose=0).fit(mat,lbl)

# predicting on loaded model
print("predicting on sgd svm regression model: ")
print svm.predict(mat)

# saving the model
svm.save("./out/SVMModel")

# loading the same model
svm.load("./out/SVMModel")

# debug_print()
svm.debug_print()
svm.release()

FrovedisServer.shut_down()
