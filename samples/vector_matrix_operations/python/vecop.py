import sys
import time
import numpy as np
import scipy
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
vec = np.loadtxt(sys.argv[1], dtype=np.float64)

t = time_spent()
vsum = vec + vec
t.show("np.add: ")

vsub = vec - vec
t.show("np.subtract: ")

vmul = vec * vec
t.show("np.multiply: ")

vdiv = vec / vec
t.show("np.divide: ")

vneg = -vec
t.show("np.negative: ")

vsq = np.square(vec)
t.show("np.square: ")

vreducesum = np.sum(vec)
t.show("np.sum: ")

vmean = np.mean(vec)
t.show("np.mean: ")

vscalsum = np.sum(vec * 2)
t.show("np.scaled_sum: ")

vsqsum = np.sum(np.square(vec))
t.show("np.squared_sum: ")

vnorm = np.linalg.norm(vec)
t.show("np.linalg.norm: ")

vdot = np.dot(vec,  vec)
t.show("np.dot: ")

vssd = np.sum(np.square(vec - (vec * 2)))
t.show("np.ssd: ")

vssmd = np.sum(np.square(vec - np.mean(vec)))
t.show("np.ssmd: ")

vaxpy = 2 * vec + vec
t.show("np.axpy: ")

vsort = np.sort(vec)
t.show("np.sort: ")

vunq, uinv = np.unique(vec, return_inverse=True)
t.show("np.unique: ")

#vbincnt = np.bincount(vec)
#t.show("np.bincount: ")

vlog = np.log(vec)
t.show("np.log: ")

vnlog = -np.log(vec)
t.show("np.negative_log: ")

vexp = np.exp(vec)
t.show("np.exp: ")

vlogsumexp = scipy.special.logsumexp(vec)
t.show("scipy.special.logsumexp: ")

vminid = np.argmin(vec) 
t.show("np.argmin: ")

vmin = np.amin(vec) 
t.show("np.amin: ")

vmaxid = np.argmax(vec) 
t.show("np.argmax: ")

vmax = np.amax(vec) 
t.show("np.amax: ")

vnnz = np.count_nonzero(vec) 
t.show("np.count_nonzero: ")

vbin = pp.binarize(vec.reshape(1,-1), 4) 
t.show("sklearn.preprocessing.binarize: ")

vclip = np.clip(vec, 2, 5) 
t.show("np.clip: ")

vtake = np.take(vec, np.arange(len(vec))) 
t.show("np.take: ")

vcast = vec.astype(dtype=np.float64) 
t.show("np.astype: ")

vzeros = np.zeros(1000, dtype=np.int32)
t.show("np.zeros: ")

vones = np.ones(1000, dtype=np.int32)
t.show("np.ones: ")

vfull = np.full(1000, 5, dtype=np.int32)
t.show("np.full: ")

#'''
print("-------- results --------")
show("data: ", vec)
show("data + data: ", vsum) 
show("data - data: ", vsub)
show("data * data: ", vmul)
show("data / data: ", vdiv)
show("-data: ", vneg)
show("square(data): ", vsq)
show("sum(data): ", vreducesum)
show("mean(data): ", vmean)
show("scaled-sum(data, 2): ", vscalsum)
show("squared_sum(data): ", vsqsum)
show("norm(data): ", vnorm)
show("(dot) data . data: ", vdot)
show("sum-squared-diff(data, 2*data): ", vssd)
show("sum-squared-mean-diff(data): ", vssmd)
show("(axpy) 2 * data + data: ", vaxpy)
show("sort(data): ", vsort)
show("unique(data): ", vunq)
#show("bincount(data): ", vbincnt)
show("log(data): ", vlog)
show("-log(data): ", vnlog)
show("exp(data): ", vexp)
show("logsumexp(data): ", vlogsumexp)
show("argmin(data): ", vminid)
show("argmax(data): ", vmaxid)
show("amin(data): ", vmin)
show("amax(data): ", vmax)
show("count_nonzero(data): ", vnnz)
show("binarize(data, 4): ", vbin)
show("clip(data, 2, 5): ", vclip)
show("take(data[0 : N]): ", vtake)
show("astype<double>(data): ", vcast)
show("zeros(1000): ", vzeros)
show("ones(1000): ", vones)
show("full(1000, 5): ", vfull)
#'''
