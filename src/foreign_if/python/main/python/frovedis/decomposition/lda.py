"""lda.py"""

#!/usr/bin/env python

from ..exrpc.server import FrovedisServer
from ..exrpc.rpclib import compute_lda_train, compute_lda_transform,\
                           compute_lda_component, \
			   check_server_exception
from ..matrix.dense import FrovedisRowmajorMatrix
from ..matrix.ml_data import FrovedisFeatureData
from ..matrix.dtype import TypeUtil
from ..mllib.model_util import M_KIND, ModelID, GLM
import numpy as np
import pickle
import os

class LatentDirichletAllocation(object):
    """Topic modeling using LDA."""

    def __init__(self, n_components=10, doc_topic_prior=None,
                 topic_word_prior=None, learning_method='batch',
                 learning_decay=.7, learning_offset=10., max_iter=10,
                 batch_size=128, evaluate_every=-1, total_samples=1e6,
                 perp_tol=1e-1, mean_change_tol=1e-3, max_doc_update_iter=100,
                 n_jobs=None, verbose=0, random_state=None,
                 algorithm="original", explore_iter=0):
        """ Frovedis LDA(VLDA) supports either of two sampling techniquies:
            Collapsed Gibbs Sampling and Metropolis Hastings. The default
            sampling algorithm is set as "original" i.e. CGS,
            if Metropolis Hastings is to be used, we are required to set the
            proposal types namely:
            document proposal: "dp" algo,
            word proposal:     "wp" algo,
            cycle proposal:    "cp" algo,
            sparse lda:        "sparse" """
        #sklearn parameters:
        self.n_components = n_components #'num_topic'
        if doc_topic_prior is None:
            self.doc_topic_prior = 1. / self.n_components
        else:
            self.doc_topic_prior = doc_topic_prior #'alpha'
        if topic_word_prior is None:
            self.topic_word_prior = 1. / self.n_components
        else:
            self.topic_word_prior = topic_word_prior #'beta'
        self.learning_method = learning_method
        self.learning_decay = learning_decay
        self.learning_offset = learning_offset
        self.max_iter = max_iter #'train_iter'
        self.batch_size = batch_size
        self.evaluate_every = evaluate_every #'num_eval_cycle'
        self.total_samples = total_samples
        self.perp_tol = perp_tol
        self.mean_change_tol = mean_change_tol
        self.max_doc_update_iter = max_doc_update_iter
        self.n_jobs = n_jobs
        self.verbose = verbose #'trace'
        self.random_state = random_state

        #frovedis related parameters:
        self.__mid = None
        self.__mdtype = None
        self.__mkind = M_KIND.LDA
        self.algorithm = algorithm
        self.explore_iter = explore_iter #number of iteration to explore
                                         #optimal hyperparams
        self._components = None

    def fit(self, X, y=None):
        """Fit LDA model on training data X.
        Input parameters:
            X: This can be a sparse matrix or an array(non-negative dense).
            Since frovedis accepts only crs_matrix, we need to
            convert the input data to FrovedisCRSMatrix.
            X will be of shape([num_topic * sizeof(vocab)])

            y: ignore (as in sklearn)

        Output parameter:
            self: an instance of LDA is returned after
            fitting X on LDA model. """
        self.release()
        self.check_parameters()
        #Frovedis currently supports only crs_matrix of itype=size_t
        X1 = FrovedisFeatureData(X, is_dense=False, itype=np.int64, \
                                 allow_int_dtype=True)
        (host, port) = FrovedisServer.getServerInstance()
        input_a = X1.get() #get crs_matrix
        x_dtype = X1.get_dtype() #get dtype
        x_itype = X1.get_itype() #get itype
        self.__mid = ModelID.get()
        self.__mdtype = x_dtype
        compute_lda_train(host, port, input_a.get(),      \
	                  self.doc_topic_prior,           \
        	          self.topic_word_prior,          \
		          self.n_components,              \
        	          self.max_iter,                  \
		          self.algorithm.encode("ascii"), \
        	          self.explore_iter,              \
			  self.evaluate_every,            \
        	          x_dtype, x_itype,               \
			  self.verbose, self.__mid)
        excpt = check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        return self

    def transform(self, X, return_distribution=True):
        """
	Transform input matrix X according to the trained model.
            X: This can be a sparse matrix or an array(non-negative dense).
            Since frovedis accepts only crs_matrix, we need to
            convert the input data to FrovedisCRSMatrix.
            X will be of shape([num_topic * sizeof(vocab)])
        Output parameter:
            doc_topic_distribution: document topic distribution . """
        if self.__mid is None:
            raise AttributeError("Transform called before fit, or the model is released!")
        self.check_parameters()
        X1 = FrovedisFeatureData(X, is_dense=False, itype=np.int64, \
                                 allow_int_dtype=True)
        input_a = X1.get() #get crs_matrix
        x_dtype = X1.get_dtype() #get dtype
        x_itype = X1.get_itype() #get itype
        x_movable = X1.is_movable()
        if x_dtype != self.__mdtype:
            raise TypeError("transform: type mismatch in fit and transpose data!")
        (host, port) = FrovedisServer.getServerInstance()
        dummy = compute_lda_transform(host, port, input_a.get(),     \
	                             self.doc_topic_prior,           \
        	                     self.topic_word_prior,          \
        	                     self.max_iter,                  \
		                     self.algorithm.encode("ascii"), \
        	                     self.explore_iter,              \
        	                     self.__mid,                     \
				     x_dtype, x_itype)
        excpt = check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        self.ppl = dummy['perplexity']
        self.llh = dummy['likelihood']
        dist_mat = dummy['dist_mat']
        nrow = dummy['nrow']
        ncol = dummy['ncol']
        dtd_mat = {'dptr': dist_mat, 'nrow': nrow, 'ncol': ncol}
        ret = FrovedisRowmajorMatrix(mat=dtd_mat, dtype=np.float64)
        if return_distribution:
            if x_movable:
                return ret.to_numpy_array()
            else:
                return ret

    def perplexity(self, X, sub_sampling=False):
        """
	Calculate approximate perplexity for data X.
    	Input:
    	    X : array-like or sparse matrix,
                Document word matrix.
    	    sub_sampling : bool
    	        Do sub-sampling or not.

        Output:
    	    score : float type
    	            Perplexity score.
        """
        self.transform(X, return_distribution=False)
        return self.ppl

    def score(self, X, y=None):
        """Calculate log-likelihood for data X.
    	Input:
    	    X : array-like or sparse matrix,
    	        Document word matrix.
    	    sub_sampling : bool
    	        Do sub-sampling or not.

        Output:
    	    score : float type
    	            Perplexity score.
        """
        self.transform(X, return_distribution=False)
        return self.llh

    def fit_transform(self, X, y=None):
        """Fit to data, then transform it.
	   Fits transformer to X and y with optional parameters fit_params
           and returns a transformed version of X.
	Input:
            X : numpy array of shape [n_samples, n_features]
		Training set.

            y : optional parameter

	Output:
	    X_new: Transformed array.
	"""
        return self.fit(X).transform(X)

    def check_parameters(self):
        """Check model parameters."""

        supported_algorithms = {'original', 'wp', 'dp', 'cp', 'sparse'}
        if self.algorithm not in supported_algorithms:
            raise ValueError("algorithm: Frovedis doesn't support the \
                              given algorithm!")
        if self.n_components <= 0:
            raise ValueError("check_parameters: Invalid 'n_components' parameter: %r"\
			      % self.n_components)
        if self.total_samples <= 0:
            raise ValueError("check_parameters: Invalid 'total_samples' parameter: %r"\
	                      % self.total_samples)
        if self.learning_offset < 0:
            raise ValueError("check_parameters: Invalid 'learning_offset' parameter: %r"\
	                      % self.learning_offset)
        if self.learning_method not in ("batch", "online"):
            raise ValueError("check_parameters: Invalid 'learning_method' parameter: %r"\
	                      % self.learning_method)

    @property
    def components_(self):
        """components_ getter"""
        if self._components is None:
            (host, port) = FrovedisServer.getServerInstance()
            dmat = compute_lda_component(host, port, self.__mid, self.__mdtype)
            excpt = rpclib.check_server_exception()
            if excpt["status"]:
                raise RuntimeError(excpt["info"])
	    #sklearn expects word-topic distribution, hence taking transpose
	    #of the matrix
            self._components = FrovedisRowmajorMatrix(mat=dmat, \
	                       dtype=np.float64).transpose().to_numpy_array()
        return self._components

    @components_.setter
    def components_(self, val):
        """components_ setter"""
        raise AttributeError(\
            "attribute 'components_' of LatentDirichletAllocation object is not writable")

    def load(self, fname, dtype=None):
        """
        NAME: load
        """
        if not os.path.exists(fname):
            raise ValueError(\
                "the model with name %s does not exist!" % fname)
        self.release()
        metadata = open(fname+"/metadata", "rb")
        self.__mkind, self.__mdtype = pickle.load(metadata)
        metadata.close()
        if dtype is not None:
            mdt = TypeUtil.to_numpy_dtype(self.__mdtype)
            if dtype != mdt:
                raise ValueError("load: type mismatches detected!" + \
                                 "expected type: " + str(mdt) + \
                                 "; given type: " + str(dtype))
        self.__mid = ModelID.get()
        GLM.load(self.__mid, self.__mkind, self.__mdtype, fname)
        return self

    def save(self, fname):
        """
        NAME: save
        """
        if self.__mid is not None:
            if os.path.exists(fname):
                raise ValueError(\
                    "another model with %s name already exists!" % fname)
            else:
                os.makedirs(fname)
            GLM.save(self.__mid, self.__mkind, self.__mdtype, fname)
            metadata = open(fname+"/metadata", "wb")
            pickle.dump(\
            (self.__mkind, self.__mdtype), metadata)
            metadata.close()
        else:
            raise AttributeError(\
                "save: requested model might have been released!")

    def release(self):
        """
        NAME: release for SGD classifier
        """
        if self.__mid is not None:
            GLM.release(self.__mid, self.__mkind, self.__mdtype)
            self.__mid = None
            self.__mdtype = None
            self._components = None
            self.ppl = None
            self.llh = None

    def __del__(self):
        """
        NAME: __del__ for SGD classifier
        """
        if FrovedisServer.isUP():
            self.release()

