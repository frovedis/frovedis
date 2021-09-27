import sys
import numpy as np
from frovedis.exrpc.server import FrovedisServer
from frovedis.matrix.dense import FrovedisRowmajorMatrix
from frovedis.mllib.mixture import GaussianMixture
#from sklearn.mixture import GaussianMixture

# initializing the Frovedis server
argvs = sys.argv
argc = len(argvs)
if (argc < 2):
    print ('Please give frovedis_server calling command as the first argument \n(e.g. "mpirun -np 2 /opt/nec/frovedis/ve/bin/frovedis_server")')
    quit()
FrovedisServer.initialize(argvs[1])

train_mat = np.loadtxt("./input/gmm_data.txt", dtype=np.int32)

# creating spectral agglomerative object
gmm_model = GaussianMixture(n_components=2)

# fitting the training matrix on gaussian mixture object
print("fit and predict on train_mat: ") 
print(gmm_model.fit_predict(train_mat))
#print(gmm_model.predict(train_mat))

print("score_samples: ")
print(gmm_model.score_samples(train_mat))

print("score: %.3f" % (gmm_model.score(train_mat)))
print("aic: %.3f" % (gmm_model.aic(train_mat)))
print("bic: %.3f" % (gmm_model.bic(train_mat)))
print(type(gmm_model.score(train_mat)))

#Weights of each components
print("weights: ")
print(gmm_model.weights_)

#Covariance of each components
print("covariances: ")
print(gmm_model.covariances_)

#Mean of each components
print("means: ")
print(gmm_model.means_)

#Whether convergence was reached
print("converged: ")
print(gmm_model.converged_)

#No. of steps required to reach convergence
print("iterations at which it is converged: %d" % (gmm_model.n_iter_))

#Lower bound value on the log-likelihood
print("likelihood: %.3f" % (gmm_model.lower_bound_))

#save the model contents and release
gmm_model.save("./out/MyGmmModel")
gmm_model.release()

#load the saved model
gmm_model.load("./out/MyGmmModel")
print("display content of loaded model for debug purposes: ")
gmm_model.debug_print()

print("prediction on loaded model: ") 
print(gmm_model.predict(train_mat))

#clean-up
gmm_model.release()
FrovedisServer.shut_down()
