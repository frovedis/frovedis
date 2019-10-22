"""
svm.py: wrapper of frovedis Linear SVM
"""

#!/usr/bin/env python
import os.path
import pickle
from .model_util import *
from ..exrpc import rpclib
from ..exrpc.server import FrovedisServer
from ..matrix.ml_data import FrovedisLabeledPoint
from ..matrix.dtype import TypeUtil

class LinearSVC(object):
    """A python wrapper of Frovedis Linear SVM"""
    # defaults are as per Frovedis
    # C (alpha): Frovedis: 0.01, Sklearn: 1.0
    # loss: Frovedis: 'hinge', Sklearn: 'squared_hinge'
    def __init__(self, penalty='l2', loss='hinge', dual=True, tol=1e-4,
                 C=0.01, multi_class='ovr', fit_intercept=True,
                 intercept_scaling=1, class_weight=None, verbose=0,
                 random_state=None, max_iter=1000, solver='sag'):
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

        regTyp = 0
        if self.penalty == 'l1':
            regTyp = 1
        elif self.penalty == 'l2':
            regTyp = 2
        else:
            raise ValueError("Invalid penalty is provided: ", self.penalty)

        #Encoder
        target = [-1, 1]
        encoded_y, logic = y.encode(unique_labels, target, need_logic=True)
        self.label_map = logic
        (host, port) = FrovedisServer.getServerInstance()
        if self.solver == 'sag':
            rpclib.svm_sgd(host, port, X.get(), encoded_y.get(), \
                           self.max_iter, self.C, regTyp, self.fit_intercept, \
                           self.tol, self.verbose, self.__mid, dtype, \
                           itype, dense)

        elif self.solver == 'lbfgs':
            rpclib.svm_lbfgs(host, port, X.get(), encoded_y.get(), \
                             self.max_iter, \
                             self.C, regTyp, self.fit_intercept, self.tol, \
                             self.verbose, self.__mid, dtype, itype, dense)
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
                    np.asarray(self.label_map.values(), dtype=np.int64)
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
            pickle.dump((self.n_classes, \
                    self.__mkind, self.__mdtype), metadata)
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

