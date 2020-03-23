"""
linear_model.py: wrapper of frovedis Logistic Regression, Linear Regression,
                 Lasso and Ridge Regression
"""

#!/usr/bin/env python
import os.path
import pickle
from sys import argv
from .model_util import *
from ..exrpc import rpclib
from ..exrpc.server import FrovedisServer
from ..matrix.ml_data import FrovedisLabeledPoint
from ..matrix.dtype import TypeUtil

class LogisticRegression(object):
    """
    A python wrapper of Frovedis Logistic Regression
    defaults are as per Frovedis
    penalty: Frovedis: none, Sklearn: l2
    max_iter: Frovedis: 1000, Sklearn: 100
    solver: Frovedis: sag (SGD), Sklearn: lbfgs
    lr_rate: Frovedis: 0.01 (added)
    """
    def __init__(self, penalty='none', dual=False, tol=1e-4, C=1.0,
                 fit_intercept=True, intercept_scaling=1, class_weight=None,
                 random_state=None, solver='sag', max_iter=1000,
                 multi_class='ovr', verbose=0, warm_start=False,
                 n_jobs=1, l1_ratio=None, lr_rate=0.01):
        self.penalty = penalty
        self.dual = dual
        self.tol = tol
        self.C = C
        self.fit_intercept = fit_intercept
        self.intercept_scaling = intercept_scaling
        self.class_weight = class_weight
        self.random_state = random_state
        self.solver = solver
        self.max_iter = max_iter
        self.multi_class = multi_class
        self.verbose = verbose
        self.warm_start = warm_start
        self.n_jobs = n_jobs
        self.l1_ratio = l1_ratio
        # extra
        self.lr_rate = lr_rate
        self.__mid = None
        self.__mdtype = None
        self.__mkind = None
        self.label_map = None
        self.n_classes = None
        self._classes = None
        self._intercept = None
        self._coef = None

    def fit(self, X, y, sample_weight=None):
        """
        NAME: fit
        """
        if self.C < 0:
            raise ValueError("fit: parameter C must be strictly positive!")
        self.release()
        inp_data = FrovedisLabeledPoint(X, y)
        (X, y) = inp_data.get()
        dtype = inp_data.get_dtype()
        itype = inp_data.get_itype()
        dense = inp_data.is_dense()
        unique_labels = inp_data.get_distinct_labels()
        self.n_classes = unique_labels.size
        self.__mid = ModelID.get()
        self.__mdtype = dtype

        if self.n_classes > 2 and self.multi_class == 'ovr':
            raise ValueError("fit: binary classfication on \
                multinomial data is requested!")

        if self.multi_class == 'auto':
            if self.n_classes == 2:
                self.multi_class = 'ovr'
            elif self.n_classes > 2:
                self.multi_class = 'multinomial'
            else:
                raise ValueError("fit: number of unique \
                    labels in y are less than 2")

        if self.penalty == 'l1':
            regTyp = 1
        elif self.penalty == 'l2':
            regTyp = 2
        elif self.penalty == 'none':
            regTyp = 0
        else:
            raise ValueError( \
                "Unsupported penalty is provided: ", self.penalty)
        rparam = 1.0 / self.C
        sv = ['newton-cg', 'liblinear', 'saga']
        if self.solver in sv:
            raise ValueError( \
            "Frovedis doesn't support solver %s for Logistic Regression \
            currently." % self.solver)

        if self.multi_class == 'ovr':
            isMult = False
            self.__mkind = M_KIND.LRM
        elif self.multi_class == 'multinomial':
            isMult = True
            self.__mkind = M_KIND.MLR
        else:
            raise ValueError( \
                "Unknown multi_class type!")

        if isMult:
            encoded_y, logic = y.encode(need_logic=True)
        else:
            target = [-1, 1] # frovedis supports -1 and 1
                             #   for binary logistic regression
            encoded_y, logic = y.encode(unique_labels, target, need_logic=True)
        self.label_map = logic
        (host, port) = FrovedisServer.getServerInstance()
        if self.solver == 'sag':
            rpclib.lr_sgd(host, port, X.get(), encoded_y.get(), self.max_iter, \
                       self.lr_rate, regTyp, rparam, isMult, \
                       self.fit_intercept, self.tol, self.verbose, \
                       self.__mid, dtype, itype, dense)
        elif self.solver == 'lbfgs':
            rpclib.lr_lbfgs(host, port, X.get(), encoded_y.get(), \
                          self.max_iter, self.lr_rate, regTyp, rparam, \
                          isMult, \
                          self.fit_intercept, self.tol, self.verbose, \
                          self.__mid, dtype, itype, dense)
        else:
            raise ValueError( \
                "Unknown solver %s for Logistic Regression." % self.solver)
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
                if self.__mkind == M_KIND.LRM:
                    n_features = len(wgt)
                    shape = (1, n_features)
                else:
                    n_features = len(wgt) // self.n_classes
                    shape = (self.n_classes, n_features)
                self._coef = np.asarray(wgt).reshape(shape)
            return self._coef
        else:
            raise AttributeError(\
            "attribute 'coef_' might have been released or called before fit")

    @coef_.setter
    def coef_(self, val):
        """coef_ setter"""
        raise AttributeError(\
            "attribute 'coef_' of LogisticRegression object is not writable")

    @property
    def intercept_(self):
        """intercept getter"""
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
        """ intercept_ setter"""
        raise AttributeError(\
        "attribute 'intercept_' of LogisticRegression object is not writable")

    @property
    def classes_(self):
        """classes_ getter"""
        if self.__mid is not None:
            if self._classes is None:
                self._classes = \
                    np.asarray(list(self.label_map.values()), dtype=np.int64)
            return self._classes
        else:
            raise AttributeError("attribute 'classes_' \
               might have been released or called before fit")

    @classes_.setter
    def classes_(self, val):
        """classes_ setter"""
        raise AttributeError(\
            "attribute 'classes_' of LogisticRegression object is not writable")

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
            pickle.dump((self.n_classes, self.__mkind, self.__mdtype), metadata)
            metadata.close()
        else:
            raise AttributeError(\
                "save: requested model might have been released!")

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

class LinearRegression(object):
    """A python wrapper of Frovedis Linear Regression
    max_iter: Frovedis: 1000 (added)
    solver: Frovedis: sag (SGD) (added)
    lr_rate: Frovedis: 0.01 (added)
    tol: Frovedis: 0.0001 (added)
    """
    def __init__(self, fit_intercept=True, normalize=False, copy_X=True,
                 n_jobs=1, max_iter=1000, tol=0.0001, lr_rate=1e-8, 
                 solver='sag', verbose=0):
        self.fit_intercept = fit_intercept
        self.normalize = normalize
        self.copy_X = copy_X
        self.n_jobs = n_jobs
        # extra
        self.max_iter = max_iter
        self.tol = tol
        self.lr_rate = lr_rate
        self.solver = solver
        self.verbose = verbose
        self.__mid = None
        self.__mdtype = None
        self.__mkind = M_KIND.LNRM
        self._intercept = None
        self._coef = None

    def fit(self, X, y, sample_weight=None):
        """
        NAME: fit
        """
        self.release()
        self.__mid = ModelID.get()
        inp_data = FrovedisLabeledPoint(X, y)
        (X, y) = inp_data.get()
        dtype = inp_data.get_dtype()
        itype = inp_data.get_itype()
        dense = inp_data.is_dense()
        self.__mdtype = dtype

        (host, port) = FrovedisServer.getServerInstance()
        if self.solver == 'sag':
            rpclib.lnr_sgd(host, port, X.get(), y.get(), \
                self.max_iter, self.lr_rate, \
                self.fit_intercept, self.tol, self.verbose, self.__mid, \
                dtype, itype, dense)
        elif self.solver == 'lbfgs':
            rpclib.lnr_lbfgs(host, port, X.get(), y.get(), \
                self.max_iter, self.lr_rate, \
                self.fit_intercept, self.tol, self.verbose, self.__mid, \
                dtype, itype, dense)
        else:
            raise ValueError( \
            "Unknown solver %s for Linear Regression." % self.solver)
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
            raise AttributeError("attribute 'coef_' \
               might have been released or called before fit")

    @coef_.setter
    def coef_(self, val):
        """coef_ setter"""
        raise AttributeError("attribute 'coef_' \
            of LinearRegression object is not writable")

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
            raise AttributeError("attribute 'intercept_' \
               might have been released or called before fit")

    @intercept_.setter
    def intercept_(self, val):
        """intercept_ setter"""
        raise AttributeError(\
            "attribute 'intercept_' of LinearRegression object is not writable")

    def predict(self, X):
        """
        NAME: predict
        """
        if self.__mid is not None:
            return GLM.predict(X, self.__mid, self.__mkind, \
                self.__mdtype, False)
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
            raise ValueError(\
                "save: the requested model might have been released!")

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

class Lasso(object):
    """A python wrapper of Frovedis Lasso Regression"""
    # defaults are as per Frovedis
    # lr_rate: Frovedis: 0.01 (added)
    # solver: Frovedis: sag (SGD) (added)
    def __init__(self, alpha=0.01, fit_intercept=True, normalize=False,
                 precompute=False, copy_X=True, max_iter=1000,
                 tol=1e-4, warm_start=False, positive=False,
                 random_state=None, selection='cyclic',
                 lr_rate=1e-8, verbose=0, solver='sag'):
        self.alpha = alpha
        self.fit_intercept = fit_intercept
        self.normalize = normalize
        self.precompute = precompute
        self.copy_X = copy_X
        self.max_iter = max_iter
        self.tol = tol
        self.warm_start = warm_start
        self.positive = positive
        self.random_state = random_state
        self.selection = selection
        # extra
        self.lr_rate = lr_rate
        self.verbose = verbose
        self.solver = solver
        self.__mid = None
        self.__mdtype = None
        self.__mkind = M_KIND.LNRM
        self._coef = None
        self._intercept = None

    def fit(self, X, y, sample_weight=None):
        """
        NAME: fit
        """
        self.release()
        self.__mid = ModelID.get()
        inp_data = FrovedisLabeledPoint(X, y)
        (X, y) = inp_data.get()
        dtype = inp_data.get_dtype()
        itype = inp_data.get_itype()
        dense = inp_data.is_dense()
        self.__mdtype = dtype
        (host, port) = FrovedisServer.getServerInstance()
        if self.solver == 'sag':
            rpclib.lasso_sgd(host, port, X.get(), y.get(), \
                    self.max_iter, self.lr_rate, \
                    self.alpha, \
                    self.fit_intercept, self.tol, \
                    self.verbose, self.__mid, dtype, itype, dense)
        elif self.solver == 'lbfgs':
            rpclib.lasso_lbfgs(host, port, X.get(), y.get(), \
                    self.max_iter, self.lr_rate, \
                    self.alpha, \
                    self.fit_intercept, self.tol,\
                    self.verbose, self.__mid, dtype, itype, dense)
        else:
            raise ValueError( \
            "Unknown solver %s for Lasso Regression." % self.solver)
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
            "attribute 'coef_' of LassoRegression object is not writable")

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
        "attribute 'intercept_' of LassoRegression object is not writable")

    def predict(self, X):
        """
        NAME: predict
        """
        if self.__mid is not None:
            return GLM.predict(X, self.__mid, self.__mkind, \
                self.__mdtype, False)
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
            raise ValueError(\
                "save: the requested model might have been released!")

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

class Ridge(object):
    """A python wrapper of Frovedis Ridge Regression"""
    # defaults are as per Frovedis
    # lr_rate: Frovedis: 0.01 (added)
    def __init__(self, alpha=0.01, fit_intercept=True, normalize=False,
                 copy_X=True, max_iter=1000, tol=1e-3, solver='sag',
                 random_state=None, lr_rate=1e-8, verbose=0):
        self.alpha = alpha
        self.fit_intercept = fit_intercept
        self.normalize = normalize
        self.copy_X = copy_X
        self.max_iter = max_iter
        self.tol = tol
        self.solver = solver
        self.random_state = random_state
        # extra
        self.lr_rate = lr_rate
        self.verbose = verbose
        self.__mid = None
        self.__mdtype = None
        self.__mkind = M_KIND.LNRM
        self._coef = None
        self._intercept = None

    def fit(self, X, y, sample_weight=None):
        """
        NAME: fit
        """
        self.release()
        self.__mid = ModelID.get()
        inp_data = FrovedisLabeledPoint(X, y)
        (X, y) = inp_data.get()
        dtype = inp_data.get_dtype()
        itype = inp_data.get_itype()
        dense = inp_data.is_dense()
        self.__mdtype = dtype

        sv = ['svd', 'cholesky', 'lsqr', 'sparse_cg']
        if self.solver in sv:
            raise ValueError( \
            "Frovedis doesn't support solver %s for Ridge \
            Regression currently." % self.solver)
        (host, port) = FrovedisServer.getServerInstance()
        if self.solver == 'sag' or self.solver == 'auto':
            rpclib.ridge_sgd(host, port, X.get(), y.get(), \
                            self.max_iter, self.lr_rate, \
                            self.alpha, \
                            self.fit_intercept, self.tol, \
                            self.verbose, self.__mid, \
                            dtype, itype, dense)
        elif self.solver == 'lbfgs':
            rpclib.ridge_lbfgs(host, port, X.get(), y.get(), \
                              self.max_iter, self.lr_rate, \
                              self.alpha, \
                              self.fit_intercept, self.tol, \
                              self.verbose, self.__mid, \
                              dtype, itype, dense)
        else:
            raise ValueError( \
            "Unknown solver %s for Ridge Regression." % self.solver)
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
            "attribute 'coef_' of LassoRegression object is not writable")

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
            raise AttributeError("attribute 'intercept_'\
                might have been released or called before fit")

    @intercept_.setter
    def intercept_(self, val):
        """intercept_ setter"""
        raise AttributeError(\
            "attribute 'intercept_' of LassoRegression object is not writable")

    def predict(self, X):
        """
        NAME: predict
        """
        if self.__mid is not None:
            return GLM.predict(X, self.__mid, self.__mkind, \
                self.__mdtype, False)
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
            raise ValueError(\
                "save: the requested model might have been released!")

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

class SGDClassifier(object):
    """
    A python wrapper for SGD classifier
    """
    def __init__(self, loss="hinge", penalty='l2', alpha=0.0001, l1_ratio=0.15,
                 fit_intercept=True, max_iter=1000, tol=1e-3, shuffle=True,
                 verbose=0, epsilon=0.1, n_jobs=None,
                 random_state=None, learning_rate="invscaling", eta0=1.0,
                 power_t=0.5, early_stopping=False, validation_fraction=0.1,
                 n_iter_no_change=5, class_weight=None, warm_start=False,
                 average=False):
        self.loss = loss
        self.penalty = penalty
        self.alpha = alpha
        self.l1_ratio = l1_ratio
        self.fit_intercept = fit_intercept
        self.max_iter = max_iter
        self.tol = tol
        self.shuffle = shuffle
        self.verbose = verbose
        self.epsilon = epsilon
        self.n_jobs = n_jobs
        self.random_state = random_state
        self.learning_rate = learning_rate
        self.eta0 = eta0
        self.power_t = power_t
        self.early_stopping = early_stopping
        self.validation_fraction = validation_fraction
        self.n_iter_no_change = n_iter_no_change
        self.class_weight = class_weight
        self.warm_start = warm_start
        self.average = average
        # extra
        self.__mid = None
        self.__mdtype = None
        self.__mkind = None
        self.label_map = None
        self.n_classes = None
        self._classes = None
        self._intercept = None
        self._coef = None

    def fit(self, X, y, coef_init=None, intercept_init=None, \
            sample_weight=None):
        """
        Fit method for SGDclassifier
        """
        self.release()
        import warnings
        if self.power_t != 0.5:
            warnings.warn(\
                " Parameter power_t has been set to" + str(self.power_t) + \
                          " However, power_t will be set to 0.5 internally")
        if self.learning_rate != 'invscaling':
            warnings.warn(" Parameter learning_rate has been set to" + \
                          str(self.learning_rate) + \
              " However, learning_rate will be set to invscaling internally")
        if self.alpha < 0:
            raise ValueError("alpha must be >= 0")
        inp_data = FrovedisLabeledPoint(X, y)
        (X, y) = inp_data.get()
        dtype = inp_data.get_dtype()
        itype = inp_data.get_itype()
        dense = inp_data.is_dense()
        unique_labels = inp_data.get_distinct_labels()
        self.n_classes = unique_labels.size
        self.__mid = ModelID.get()
        self.__mdtype = dtype
        rparam = self.alpha

        if self.penalty == 'l1':
            regTyp = 1
        elif self.penalty == 'l2':
            regTyp = 2
        elif self.penalty == 'none':
            regTyp = 0
        else:
            raise ValueError( \
                "Unsupported penalty is provided: ", self.penalty)

        (host, port) = FrovedisServer.getServerInstance()
        if self.loss == "log":
            if self.n_classes == 2:
                isMult = False
                self.__mkind = M_KIND.LRM
                target = [-1, 1]
                encoded_y, logic = \
                     y.encode(unique_labels, target, need_logic=True)
            elif self.n_classes > 2:
                isMult = True
                self.__mkind = M_KIND.MLR
                encoded_y, logic = y.encode(need_logic=True)
            else:
                raise ValueError(\
                    "fit: number of unique labels in y are less than 2")
            self.label_map = logic
            rpclib.lr_sgd(host, port, X.get(), encoded_y.get(), self.max_iter, \
                       self.eta0, regTyp, rparam, isMult, \
                       self.fit_intercept, self.tol, self.verbose,\
                       self.__mid, dtype, itype, dense)
        elif self.loss == "hinge":
            if self.n_classes != 2:
                raise ValueError(\
        "SGDClassifier: loss = 'hinge' supports only binary classification!")
            self.__mkind = M_KIND.SVM
            target = [-1, 1]
            encoded_y, logic = y.encode(unique_labels, target, need_logic=True)
            self.label_map = logic
            rpclib.svm_sgd(host, port, X.get(), encoded_y.get(), \
                        self.max_iter, self.eta0, \
                        regTyp, rparam, self.fit_intercept, self.tol, \
                        self.verbose, self.__mid, dtype, itype, dense)
        elif self.loss == "squared_loss":
            self.__mkind = M_KIND.LNRM
            self.n_classes = -1 # meaningless for regression case
            rpclib.lnr2_sgd(host, port, X.get(), y.get(), \
                        self.max_iter, self.eta0, \
                        regTyp, rparam, self.fit_intercept, self.tol, \
                        self.verbose, self.__mid, dtype, itype, dense)
        else:
            raise ValueError("Supported loss are only logic and hinge!")

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
                if self.__mkind == M_KIND.LRM or self.__mkind == M_KIND.SVM \
                    or self.__mkind == M_KIND.LNRM:
                    n_features = len(wgt)
                    shape = (1, n_features)
                else: # MLR case
                    n_features = len(wgt) // self.n_classes
                    shape = (self.n_classes, n_features)
                self._coef = np.asarray(wgt).reshape(shape)
            return self._coef
        else:
            raise AttributeError(\
            "attribute 'coef_' might have been released or called before fit")

    @coef_.setter
    def coef_(self, val):
        """coef_ setter"""
        raise AttributeError(\
            "attribute 'coef_' of SGDClassifier object is not writable")

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
            "attribute 'intercept_' of SGDClassifier object is not writable")

    @property
    def classes_(self):
        """classes_ getter"""
        if self.__mid is not None:
            if self.__mkind == M_KIND.LNRM:
                raise AttributeError(\
                "attribute 'classes_' is not available for squared_loss")
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
            "attribute 'classes_' of SGDClassifier object is not writable")

    def predict(self, X):
        """
        NAME: predict for SGD classifier
        """
        if self.__mid is not None:
            frov_pred = GLM.predict(X, self.__mid, self.__mkind, \
                                    self.__mdtype, False)
            if self.__mkind == M_KIND.LNRM:
                return np.asarray(frov_pred, dtype=np.float64)
            else:
                new_pred = \
                [self.label_map[frov_pred[i]] for i in range(0, len(frov_pred))]
                return np.asarray(new_pred, dtype=np.int64)
        else:
            raise ValueError( \
            "predict is called before calling fit, or the model is released.")

    def predict_proba(self, X):
        """
        NAME: predict_proba
        """
        if self.__mid is not None:
            if self.__mkind == M_KIND.LNRM:
                raise AttributeError(\
                "attribute 'predict_proba' is not available for squared_loss")
            proba = GLM.predict(X, self.__mid, self.__mkind, \
                               self.__mdtype, True)
            return np.asarray(proba)
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
        if self.__mkind != M_KIND.LNRM:
            target = open(fname+"/label_map", "rb")
            self.label_map = pickle.load(target)
            target.close()
        metadata = open(fname+"/metadata", "rb")
        self.loss, self.n_classes, self.__mkind, self.__mdtype = \
            pickle.load(metadata)
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
            if self.__mkind != M_KIND.LNRM:
                target = open(fname+"/label_map", "wb")
                pickle.dump(self.label_map, target)
                target.close()
            metadata = open(fname+"/metadata", "wb")
            pickle.dump(\
            (self.loss, self.n_classes, self.__mkind, self.__mdtype), metadata)
            metadata.close()
        else:
            raise AttributeError(\
                "save: requested model might have been released!")

    def debug_print(self):
        """
        NAME: debug_print for SGD classifier
        """
        if self.__mid is not None:
            GLM.debug_print(self.__mid, self.__mkind, self.__mdtype)

    def release(self):
        """
        NAME: release for SGD classifier
        """
        if self.__mid is not None:
            GLM.release(self.__mid, self.__mkind, self.__mdtype)
            self.__mid = None
            self._coef = None
            self._intercept = None
            self._classes = None

    def __del__(self):
        """
        NAME: __del__ for SGD classifier
        """
        if FrovedisServer.isUP():
            self.release()

