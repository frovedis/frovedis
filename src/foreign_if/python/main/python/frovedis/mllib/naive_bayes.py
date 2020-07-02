"""
naive_bayes.py: wrapper of multinomial and bernoulli naive bayes
"""

#!/usr/bin/env python

import os.path
import pickle
from .model_util import *
from ..base import *
from ..exrpc import rpclib
from ..exrpc.server import FrovedisServer
from ..matrix.ml_data import FrovedisLabeledPoint
from ..matrix.dtype import TypeUtil
from .metrics import *
import numpy as np

# NaiveBayes multinomial class
class MultinomialNB(BaseEstimator):
    """A python wrapper of Frovedis Multinomial Naive Bayes
    parameter      :    default value
    alpha (lambda) :    1.0
    fit_prior      :    True
    class_prior    :    None
    verbose        :    0
    """

    # defaults are as per Frovedis/scikit-learn
    # NaiveBayes multinomial constructor
    def __init__(self, alpha=1.0, fit_prior=True, class_prior=None, verbose=0):
        self.alpha = alpha
        self.fit_prior = fit_prior
        self.class_prior = class_prior
        self.verbose = verbose
        # extra
        self.__mid = None
        self.__mdtype = None
        self.__mkind = M_KIND.NBM
        self.Algo = "multinomial"
        self._feature_log_prob = None
        self._class_log_prior = None
        self._class_count = None

    def validate(self):
        """
        NAME: validate
        """
        if self.alpha < 0:
            raise ValueError("alpha should be greater than or equal to 1")

    # Fit NaiveBayes multinomial classifier according to X (input data),
    # y (Label).
    def fit(self, X, y):
        """
        NAME: fit
        """
        self.validate()
        self.release()
        self.__mid = ModelID.get()
        inp_data = FrovedisLabeledPoint(X, y)
        (X, y) = inp_data.get()
        (data, label) = inp_data.get()
        dtype = inp_data.get_dtype()
        itype = inp_data.get_itype()
        dense = inp_data.is_dense()
        self.__mdtype = dtype
        (host, port) = FrovedisServer.getServerInstance()
        rpclib.nb_train(host, port, data.get(),
                        label.get(), self.alpha, self.__mid,
                        self.Algo.encode('ascii'), self.verbose, dtype, \
                        itype, dense)
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        self._class_log_prior = None
        self._feature_log_prob = None
        self._class_count = None
        return self

    @property
    def class_log_prior_(self):
        """class_log_prior_ getter"""
        if self.__mid is not None:
            if self._class_log_prior is None:
                (host, port) = FrovedisServer.getServerInstance()
                clp = rpclib.get_pi_vector(host, port, self.__mid, \
                      self.__mkind, self.__mdtype)
                excpt = rpclib.check_server_exception()
                if excpt["status"]:
                    raise RuntimeError(excpt["info"])
                self._class_log_prior = np.asarray(clp)
            return self._class_log_prior
        else:
            raise AttributeError("attribute 'class_log_prior_' might have \
                    been released or called before fit")

    @class_log_prior_.setter
    def class_log_prior_(self, val):
        """class_log_prior_ setter"""
        raise AttributeError("attribute 'class_log_prior_' of Multinomial \
                    NaiveBayes object is not writable")

    @property
    def feature_log_prob_(self):
        """feature_log_prob_ getter"""
        if self.__mid is not None:
            if self._feature_log_prob is None:
                (host, port) = FrovedisServer.getServerInstance()
                tht = rpclib.get_theta_vector(host, port, self.__mid, \
                    self.__mkind, self.__mdtype)
                excpt = rpclib.check_server_exception()
                if excpt["status"]:
                    raise RuntimeError(excpt["info"])
                self._feature_log_prob = np.asarray(tht)
            return self._feature_log_prob
        else:
            raise AttributeError("attribute 'feature_log_prob_' might have \
                    been released or called before fit")

    @feature_log_prob_.setter
    def feature_log_prob_(self, val):
        """feature_log_prob_ setter"""
        raise AttributeError("attribute 'feature_log_prob_' of Multinomial \
                    NaiveBayes object is not writable")

    @property
    def class_count_(self):
        """class_count_ getter"""
        if self.__mid is not None:
            if self._class_count is None:
                (host, port) = FrovedisServer.getServerInstance()
                ccnt = rpclib.get_cls_counts_vector(host, port, self.__mid, \
                    self.__mkind, self.__mdtype)
                excpt = rpclib.check_server_exception()
                if excpt["status"]:
                    raise RuntimeError(excpt["info"])
                self._class_count = np.asarray(ccnt)
            return self._class_count
        else:
            raise AttributeError("attribute 'class_count_' might have been \
                    released or called before fit")

    @class_count_.setter
    def class_count_(self, val):
        """class_count_ setter"""
        raise AttributeError("attribute 'class_count_' of Multinomial \
                    NaiveBayes object is not writable")

    # Perform classification on an array of test vectors X.
    def predict(self, X):
        """
        NAME: predict
        """
        if self.__mid is not None:
            pred = GLM.predict( \
                X, self.__mid, self.__mkind, self.__mdtype, False)
            return np.asarray(pred, dtype=np.int64)
        else:
            raise ValueError( \
            "predict is called before calling fit, \
            or the model is released.\n")

    # Perform classification on an array and return probability estimates
    # for the test vector X.
    def predict_proba(self, X):
        """
        NAME: predict_proba
        """
        if self.__mid is not None:
            proba = GLM.predict(X, self.__mid, self.__mkind, \
                        self.__mdtype, True)
            return np.asarray(proba)
        else:
            raise ValueError(\
            "predict is called before calling fit, or the model is released.")

    # calculate the mean accuracy on the given test data and labels.
    def score(self, X, y):
        """
        NAME: score
        """
        if self.__mid is not None:
            return accuracy_score(y, self.predict(X))

    # Show the model
    def debug_print(self):
        """
        NAME: debug_print
        """
        if self.__mid is not None:
            GLM.debug_print(self.__mid, self.__mkind, self.__mdtype)

    # Load the model from a file
    def load(self, fname, dtype=None):
        """
        NAME: load
        """
        if isinstance(fname, str) == False:
            raise TypeError("Expected: String, Got: " + str(type(fname)))
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
                                 "expected type: " + mdt + \
                                 "; given type: " + dtype)
        self.__mid = ModelID.get()
        (host, port) = FrovedisServer.getServerInstance()
        model_file = fname + "/model"
        algo = rpclib.load_frovedis_nbm(host, port, self.__mid, \
                    self.__mdtype, model_file.encode('ascii'))
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        if algo != self.Algo:
            msg = "incorrect data file is loaded! "
            msg += "expected model type: " + \
                    self.Algo + ", loaded model type: " + algo
            raise ValueError(msg)
        return self

    # Save model to a file
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
            GLM.save(self.__mid, self.__mkind, self.__mdtype, fname+"/model")
            metadata = open(fname+"/metadata", "wb")
            pickle.dump((self.__mkind, self.__mdtype), metadata)
            metadata.close()
        else:
            raise ValueError("save: the requested model might have been \
                    released!")

    # Release the model-id to generate new model-id
    def release(self):
        """
        NAME: release
        """
        if self.__mid is not None:
            GLM.release(self.__mid, self.__mkind, self.__mdtype)
            self.__mid = None

    # Check FrovedisServer is up then release
    def __del__(self):
        """
        NAME: __del__
        """
        if FrovedisServer.isUP():
            self.release()

# Not implemented yet in Frovedis
# NaiveBayes Gaussian class
class GaussianNB(BaseEstimator):
    """A python wrapper of Frovedis Gaussian Naive Bayes
    parameter      :    default value
    alpha (lambda) :    1.0
    priors         :    None
    verbose        :    0
    """
    # defaults are as per Frovedis/scikit-learn
    # NaiveBayes Gaussian constructor
    def __init__(self, priors=None, verbose=0):
        self.priors = priors
        # extra
        self.alpha = 1.0
        self.__mid = None
        self.__mdtype = None
        self.__mkind = M_KIND.NBM
        self.Algo = "gaussian"
        self.verbose = verbose

    def validate(self):
        """
        NAME: validate
        """
        if self.alpha < 0:
            raise ValueError("alpha should be greater than or equal to 1")

    # Fit Gaussian NaiveBayes classifier according to X (input data), y (Label)
    def fit(self, X, y):
        """
        NAME: fit
        """
        self.validate()
        self.release()
        self.__mid = ModelID.get()
        inp_data = FrovedisLabeledPoint(X, y)
        (data, label) = inp_data.get()
        dtype = inp_data.get_dtype()
        itype = inp_data.get_itype()
        dense = inp_data.is_dense()
        self.__mdtype = dtype
        (host, port) = FrovedisServer.getServerInstance()
        rpclib.nb_train(host, port, data.get(),
                        label.get(), self.alpha, self.__mid,
                        self.Algo.encode('ascii'), \
                        self.verbose, dtype, itype, dense)
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])

    # Perform classification on an array of test vectors X.
    def predict(self, X):
        """
        NAME: predict
        """
        if self.__mid is not None:
            pred = GLM.predict(X, self.__mid, self.__mkind, self.__mdtype, \
                               False)
            return np.asarray(pred, dtype=np.int64)
        else:
            raise ValueError( \
            "predict is called before calling fit, \
            or the model is released.")

    # Perform classification on an array and return probability
    # estimates for the test vector X.
    def predict_proba(self, X):
        """
        NAME: predict_proba
        """
        if self.__mid is not None:
            proba = GLM.predict(X, self.__mid, self.__mkind, \
                        self.__mdtype, True)
            return np.asarray(proba)
        else:
            raise ValueError( \
            "predict is called before calling fit, \
            or the model is released.")

    # calculate the mean accuracy on the given test data and labels.
    def score(self, X, y):
        """
        NAME: score
        """
        if self.__mid is not None:
            return accuracy_score(y, self.predict(X))

    # Show the model
    def debug_print(self):
        """
        NAME: debug_print
        """
        if self.__mid is not None:
            GLM.debug_print(self.__mid, self.__mkind, self.__mdtype)

    # Load the model from a file
    def load(self, fname, dtype=None):
        """
        NAME: load
        """
        if isinstance(fname, str) == False:
            raise TypeError("Expected: String, Got: " + str(type(fname)))
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
                                 "expected type: " + mdt + \
                                 "; given type: " + dtype)
        self.__mid = ModelID.get()
        (host, port) = FrovedisServer.getServerInstance()
        model_file = fname + "/model"
        algo = rpclib.load_frovedis_nbm( \
            host, port, self.__mid, self.__mdtype, model_file.encode('ascii'))
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        if algo != self.Algo:
            msg = "incorrect data file is loaded! "
            msg += "expected model type: " + \
                    self.Algo + ", loaded model type: " + algo
            raise ValueError(msg)
        return self

    # Save model to a file
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
            GLM.save(self.__mid, self.__mkind, self.__mdtype, fname+"/model")
            metadata = open(fname+"/metadata", "wb")
            pickle.dump((self.__mkind, self.__mdtype), metadata)
            metadata.close()
        else:
            raise ValueError(\
                "save: the requested model might have been released!")

    # Release the model-id to generate new model-id
    def release(self):
        """
        NAME: release
        """
        if self.__mid is not None:
            GLM.release(self.__mid, self.__mkind, self.__mdtype)
            self.__mid = None

    # Check FrovedisServer is up then release
    def __del__(self):
        """
        NAME: __del__
        """
        if FrovedisServer.isUP():
            self.release()

# NaiveBayes Bernoulli class
class BernoulliNB(BaseEstimator):
    """A python wrapper of Frovedis Bernouli Naive Bayes
    parameter      :    default value
    alpha (lambda) :    1.0
    fit_prior      :    True
    class_prior    :    None
    binarize       :    0.0
    verbose        :    0
    """
    # defaults are as per Frovedis/scikit-learn
    # NaiveBayes Bernoulli constructor
    def __init__(self, alpha=1.0, fit_prior=True, class_prior=None, \
                binarize=0.0, verbose=0):
        self.alpha = alpha
        self.fit_prior = fit_prior
        self.class_prior = class_prior
        self.verbose = verbose
        self.binarize = binarize
        # extra
        self.__mid = None
        self.__mdtype = None
        self.__mkind = M_KIND.NBM
        self.Algo = "bernoulli"
        self._feature_log_prob = None
        self._class_log_prior = None
        self._class_count = None

    def validate(self):
        """
        NAME: validate
        """
        if self.alpha < 0:
            raise ValueError("alpha should be greater than or equal to 1")

    # Fit NaiveBayes multinomial classifier according to X (input data),
    # y (Label).
    def fit(self, X, y):
        """
        NAME: fit
        """
        self.validate()
        self.release()
        self.__mid = ModelID.get()
        inp_data = FrovedisLabeledPoint(X, y)
        (data, label) = inp_data.get()
        dtype = inp_data.get_dtype()
        itype = inp_data.get_itype()
        dense = inp_data.is_dense()
        self.__mdtype = dtype
        (host, port) = FrovedisServer.getServerInstance()
        rpclib.nb_train(host, port, data.get(),
                        label.get(), self.alpha, self.__mid,
                        self.Algo.encode('ascii'), self.verbose, dtype, itype, \
                                        dense)
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        self._class_log_prior = None
        self._feature_log_prob = None
        self._class_count = None
        return self

    @property
    def class_log_prior_(self):
        """class_log_prior_ getter"""
        if self.__mid is not None:
            if self._class_log_prior is None:
                (host, port) = FrovedisServer.getServerInstance()
                clp = rpclib.get_pi_vector(host, port, self.__mid, \
                    self.__mkind, self.__mdtype)
                excpt = rpclib.check_server_exception()
                if excpt["status"]:
                    raise RuntimeError(excpt["info"])
                self._class_log_prior = np.asarray(clp)
            return self._class_log_prior
        else:
            raise AttributeError("attribute 'class_log_prior_' might have \
                    been released or called before fit")

    @class_log_prior_.setter
    def class_log_prior_(self, val):
        """class_log_prior_ setter"""
        raise AttributeError("attribute 'class_log_prior_' of Multinomial \
                    NaiveBayes object is not writable")


    # Perform classification on an array of test vectors X.
    def predict(self, X):
        """
        NAME: predict
        """
        if self.__mid is not None:
            pred = GLM.predict(X, self.__mid, self.__mkind, \
                               self.__mdtype, False)
            return np.asarray(pred, dtype=np.int64)
        else:
            raise ValueError( \
            "predict is called before calling fit, \
             or the model is released.")

    @property
    def feature_log_prob_(self):
        """feature_log_prob_ getter"""
        if self.__mid is not None:
            if self._feature_log_prob is None:
                (host, port) = FrovedisServer.getServerInstance()
                tht = rpclib.get_theta_vector(host, port, self.__mid, \
                    self.__mkind, self.__mdtype)
                excpt = rpclib.check_server_exception()
                if excpt["status"]:
                    raise RuntimeError(excpt["info"])
                self._feature_log_prob = np.asarray(tht)
            return self._feature_log_prob
        else:
            raise AttributeError("attribute 'feature_log_prob_' might have \
                    been released or called before fit")

    @feature_log_prob_.setter
    def feature_log_prob_(self, val):
        """feature_log_prob_ setter"""
        raise AttributeError("attribute 'feature_log_prob_' of Multinomial \
                    NaiveBayes object is not writable")

    @property
    def class_count_(self):
        """class_count_ getter"""
        if self.__mid is not None:
            if self._class_count is None:
                (host, port) = FrovedisServer.getServerInstance()
                ccnt = rpclib.get_cls_counts_vector(host, port, self.__mid, \
                    self.__mkind, self.__mdtype)
                excpt = rpclib.check_server_exception()
                if excpt["status"]:
                    raise RuntimeError(excpt["info"])
                self._class_count = np.asarray(ccnt)
            return self._class_count
        else:
            raise AttributeError("attribute 'class_count_' might have been \
                    released or called before fit")

    @class_count_.setter
    def class_count_(self, val):
        """class_count_ setter"""
        raise AttributeError("attribute 'class_count_' of Multinomial \
                    NaiveBayes object is not writable")

    # Perform classification on an array and return probability estimates
    # for the test vector X.
    def predict_proba(self, X):
        """
        NAME: predict_proba
        """
        if self.__mid is not None:
            proba = GLM.predict(X, self.__mid, self.__mkind, \
                self.__mdtype, True)
            return np.asarray(proba)
        else:
            raise ValueError( \
            "predict is called before calling fit, or the model is released.")

    # calculate the mean accuracy on the given test data and labels.
    def score(self, X, y):
        """
        NAME: score
        """
        if self.__mid is not None:
            return accuracy_score(y, self.predict(X))

    # Show the model
    def debug_print(self):
        """
        NAME: debug_print
        """
        if self.__mid is not None:
            GLM.debug_print(self.__mid, self.__mkind, self.__mdtype)

    # Load the model from a file
    def load(self, fname, dtype=None):
        """
        NAME: load
        """
        if isinstance(fname, str) == False:
            raise TypeError("Expected: String, Got: " + str(type(fname)))
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
                                 "expected type: " + mdt + \
                                 "; given type: " + dtype)
        self.__mid = ModelID.get()
        (host, port) = FrovedisServer.getServerInstance()
        model_file = fname + "/model"
        algo = rpclib.load_frovedis_nbm(host, port, self.__mid, self.__mdtype, \
                   model_file.encode('ascii'))
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        if algo != self.Algo:
            msg = "incorrect data file is loaded! "
            msg += "expected model type: " + \
                    self.Algo + ", loaded model type: " + algo
            raise ValueError(msg)
        return self

    # Save model to a file
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
            GLM.save(self.__mid, self.__mkind, self.__mdtype, fname+"/model")
            metadata = open(fname+"/metadata", "wb")
            pickle.dump((self.__mkind, self.__mdtype), metadata)
            metadata.close()
        else:
            raise ValueError(\
                "save: the requested model might have been released!")

    # Release the model-id to generate new model-id
    def release(self):
        """
        NAME: release
        """
        if self.__mid is not None:
            GLM.release(self.__mid, self.__mkind, self.__mdtype)
            self.__mid = None

    # Check FrovedisServer is up then release
    def __del__(self):
        """
        NAME: __del__
        """
        if FrovedisServer.isUP():
            self.release()
