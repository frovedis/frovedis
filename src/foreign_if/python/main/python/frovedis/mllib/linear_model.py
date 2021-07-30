"""
linear_model.py: wrapper of frovedis Logistic Regression, Linear Regression,
                 Lasso, Ridge Regression, SGDClassifier and SGDRegressor
"""
import os.path
import pickle
import warnings
import numpy as np
from .model_util import *
from .metrics import *
from ..base import BaseEstimator
from ..exrpc import rpclib
from ..exrpc.server import FrovedisServer, set_association, \
                           check_association, do_if_active_association
from ..matrix.ml_data import FrovedisLabeledPoint
from ..matrix.dtype import TypeUtil
from ..utils import check_sample_weight

class LogisticRegression(BaseEstimator):
    """
    A python wrapper of Frovedis Logistic Regression
    defaults are as per Frovedis
    penalty: Frovedis: none, Sklearn: l2
    max_iter: Frovedis: 1000, Sklearn: 100
    solver: Frovedis: sag (SGD), Sklearn: lbfgs
    lr_rate: Frovedis: 0.01 (added)
    use_shrink: Frovedis: false (added)
    """
    def __init__(self, penalty='l2', dual=False, tol=1e-4, C=1.0,
                 fit_intercept=True, intercept_scaling=1, class_weight=None,
                 random_state=None, solver='lbfgs', max_iter=1000,
                 multi_class='auto', verbose=0, warm_start=False,
                 n_jobs=1, l1_ratio=None, lr_rate=0.01, use_shrink=False):
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
        self.use_shrink = use_shrink
        self.__mid = ModelID.get()
        self.__mdtype = None
        self.__mkind = None
        self.label_map = None
        self.n_classes = None
        self._classes = None
        self._intercept = None
        self._coef = None
        self._n_iter = None
        self.n_samples = None
        self.n_features = None
        self.isMult = None
        self.isFitted = False

    def check_input(self, X, y, F):
        """checks input X"""
        # for binary case: frovedis supports -1 and 1
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

    @set_association
    def fit(self, X, y, sample_weight=None):
        """
        NAME: fit
        """
        if self.C < 0:
            raise ValueError("fit: parameter C must be strictly positive!")
        self.reset_metadata()
        X, y, dtype, itype, dense = self.check_input(X, y, "fit")
        self.__mdtype = dtype

        if dense and self.use_shrink:
            raise ValueError("fit: use_shrink is applicable only for " \
                             + "sparse data!")

        if self.use_shrink:
            if self.solver == "lbfgs":
                raise ValueError("fit: use_shrink is applicable only for " \
                                 + "sgd solver!")

        self.__mkind = M_KIND.LR
        if self.multi_class == 'auto' or self.multi_class == 'ovr':
            if self.n_classes == 2:
                isMult = False
            else:
                isMult = True
        elif self.multi_class == 'multinomial':
            isMult = True # even for binary data
        else:
            raise ValueError("Unknown multi_class: %s!" % self.multi_class)

        if isMult and self.solver != 'sag':
            self.solver = 'sag' #only sag solver supports multinomial currently
            warnings.warn("fit: multinomial classification problem is " +
                          "detected... switching solver to 'sag'.\n")

        if self.penalty == 'l1':
            regTyp = 1
        elif self.penalty == 'l2':
            regTyp = 2
        elif self.penalty == 'none':
            regTyp = 0
        else:
            raise ValueError("Unsupported penalty is provided: ", self.penalty)

        rparam = 1.0 / self.C * 0.5 # added 0.5 as in sklearn
        sample_weight = check_sample_weight(self, sample_weight)
        solver = self.solver
        if solver == 'sag':
            solver = 'sgd'
        elif solver == 'lbfgs':
            regTyp = 2 # l2 is supported for lbfgs
        else:
            raise ValueError( \
                "Unknown solver %s for Logistic Regression." % solver)
        (host, port) = FrovedisServer.getServerInstance()
        n_iter = rpclib.lr(host, port, X.get(), y.get(), \
                       sample_weight, len(sample_weight), self.max_iter, \
                       self.lr_rate, regTyp, rparam, isMult, \
                       self.fit_intercept, self.tol, self.verbose, \
                       self.__mid, dtype, itype, dense, \
                       solver.encode('ascii'), \
                       self.use_shrink, self.warm_start)

        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        self._coef = None
        self._intercept = None
        self._n_iter = np.asarray([n_iter], dtype = np.int32)
        self.isMult = isMult
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
            if not self.isMult:
                n_features = len(wgt)
                shape = (1, n_features)
            else:
                n_features = len(wgt) // self.n_classes
                shape = (self.n_classes, n_features)
            self._coef = np.asarray(wgt).reshape(shape)
        return self._coef

    @coef_.setter
    def coef_(self, val):
        """coef_ setter"""
        raise AttributeError(\
            "attribute 'coef_' of LogisticRegression object is not writable")

    @property
    @check_association
    def intercept_(self):
        """intercept getter"""
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
        """ intercept_ setter"""
        raise AttributeError(\
        "attribute 'intercept_' of LogisticRegression object is not writable")

    @property
    def classes_(self):
        """classes_ getter"""
        if not self.is_fitted():
            raise AttributeError("attribute 'classes_'" \
               "might have been released or called before fit")
        if self._classes is None:
            self._classes = np.sort(list(self.label_map.values()))
        return self._classes

    @classes_.setter
    def classes_(self, val):
        """classes_ setter"""
        raise AttributeError(\
          "attribute 'classes_' of LogisticRegression object is not writable")

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
            "attribute 'n_iter_' of LogisticRegression object is not writable")

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
        proba = GLM.predict(X, self.__mid, self.__mkind, \
                            self.__mdtype, True, self.n_classes)
        n_samples = len(proba) // self.n_classes
        shape = (n_samples, self.n_classes)
        return np.asarray(proba, dtype=np.float64).reshape(shape)

    # calculate the mean accuracy on the given test data and labels.
    def score(self, X, y, sample_weight=None):
        """
        NAME: score
        """
        return accuracy_score(y, self.predict(X), sample_weight=sample_weight)

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
        metadata = open(fname + "/metadata", "rb")
        self.n_classes, self.__mkind, self.__mdtype = pickle.load(metadata)
        metadata.close()
        if dtype is not None:
            mdt = TypeUtil.to_numpy_dtype(self.__mdtype)
            if dtype != mdt:
                raise ValueError("load: type mismatches detected! " + \
                                 "expected type: " + str(mdt) + \
                                 "; given type: " + str(dtype))
        GLM.load(self.__mid, self.__mkind, self.__mdtype, fname+"/model")
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
        pickle.dump((self.n_classes, self.__mkind, self.__mdtype), metadata)
        metadata.close()

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
        self._classes = None
        self._n_iter = None
        self.n_samples = None
        self.isMult = None
        self.isFitted = False
        self.n_features = None
        self.n_classes = None # check if release is merged
        self.label_map = None

    def release(self):
        """
        resets after-fit populated attributes to None
        along with relasing server side memory
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
        destructs the python object
        """
        self.reset_metadata()

    def is_fitted(self):
        """ function to confirm if the model is already fitted """
        return self.isFitted

class LinearRegression(BaseEstimator):
    """A python wrapper of Frovedis Linear Regression
    max_iter: Frovedis: 1000 (added)
    solver: Frovedis: None (default value is chosen
            based on training matrix type) (added)
    lr_rate: Frovedis: 0.01 (added)
    tol: Frovedis: 0.0001 (added)
    """
    def __init__(self, fit_intercept=True, normalize=False, copy_X=True,
                 n_jobs=None, max_iter=None, tol=0.0001, lr_rate=1e-8,
                 solver=None, verbose=0, warm_start = False):
        self.fit_intercept = fit_intercept
        self.normalize = normalize
        self.copy_X = copy_X
        self.n_jobs = n_jobs
        self.warm_start = warm_start
        # extra
        self.max_iter = max_iter
        self.tol = tol
        self.lr_rate = lr_rate
        self.solver = solver
        self.verbose = verbose
        self.__mid = ModelID.get()
        self.__mdtype = None
        self.__mkind = M_KIND.LNRM
        self._intercept = None
        self._coef = None
        self.n_samples = None
        self.n_features = None
        self._n_iter = None
        self.singular_ = None
        self.rank_ = None
        self.isFitted = None

    def check_input(self, X, y, F):
        """checks input X"""
        inp_data = FrovedisLabeledPoint(X, y, \
                   caller = "[" + self.__class__.__name__ + "] " + F + ": ",\
                   dense_kind = 'colmajor', densify=False)

        X, y = inp_data.get()
        self.n_samples = inp_data.numRows()
        self.n_features = inp_data.numCols()
        itype = inp_data.get_itype()
        dense = inp_data.is_dense()
        dtype = inp_data.get_dtype()
        return X, y, dtype, itype, dense

    @set_association
    def fit(self, X, y, sample_weight=None):
        """
        NAME: fit
        """
        self.reset_metadata()
        X, y, dtype, itype, dense = self.check_input(X, y, "fit")
        self.__mdtype = dtype

        # select default solver, when None is given
        if self.solver is None:
            if dense:
                self.solver = 'lapack'      # ?gelsd for dense X
            else:
                self.solver = 'sparse_lsqr' # sparse_lsqr for sparse X
        else:
            self.solver = self.solver

        if self.solver in ('lapack', 'scalapack'):
            if not dense:
                raise TypeError("%s solver supports only dense feature data!" \
                                % (self.solver))
            if self.warm_start:
                raise TypeError("%s solver does not support warm_start!" \
                                % (self.solver))
        elif self.solver in ('sparse_lsqr'):
            if dense:
                raise TypeError("%s solver supports only sparse feature data!" \
                                % (self.solver))
            if self.warm_start:
                raise TypeError("%s solver does not support warm_start!" \
                                % (self.solver))
        elif self.solver not in ('sag', 'sgd', 'lbfgs'):
            raise ValueError( \
            "Unknown solver %s for Linear Regression." % self.solver)

        if self.max_iter is None:
            if self.solver == 'sparse_lsqr':
                niter = 2 * X.numCols() 
            else:
                niter = 1000 # default for sag and lbfgs
        else:
            niter = self.max_iter

        sample_weight = check_sample_weight(self, sample_weight)
        (host, port) = FrovedisServer.getServerInstance()
        solver = self.solver
        if solver == 'sag':
            solver = 'sgd'
        res = rpclib.lnr(host, port, X.get(), y.get(), \
                         sample_weight, len(sample_weight), \
                         niter, self.lr_rate, \
                         self.fit_intercept, self.tol, self.verbose, self.__mid, \
                         dtype, itype, dense, solver.encode('ascii'), self.warm_start)
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        if solver == 'lapack': #singular_ and rank_ available only for lapack solver
            sval = res['singular']
            self.singular_ = np.asarray(sval, TypeUtil.to_numpy_dtype(dtype))
            self.rank_ = int(res['rank'])
        if solver not in ('lapack', 'scalapack'):
            self._n_iter = res['n_iter']
        self._coef = None
        self._intercept = None
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
            self._coef = np.asarray(wgt)
        return self._coef

    @coef_.setter
    def coef_(self, val):
        """coef_ setter"""
        raise AttributeError("attribute 'coef_' \
            of LinearRegression object is not writable")

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
            self._intercept = icpt
        return self._intercept

    @intercept_.setter
    def intercept_(self, val):
        """intercept_ setter"""
        raise AttributeError(\
            "attribute 'intercept_' of LinearRegression object is not writable")

    @property
    def n_iter_(self):
        """n_iter_ getter"""
        if  not self.is_fitted():
            raise AttributeError("attribute 'n_iter_'" \
               "might have been released or called before fit")
        return self._n_iter

    @n_iter_.setter
    def n_iter_(self, val):
        """n_iter_ setter"""
        raise AttributeError(\
            "attribute 'n_iter_' of LinearRegression object is not writable")

    @check_association
    def predict(self, X):
        """
        NAME: predict
        """
        ret = GLM.predict(X, self.__mid, self.__mkind, \
                          self.__mdtype, False)
        return np.asarray(ret, dtype = np.float64)

    # calculate the root mean square value on the given test data and labels.
    def score(self, X, y, sample_weight=None):
        """
        NAME: score
        """
        return r2_score(y, self.predict(X), sample_weight=sample_weight)

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
        GLM.load(self.__mid, self.__mkind, self.__mdtype, fname+"/model")
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
        self.n_samples = None
        self._n_iter = None
        self.isFitted = None
        self.n_features = None

    def release(self):
        """
        resets after-fit populated attributes to None
        along with relasing server side memory
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
        destructs the python object
        """
        self.reset_metadata()

    def is_fitted(self):
        """ function to confirm if the model is already fitted """
        return self.isFitted

class Lasso(BaseEstimator):
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
        self.__mid = ModelID.get()
        self.__mdtype = None
        self.__mkind = M_KIND.LSR
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

    @set_association
    def fit(self, X, y, sample_weight=None):
        """
        NAME: fit
        """
        self.reset_metadata()
        X, y, dtype, itype, dense = self.check_input(X, y, "fit")
        self.__mdtype = dtype

        if self.max_iter is None:
            self.max_iter = 1000

        sample_weight = check_sample_weight(self, sample_weight)
        (host, port) = FrovedisServer.getServerInstance()
        supported_solver = ['sgd', 'lbfgs']
        solver = self.solver
        if solver == 'sag':
            solver = 'sgd'
        if solver not in supported_solver:
            raise ValueError( \
            "Unknown solver %s for Lasso Regression." % solver)
        n_iter = rpclib.lasso(host, port, X.get(), y.get(), \
                       sample_weight, len(sample_weight), \
                       self.max_iter, self.lr_rate, \
                       self.alpha, self.fit_intercept, self.tol, \
                       self.verbose, self.__mid, dtype, itype, dense, \
                       solver.encode('ascii'), self.warm_start)
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
            self._coef = np.asarray(wgt)
        return self._coef

    @coef_.setter
    def coef_(self, val):
        """coef_ setter"""
        raise AttributeError(\
            "attribute 'coef_' of LassoRegression object is not writable")

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
            self._intercept = icpt
        return self._intercept

    @intercept_.setter
    def intercept_(self, val):
        """intercept_ setter"""
        raise AttributeError(\
        "attribute 'intercept_' of LassoRegression object is not writable")

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
            "attribute 'n_iter_' of Lasso Regression object is not writable")

    @check_association
    def predict(self, X):
        """
        NAME: predict
        """
        ret = GLM.predict(X, self.__mid, self.__mkind, \
                          self.__mdtype, False)
        return np.asarray(ret, dtype = np.float64)

    # calculate the root mean square value on the given test data and labels.
    def score(self, X, y, sample_weight=None):
        """
        NAME: score
        """
        return r2_score(y, self.predict(X), sample_weight=sample_weight)

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
        GLM.load(self.__mid, self.__mkind, self.__mdtype, fname+"/model")
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
        self.isFitted = None
        self.n_features = None

    def release(self):
        """
        resets after-fit populated attributes to None
        along with relasing server side memory
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
        destructs the python object
        """
        self.reset_metadata()

    def is_fitted(self):
        """ function to confirm if the model is already fitted """
        return self.isFitted

class Ridge(BaseEstimator):
    """A python wrapper of Frovedis Ridge Regression"""
    # defaults are as per Frovedis
    # lr_rate: Frovedis: 0.01 (added)
    def __init__(self, alpha=0.01, fit_intercept=True, normalize=False,
                 copy_X=True, max_iter=None, tol=1e-3, solver='auto',
                 random_state=None, lr_rate=1e-8, verbose=0,
                 warm_start = False):
        self.alpha = alpha
        self.fit_intercept = fit_intercept
        self.normalize = normalize
        self.copy_X = copy_X
        self.max_iter = max_iter
        self.tol = tol
        self.solver = solver
        self.random_state = random_state
        self.warm_start = warm_start
        # extra
        self.lr_rate = lr_rate
        self.verbose = verbose
        self.__mid = ModelID.get()
        self.__mdtype = None
        self.__mkind = M_KIND.RR
        self._coef = None
        self._intercept = None
        self.n_samples = None
        self.n_features = None
        self._n_iter = None
        self.isFitted = False

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

    @set_association
    def fit(self, X, y, sample_weight=None):
        """
        NAME: fit
        """
        self.reset_metadata()
        X, y, dtype, itype, dense = self.check_input(X, y, "fit")
        self.__mdtype = dtype
        sv = ['svd', 'cholesky', 'lsqr', 'sparse_cg']
        if self.solver in sv:
            raise ValueError( \
            "Frovedis doesn't support solver %s for Ridge "\
            "Regression currently." % self.solver)
        if self.max_iter is None:
            self.max_iter = 1000
        sample_weight = check_sample_weight(self, sample_weight)
        (host, port) = FrovedisServer.getServerInstance()
        supported_solver = ['sgd', 'lbfgs']
        solver = self.solver
        if solver in ['sag', 'auto']:
            solver = 'sgd'
        if solver not in supported_solver:
            raise ValueError( \
            "Unknown solver %s for Ridge Regression." % solver)
        n_iter = rpclib.ridge(host, port, X.get(), y.get(), \
                       sample_weight, len(sample_weight), \
                       self.max_iter, self.lr_rate, \
                       self.alpha, self.fit_intercept, self.tol, \
                       self.verbose, self.__mid, \
                       dtype, itype, dense, solver.encode('ascii'),
                       self.warm_start)
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        self._coef = None
        self._intercept = None
        self._n_iter = np.asarray([n_iter], dtype = np.int32)
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
            self._coef = np.asarray(wgt)
        return self._coef

    @coef_.setter
    def coef_(self, val):
        """coef_ setter"""
        raise AttributeError(\
            "attribute 'coef_' of Ridge regression object is not writable")

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
            self._intercept = icpt
        return self._intercept

    @intercept_.setter
    def intercept_(self, val):
        """intercept_ setter"""
        raise AttributeError(\
            "attribute 'intercept_' of Ridge regression object is not writable")

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
            "attribute 'n_iter_' of Ridge Regression object is not writable")

    @check_association
    def predict(self, X):
        """
        NAME: predict
        """
        ret = GLM.predict(X, self.__mid, self.__mkind, \
                          self.__mdtype, False)
        return np.asarray(ret, dtype = np.float64)

    # calculate the root mean square value on the given test data and labels.
    def score(self, X, y, sample_weight=None):
        """
        NAME: score
        """
        return r2_score(y, self.predict(X), sample_weight=sample_weight)

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
        GLM.load(self.__mid, self.__mkind, self.__mdtype, fname+"/model")
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
        self.isFitted = False
        self.n_features = None

    def release(self):
        """
        resets after-fit populated attributes to None
        along with relasing server side memory
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
        destructs the python object
        """
        self.reset_metadata()

    def is_fitted(self):
        """ function to confirm if the model is already fitted """
        return self.isFitted

class SGDClassifier(BaseEstimator):
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
        self.__mid = ModelID.get()
        self.__mdtype = None
        self.__mkind = None
        self.label_map = None
        self.n_classes = None
        self._classes = None
        self._intercept = None
        self._coef = None
        self._n_iter = None
        self.n_samples = None
        self.n_features = None
        self.isFitted = None
        self.is_mult = None

    def validate(self):
        """validates hyper parameters"""
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

    def check_input(self, X, y, F):
        """checks input X"""
        if self.loss == "squared_loss":
            inp_data = FrovedisLabeledPoint(X, y, \
                       caller = "[" + self.__class__.__name__ + "] " + F  + ": ",\
                       dense_kind = 'colmajor', densify=False)
            X, y = inp_data.get()
        else:
            # for binary case: frovedis supports -1 and 1
            inp_data = FrovedisLabeledPoint(X, y, \
                       caller = "[" + self.__class__.__name__ + "] " + F + ": ",\
                       encode_label = True, binary_encoder=[-1, 1], \
                       dense_kind = 'colmajor', densify=False)
            X, y, logic = inp_data.get()
            self._classes = inp_data.get_distinct_labels()
            self.n_classes = len(self._classes)
            self.label_map = logic
        self.n_samples = inp_data.numRows()
        self.n_features = inp_data.numCols()
        dtype = inp_data.get_dtype()
        itype = inp_data.get_itype()
        dense = inp_data.is_dense()
        return X, y, dtype, itype, dense

    @set_association
    def fit(self, X, y, coef_init=None, intercept_init=None, \
            sample_weight=None):
        """
        Fit method for SGDclassifier
        """
        self.reset_metadata()
        self.validate()

        X, y, dtype, itype, dense = self.check_input(X, y, "fit")
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

        sample_weight = check_sample_weight(self, sample_weight)
        (host, port) = FrovedisServer.getServerInstance()
        if self.loss == "log":
            self.__mkind = M_KIND.LR
            if self.n_classes == 2:
                self.is_mult = False
            else:
                self.is_mult = True
            n_iter = rpclib.lr(host, port, X.get(), y.get(),
                            sample_weight, len(sample_weight), \
                            self.max_iter, self.eta0, \
                            regTyp, rparam, self.is_mult, \
                            self.fit_intercept, self.tol, self.verbose, \
                            self.__mid, dtype, itype, dense, "sgd".encode('ascii'), False, \
                            self.warm_start)
        elif self.loss == "hinge":
            if self.n_classes != 2:
                raise ValueError("SGDClassifier: loss = 'hinge' supports" + \
                                 " only binary classification!")
            self.__mkind = M_KIND.SVM
            n_iter = rpclib.svm(host, port, X.get(), y.get(), \
                            sample_weight, len(sample_weight), \
                            self.max_iter, self.eta0, \
                            regTyp, rparam, self.fit_intercept, self.tol, \
                            self.verbose, self.__mid, dtype, itype, dense, \
                            'sgd'.encode('ascii'), self.warm_start)
        elif self.loss == "squared_loss":
            model_kind = [M_KIND.LNRM, M_KIND.LSR, M_KIND.RR]
            self.__mkind = model_kind[regTyp]
            n_iter = rpclib.lnr2_sgd(host, port, X.get(), y.get(), \
                            sample_weight, len(sample_weight), \
                            self.max_iter, self.eta0, \
                            regTyp, rparam, self.fit_intercept, self.tol, \
                            self.verbose, self.__mid, dtype, itype, dense, \
                            self.warm_start)
        else:
            raise ValueError("SGDClassifier: supported losses are log, " + \
                             "hinge and squared_loss only!")
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
            if not self.is_mult:
                n_features = len(wgt)
                shape = (1, n_features)
            else: # MLR case
                n_features = len(wgt) // self.n_classes
                shape = (self.n_classes, n_features)
            self._coef = np.asarray(wgt).reshape(shape)
        return self._coef

    @coef_.setter
    def coef_(self, val):
        """coef_ setter"""
        raise AttributeError(\
            "attribute 'coef_' of SGDClassifier object is not writable")

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
            "attribute 'intercept_' of SGDClassifier object is not writable")

    @property
    def classes_(self):
        """classes_ getter"""
        if not self.is_fitted():
            raise AttributeError("attribute 'classes_' might have been " \
                                 "released or called before fit")
        if self.__mkind == M_KIND.LNRM:
            raise AttributeError(\
            "attribute 'classes_' is not available for squared_loss")
        if self._classes is None:
            self._classes = np.sort(list(self.label_map.values()))
        return self._classes

    @classes_.setter
    def classes_(self, val):
        """classes_ setter"""
        raise AttributeError(\
            "attribute 'classes_' of SGDClassifier object is not writable")

    @property
    def n_iter_(self):
        """n_iter_ getter"""
        if self.__mid is None:
            raise AttributeError("attribute 'n_iter_'" \
               "might have been released or called before fit")
        return self._n_iter

    @n_iter_.setter
    def n_iter_(self, val):
        """n_iter_ setter"""
        raise AttributeError(\
            "attribute 'n_iter_' of SGDClassifier  object is not writable")

    def predict(self, X):
        """
        NAME: predict for SGD classifier
        """
        frov_pred = GLM.predict(X, self.__mid, self.__mkind, \
                                self.__mdtype, False)
        if self.__mkind in [M_KIND.LNRM, M_KIND.LSR, M_KIND.RR]:
            return np.asarray(frov_pred, dtype=np.float64)
        else:
            return np.asarray([self.label_map[frov_pred[i]] \
                          for i in range(0, len(frov_pred))])

    @check_association
    def predict_proba(self, X):
        """
        NAME: predict_proba
        """
        if self.__mkind == M_KIND.LNRM or self.__mkind == M_KIND.SVM:
            raise AttributeError("attribute 'predict_proba' is not " \
                                 "available for %s loss" % (self.loss))
        proba = GLM.predict(X, self.__mid, self.__mkind, \
                           self.__mdtype, True, self.n_classes)
        n_samples = len(proba) // self.n_classes
        shape = (n_samples, self.n_classes)
        return np.asarray(proba, dtype=np.float64).reshape(shape)

    # calculate the mean accuracy on the given test data and labels.
    def score(self, X, y, sample_weight=None):
        """
        NAME: score
        """
        if self.__mkind in [M_KIND.LNRM, M_KIND.LSR, M_KIND.RR]:
            return r2_score(y, self.predict(X), sample_weight=sample_weight)
        else:
            return accuracy_score(y, self.predict(X), sample_weight=sample_weight)

    @set_association
    def load(self, fname, dtype=None):
        """
        NAME: load
        """
        if not os.path.exists(fname):
            raise ValueError(\
                "the model with name %s does not exist!" % fname)
        self.reset_metadata()
        if self.__mkind not in [M_KIND.LNRM, M_KIND.LSR, M_KIND.RR]:
            target = open(fname + "/label_map", "rb")
            self.label_map = pickle.load(target)
            target.close()
            self._classes = np.sort(list(self.label_map.values()))
            self.n_classes = len(self._classes)
        metadata = open(fname + "/metadata", "rb")
        self.loss, self.__mkind, self.__mdtype = \
            pickle.load(metadata)
        metadata.close()
        if dtype is not None:
            mdt = TypeUtil.to_numpy_dtype(self.__mdtype)
            if dtype != mdt:
                raise ValueError("load: type mismatches detected! " + \
                                 "expected type: " + str(mdt) + \
                                 "; given type: " + str(dtype))
        GLM.load(self.__mid, self.__mkind, self.__mdtype, fname+"/model")
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
        if self.__mkind != M_KIND.LNRM:
            target = open(fname + "/label_map", "wb")
            pickle.dump(self.label_map, target)
            target.close()
        metadata = open(fname + "/metadata", "wb")
        pickle.dump(\
        (self.loss, self.__mkind, self.__mdtype), metadata)
        metadata.close()

    @check_association
    def debug_print(self):
        """
        NAME: debug_print for SGD classifier
        """
        GLM.debug_print(self.__mid, self.__mkind, self.__mdtype)

    def reset_metadata(self):
        """
        resets after-fit populated attributes to None
        """
        self._coef = None
        self._intercept = None
        self._classes = None
        self._n_iter = None
        self.n_samples = None
        self.isFitted = None
        self.is_mult = None
        self.n_features = None

    def release(self):
        """
        resets after-fit populated attributes to None
        along with relasing server side memory
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

class SGDRegressor(BaseEstimator):
    """
    A python wrapper for SGD regressor
    """
    def __init__(self, loss="squared_loss", penalty='l2', alpha=0.0001, l1_ratio=0.15,
                 fit_intercept=True, max_iter=1000, tol=1e-3, shuffle=True,
                 verbose=0, epsilon=0.1, random_state=None, learning_rate="invscaling",
                 eta0=0.001, power_t=0.25, early_stopping=False, validation_fraction=0.1,
                 n_iter_no_change=5, warm_start=False, average=False):
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
        self.random_state = random_state
        self.learning_rate = learning_rate
        self.eta0 = eta0
        self.power_t = power_t
        self.early_stopping = early_stopping
        self.validation_fraction = validation_fraction
        self.n_iter_no_change = n_iter_no_change
        self.warm_start = warm_start
        self.average = average
        # extra
        self.__mid = ModelID.get()
        self.__mdtype = None
        self.__mkind = None
        self._intercept = None
        self._coef = None
        self.n_samples = None
        self._n_iter = None
        self.n_features = None
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

    def validate(self):
        """
        NAME: validate
        """
        if self.tol < 0:
            raise ValueError("fit: tol parameter must be zero or positive!")

        if self.max_iter <= 0:
            raise ValueError("fit: max_iter must be a positive value!")

        if self.power_t != 0.25:
            warnings.warn(\
                " Parameter power_t has been set to" + str(self.power_t) + \
                          " However, power_t will be set to 0.25 internally")

        supported_learning_rate = ("invscaling", "optimal")
        if self.learning_rate not in supported_learning_rate:
            raise ValueError("fit: Unsupported learning_rate : " + str(self.learning_rate))

        if self.alpha < 0:
            raise ValueError("fit: alpha must be >= 0")

        if self.eta0 < 0:
            raise ValueError("fit: eta0 parameter must be zero or positive!")

    @set_association
    def fit(self, X, y, coef_init=None, intercept_init=None, \
            sample_weight=None):
        """
        Fit method for SGDRegressor
        """
        # release old model, if any
        self.reset_metadata()
        self.validate()
        # perform the fit
        X, y, dtype, itype, dense = self.check_input(X, y, "fit")
        self.__mdtype = dtype

        if self.penalty == 'l1':
            regTyp = 1
        elif self.penalty == 'l2':
            regTyp = 2
        elif self.penalty == 'none':
            regTyp = 0
        else:
            raise ValueError( \
                "Unsupported penalty is provided: ", self.penalty)

        svrloss = {'epsilon_insensitive': 1,
                   'squared_epsilon_insensitive': 2}

        lnrloss = {'squared_loss': 1}

        sample_weight = check_sample_weight(self, sample_weight)
        (host, port) = FrovedisServer.getServerInstance()
        if self.loss in list(svrloss.keys()):
            self.__mkind = M_KIND.SVR
            intLoss = svrloss[self.loss]
            if self.epsilon < 0:
                raise ValueError("fit: epsilon parameter must be zero or positive!")
            n_iter = rpclib.svm_regressor(host, port, X.get(), y.get(), \
                                          sample_weight, len(sample_weight), \
                                          self.max_iter, self.eta0, \
                                          self.epsilon, regTyp, self.alpha, \
                                          self.fit_intercept, self.tol, \
                                          intLoss, self.verbose, \
                                          self.__mid, dtype, itype, dense, \
                                          "sgd".encode('ascii'), self.warm_start)
        elif self.loss in list(lnrloss.keys()):
            model_kind = [M_KIND.LNRM, M_KIND.LSR, M_KIND.RR]
            self.__mkind = model_kind[regTyp]
            n_iter = rpclib.lnr2_sgd(host, port, X.get(), y.get(), \
                                     sample_weight, len(sample_weight), \
                                     self.max_iter, self.eta0, \
                                     regTyp, self.alpha, \
                                     self.fit_intercept, self.tol, \
                                     self.verbose, self.__mid, dtype, itype, dense, \
                                     "sgd".encode('ascii'), self.warm_start)
        else:
            raise ValueError(\
             "fit: supported losses are epsilon_insensitive, " \
              + "squared_epsilon_insensitive and squared_loss!")

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
            self._coef = np.asarray(wgt)
        return self._coef

    @coef_.setter
    def coef_(self, val):
        """coef_ setter"""
        raise AttributeError(\
            "attribute 'coef_' of SGDRegressor object is not writable")

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
            "attribute 'intercept_' of SGDRegressor object is not writable")

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
            "attribute 'n_iter_' of SGDRegressor object is not writable")

    @check_association
    def predict(self, X):
        """
        NAME: predict for SGDRegressor
        """
        ret = GLM.predict(X, self.__mid, self.__mkind, \
                          self.__mdtype, False)
        return np.asarray(ret, dtype=np.float64)

    def score(self, X, y, sample_weight=None):
        """
        NAME: score
        """
        return r2_score(y, self.predict(X), sample_weight=sample_weight)

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
        self.loss, self.__mkind, self.__mdtype = pickle.load(metadata)
        metadata.close()
        if dtype is not None:
            mdt = TypeUtil.to_numpy_dtype(self.__mdtype)
            if dtype != mdt:
                raise ValueError("load: type mismatches detected! " + \
                                 "expected type: " + str(mdt) + \
                                 "; given type: " + str(dtype))
        GLM.load(self.__mid, self.__mkind, self.__mdtype, fname+"/model")
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
        pickle.dump(\
        (self.loss, self.__mkind, self.__mdtype), metadata)
        metadata.close()

    @check_association
    def debug_print(self):
        """
        NAME: debug_print for SGDRegressor
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
        self.isFitted = None
        self.n_features = None

    def release(self):
        """
        resets after-fit populated attributes to None
        along with relasing server side memory
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
