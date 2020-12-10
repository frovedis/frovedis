import sys
import time
import numpy as np
from sklearn import preprocessing as pp

def show(msg, X):
    #if sys.version_info[0] < 3:
        print(msg),
        print(X)
    #else:
    #    print(msg) print(X)

class time_spent(object):
    def __init__(self):
        self.stime = time.time()

    def show(self, msg):
        print("%s %.6f sec" % (msg, time.time() - self.stime))
        self.stime = time.time()

if (len(sys.argv) < 2):
  raise AttributeError("Expected at least two arguments!")
m = np.loadtxt(sys.argv[1], dtype=np.float64)
  
t = time_spent()
msumall = np.sum(m)
t.show("np.sum(): ")

msum0 = np.sum(m, axis = 0)
t.show("np.sum(axis = 0): ")

msum1 = np.sum(m, axis = 1)
t.show("np.sum(axis = 1): ")

msqsumall = np.sum(np.square(m))
t.show("np.squared_sum(): ")

msqsum0 = np.sum(np.square(m), axis = 0)
t.show("np.squared_sum(axis = 0): ")

msqsum1 = np.sum(np.square(m), axis = 1)
t.show("np.squared_sum(axis = 1): ")

mmeanall = np.mean(m)
t.show("np.mean(): ")

mmean0 = np.mean(m, axis = 0)
t.show("np.mean(axis = 0): ")

mmean1 = np.mean(m, axis = 1)
t.show("np.mean(axis = 1): ")

binmat = pp.binarize(m, 4)
t.show("np.binarize(): ")

'''
print("-------- results --------")
show("sum: ", msumall)
show("sum (axis = 0): ", msum0)
show("sum (axis = 1): ", msum1)
show("squared sum: ", msqsumall)
show("squared sum (axis = 0): ", msqsum0)
show("squared sum (axis = 1): ", msqsum1)
show("mean: ", mmeanall) 
show("mean (axis = 0): ", mmean0) 
show("mean (axis = 1): ", mmean1) 
show("binarize(thr = 4): ", binmat)
'''
