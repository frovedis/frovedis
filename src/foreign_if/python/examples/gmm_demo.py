import sys
import numpy as np
from frovedis.exrpc.server import FrovedisServer
from frovedis.matrix.dense import FrovedisRowmajorMatrix
from frovedis.mllib.mixture import GaussianMixture

# initializing the Frovedis server
argvs = sys.argv
argc = len(argvs)
if (argc < 2):
    print ('Please give frovedis_server calling command as the first argument \n(e.g. "mpirun -np 2 -x /opt/nec/nosupport/frovedis/ve/bin/frovedis_server")')
    quit()
FrovedisServer.initialize(argvs[1])

train_mat = np.loadtxt("./input/gmm_data.txt")

# creating spectral agglomerative object
n_components = 2
gmm_model = GaussianMixture(n_components=n_components)

# fitting the training matrix on gaussian mixture object
print("Fit and Predict:\n", gmm_model.fit_predict(train_mat))

#print(gmm_model.predict(train_mat))

print("Score:\n",gmm_model.score(train_mat))

#Weights of each components
print("Weights:\n",gmm_model.weights_)

#Covariance of each components
print("Covariances:\n",gmm_model.covariances_)

#Mean of each components
print("Means:\n",gmm_model.means_)

#Whether convergence was reached
print("Converged: ", gmm_model.converged_)

#No. of steps required to reach convergence
print("Iterations to converge: ", gmm_model.n_iter_)

#Lower bound value on the log-likelihood
print("Likelihood: ", gmm_model.lower_bound_)

gmm_model.save("./out/MyGmmModel")
gmm_model.release()

#load the saved model
gmm_model.load("./out/MyGmmModel")
print("Prediction on loaded model:\n", gmm_model.predict(train_mat))
gmm_model.release()
FrovedisServer.shut_down()


