"""truncated_svd.py"""

import warnings
from ...base import *
from ...exrpc.server import FrovedisServer, set_association, \
                            check_association, do_if_active_association
from ...exrpc.rpclib import compute_truncated_svd, compute_var_sum
from ...exrpc.rpclib import compute_svd_transform
from ...exrpc.rpclib import compute_svd_self_transform
from ...exrpc.rpclib import compute_svd_inverse_transform
from ...exrpc.rpclib import check_server_exception
from ...matrix.ml_data import FrovedisFeatureData
from ...matrix.crs import FrovedisCRSMatrix
from ...matrix.dense import FrovedisRowmajorMatrix, FrovedisColmajorMatrix
from ...matrix.results import svdResult
from ...matrix.dtype import TypeUtil
import numpy as np
from scipy.sparse import issparse

class TruncatedSVD(BaseEstimator):
    """Dimensionality reduction using truncated SVD (aka LSA)."""

    def __init__(self, n_components=2, algorithm="arpack", n_iter=5,
                 random_state=None, tol=0., use_shrink=False):
        self.algorithm = algorithm
        self.n_components = n_components
        self.n_iter = n_iter
        self.random_state = random_state
        self.tol = tol
        self.use_shrink = use_shrink
        self.svd_res_ = None
        self.var_sum = None
        self._components = None
        self._explained_variance = None
        self._explained_variance_ratio = None
        self._singular_values = None

    @set_association
    def fit(self, X, y=None):
        """Fits LSA model on training data X."""
        (host, port) = FrovedisServer.getServerInstance()
        if self.algorithm != "arpack":
            raise ValueError("algorithm: currently Frovedis supports only " \
                              + "arpack!")
        # if X is not a sparse data, it would be converted as rowmajor matrix
        inp_data = FrovedisFeatureData(X, \
                     caller = "[" + self.__class__.__name__ + "] fit: ",\
                     dense_kind='rowmajor', densify=False)
        dense = inp_data.is_dense()
        if dense:
            X = inp_data.get()
            to_sample = False # ddof = 0 in np.var(...)
            self.var_sum = compute_var_sum(host, port, X.get(),
                                           to_sample, dense, X.get_dtype())
            excpt = check_server_exception()
            if excpt["status"]:
                raise RuntimeError(excpt["info"])
        elif issparse(X):
            try:
                from sklearn.utils.sparsefuncs import mean_variance_axis
                _, full_var = mean_variance_axis(X, axis=0)
                self.var_sum = full_var.sum()
            except: # for system without sklearn
                self.var_sum = None
        else:
            self.var_sum = None
        X = inp_data.get()
        x_dtype = inp_data.get_dtype()
        x_itype = inp_data.get_itype()
        self.__mdtype = x_dtype
        if dense and self.use_shrink:
            warnings.warn("fit: use_shrink is applicable only for sparse data!")

        res = compute_truncated_svd(host, port, X.get(),
                                    self.n_components,
                                    x_dtype, x_itype, dense, self.use_shrink)
        excpt = check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        self.svd_res_ = svdResult(res, TypeUtil.to_numpy_dtype(x_dtype))
        return self

    def fit_transform(self, X, y=None):
        """Fits LSA model to X and performs dimensionality reduction on X."""
        self.fit(X)
        umat = self.svd_res_.umat_            # dense (colmajor matrix)
        svec = self.svd_res_.singular_values_ # std::vector
        (host, port) = FrovedisServer.getServerInstance()
        res = compute_svd_self_transform(host, port, 
                                         umat.get(), svec.get(), 
                                         self.__mdtype)
        excpt = check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        dmat = FrovedisRowmajorMatrix(res, 
               dtype=TypeUtil.to_numpy_dtype(self.__mdtype))
        if isinstance(X, (FrovedisCRSMatrix, FrovedisRowmajorMatrix)):
            return dmat
        else:
            return dmat.to_numpy_array()

    @check_association
    def transform(self, X):
        """Performs dimensionality reduction on X."""
        (host, port) = FrovedisServer.getServerInstance()
        if isinstance(X, (FrovedisCRSMatrix, FrovedisRowmajorMatrix)):
            inp_data = FrovedisFeatureData(X, dense_kind='rowmajor', 
                                           dtype=self.__mdtype)
            X = inp_data.get()
            x_dtype = inp_data.get_dtype()
            x_itype = inp_data.get_itype()
            dense = inp_data.is_dense()
            component = self.svd_res_.vmat_ # always colmajor matrix
            if (x_dtype != self.__mdtype):
                raise TypeError("Type mismatches in input X-mat and " \
                                + "svd component!")
            res = compute_svd_transform(host, port, X.get(),
                                        x_dtype, x_itype, dense,
                                        component.get())
            excpt = check_server_exception()
            if excpt["status"]:
                raise RuntimeError(excpt["info"])
            return FrovedisRowmajorMatrix(res, 
                   dtype=TypeUtil.to_numpy_dtype(self.__mdtype))
        else:
            return np.asarray(X * self.components_.T)

    @check_association
    def inverse_transform(self, X):
        """Transforms X back to its original space."""
        (host, port) = FrovedisServer.getServerInstance()
        if isinstance(X, (FrovedisCRSMatrix, FrovedisRowmajorMatrix)):
            inp_data = FrovedisFeatureData(X, dense_kind='rowmajor', 
                                           dtype=self.__mdtype)
            X = inp_data.get()
            x_dtype = inp_data.get_dtype()
            x_itype = inp_data.get_itype()
            dense = inp_data.is_dense()
            component = self.svd_res_.vmat_ # always colmajor matrix
            if (x_dtype != self.__mdtype):
                raise TypeError("Type mismatches in input X-mat and " \
                                + "svd component!")
            res = compute_svd_inverse_transform(host, port, X.get(),
                                                x_dtype, x_itype, dense,
                                                component.get())
            excpt = check_server_exception()
            if excpt["status"]:
                raise RuntimeError(excpt["info"])
            return FrovedisRowmajorMatrix(res, 
                   dtype=TypeUtil.to_numpy_dtype(self.__mdtype))
        else:
            return np.asarray(X * self.components_)

    @check_association
    def __set_results(self):
        """ it should be called after fit().
        it sets the ouput properties """
        (U, s, VT) = self.svd_res_.to_numpy_results()
        self._components = VT
        self._singular_values = s
        #self.X_transformed = U * np.diag(s)
        self.X_transformed = U * s
        exp_var = np.var(self.X_transformed, axis=0)
        self._explained_variance = np.asarray(exp_var)[0]
        if self.var_sum is not None:
            self._explained_variance_ratio = \
                    self._explained_variance / self.var_sum
        return self

    @property
    def components_(self):
        """ returns the output VT """
        if self._components is None:
            self.__set_results()
        return self._components

    @components_.setter
    def components_(self, val):
        """components_ setter"""
        raise AttributeError("attribute 'components_' of TruncatedSVD " + \
                             "object is not writable")

    @property
    def singular_values_(self):
        """ retuns the output singular values """
        if self._singular_values is None:
            self.__set_results()
        return self._singular_values

    @singular_values_.setter
    def singular_values_(self, val):
        """singular_values_ setter"""
        raise AttributeError("attribute 'singular_values_' of TruncatedSVD " +\
                             "object is not writable")

    @property
    def explained_variance_(self):
        """ retuns the output singular explained_variance """
        if self._explained_variance is None:
            self.__set_results()
        return self._explained_variance

    @explained_variance_.setter
    def explained_variance_(self, val):
        """explained_variance_ setter"""
        raise AttributeError("attribute 'explained_variance_' " + \
                             "of TruncatedSVD object is not writable")

    @property
    def explained_variance_ratio_(self):
        """ retuns the output singular explained_variance_ratio in
        case input is python data or scipy sparse matrix"""
        if self.var_sum is None:
            raise ValueError("explained_variance_ratio_: can be " + \
              "obtained only when input is numpy data or scipy sparse " + \
              "data or FrovedisRowmajorMatrix")
        elif self._explained_variance_ratio is None:
            self.__set_results()
        return self._explained_variance_ratio

    @explained_variance_ratio_.setter
    def explained_variance_ratio_(self, val):
        """explained_variance_ratio_ setter"""
        raise AttributeError("attribute 'explained_variance_ratio_' of " + \
                             "TruncatedSVD object is not writable")

    def release(self):
        """
        resets after-fit populated attributes to None
        along with relasing server side memory
        """
        self.__release_server_heap()
        self.svd_res_ = None
        self.__mdtype = None
        self._explained_variance_ratio = None
        self._explained_variance = None
        self._singular_values = None
        self._components = None
        self.var_sum = None
        self.X_transformed = None

    @do_if_active_association
    def __release_server_heap(self):
        """
        to release model pointer from server heap
        """
        self.svd_res_.release()

    def __del__(self):
        """
        destructs the python object
        """
        self.release()

    def is_fitted(self):
        """ function to confirm if the model is already fitted """
        return self.svd_res_ is not None
