"""
svm.py: wrapper of frovedis Linear SVM
"""

#!/usr/bin/env python
import os.path
import pickle
import sys
from .model_util import *
from ..base import BaseEstimator
from .metrics import *
from ..exrpc import rpclib
from ..exrpc.server import FrovedisServer
from ..matrix.ml_data import FrovedisLabeledPoint
from ..matrix.dtype import TypeUtil
from ..matrix.dense import FrovedisDenseMatrix,FrovedisRowmajorMatrix
from ..matrix.dvector import FrovedisDvector
from ..matrix.crs import FrovedisCRSMatrix

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
                 lr_rate=0.01, solver='sag'):
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
        # extra
        self.lr_rate = lr_rate
        self.solver = solver
        self.__mid = None
        self.__mdtype = None
        self.__mkind = M_KIND.SVM
        self.n_classes = None
        self._coef = None
        self._intercept = None
        self._classes = None
        self.label_map = None

    def fit(self, X, y, sample_weight=None):
        """
        NAME: fit
        """
        if self.loss != 'hinge':
            raise ValueError("fit: frovedis svm supports only hinge loss!")
        if self.C < 0:
            raise ValueError("fit: parameter C must be strictly positive!")
        self.release()
        self.__mid = ModelID.get()
        inp_data = FrovedisLabeledPoint(X, y)
        (X, y) = inp_data.get()
        dtype = inp_data.get_dtype()
        itype = inp_data.get_itype()
        dense = inp_data.is_dense()
        self.__mdtype = dtype
        unique_labels = inp_data.get_distinct_labels()
        self.n_classes = unique_labels.size

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

        #Encoder
        target = [-1, 1]
        encoded_y, logic = y.encode(unique_labels, target, need_logic=True)
        self.label_map = logic
        (host, port) = FrovedisServer.getServerInstance()
        if dense and X.get_mtype() != 'C':
            raise TypeError("fit: please provide column-major points " +
                            "for frovedis linear svm classifier!")
        if self.solver == 'sag':
            rpclib.svm_sgd(host, port, X.get(), encoded_y.get(), \
                           self.max_iter, self.lr_rate, \
                           regTyp, rparam, self.fit_intercept, \
                           self.tol, self.verbose, self.__mid, dtype, \
                           itype, dense)

        elif self.solver == 'lbfgs':
            rpclib.svm_lbfgs(host, port, X.get(), encoded_y.get(), \
                             self.max_iter, self.lr_rate, \
                             regTyp, rparam, self.fit_intercept, \
                             self.tol, self.verbose, self.__mid, dtype, \
                             itype, dense)
        else:
            raise ValueError( \
              "Unknown solver %s for Linear SVM." % self.solver)
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        self._coef = None
        self._intercept = None
        self._classes = None
        return self

    @property
    def coef_(self):
        """coef_ getter"""
        if self.__mid is not None:
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
        else:
            raise AttributeError(\
            "attribute 'coef_' might have been released or called before fit")

    @coef_.setter
    def coef_(self, val):
        """coef_ setter"""
        raise AttributeError(\
            "attribute 'coef_' of LinearSVC object is not writable")

    @property
    def intercept_(self):
        """intercept_ getter"""
        if self.__mid is not None:
            if self._intercept is None:
                (host, port) = FrovedisServer.getServerInstance()
                icpt = rpclib.get_intercept_vector(host, port, self.__mid, \
                    self.__mkind, self.__mdtype)
                excpt = rpclib.check_server_exception()
                if excpt["status"]:
                    raise RuntimeError(excpt["info"])
                self._intercept = np.asarray(icpt)
            return self._intercept
        else:
            raise AttributeError(\
        "attribute 'intercept_' might have been released or called before fit")

    @intercept_.setter
    def intercept_(self, val):
        """intercept_ setter"""
        raise AttributeError(\
            "attribute 'intercept_' of LinearSVC object is not writable")

    @property
    def classes_(self):
        """classes_ getter"""
        if self.__mid is not None:
            if self._classes is None:
                self._classes = \
                    np.asarray(list(self.label_map.values()), dtype=np.int64)
            return self._classes
        else:
            raise AttributeError(\
        "attribute 'classes_' might have been released or called before fit")

    @classes_.setter
    def classes_(self, val):
        """classes_ setter"""
        raise AttributeError(\
        "attribute 'classes_' of LinearSVC object is not writable")

    def predict(self, X):
        """
        NAME: predict
        """
        if self.__mid is not None:
            frov_pred = GLM.predict(X, self.__mid, self.__mkind, \
                                    self.__mdtype, False)
            new_pred = \
            [self.label_map[frov_pred[i]] for i in range(0, len(frov_pred))]
            return np.asarray(new_pred, dtype=np.int64)
        else:
            raise ValueError( \
            "predict is called before calling fit, or the model is released.")

    def load(self, fname, dtype=None):
        """
        NAME: load
        """
        if not os.path.exists(fname):
            raise ValueError(\
                "the model with name %s does not exist!" % fname)
        self.release()
        target = open(fname+"/label_map", "rb")
        self.label_map = pickle.load(target)
        target.close()
        metadata = open(fname+"/metadata", "rb")
        self.n_classes, self.__mkind, self.__mdtype = pickle.load(metadata)
        metadata.close()
        if dtype is not None:
            mdt = TypeUtil.to_numpy_dtype(self.__mdtype)
            if dtype != mdt:
                raise ValueError("load: type mismatches detected!" + \
                                 "expected type: " + str(mdt) + \
                                 "; given type: " + str(dtype))
        self.__mid = ModelID.get()
        GLM.load(self.__mid, self.__mkind, self.__mdtype, fname+"/model")
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
            GLM.save(self.__mid, self.__mkind, self.__mdtype, fname+"/model")
            target = open(fname+"/label_map", "wb")
            pickle.dump(self.label_map, target)
            target.close()
            metadata = open(fname+"/metadata", "wb")
            pickle.dump((self.n_classes, \
                    self.__mkind, self.__mdtype), metadata)
            metadata.close()
        else:
            raise AttributeError(\
                    "save: requested model might have been released!")

    def score(self, X, y):
        """
        NAME: score
        """
        if self.__mid is not None:
            return accuracy_score(y, self.predict(X))

    def debug_print(self):
        """
        NAME: debug_print
        """
        if self.__mid is not None:
            GLM.debug_print(self.__mid, self.__mkind, self.__mdtype)

    def release(self):
        """
        NAME: release
        """
        if self.__mid is not None:
            GLM.release(self.__mid, self.__mkind, self.__mdtype)
            self.__mid = None
            self._coef = None
            self._intercept = None
            self._classes = None

    def __del__(self):
        """
        NAME: __del__
        """
        if FrovedisServer.isUP():
            self.release()

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
        self.n_classes = None
        self._coef = None
        self._intercept = None
        self._support = None
        self._support_vectors = None
        self._classes = None
        self.label_map = None

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
            self.max_iter = sys.maxsize
        elif self.max_iter <= 0:
            raise ValueError("validate: max_iter can either be -1 or positive!")

    def fit(self, X, y, sample_weight=None):
        """
        NAME: fit
        """
        self.release()
        self.__mid = ModelID.get()
        if(isinstance(y, FrovedisDvector)):
            self.__ytype = TypeUtil.get_numpy_dtype(y.get_dtype())
        else:
            self.__ytype = np.asarray(y).dtype

        if(not isinstance(X, (FrovedisDenseMatrix, FrovedisCRSMatrix))):
            self.xvar = np.var(X)
        else:
            self.xvar = None

        train_data = FrovedisFeatureData(X, dense_kind='rowmajor')
        X = train_data.get()
        dtype = train_data.get_dtype()
        itype = train_data.get_itype()
        dense = train_data.is_dense()
        y = FrovedisDvector.as_dvec(y,dtype=TypeUtil.to_numpy_dtype(dtype))
        if (dtype != y.get_dtype()):
            raise TypeError("fit: different dtypes are encountered for " \
                            + "X and y!")
        self.__mdtype = dtype
        self._classes = np.asarray(y.get_unique_elements())
        self.n_features = train_data.numCols()
        self.n_classes = self._classes.size

        if self.n_classes > 2:
            raise ValueError("fit: frovedis svc does not support multinomial " \
                              + "data currently!")

        if(type(self.gamma).__name__ == 'float'): pass
        elif(self.gamma == "scale"):
            if self.xvar is None :
                raise ValueError("fit: gamma = scale is supported only for python data!")
            else:
                self.gamma = 1.0 / (self.n_features * self.xvar)
        elif self.gamma == "auto":
            self.gamma = 1.0 / self.n_features
        else:
            raise ValueError("fit: unsupported gamma is encountered!")

        target = [-1, 1]
        encoded_y, logic = y.encode(self._classes, target, need_logic=True)
        self.label_map = logic
        # validate hyper-parameters
        self.validate()
        (host, port) = FrovedisServer.getServerInstance()
        if self.kernel == 'linear':
            lrate = 0.01
            regType = 2
            icpt = True
            rparam = 1.0 / self.C
            if dense and X.get_mtype() != 'C':
                raise TypeError("fit: please provide column-major points " +
                                "for frovedis svm with sgd classification!")
            rpclib.svm_sgd(host, port, X.get(), encoded_y.get(), \
                       self.max_iter, lrate, regType, rparam, icpt, \
                       self.tol, self.verbose, self.__mid, dtype, \
                       itype, dense)
            self._coef = None
            self._intercept = None
        else:
            if dense and X.get_mtype() != 'R':
                raise TypeError("fit: please provide row-major points " +
                                "for frovedis svc classification!")
            rpclib.frovedis_svc(host, port, X.get(), encoded_y.get(), \
                       self.tol, self.C, self.cache_size, self.max_iter, \
                       self.kernel.encode("ascii"), self.gamma, self.coef0, \
                       self.degree, self.verbose, self.__mid, \
                       dtype, itype, dense)

        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        return self

    @property
    def coef_(self):
        """coef_ getter"""
        if self.__mid is not None:
            if self.kernel == 'linear' and self._coef is None:
                (host, port) = FrovedisServer.getServerInstance()
                wgt = rpclib.get_weight_vector(host, port, self.__mid, \
                      self.__mkind, self.__mdtype)
                excpt = rpclib.check_server_exception()
                if excpt["status"]:
                    raise RuntimeError(excpt["info"])
                n_features = len(wgt)
                shape = (1, n_features)
                self._coef = np.asarray(wgt).reshape(shape)
            else:
                raise AttributeError("coef_ is supported for only kernel = linear!")
            return self._coef
        else:
            raise AttributeError(\
            "attribute 'coef_' might have been released or called before fit")

    @coef_.setter
    def coef_(self, val):
        """coef_ setter"""
        raise AttributeError(\
            "attribute 'coef_' of SVC object is not writable")

    @property
    def intercept_(self):
        """intercept_ getter"""
        if self.__mid is not None:
            if self.kernel == 'linear' and self._intercept is None:
                (host, port) = FrovedisServer.getServerInstance()
                icpt = rpclib.get_intercept_vector(host, port, self.__mid, \
                    self.__mkind, self.__mdtype)
                excpt = rpclib.check_server_exception()
                if excpt["status"]:
                    raise RuntimeError(excpt["info"])
                self._intercept = np.asarray(icpt)
            else:
                raise AttributeError("intercept_ is supported for only kernel = linear!")
            return self._intercept
        else:
            raise AttributeError(\
        "attribute 'intercept_' might have been released or called before fit")

    @intercept_.setter
    def intercept_(self, val):
        """intercept_ setter"""
        raise AttributeError(\
            "attribute 'intercept_' of SVC object is not writable")

    @property
    def classes_(self):
        """classes_ getter"""
        if self.__mid is not None:
            if self._classes is None: #COMMENT: confirm dtype of labels
                self._classes = \
                    np.asarray(list(self.label_map.values()), dtype=self.__ytype)
            return self._classes
        else:
            raise AttributeError(\
        "attribute 'classes_' might have been released or called before fit")

    @classes_.setter
    def classes_(self, val):
        """classes_ setter"""
        raise AttributeError(\
        "attribute 'classes_' of SVC object is not writable")

    @property
    def support_(self):
        """support_ getter"""
        if self.__mid is not None:
            if self._support is None:
                (host, port) = FrovedisServer.getServerInstance()
                support = rpclib.get_support_idx(host, port, self.__mid, \
                    self.__mkind, self.__mdtype)
                excpt = rpclib.check_server_exception()
                if excpt["status"]:
                    raise RuntimeError(excpt["info"])
                self._support = np.asarray(support, dtype=np.int32)
            return self._support
        else:
            raise AttributeError(\
        "attribute 'support_' might have been released or called before fit")

    @support_.setter
    def support_(self, val):
        """support_ setter"""
        raise AttributeError(\
            "attribute 'support_' of SVC object is not writable")

    @property
    def support_vectors_(self):
        """support_vectors_ getter"""
        if self.__mid is not None:
            if self._support_vectors is None:
                (host, port) = FrovedisServer.getServerInstance()
                sup_vec = rpclib.get_support_vector(host, port, self.__mid, \
                    self.__mkind, self.__mdtype)
                excpt = rpclib.check_server_exception()
                if excpt["status"]:
                    raise RuntimeError(excpt["info"])
                self._support_vectors = np.asarray(sup_vec, dtype=np.float64)
            return self._support_vectors
        else:
            raise AttributeError("attribute 'support_vectors_' might have been \
                                  released or called before fit")

    @support_vectors_.setter
    def support_vectors_(self, val):
        """support_vectors_ setter"""
        raise AttributeError(\
            "attribute 'support_vectors_' of SVC object is not writable")

    def predict(self, X):
        """
        NAME: predict
        """
        if self.__mid is not None:
            frov_pred = GLM.predict(X, self.__mid, self.__mkind, \
                                    self.__mdtype, False)
            new_pred = \
            [self.label_map[frov_pred[i]] for i in range(0, len(frov_pred))]
            return np.asarray(new_pred, dtype=self.__ytype)
        else:
            raise ValueError( \
            "predict is called before calling fit, or the model is released.")

    def predict_proba(self, X):
        """
        NAME: predict_proba
        """
        #this check is not applicable for frovedis wrapper since it doesn't 
        #perform k-fold validation for calculating probability
        #if not self.probability == True :
        #    raise AttributeError("predict_proba() can be called only when probability is set to True!")

        if self.kernel == 'linear':
            raise AttributeError("predict_proba() is not supported for " \
                                + "linear svc kernel!")

        if self.__mid is not None:
            proba = GLM.predict(X, self.__mid, self.__mkind, \
                                self.__mdtype, True, self.n_classes)
            n_samples = len(proba) // self.n_classes
            shape = (n_samples, self.n_classes)
            return np.asarray(proba).reshape(shape)
        else:
            raise ValueError( \
            "predict_proba is called before calling fit, or the model is released.")

    def load(self, fname, dtype=None):
        """
        NAME: load
        """
        if not os.path.exists(fname):
            raise ValueError(\
                "the model with name %s does not exist!" % fname)
        self.release()
        target = open(fname+"/label_map", "rb")
        self.label_map = pickle.load(target)
        target.close()
        metadata = open(fname+"/metadata", "rb")
        self.n_classes, self.__mkind, self.__mdtype = pickle.load(metadata)
        metadata.close()
        if dtype is not None:
            mdt = TypeUtil.to_numpy_dtype(self.__mdtype)
            if dtype != mdt:
                raise ValueError("load: type mismatches detected!" + \
                                 "expected type: " + str(mdt) + \
                                 "; given type: " + str(dtype))
        self.__mid = ModelID.get()
        GLM.load(self.__mid, self.__mkind, self.__mdtype, fname+"/model")
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
            GLM.save(self.__mid, self.__mkind, self.__mdtype, fname+"/model")
            target = open(fname+"/label_map", "wb")
            pickle.dump(self.label_map, target)
            target.close()
            metadata = open(fname+"/metadata", "wb")
            pickle.dump((self.n_classes, \
                    self.__mkind, self.__mdtype), metadata)
            metadata.close()
        else:
            raise AttributeError(\
                    "save: requested model might have been released!")

    def score(self, X, y):
        """
        NAME: score
        """
        if self.__mid is not None:
            return accuracy_score(y, self.predict(X))

    def release(self):
        """
        NAME: release
        """
        if self.__mid is not None:
            GLM.release(self.__mid, self.__mkind, self.__mdtype)
            self.__mid = None
            self._coef = None
            self._intercept = None
            self._classes = None
            self.n_classes = None
            self._support = None
            self._support_vectors = None
            self.label_map = None

    def __del__(self):
        """
        NAME: __del__
        """
        if FrovedisServer.isUP():
            self.release()

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
                 lr_rate=0.01, solver='sag'):
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
        # extra
        self.lr_rate = lr_rate
        self.solver = solver
        self.__mid = None
        self.__mdtype = None
        self.__mkind = M_KIND.SVR
        self._coef = None
        self._intercept = None

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

    def fit(self, X, y, sample_weight=None):
        """
        NAME: fit
        """
        # release old model, if any
        self.release()
        # perform the fit
        self.__mid = ModelID.get()
        inp_data = FrovedisLabeledPoint(X, y)
        (X, y) = inp_data.get()
        dtype = inp_data.get_dtype()
        itype = inp_data.get_itype()
        dense = inp_data.is_dense()
        self.__mdtype = dtype
        self.validate()

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

        (host, port) = FrovedisServer.getServerInstance()
        if self.solver == 'sag':
            if dense and X.get_mtype() != 'C':
                raise TypeError("fit: please provide column-major points " +
                                "for frovedis linear SVM regressor!")
            rpclib.svm_regressor_sgd(host, port, X.get(), y.get(), \
                                     self.max_iter, self.lr_rate, \
                                     self.epsilon, regTyp, rparam, \
                                     self.fit_intercept, self.tol, \
                                     intLoss, self.verbose, self.__mid, \
                                     dtype, itype, dense)

        elif self.solver == 'lbfgs':
            raise ValueError("Currently LinearSVM Regressor doesn't support lbfgs solver!")
        else:
            raise ValueError( \
              "Unknown solver %s for Linear SVM Regressor." % self.solver)
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        self._coef = None
        self._intercept = None
        return self

    @property
    def coef_(self):
        """coef_ getter"""
        if self.__mid is not None:
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
        else:
            raise AttributeError(\
            "attribute 'coef_' might have been released or called before fit")

    @coef_.setter
    def coef_(self, val):
        """coef_ setter"""
        raise AttributeError(\
            "attribute 'coef_' of LinearSVR object is not writable")

    @property
    def intercept_(self):
        """intercept_ getter"""
        if self.__mid is not None:
            if self._intercept is None:
                (host, port) = FrovedisServer.getServerInstance()
                icpt = rpclib.get_intercept_vector(host, port, self.__mid, \
                    self.__mkind, self.__mdtype)
                excpt = rpclib.check_server_exception()
                if excpt["status"]:
                    raise RuntimeError(excpt["info"])
                self._intercept = np.asarray(icpt)
            return self._intercept
        else:
            raise AttributeError(\
        "attribute 'intercept_' might have been released or called before fit")

    @intercept_.setter
    def intercept_(self, val):
        """intercept_ setter"""
        raise AttributeError(\
            "attribute 'intercept_' of LinearSVR object is not writable")

    def predict(self, X):
        """
        NAME: predict
        """
        if self.__mid is not None:
            frov_pred = GLM.predict(X, self.__mid, self.__mkind, \
                                    self.__mdtype, False)
            return np.asarray(frov_pred)
            
        else:
            raise ValueError( \
            "predict is called before calling fit, or the model is released.")

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
        GLM.load(self.__mid, self.__mkind, self.__mdtype, fname+"/model")
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
            GLM.save(self.__mid, self.__mkind, self.__mdtype, fname+"/model")
            metadata = open(fname+"/metadata", "wb")
            pickle.dump((self.__mkind, self.__mdtype), metadata)
            metadata.close()
        else:
            raise AttributeError(\
                    "save: requested model might have been released!")

    def score(self, X, y):
        """
        NAME: score
        """
        if self.__mid is not None:
            return r2_score(y, self.predict(X))

    def debug_print(self):
        """
        NAME: debug_print
        """
        if self.__mid is not None:
            GLM.debug_print(self.__mid, self.__mkind, self.__mdtype)

    def release(self):
        """
        NAME: release
        """
        if self.__mid is not None:
            GLM.release(self.__mid, self.__mkind, self.__mdtype)
            self.__mid = None
            self._coef = None
            self._intercept = None

    def __del__(self):
        """
        NAME: __del__
        """
        if FrovedisServer.isUP():
            self.release()

