"""
svm.py: wrapper of frovedis Linear SVM
"""

import os.path
import pickle
import sys
import numbers
import numpy as np
from ..base import BaseEstimator
from ..exrpc import rpclib
from ..exrpc.server import FrovedisServer, set_association, \
                           check_association, do_if_active_association
from ..matrix.ml_data import FrovedisLabeledPoint
from ..matrix.dtype import TypeUtil
from ..matrix.dense import FrovedisDenseMatrix, FrovedisRowmajorMatrix
from ..matrix.dvector import FrovedisDvector
from ..matrix.crs import FrovedisCRSMatrix
from scipy.sparse import issparse
from .model_util import *
from .metrics import *

class LinearSVC(BaseEstimator):
    """A python wrapper of Frovedis Linear SVM"""
    # defaults are as per Frovedis
    # loss: Frovedis: 'hinge', Sklearn: 'squared_hinge'
    # lr_rate: Frovedis: 0.01 (added)
    # solver: Frovedis: sag (SGD) (added)
    def __init__(self, penalty='l2', loss='hinge', dual=True, tol=1e-4,
                 C=1.0, multi_class='ovr', fit_intercept=True,
                 intercept_scaling=1, class_weight=None, verbose=0,
                 random_state=None, max_iter=1000,
                 lr_rate=0.01, solver='sag', warm_start = False):
        self.penalty = penalty
        self.loss = loss
        self.dual = dual
        self.tol = tol
        self.C = C
        self.multi_class = multi_class
        self.fit_intercept = fit_intercept
        self.intercept_scaling = intercept_scaling
        self.class_weight = class_weight
        self.verbose = verbose
        self.random_state = random_state
        self.max_iter = max_iter
        self.warm_start = warm_start
        # extra
        self.lr_rate = lr_rate
        self.solver = solver
        self.__mid = ModelID.get()
        self.__mdtype = None
        self.__mkind = M_KIND.SVM
        self.n_classes = None
        self._coef = None
        self._intercept = None
        self._classes = None
        self.label_map = None
        self._n_iter = None
        self.n_samples = None
        self.n_features = None
        self.isFitted = None

    def check_input(self, X, y, F):
        """checks input X and y"""
        inp_data = FrovedisLabeledPoint(X, y, \
                   caller = "[" + self.__class__.__name__ + "] " + F + ": ",\
                   encode_label = True, binary_encoder=[-1, 1], \
                   dense_kind = 'colmajor', densify=False)

        X, y, logic = inp_data.get()
        self._classes = inp_data.get_distinct_labels()
        self.n_classes = len(self._classes)
        self.n_samples = inp_data.numRows()
        self.n_features = inp_data.numCols()
        self.label_map = logic
        dtype = inp_data.get_dtype()
        itype = inp_data.get_itype()
        dense = inp_data.is_dense()
        return X, y, dtype, itype, dense

    def check_sample_weight(self, sample_weight):
        """checks sample_weight"""
        if sample_weight is None:
            weight = np.array([], dtype=np.float64)
        elif isinstance(sample_weight, numbers.Number):
            weight = np.full(self.n_samples, sample_weight, dtype=np.float64)
        else:
            weight = np.ravel(sample_weight)
            if len(weight) != self.n_samples:
                raise ValueError("sample_weight.shape == {}, expected {}!"\
                       .format(sample_weight.shape, (self.n_samples,)))
        return np.asarray(weight, dtype=np.float64)

    @set_association
    def fit(self, X, y, sample_weight=None):
        """
        NAME: fit
        """
        if self.loss != 'hinge':
            raise ValueError("fit: frovedis svm supports only hinge loss!")
        if self.C < 0:
            raise ValueError("fit: parameter C must be strictly positive!")
        self.reset_metadata()
        # for binary case: frovedis supports -1 and 1
        X, y, dtype, itype, dense = self.check_input(X, y, "fit")
        self.__mdtype = dtype

        if self.n_classes > 2:
            raise ValueError("fit: frovedis svm does not support multinomial\
                    data currently!")

        if self.penalty == 'l1':
            regTyp = 1
        elif self.penalty == 'l2':
            regTyp = 2
        elif self.penalty == 'none':
            regTyp = 0
        else:
            raise ValueError("Unsupported penalty is provided: ", self.penalty)
        rparam = 1.0 / self.C

        sample_weight = self.check_sample_weight(sample_weight)
        (host, port) = FrovedisServer.getServerInstance()
        supported_solver = ['sgd', 'lbfgs']
        solver = self.solver
        if solver == 'sag':
            solver = 'sgd'
        if solver not in supported_solver:
            raise ValueError( \
            "Unknown solver %s for Linear SVM." % solver)
        if solver == 'lbfgs':
            regTyp = 2 #lbfgs supports only l2 regularization
        n_iter = rpclib.svm(host, port, X.get(), y.get(), \
                        sample_weight, len(sample_weight), \
                        self.max_iter, self.lr_rate, \
                        regTyp, rparam, self.fit_intercept, \
                        self.tol, self.verbose, self.__mid, dtype, \
                        itype, dense, solver.encode('ascii'), self.warm_start)
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        self._coef = None
        self._intercept = None
        self._n_iter = n_iter
        self.isFitted = True
        return self

    @property
    @check_association
    def coef_(self):
        """coef_ getter"""
        if self._coef is None:
            (host, port) = FrovedisServer.getServerInstance()
            wgt = rpclib.get_weight_vector(host, port, self.__mid, \
                self.__mkind, self.__mdtype)
            excpt = rpclib.check_server_exception()
            if excpt["status"]:
                raise RuntimeError(excpt["info"])
            n_features = len(wgt)
            shape = (1, n_features)
            self._coef = np.asarray(wgt).reshape(shape)
        return self._coef

    @coef_.setter
    def coef_(self, val):
        """coef_ setter"""
        raise AttributeError(\
            "attribute 'coef_' of LinearSVC object is not writable")

    @property
    @check_association
    def intercept_(self):
        """intercept_ getter"""
        if self._intercept is None:
            (host, port) = FrovedisServer.getServerInstance()
            icpt = rpclib.get_intercept_vector(host, port, self.__mid, \
                self.__mkind, self.__mdtype)
            excpt = rpclib.check_server_exception()
            if excpt["status"]:
                raise RuntimeError(excpt["info"])
            self._intercept = np.asarray(icpt)
        return self._intercept

    @intercept_.setter
    def intercept_(self, val):
        """intercept_ setter"""
        raise AttributeError(\
            "attribute 'intercept_' of LinearSVC object is not writable")

    @property
    def classes_(self):
        """classes_ getter"""
        if not self.is_fitted():
            raise AttributeError(\
            "attribute 'classes_' might have been released or called before fit")
        if self._classes is None:
            self._classes = np.sort(list(self.label_map.values()))
        return self._classes

    @classes_.setter
    def classes_(self, val):
        """classes_ setter"""
        raise AttributeError(\
        "attribute 'classes_' of LinearSVC object is not writable")

    @check_association
    def predict(self, X):
        """
        NAME: predict
        """
        frov_pred = GLM.predict(X, self.__mid, self.__mkind, \
                                self.__mdtype, False)
        return np.asarray([self.label_map[frov_pred[i]] \
                          for i in range(0, len(frov_pred))])

    @property
    def n_iter_(self):
        """n_iter_ getter"""
        if not self.is_fitted():
            raise AttributeError("attribute 'n_iter_'" \
               "might have been released or called before fit")
        return self._n_iter

    @n_iter_.setter
    def n_iter_(self, val):
        """n_iter_ setter"""
        raise AttributeError(\
            "attribute 'n_iter_' of LinearSVC object is not writable")

    @set_association
    def load(self, fname, dtype=None):
        """
        NAME: load
        """
        if not os.path.exists(fname):
            raise ValueError(\
                "the model with name %s does not exist!" % fname)
        self.reset_metadata()
        target = open(fname + "/label_map", "rb")
        self.label_map = pickle.load(target)
        target.close()
        self._classes = np.sort(list(self.label_map.values()))
        metadata = open(fname+"/metadata", "rb")
        self.n_classes, self.__mkind, self.__mdtype = pickle.load(metadata)
        metadata.close()
        if dtype is not None:
            mdt = TypeUtil.to_numpy_dtype(self.__mdtype)
            if dtype != mdt:
                raise ValueError("load: type mismatches detected!" + \
                                 "expected type: " + str(mdt) + \
                                 "; given type: " + str(dtype))
        GLM.load(self.__mid, self.__mkind, self.__mdtype, fname + "/model")
        self.isFitted = True
        return self

    @check_association
    def save(self, fname):
        """
        NAME: save
        """
        if os.path.exists(fname):
            raise ValueError(\
                "another model with %s name already exists!" % fname)
        os.makedirs(fname)
        GLM.save(self.__mid, self.__mkind, self.__mdtype, fname + "/model")
        target = open(fname + "/label_map", "wb")
        pickle.dump(self.label_map, target)
        target.close()
        metadata = open(fname + "/metadata", "wb")
        pickle.dump((self.n_classes, \
                self.__mkind, self.__mdtype), metadata)
        metadata.close()

    def score(self, X, y, sample_weight=None):
        """
        NAME: score
        """
        return accuracy_score(y, self.predict(X), sample_weight=sample_weight)

    @check_association
    def debug_print(self):
        """
        NAME: debug_print
        """
        GLM.debug_print(self.__mid, self.__mkind, self.__mdtype)

    def reset_metadata(self):
        """
        resets after-fit populated attributes to None
        """
        self._coef = None
        self._intercept = None
        self.label_map = None
        self._classes = None
        self._n_iter = None
        self.n_samples = None
        self.isFitted = None
        self.n_features = None

    def release(self):
        """
        resets after-fit populated attributes to None
        """
        self.__release_server_heap()
        self.reset_metadata()

    @do_if_active_association
    def __release_server_heap(self):
        """
        to release model pointer from server heap
        """
        GLM.release(self.__mid, self.__mkind, self.__mdtype)

    def __del__(self):
        """
        NAME: __del__
        """
        self.reset_metadata()

    def is_fitted(self):
        """ function to confirm if the model is already fitted """
        return self.isFitted

class SVC(BaseEstimator):
    """A python wrapper of Frovedis SVM Kernel Classifier"""
    # defaults are as per Frovedis
    # cache_size-> Sklearn:200, Frovedis:128

    def __init__(self, C=1.0, kernel='rbf', degree=3, gamma='scale', coef0=0.0,
                 shrinking=True, probability=False, tol=0.001, cache_size=128,
                 class_weight=None, verbose=False, max_iter=-1,
                 decision_function_shape='ovr', break_ties=False,
                 random_state=None):
        self.C = C
        self.kernel = kernel
        self.degree = degree
        self.gamma = gamma
        self.coef0 = coef0
        self.shrinking = shrinking
        self.probability = probability
        self.tol = tol
        self.cache_size = cache_size
        self.class_weight = class_weight
        self.verbose = verbose
        self.max_iter = max_iter
        self.decision_function_shape = decision_function_shape
        self.break_ties = break_ties
        self.random_state = random_state
        # extra
        self.__mid = None
        self.__mdtype = None
        if self.kernel == 'linear':
            self.__mkind = M_KIND.SVM
        else:
            self.__mkind = M_KIND.KSVC
        self._coef = None
        self._intercept = None
        self._support = None
        self._support_vectors = None
        self._classes = None
        self.label_map = None
        self.n_classes = None
        self.n_features = None
        self.n_samples = None
        self._n_iter = None

    def check_input(self, X, y, F, dense_kind):
        """checks input X"""
        inp_data = FrovedisLabeledPoint(X, y, \
                   caller = "[" + self.__class__.__name__ + "] " + F + ": ",\
                   encode_label = True, binary_encoder=[-1, 1], \
                   dense_kind = dense_kind, densify=False)

        X, y, logic = inp_data.get()
        self._classes = inp_data.get_distinct_labels()
        self.n_classes = len(self._classes)
        self.n_samples = inp_data.numRows()
        self.n_features = inp_data.numCols()
        self.label_map = logic
        dtype = inp_data.get_dtype()
        itype = inp_data.get_itype()
        dense = inp_data.is_dense()
        return X, y, dtype, itype, dense

    def check_sample_weight(self, sample_weight):
        """checks input X and y"""
        if sample_weight is None:
            weight = np.array([], dtype=np.float64)
        elif isinstance(sample_weight, numbers.Number):
            weight = np.full(self.n_samples, sample_weight, dtype=np.float64)
        else:
            weight = np.ravel(sample_weight)
            if len(weight) != self.n_samples:
                raise ValueError("sample_weight.shape == {}, expected {}!"\
                       .format(sample_weight.shape, (self.n_samples,)))
        return np.asarray(weight, dtype=np.float64)


    def validate(self):
        """
        NAME: validate
        """
        if self.C < 0:
            raise ValueError("validate: parameter C must be strictly positive!")

        supported_kernel = ("linear", "poly", "rbf", "sigmoid")
        if self.kernel not in supported_kernel:
            raise ValueError("Unsupported kernel for frovedis SVM Classifier:"
                             + "'{}'".format(self.kernel))

        if self.degree < 0:
            raise ValueError("validate: degree must be strictly positive!")

        if self.tol < 0:
            raise ValueError("validate: tol must be strictly positive!")

        if self.cache_size <= 2:
            raise ValueError("validate: cache_size must be greater than 2!")
        if self.max_iter == -1:
            self.max_iter_ = np.iinfo(np.int32).max
        elif self.max_iter <= 0:
            raise ValueError("validate: max_iter can either be -1 or positive!")
        else:
            self.max_iter_ = self.max_iter

    @set_association
    def fit(self, X, y, sample_weight=None):
        """
        NAME: fit
        """
        self.release()
        self.validate()

        if not isinstance(X, (FrovedisDenseMatrix, FrovedisCRSMatrix)) \
           and not issparse(X):
            self.xvar = np.var(X)
        else:
            self.xvar = None

        # for binary case: frovedis supports -1 and 1
        if self.kernel == 'linear':
            X, y, dtype, itype, dense = self.check_input(X, y, "fit", 'colmajor')
        else:
            X, y, dtype, itype, dense = self.check_input(X, y, "fit", 'rowmajor')
        self.__mid = ModelID.get()
        self.__mdtype = dtype

        if self.n_classes > 2:
            raise ValueError("fit: frovedis svc does not support multinomial " \
                              + "classification currently!")

        if type(self.gamma).__name__ == 'float': pass
        elif self.gamma == "scale":
            if self.xvar is None :
                raise ValueError("fit: gamma = scale is supported only for numpy array-like data!")
            self.gamma = 1.0 / (self.n_features * self.xvar)
        elif self.gamma == "auto":
            self.gamma = 1.0 / self.n_features
        else:
            raise ValueError("fit: unsupported gamma is encountered!")

        sample_weight = self.check_sample_weight(sample_weight)
        (host, port) = FrovedisServer.getServerInstance()
        if self.kernel == 'linear':
            lrate = 0.01
            regType = 2
            icpt = True
            rparam = 1.0 / self.C
            solver = "sgd"
            warm_start = False
            self._n_iter = rpclib.svm(host, port, X.get(), y.get(), \
                           sample_weight, len(sample_weight), \
                           self.max_iter_, lrate, regType, rparam, icpt, \
                           self.tol, self.verbose, self.__mid, dtype, \
                           itype, dense, solver.encode('ascii'), warm_start)
            self._coef = None
            self._intercept = None
        else:
            rpclib.frovedis_svc(host, port, X.get(), y.get(), \
                       self.tol, self.C, self.cache_size, self.max_iter_, \
                       self.kernel.encode("ascii"), self.gamma, self.coef0, \
                       self.degree, self.verbose, self.__mid, \
                       dtype, itype, dense)

        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        return self

    @property
    @check_association
    def coef_(self):
        """coef_ getter"""
        if self.kernel != 'linear' and self._coef is not None:
            raise AttributeError("coef_ is supported for only kernel = linear!")
        (host, port) = FrovedisServer.getServerInstance()
        wgt = rpclib.get_weight_vector(host, port, self.__mid, \
              self.__mkind, self.__mdtype)
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        n_features = len(wgt)
        shape = (1, n_features)
        self._coef = np.asarray(wgt).reshape(shape)
        return self._coef

    @coef_.setter
    def coef_(self, val):
        """coef_ setter"""
        raise AttributeError(\
            "attribute 'coef_' of SVC object is not writable")

    @property
    @check_association
    def intercept_(self):
        """intercept_ getter"""
        if self.kernel != 'linear' and self._intercept is not None:
            raise AttributeError("intercept_ is supported for only kernel = linear!")
        (host, port) = FrovedisServer.getServerInstance()
        icpt = rpclib.get_intercept_vector(host, port, self.__mid, \
            self.__mkind, self.__mdtype)
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        self._intercept = np.asarray(icpt)
        return self._intercept

    @intercept_.setter
    def intercept_(self, val):
        """intercept_ setter"""
        raise AttributeError(\
            "attribute 'intercept_' of SVC object is not writable")

    @property
    def classes_(self):
        """classes_ getter"""
        if self.__mid is None:
            raise AttributeError(\
              "attribute 'classes_' might have been released or called before fit")
        if self._classes is None:
            self._classes = np.sort(list(self.label_map.values()))
        return self._classes

    @classes_.setter
    def classes_(self, val):
        """classes_ setter"""
        raise AttributeError(\
        "attribute 'classes_' of SVC object is not writable")

    @property
    @check_association
    def support_(self):
        """support_ getter"""
        if self._support is None:
            (host, port) = FrovedisServer.getServerInstance()
            support = rpclib.get_support_idx(host, port, self.__mid, \
                self.__mkind, self.__mdtype)
            excpt = rpclib.check_server_exception()
            if excpt["status"]:
                raise RuntimeError(excpt["info"])
            self._support = np.asarray(support, dtype=np.int32)
        return self._support

    @support_.setter
    def support_(self, val):
        """support_ setter"""
        raise AttributeError(\
            "attribute 'support_' of SVC object is not writable")

    @property
    @check_association
    def support_vectors_(self):
        """support_vectors_ getter"""
        if self._support_vectors is None:
            (host, port) = FrovedisServer.getServerInstance()
            sup_vec = rpclib.get_support_vector(host, port, self.__mid, \
                self.__mkind, self.__mdtype)
            excpt = rpclib.check_server_exception()
            if excpt["status"]:
                raise RuntimeError(excpt["info"])
            n_sv = len(sup_vec) // self.n_features
            self._support_vectors = np.asarray(sup_vec, dtype=np.float64) \
                                      .reshape(n_sv, self.n_features)
        return self._support_vectors

    @support_vectors_.setter
    def support_vectors_(self, val):
        """support_vectors_ setter"""
        raise AttributeError(\
            "attribute 'support_vectors_' of SVC object is not writable")

    @check_association
    def predict(self, X):
        """
        NAME: predict
        """
        frov_pred = GLM.predict(X, self.__mid, self.__mkind, \
                                self.__mdtype, False)
        return np.asarray([self.label_map[frov_pred[i]] \
                          for i in range(0, len(frov_pred))])

    @check_association
    def predict_proba(self, X):
        """
        NAME: predict_proba
        """
        #this check is not applicable for frovedis wrapper since it doesn't
        #perform k-fold validation for calculating probability
        #if not self.probability == True :
        #    raise AttributeError("predict_proba() can be called only
        #                          when probability is set to True!")

        if self.kernel == 'linear':
            raise AttributeError("predict_proba() is not supported for " \
                                + "linear svc kernel!")

        proba = GLM.predict(X, self.__mid, self.__mkind, \
                            self.__mdtype, True, self.n_classes)
        n_samples = len(proba) // self.n_classes
        shape = (n_samples, self.n_classes)
        return np.asarray(proba).reshape(shape)

    @property
    def n_iter_(self):
        """n_iter_ getter"""
        if self.__mid is None:
            raise AttributeError("attribute 'n_iter_'" \
               "might have been released or called before fit")
        if self.kernel != 'linear':
            raise AttributeError("n_iter_ is supported for only kernel = linear!")
        return self._n_iter

    @n_iter_.setter
    def n_iter_(self, val):
        """n_iter_ setter"""
        raise AttributeError(\
            "attribute 'n_iter_' of SVC object is not writable")

    @set_association
    def load(self, fname, dtype=None):
        """
        NAME: load
        """
        if not os.path.exists(fname):
            raise ValueError(\
                "the model with name %s does not exist!" % fname)
        self.release()
        target = open(fname + "/label_map", "rb")
        self.label_map = pickle.load(target)
        target.close()
        self._classes = np.sort(list(self.label_map.values()))
        metadata = open(fname + "/metadata", "rb")
        self.n_classes, self.__mkind, self.__mdtype = pickle.load(metadata)
        metadata.close()
        if dtype is not None:
            mdt = TypeUtil.to_numpy_dtype(self.__mdtype)
            if dtype != mdt:
                raise ValueError("load: type mismatches detected! " + \
                                 "expected type: " + str(mdt) + \
                                 "; given type: " + str(dtype))
        self.__mid = ModelID.get()
        GLM.load(self.__mid, self.__mkind, self.__mdtype, fname + "/model")
        return self

    @check_association
    def save(self, fname):
        """
        NAME: save
        """
        if os.path.exists(fname):
            raise ValueError(\
                "another model with %s name already exists!" % fname)
        os.makedirs(fname)
        GLM.save(self.__mid, self.__mkind, self.__mdtype, fname + "/model")
        target = open(fname + "/label_map", "wb")
        pickle.dump(self.label_map, target)
        target.close()
        metadata = open(fname + "/metadata", "wb")
        pickle.dump((self.n_classes, \
                self.__mkind, self.__mdtype), metadata)
        metadata.close()

    def score(self, X, y, sample_weight=None):
        """
        NAME: score
        """
        return accuracy_score(y, self.predict(X), sample_weight=sample_weight)

    def release(self):
        """
        resets after-fit populated attributes to None
        """
        self.__release_server_heap()
        self.__mid = None
        self._coef = None
        self._intercept = None
        self._classes = None
        self.label_map = None
        self.n_classes = None
        self.n_features = None
        self.n_samples = None
        self._support = None
        self._support_vectors = None
        self._n_iter = None

    @do_if_active_association
    def __release_server_heap(self):
        """
        to release model pointer from server heap
        """
        GLM.release(self.__mid, self.__mkind, self.__mdtype)

    def __del__(self):
        """
        NAME: __del__
        """
        self.release()

    def is_fitted(self):
        """ function to confirm if the model is already fitted """
        return self.__mid is not None


class LinearSVR(BaseEstimator):
    """Python wrapper of Frovedis Linear SVR"""
    # defaults are as per Frovedis
    # penalty: Frovedis: l2 (added)
    # lr_rate: Frovedis: 0.01 (added)
    # solver: Frovedis: sag (SGD) (added)
    def __init__(self, epsilon=0.0, tol=1e-4, C=1.0,
                 loss='epsilon_insensitive', fit_intercept=True,
                 intercept_scaling=1, dual=True, verbose=0,
                 random_state=None, max_iter=1000,
                 penalty='l2',
                 lr_rate=0.01, solver='sag', warm_start = False):
        self.penalty = penalty
        self.epsilon = epsilon
        self.tol = tol
        self.C = C
        self.loss = loss
        self.fit_intercept = fit_intercept
        self.intercept_scaling = intercept_scaling
        self.dual = dual
        self.verbose = verbose
        self.random_state = random_state
        self.max_iter = max_iter
        self.warm_start = warm_start
        # extra
        self.lr_rate = lr_rate
        self.solver = solver
        self.__mid = ModelID.get()
        self.__mdtype = None
        self.__mkind = M_KIND.SVR
        self._coef = None
        self._intercept = None
        self.n_samples = None
        self.n_features = None
        self._n_iter = None
        self.isFitted = None

    def check_input(self, X, y, F):
        """checks input X"""
        inp_data = FrovedisLabeledPoint(X, y, \
                   caller = "[" + self.__class__.__name__ + "] " + F + ": ",\
                   dense_kind = 'colmajor', densify=False)

        X, y = inp_data.get()
        self.n_samples = inp_data.numRows()
        self.n_features = inp_data.numCols()
        dtype = inp_data.get_dtype()
        itype = inp_data.get_itype()
        dense = inp_data.is_dense()
        return X, y, dtype, itype, dense

    def check_sample_weight(self, sample_weight):
        """checks input X and y"""
        if sample_weight is None:
            weight = np.array([], dtype=np.float64)
        elif isinstance(sample_weight, numbers.Number):
            weight = np.full(self.n_samples, sample_weight, dtype=np.float64)
        else:
            weight = np.ravel(sample_weight)
            if len(weight) != self.n_samples:
                raise ValueError("sample_weight.shape == {}, expected {}!"\
                       .format(sample_weight.shape, (self.n_samples,)))
        return np.asarray(weight, dtype=np.float64)


    def validate(self):
        """
        NAME: validate
        """
        if self.epsilon < 0:
            raise ValueError("fit: epsilon parameter must be zero or positive!")

        if self.tol < 0:
            raise ValueError("fit: tol parameter must be zero or positive!")

        if self.C < 0:
            raise ValueError("fit: parameter C must be strictly positive!")

        supported_loss = ("epsilon_insensitive", "squared_epsilon_insensitive")
        if self.loss not in supported_loss:
            raise ValueError("fit: Invalid loss type for LinearSVM Regressor provided!"
                            + "'{}'".format(self.loss))

        if self.max_iter <= 0:
            raise ValueError("fit: max_iter must be a positive value!")

        if self.lr_rate <= 0:
            raise ValueError("fit: lr_rate must be a positive value!")

    @set_association
    def fit(self, X, y, sample_weight=None):
        """
        NAME: fit
        """
        # release old model, if any
        self.reset_metadata()
        self.validate()
        # perform the fit
        X, y, dtype, itype, dense = self.check_input(X, y, "fit")
        self.__mdtype = dtype

        rparam = 1.0 / self.C
        if self.penalty == 'l1':
            regTyp = 1
        elif self.penalty == 'l2':
            regTyp = 2
        elif self.penalty == 'none':
            regTyp = 0
        else:
            raise ValueError("Unsupported penalty is provided: ", self.penalty)

        if self.loss == 'epsilon_insensitive':
            intLoss = 1 # L1 loss
        elif self.loss == 'squared_epsilon_insensitive':
            intLoss = 2 # L2 loss
        else:
            raise ValueError("Invalid loss is provided: ", self.loss)

        sample_weight = self.check_sample_weight(sample_weight)
        (host, port) = FrovedisServer.getServerInstance()

        if self.solver == 'sag':
            n_iter = rpclib.svm_regressor(host, port, X.get(), y.get(), \
                                          sample_weight, len(sample_weight), \
                                          self.max_iter, self.lr_rate, \
                                          self.epsilon, regTyp, rparam, \
                                          self.fit_intercept, self.tol, \
                                          intLoss, self.verbose, self.__mid, \
                                          dtype, itype, dense, \
                                          'sgd'.encode('ascii'), self.warm_start)
        elif self.solver == 'lbfgs':
            raise ValueError("fit: currently LinearSVM Regressor doesn't " + \
                             "support lbfgs solver!")
        else:
            raise ValueError( \
              "Unknown solver %s for Linear SVM Regressor." % self.solver)
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        self._coef = None
        self._intercept = None
        self._n_iter = n_iter
        self.isFitted = True
        return self

    @property
    @check_association
    def coef_(self):
        """coef_ getter"""
        if self._coef is None:
            (host, port) = FrovedisServer.getServerInstance()
            wgt = rpclib.get_weight_vector(host, port, self.__mid, \
                self.__mkind, self.__mdtype)
            excpt = rpclib.check_server_exception()
            if excpt["status"]:
                raise RuntimeError(excpt["info"])
            n_features = len(wgt)
            shape = (1, n_features)
            self._coef = np.asarray(wgt).reshape(shape)
        return self._coef

    @coef_.setter
    def coef_(self, val):
        """coef_ setter"""
        raise AttributeError(\
            "attribute 'coef_' of LinearSVR object is not writable")

    @property
    @check_association
    def intercept_(self):
        """intercept_ getter"""
        if self._intercept is None:
            (host, port) = FrovedisServer.getServerInstance()
            icpt = rpclib.get_intercept_vector(host, port, self.__mid, \
                self.__mkind, self.__mdtype)
            excpt = rpclib.check_server_exception()
            if excpt["status"]:
                raise RuntimeError(excpt["info"])
            self._intercept = np.asarray(icpt)
        return self._intercept

    @intercept_.setter
    def intercept_(self, val):
        """intercept_ setter"""
        raise AttributeError(\
            "attribute 'intercept_' of LinearSVR object is not writable")

    @check_association
    def predict(self, X):
        """
        NAME: predict
        """
        frov_pred = GLM.predict(X, self.__mid, self.__mkind, \
                                self.__mdtype, False)
        return np.asarray(frov_pred)

    @property
    def n_iter_(self):
        """n_iter_ getter"""
        if not self.is_fitted():
            raise AttributeError("attribute 'n_iter_'" \
               "might have been released or called before fit")
        return self._n_iter

    @n_iter_.setter
    def n_iter_(self, val):
        """n_iter_ setter"""
        raise AttributeError(\
            "attribute 'n_iter_' of LinearSVR object is not writable")

    @set_association
    def load(self, fname, dtype=None):
        """
        NAME: load
        """
        if not os.path.exists(fname):
            raise ValueError(\
                "the model with name %s does not exist!" % fname)
        self.reset_metadata()
        metadata = open(fname + "/metadata", "rb")
        self.__mkind, self.__mdtype = pickle.load(metadata)
        metadata.close()
        if dtype is not None:
            mdt = TypeUtil.to_numpy_dtype(self.__mdtype)
            if dtype != mdt:
                raise ValueError("load: type mismatches detected! " + \
                                 "expected type: " + str(mdt) + \
                                 "; given type: " + str(dtype))
        GLM.load(self.__mid, self.__mkind, self.__mdtype, fname + "/model")
        self.isFitted = True
        return self

    @check_association
    def save(self, fname):
        """
        NAME: save
        """
        if os.path.exists(fname):
            raise ValueError(\
                "another model with %s name already exists!" % fname)
        os.makedirs(fname)
        GLM.save(self.__mid, self.__mkind, self.__mdtype, fname + "/model")
        metadata = open(fname + "/metadata", "wb")
        pickle.dump((self.__mkind, self.__mdtype), metadata)
        metadata.close()

    def score(self, X, y, sample_weight=None):
        """
        NAME: score
        """
        return r2_score(y, self.predict(X), sample_weight=sample_weight)

    @check_association
    def debug_print(self):
        """
        NAME: debug_print
        """
        GLM.debug_print(self.__mid, self.__mkind, self.__mdtype)

    def reset_metadata(self):
        """
        resets after-fit populated attributes to None
        """
        self._coef = None
        self._intercept = None
        self._n_iter = None
        self.n_samples = None
        self.n_features = None
        self.isFitted = None

    def release(self):
        """
        resets after-fit populated attributes to None
        """
        self.__release_server_heap()
        self.reset_metadata()

    @do_if_active_association
    def __release_server_heap(self):
        """
        to release model pointer from server heap
        """
        GLM.release(self.__mid, self.__mkind, self.__mdtype)

    def __del__(self):
        """
        NAME: __del__
        """
        self.reset_metadata()

    def is_fitted(self):
        """ function to confirm if the model is already fitted """
        return self.isFitted
