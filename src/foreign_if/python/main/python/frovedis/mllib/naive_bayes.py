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
import numbers

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
        self._feature_count = None
        self.n_samples = None
        self._coef = None
        self._intercept = None

    def validate(self):
        """
        NAME: validate
        """
        if self.alpha < 0:
            raise ValueError("alpha should be greater than or equal to 1")

    def check_input(self, X, y, F):
        """checks input X"""
        inp_data = FrovedisLabeledPoint(X, y, \
                   caller = "[" + self.__class__.__name__ + "] fit: ",\
                   encode_label = True, binary_encoder=[0, 1], \
                   dense_kind = 'rowmajor', densify=False)
        X, y, logic = inp_data.get()
        self._classes = inp_data.get_distinct_labels()
        self.n_classes = len(self._classes)
        self.n_features_ = inp_data.numCols()
        self.n_samples = inp_data.numRows()
        self.label_map = logic
        dtype = inp_data.get_dtype()
        itype = inp_data.get_itype()
        dense = inp_data.is_dense()
        return X, y, dtype, itype, dense

    def check_sample_weight(self, sample_weight):
        if sample_weight is None:
            sample_weight = np.array([])
        elif isinstance(sample_weight, numbers.Number):
            sample_weight = np.full(self.n_samples, sample_weight, dtype=np.float64)
        else:
            sample_weight = np.ravel(sample_weight)
            if len(sample_weight) != self.n_samples:
                 raise ValueError("sample_weight.shape == {}, expected {}!"\
                       .format(sample_weight.shape, (self.n_samples,)))
        return np.asarray(sample_weight, dtype = np.float64)

    def check_class_prior(self):
        if self.class_prior == None:
            class_prior = np.array([])
        elif isinstance(self.class_prior, numbers.Number):
            class_prior = np.full(self.n_classes, self.class_prior, dtype=np.float64)
        else:
            class_prior = np.ravel(self.class_prior)
            if len(class_prior) != self.n_classes:
                raise ValueError("Number of priors must match number of" \
                                 " classes.")
        return np.asarray(class_prior, dtype = np.float64)


    # Fit NaiveBayes multinomial classifier according to X (input data),
    # y (Label).
    def fit(self, X, y, sample_weight = None):
        """
        NAME: fit
        """
        self.release()
        self.validate()
        X, y, dtype, itype, dense = self.check_input(X, y, "fit")
        self.__mdtype = dtype
        self.__mid = ModelID.get()
        (host, port) = FrovedisServer.getServerInstance()
        binarize = 0.0 # not applicable for multinomial NB
       
        sample_weight = self.check_sample_weight(sample_weight)
        class_prior = self.check_class_prior()

        rpclib.nb_train(host, port, X.get(), \
                        y.get(), self.alpha, self.fit_prior, \
                        class_prior, len(class_prior), \
                        sample_weight, len(sample_weight), \
                        self.__mid, self.Algo.encode('ascii'), binarize, \
                        self.verbose, dtype, itype, dense)
        
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
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
                self._class_log_prior = np.asarray(clp, dtype=np.float64)
            return self._class_log_prior
        else:
            raise AttributeError("attribute 'class_log_prior_' might have \
                    been released or called before fit")

    @class_log_prior_.setter
    def class_log_prior_(self, val):
        """class_log_prior_ setter"""
        raise AttributeError("attribute 'class_log_prior_' of "
                             "MultinomialNB object is not writable")

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
                dbl_tht = np.asarray(tht, dtype=np.float64)
                if len(dbl_tht) != self.n_classes * self.n_features_:
                    raise RuntimeError("feature_log_prob_: size differs " + \
                                       "in attribute extraction; report bug!")
                self._feature_log_prob = dbl_tht.reshape(self.n_classes, \
                                                        self.n_features_)
            return self._feature_log_prob
        else:
            raise AttributeError("attribute 'feature_log_prob_' might have " \
                                 "been released or called before fit")

    @feature_log_prob_.setter
    def feature_log_prob_(self, val):
        """feature_log_prob_ setter"""
        raise AttributeError("attribute 'feature_log_prob_' of MultinomialNB " \
                             "object is not writable")

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
                self._class_count = np.asarray(ccnt, dtype=np.float64)
            return self._class_count
        else:
            raise AttributeError("attribute 'class_count_' might have been " \
                                 "released or called before fit")

    @class_count_.setter
    def class_count_(self, val):
        """class_count_ setter"""
        raise AttributeError("attribute 'class_count_' of MultinomialNB " \
                             "object is not writable")
    @property
    def classes_(self):
        """classes_ getter"""
        if self.__mid is not None:
            if self._classes is None:
                self._classes = np.sort(list(self.label_map.values()))
            return self._classes
        else:
            raise AttributeError("attribute 'classes_' " \
               "might have been released or called before fit")

    @classes_.setter
    def classes_(self, val):
        """classes_ setter"""
        raise AttributeError(\
            "attribute 'classes_' of MultinomialNB "
            "object is not writable")

    @property
    def feature_count_(self):
        """feature_count_ getter"""
        if self.__mid is not None:
            if self._feature_count is None:
                (host, port) = FrovedisServer.getServerInstance()
                fcp = rpclib.get_feature_count(host, port, self.__mid, \
                      self.__mkind, self.__mdtype)
                excpt = rpclib.check_server_exception()
                if excpt["status"]:
                    raise RuntimeError(excpt["info"])
                self._feature_count = np.asarray(fcp, dtype=np.float64). \
                                      reshape(self.n_classes, self.n_features_)
            return self._feature_count
        else:
            raise AttributeError("attribute 'feature_count_' might have \
                    been released or called before fit")

    @feature_count_.setter
    def feature_count_(self, val):
        """feature_count_ setter"""
        raise AttributeError("attribute 'feature_count_' of "
                             "MultinomialNB object is not writable")

    @property
    def coef_(self):
        """coef_ getter"""
        if self.__mid is not None:
            if self._coef is None:
                self._coef = (self.feature_log_prob_[1:] \
                if self.n_classes == 2 else self.feature_log_prob_)
            return self._coef
        else:
            raise AttributeError("attribute 'coef_' " \
               "might have been released or called before fit")

    @coef_.setter
    def coef_(self, val):
        """coef_ setter"""
        raise AttributeError(\
            "attribute 'coef_' of MultinomialNB "
            "object is not writable")

    @property
    def intercept_(self):
        """intercept_ getter"""
        if self.__mid is not None:
            if self._intercept is None:
                self._intercept = (self.class_log_prior_[1:] \
                if self.n_classes == 2 else self.class_log_prior_)
            return self._intercept
        else:
            raise AttributeError("attribute 'intercept_' " \
               "might have been released or called before fit")

    @intercept_.setter
    def intercept_(self, val):
        """intercept_ setter"""
        raise AttributeError(\
            "attribute 'intercept_' of MultinomialNB "
            "object is not writable")

    # Perform classification on an array of test vectors X.
    def predict(self, X):
        """
        NAME: predict
        """
        if self.__mid is not None:
            frov_pred = GLM.predict( \
                X, self.__mid, self.__mkind, self.__mdtype, False)
            return np.asarray([self.label_map[frov_pred[i]] \
                              for i in range(0, len(frov_pred))])
        else:
            raise ValueError( \
            "predict is called before calling fit, " \
            "or the model is released.\n")

    # Perform classification on an array and return probability estimates
    # for the test vector X.
    def predict_proba(self, X):
        """
        NAME: predict_proba
        """
        if self.__mid is not None:
            proba = GLM.predict(X, self.__mid, self.__mkind, \
                        self.__mdtype, True, self.n_classes)
            n_samples = len(proba) // self.n_classes
            shape = (n_samples, self.n_classes)
            return np.asarray(proba, dtype=np.float64).reshape(shape)
        else:
            raise ValueError(\
            "predict_proba is called before calling fit, "
            "or the model is released.")

    # calculate the mean accuracy on the given test data and labels.
    def score(self, X, y, sample_weight=None):
        """
        NAME: score
        """
        return accuracy_score(y, self.predict(X), sample_weight=sample_weight)

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
        target = open(fname+"/label_map", "rb")
        self.label_map = pickle.load(target)
        target.close()
        self._classes = np.sort(list(self.label_map.values()))
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
            target = open(fname+"/label_map", "wb")
            pickle.dump(self.label_map, target)
            target.close()
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
            self._classes = None
            self._class_log_prior = None
            self._feature_log_prob = None
            self._class_count = None
            self._feature_count = None
            self._coef = None
            self._intercept = None

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
        inp_data = FrovedisLabeledPoint(X, y, \
                   caller = "[" + self.__class__.__name__ + "] fit: ",\
                   encode_label = True, binary_encoder=[0, 1], \
                   dense_kind = 'rowmajor', densify=False)
        X, y, logic = inp_data.get()
        self._classes = inp_data.get_distinct_labels()
        self.n_classes = len(self._classes)
        self.n_features_ = inp_data.numCols()
        self.label_map = logic
        dtype = inp_data.get_dtype()
        itype = inp_data.get_itype()
        dense = inp_data.is_dense()
        self.__mdtype = dtype
        self.__mid = ModelID.get()
        (host, port) = FrovedisServer.getServerInstance()
        rpclib.nb_train(host, port, X.get(), \
                        y.get(), self.alpha, self.__mid, \
                        self.Algo.encode('ascii'), \
                        self.verbose, dtype, itype, dense)
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])

    @property
    def classes_(self):
        """classes_ getter"""
        if self.__mid is not None:
            if self._classes is None:
                self._classes = np.sort(list(self.label_map.values()))
            return self._classes
        else:
            raise AttributeError("attribute 'classes_' " \
               "might have been released or called before fit")

    @classes_.setter
    def classes_(self, val):
        """classes_ setter"""
        raise AttributeError(\
            "attribute 'classes_' of GaussianNB "
            "object is not writable")

    # Perform classification on an array of test vectors X.
    def predict(self, X):
        """
        NAME: predict
        """
        if self.__mid is not None:
            frov_pred = GLM.predict(X, self.__mid, self.__mkind, \
                                    self.__mdtype, False)
            return np.asarray([self.label_map[frov_pred[i]] \
                              for i in range(0, len(frov_pred))])
        else:
            raise ValueError( \
            "predict is called before calling fit, " \
            "or the model is released.")

    # Perform classification on an array and return probability
    # estimates for the test vector X.
    def predict_proba(self, X):
        """
        NAME: predict_proba
        """
        if self.__mid is not None:
            proba = GLM.predict(X, self.__mid, self.__mkind, \
                        self.__mdtype, True, self.n_classes)
            n_samples = len(proba) // self.n_classes
            shape = (n_samples, self.n_classes)
            return np.asarray(proba, dtype=np.float64).reshape(shape)
        else:
            raise ValueError( \
            "predict_proba is called before calling fit, " \
            "or the model is released.")

    # calculate the mean accuracy on the given test data and labels.
    def score(self, X, y, sample_weight=None):
        """
        NAME: score
        """
        return accuracy_score(y, self.predict(X), sample_weight=sample_weight)

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
        target = open(fname+"/label_map", "rb")
        self.label_map = pickle.load(target)
        target.close()
        self._classes = np.sort(list(self.label_map.values()))
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
            target = open(fname+"/label_map", "wb")
            pickle.dump(self.label_map, target)
            target.close()
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
            self._classes = None
            self._class_log_prior = None
            self._feature_log_prob = None
            self._class_count = None

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
        self._feature_count = None
        self._coef = None
        self._intercept = None

    def validate(self):
        """
        NAME: validate
        """
        if self.alpha < 0:
            raise ValueError("alpha should be greater than or equal to 1")

    def check_input(self, X, y, F):
        """checks input X"""
        inp_data = FrovedisLabeledPoint(X, y, \
                   caller = "[" + self.__class__.__name__ + "] fit: ",\
                   encode_label = True, binary_encoder=[0, 1], \
                   dense_kind = 'rowmajor', densify=False)
        X, y, logic = inp_data.get()
        self._classes = inp_data.get_distinct_labels()
        self.n_classes = len(self._classes)
        self.n_features_ = inp_data.numCols()
        self.n_samples = inp_data.numRows()
        self.label_map = logic
        dtype = inp_data.get_dtype()
        itype = inp_data.get_itype()
        dense = inp_data.is_dense()
        return X, y, dtype, itype, dense

    def check_sample_weight(self, sample_weight):
        if sample_weight is None:
            sample_weight = np.array([])
        elif isinstance(sample_weight, numbers.Number):
            sample_weight = np.full(self.n_samples, sample_weight, dtype=np.float64)
        else:
            sample_weight = np.ravel(sample_weight)
            if len(sample_weight) != self.n_samples:
                 raise ValueError("sample_weight.shape == {}, expected {}!"\
                       .format(sample_weight.shape, (self.n_samples,)))
        return np.asarray(sample_weight, dtype = np.float64)

    def check_class_prior(self):
        if self.class_prior == None:
            class_prior = np.array([])
        elif isinstance(self.class_prior, numbers.Number):
            class_prior = np.full(self.n_classes, self.class_prior, dtype=np.float64)
        else:
            class_prior = np.ravel(self.class_prior)
            if len(class_prior) != self.n_classes:
                raise ValueError("Number of priors must match number of" \
                                 " classes.")
        return np.asarray(class_prior, dtype = np.float64)

    # Fit NaiveBayes bernoulli classifier according to X (input data),
    # y (Label).
    def fit(self, X, y, sample_weight = None):
        """
        NAME: fit
        """
        self.validate()
        self.release()
        X, y, dtype, itype, dense = self.check_input(X, y, "fit")
        self.__mid = ModelID.get()
        self.__mdtype = dtype
        (host, port) = FrovedisServer.getServerInstance()

        sample_weight = self.check_sample_weight(sample_weight)
        class_prior= self.check_class_prior()

        rpclib.nb_train(host, port, X.get(), \
                        y.get(), self.alpha, self.fit_prior, \
                        class_prior, len(class_prior), \
                        sample_weight, len(sample_weight), \
                        self.__mid, self.Algo.encode('ascii'), self.binarize, \
                        self.verbose, dtype, itype, dense)
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
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
                self._class_log_prior = np.asarray(clp, dtype=np.float64)
            return self._class_log_prior
        else:
            raise AttributeError("attribute 'class_log_prior_' might have " \
                                 "been released or called before fit")

    @class_log_prior_.setter
    def class_log_prior_(self, val):
        """class_log_prior_ setter"""
        raise AttributeError("attribute 'class_log_prior_' of "
                             "BernoulliNB object is not writable")

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
                dbl_tht = np.asarray(tht, dtype=np.float64)
                if len(dbl_tht) != self.n_classes * self.n_features_:
                    raise RuntimeError("feature_log_prob_: size differs " + \
                                       "in attribute extraction; report bug!")
                self._feature_log_prob = dbl_tht.reshape(self.n_classes, \
                                                        self.n_features_)
            return self._feature_log_prob
        else:
            raise AttributeError("attribute 'feature_log_prob_' might have " \
                                 "been released or called before fit")

    @feature_log_prob_.setter
    def feature_log_prob_(self, val):
        """feature_log_prob_ setter"""
        raise AttributeError("attribute 'feature_log_prob_' of " \
                             "BernoulliNB object is not writable")

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
                self._class_count = np.asarray(ccnt, dtype=np.float64)
            return self._class_count
        else:
            raise AttributeError("attribute 'class_count_' might have been " \
                                 "released or called before fit")

    @class_count_.setter
    def class_count_(self, val):
        """class_count_ setter"""
        raise AttributeError("attribute 'class_count_' of "
                             "BernoulliNB object is not writable")

    @property
    def classes_(self):
        """classes_ getter"""
        if self.__mid is not None:
            if self._classes is None:
                self._classes = np.sort(list(self.label_map.values()))
            return self._classes
        else:
            raise AttributeError("attribute 'classes_' " \
               "might have been released or called before fit")

    @classes_.setter
    def classes_(self, val):
        """classes_ setter"""
        raise AttributeError(\
            "attribute 'classes_' of BernoulliNB "
            "object is not writable")

    @property
    def feature_count_(self):
        """feature_count_ getter"""
        if self.__mid is not None:
            if self._feature_count is None:
                (host, port) = FrovedisServer.getServerInstance()
                fcp = rpclib.get_feature_count(host, port, self.__mid, \
                      self.__mkind, self.__mdtype)
                excpt = rpclib.check_server_exception()
                if excpt["status"]:
                    raise RuntimeError(excpt["info"])
                self._feature_count = np.asarray(fcp, dtype=np.float64). \
                                      reshape(self.n_classes, self.n_features_)
            return self._feature_count
        else:
            raise AttributeError("attribute 'feature_count_' might have \
                    been released or called before fit")

    @feature_count_.setter
    def feature_count_(self, val):
        """feature_count_ setter"""
        raise AttributeError("attribute 'feature_count_' of "
                             "BernoulliNB object is not writable")

    @property
    def coef_(self):
        """coef_ getter"""
        if self.__mid is not None:
            if self._coef is None:
                self._coef = (self.feature_log_prob_[1:] \
                if self.n_classes == 2 else self.feature_log_prob_)
            return self._coef
        else:
            raise AttributeError("attribute 'coef_' " \
               "might have been released or called before fit")

    @coef_.setter
    def coef_(self, val):
        """coef_ setter"""
        raise AttributeError(\
            "attribute 'coef_' of BernoulliNB "
            "object is not writable")

    @property
    def intercept_(self):
        """intercept_ getter"""
        if self.__mid is not None:
            if self._intercept is None:
                self._intercept = (self.class_log_prior_[1:] \
                if self.n_classes == 2 else self.class_log_prior_)
            return self._intercept
        else:
            raise AttributeError("attribute 'intercept_' " \
               "might have been released or called before fit")

    @intercept_.setter
    def intercept_(self, val):
        """intercept_ setter"""
        raise AttributeError(\
            "attribute 'intercept_' of BernoulliNB "
            "object is not writable")

    # Perform classification on an array of test vectors X.
    def predict(self, X):
        """
        NAME: predict
        """
        if self.__mid is not None:
            frov_pred = GLM.predict(X, self.__mid, self.__mkind, \
                               self.__mdtype, False)
            return np.asarray([self.label_map[frov_pred[i]] \
                              for i in range(0, len(frov_pred))])            
        else:
            raise ValueError( \
            "predict is called before calling fit, " \
            "or the model is released.")

    # Perform classification on an array and return probability estimates
    # for the test vector X.
    def predict_proba(self, X):
        """
        NAME: predict_proba
        """
        if self.__mid is not None:
            proba = GLM.predict(X, self.__mid, self.__mkind, \
                self.__mdtype, True, self.n_classes)
            n_samples = len(proba) // self.n_classes
            shape = (n_samples, self.n_classes)
            return np.asarray(proba, dtype=np.float64).reshape(shape)
        else:
            raise ValueError( \
            "predict_proba is called before calling fit, "
            "or the model is released.")

    # calculate the mean accuracy on the given test data and labels.
    def score(self, X, y, sample_weight=None):
        """
        NAME: score
        """
        return accuracy_score(y, self.predict(X), sample_weight=sample_weight)

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
        target = open(fname+"/label_map", "rb")
        self.label_map = pickle.load(target)
        target.close()
        self._classes = np.sort(list(self.label_map.values()))
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
            target = open(fname+"/label_map", "wb")
            pickle.dump(self.label_map, target)
            target.close()
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
            self._classes = None
            self._class_log_prior = None
            self._feature_log_prob = None
            self._class_count = None
            self._feature_count = None
            self._coef = None
            self._intercept = None

    # Check FrovedisServer is up then release
    def __del__(self):
        """
        NAME: __del__
        """
        if FrovedisServer.isUP():
            self.release()
