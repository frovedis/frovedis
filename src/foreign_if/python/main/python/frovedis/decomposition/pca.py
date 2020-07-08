"""pca.py"""
#!/usr/bin/env python

import numpy as np
from ..base import *
from ..exrpc.server import FrovedisServer
from ..exrpc.rpclib import compute_pca, check_server_exception
from ..exrpc.rpclib import pca_transform, pca_inverse_transform
from ..exrpc.rpclib import get_double_array, get_float_array
from ..exrpc.rpclib import release_frovedis_array
from ..matrix.ml_data import FrovedisFeatureData
from ..matrix.results import PcaResult
from ..matrix.dtype import TypeUtil
from frovedis.matrix.dense import FrovedisRowmajorMatrix

class PCA(BaseEstimator):
    """Principal Components Analysis (PCA) """

    # TODO: support whiten
    def __init__(self, n_components=None, copy=True, whiten=False,
                 svd_solver='auto', tol=0.0, iterated_power='auto',
                 random_state=None):
        self.n_components = n_components
        self.copy = copy
        self.whiten = whiten
        self.svd_solver = svd_solver
        self.tol = tol
        self.iterated_power = iterated_power
        self.random_state = random_state
        # add attributes
        self.pca_res_ = None

        # data to be written
        self._explained_variance_ratio = None
        self._explained_variance = None
        self._singular_values = None
        self._components = None
        self._mean = None
        self.noise_variance_ = None
        #extras
        self.n_features_ = None
        self.__dtype = None
        self.n_samples_ = None
        self.n_components_ = None

    def fit(self, X, y=None):
        """Fit PCA on training data."""
        supported_solvers = {'arpack', 'auto'}
        if self.svd_solver not in supported_solvers:
            raise ValueError("svd_solver: currently Frovedis "
                             "supports only arpack!")

        if self.svd_solver == "auto":
            self.svd_solver = "arpack"

        if self.n_components == 'mle':
            raise ValueError("n_components=%r cannot be a string "
                             "with svd_solver='%s'"
                             % (self.n_components, self.svd_solver))

        inp_data = FrovedisFeatureData(X, dense_kind='rowmajor')
        self.n_samples_ = inp_data.numRows()
        self.n_features_ = inp_data.numCols()
        # handling n_components == None case with arpack solver
        if self.n_components is None:
            self.n_components = min(self.n_samples_, self.n_features_) - 1
        elif not 1 <= self.n_components < min(self.n_samples_, \
                                            self.n_features_):
            raise ValueError("n_components = %r must be between 1 and "
                             "min(n_samples, n_features) - 1 = %r with "
                             "svd_solver='%s'"
                             % (self.n_components,
                                min(self.n_samples_, self.n_features_) - 1,
                                self.svd_solver))
        input_x = inp_data.get()
        x_dtype = inp_data.get_dtype()
        #x_itype = inp_data.get_itype()
        self.__dtype = TypeUtil.to_numpy_dtype(x_dtype)
        dense = inp_data.is_dense()
        mvbl = False # auto-handled by python ref-count
        if not dense:
            raise ValueError("PCA supports only dense input for fit")
        (host, port) = FrovedisServer.getServerInstance()
        res = compute_pca(host, port, input_x.get(), self.n_components,
                          self.whiten, x_dtype, self.copy, mvbl)
        excpt = check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        self.pca_res_ = PcaResult(res, TypeUtil.to_numpy_dtype(x_dtype))
        self.noise_variance_ = self.pca_res_._noise_variance
        self.n_components_ = self.n_components
        # support of "copy=False for python ndarray/matrix inputs"
        if not self.copy and inp_data.is_movable():
            if type(X).__name__ == 'matrix':
                if X.dtype == np.float32 or X.dtype == np.float64:
                    input_x.to_numpy_matrix_inplace(X)
                else:
                    pass   # for integer type matrix,
                           #    there will be no overwrite
            elif type(X).__name__ == 'ndarray':
                if X.dtype == np.float32 or X.dtype == np.float64:
                    input_x.to_numpy_array_inplace(X)
                else:
                    pass # for integer type array, there will be no overwrite
            else:
                pass   #sklearn does not oevrwrite X,
                       #when it is not matrix/ndarray
        return self

    @property
    def components_(self):
        """components_ getter"""
        if self.pca_res_ is None:
            raise AttributeError("components_: PCA object is not fitted!")
        elif self.pca_res_._components is None:
            raise AttributeError("pca_res_._components: pca result "\
                                 "object might have been released!")
        else:
            if self._components is None:
                self._components = self.pca_res_._components \
                                       .to_frovedis_rowmatrix() \
                                       .transpose().to_numpy_array()
            return self._components

    @components_.setter
    def components_(self, val):
        """components_ setter"""
        raise AttributeError(\
            "attribute 'components_' of PCA object is not writable")

    @property
    def explained_variance_ratio_(self):
        """explained_variance_ratio_ getter"""
        if self.pca_res_ is None:
            raise AttributeError(\
                "explained_variance_ratio_: PCA object is not fitted!")
        elif self.pca_res_._explained_variance_ratio is None:
            raise AttributeError(\
                "pca_res_.explained_variance_ratio_: pca result "
                "object might have been released!")
        else:
            if self._explained_variance_ratio is None:
                self._explained_variance_ratio = self.pca_res_ \
                                                 ._explained_variance_ratio \
                                                 .to_numpy_array()
            return self._explained_variance_ratio

    @explained_variance_ratio_.setter
    def explained_variance_ratio_(self, val):
        """explained_variance_ratio_ setter"""
        raise AttributeError(\
        "attribute 'explained_variance_ratio_' of PCA object is not writable")

    @property
    def explained_variance_(self):
        """explained_variance_ getter"""
        if self.pca_res_ is None:
            raise AttributeError(\
                "explained_variance_: PCA object is not fitted!")
        elif self.pca_res_._explained_variance is None:
            raise AttributeError("pca_res_._explained_variance: pca result "\
              "object might have been released!")

        if self._explained_variance is None:
            self._explained_variance = self.pca_res_._explained_variance \
                                                    .to_numpy_array()
        return self._explained_variance

    @explained_variance_.setter
    def explained_variance_(self, val):
        """explained_variance_ setter"""
        raise AttributeError(\
            "attribute 'explained_variance_' of PCA object is not writable")

    @property
    def mean_(self):
        """mean_ getter"""
        if self.pca_res_ is None:
            raise AttributeError("mean_: PCA object is not fitted!")
        elif self.pca_res_._mean is None:
            raise AttributeError("pca_res_._mean: pca result "\
              "object might have been released!")

        if self._mean is None:
            self._mean = self.pca_res_._mean.to_numpy_array()
        return self._mean

    @mean_.setter
    def mean_(self, val):
        """mean_ setter"""
        raise AttributeError("attribute 'mean_' of PCA object is not writable")

    @property
    def singular_values_(self):
        """singular_values_ getter"""
        if self.pca_res_ is None:
            raise AttributeError("singular_values_: PCA object is not fitted!")
        elif self.pca_res_._singular_values is None:
            raise AttributeError("pca_res_._singular_values: pca result "\
              "object might have been released!")

        if self._singular_values is None:
            self._singular_values = self.pca_res_._singular_values
        return self._singular_values.to_numpy_array()

    @singular_values_.setter
    def singular_values_(self, val):
        """singular_values_ setter"""
        raise AttributeError(\
            "attribute 'singular_values_' of PCA object is not writable")

    def fit_transform(self, X):
        """fit_transform"""
        self.fit(X)
        if self.pca_res_._score is None: # should not occur, if fit works well
            raise AttributeError("pca_res_._score: pca result "\
              "object does not have score computed!")
        return self.pca_res_._score.to_numpy_array()

    def transform(self, X):
        """transform"""
        if self.pca_res_ is None:
            raise AttributeError("transform: PCA object is not fitted!")
        inp_data = FrovedisFeatureData(X, dense_kind='rowmajor', \
                                       dtype=self.__dtype)
        input_x = inp_data.get()
        x_dtype = inp_data.get_dtype()
        if x_dtype != self.pca_res_.get_dtype():
            raise TypeError(\
                "transform: model dtype differs from input matrix dtype!")

        pc_ptr = self.pca_res_._components.get()
        exp_var_ptr = self.pca_res_._explained_variance.get()
        mean_ptr = self.pca_res_._mean.get()
        (host, port) = FrovedisServer.getServerInstance()
        dummy_res = \
            pca_transform(host, port, input_x.get(), pc_ptr, exp_var_ptr,\
                                  mean_ptr, x_dtype, self.whiten)
        excpt = check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        res = FrovedisRowmajorMatrix(mat=dummy_res, dtype=self.__dtype)

        if inp_data.is_movable(): # input was a python data
            return res.to_numpy_array()
        else:
            return res

    def inverse_transform(self, X):
        """inverse_transform"""
        if self.pca_res_ is None:
            raise AttributeError(\
                "inverse_transform: PCA object is not fitted!")
        inp_data = FrovedisFeatureData(X, dense_kind='rowmajor',\
                                       dtype=self.__dtype)
        input_x = inp_data.get()
        x_dtype = inp_data.get_dtype()
        if x_dtype != self.pca_res_.get_dtype():
            raise TypeError(\
            "inverse_transform: model dtype differs from input matrix dtype!")

        pc_ptr = self.pca_res_._components.get()
        exp_var_ptr = self.pca_res_._explained_variance.get()
        mean_ptr = self.pca_res_._mean.get()
        (host, port) = FrovedisServer.getServerInstance()
        dummy_res = pca_inverse_transform(host, port, input_x.get(), pc_ptr,
                                          exp_var_ptr,
                                          mean_ptr, x_dtype, self.whiten)
        excpt = check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        res = FrovedisRowmajorMatrix(mat=dummy_res, dtype=self.__dtype)

        if inp_data.is_movable(): # input was a python data
            return res.to_numpy_array()
        else:
            return res

    def save(self, path):
        """save"""
        if self.pca_res_:
            self.pca_res_.save(path)

    def save_binary(self, path):
        """save_binary"""
        if self.pca_res_:
            self.pca_res_.save_binary(path)

    def load(self, path, dtype):
        """load"""
        self.pca_res_ = PcaResult(dtype=dtype)
        self.pca_res_.load(path=path, dtype=dtype)

    def load_binary(self, path, dtype):
        """load_binary"""
        self.pca_res_ = PcaResult(dtype=dtype)
        self.pca_res_.load_binary(path=path, dtype=dtype)
