#!/usr/bin/env python

from ..exrpc.server import FrovedisServer 
from ..exrpc.rpclib import compute_truncated_svd, check_server_exception
from ..matrix.ml_data import FrovedisFeatureData
from ..matrix.crs import FrovedisCRSMatrix
from ..matrix.dense import FrovedisRowmajorMatrix, FrovedisColmajorMatrix
from ..matrix.results import svdResult
from ..matrix.dtype import DTYPE, TypeUtil
import numpy as np
from scipy.sparse import issparse

class TruncatedSVD:
    """Dimensionality reduction using truncated SVD (aka LSA)."""

    def __init__(self, n_components=2, algorithm="arpack", n_iter=5,
                 random_state=None, tol=0.):
        self.algorithm = algorithm
        self.n_components = n_components
        self.n_iter = n_iter
        self.random_state = random_state
        self.tol = tol
        self.svd = None
        self.var_sum = None
        self._components = None
        self._explained_variance = None
        self._explained_variance_ratio = None
        self._singular_values = None

    def fit(self, X, y=None):
        """Fit LSA model on training data X."""
        if self.algorithm != "arpack": 
           raise ValueError("algorithm: currently Frovedis supports only arpack!")
        if (isinstance(X,FrovedisCRSMatrix) or 
            isinstance(X,FrovedisRowmajorMatrix) or
            isinstance(X,FrovedisColmajorMatrix)): 
            self.var_sum = None
        elif issparse(X):
            try:
                from sklearn.utils.sparsefuncs import mean_variance_axis
                _, full_var = mean_variance_axis(X, axis=0)
                self.var_sum = full_var.sum()
            except: #for system without sklearn
                self.var_sum = None 
        else:
            self.var_sum = np.var(X, axis=0).sum()
        # if X is not a sparse data, it would be converted as rowmajor matrix
        inp_data = FrovedisFeatureData(X,dense_kind='rowmajor')
        X = inp_data.get()
        x_dtype = inp_data.get_dtype()
        x_itype = inp_data.get_itype()
        dense = inp_data.is_dense()
        (host,port) = FrovedisServer.getServerInstance()
        res = compute_truncated_svd(host, port, X.get(),
                                    self.n_components,
                                    x_dtype, x_itype, dense)
        excpt = check_server_exception()
        if excpt["status"]: raise RuntimeError(excpt["info"])
        self.svd = svdResult(res, TypeUtil.to_numpy_dtype(x_dtype))
        return self

    def __set_results(self):
        if self.svd is None: raise ValueError("__set_results is called before fit!")
        (U,s,VT) = self.svd.to_numpy_results()
        self._components = VT
        self._singular_values= s
        X_transformed = U * np.diag(s)
        exp_var = np.var(X_transformed, axis=0)
        self._explained_variance = np.asarray(exp_var)[0]
        if self.var_sum is not None:
            self._explained_variance_ratio = self._explained_variance / self.var_sum
        return self

    @property
    def components_(self):
        if self._components is None:
          self.__set_results()
        return self._components
        
    @property
    def singular_values_(self):
        if self._singular_values is None:
          self.__set_results()
        return self._singular_values
        
    @property
    def explained_variance_(self):
        if self._explained_variance is None:
          self.__set_results()
        return self._explained_variance
        
    @property
    def explained_variance_ratio_(self):
        if self.var_sum is None:
            raise ValueError("explained_variance_ratio_: can be obtained only when input is numpy data or scipy sparse data") 
        elif self._explained_variance_ratio is None:
            self.__set_results()
        return self._explained_variance_ratio
        
