#!/usr/bin/env python

import os
import numpy as np
from ..exrpc import rpclib
from ..exrpc.server import FrovedisServer
from .dense import FrovedisDenseMatrix
from .vector import FrovedisVector
from .dtype import TypeUtil


class GetrfResult(object):
    """"A python container for holding pointers of Frovedis server side
    getrf results """

    def __init__(self, dummy=None):  # constructor
        self.__mtype = None
        self.__ipiv_ptr = None
        self.__info = None
        if dummy is not None:
            self.load_dummy(dummy)

    def load_dummy(self, dummy):
        """load_dummy"""
        self.release()
        try:
            self.__mtype = dummy['mtype']
            self.__ipiv_ptr = dummy['dptr']
            self.__info = dummy['info']
        except KeyError:
            raise TypeError("[INTERNAL ERROR] Invalid input encountered.")

    def release(self):
        """release"""
        if self.__mtype is not None:
            (host, port) = FrovedisServer.getServerInstance()
            rpclib.release_ipiv(host, port, self.mtype().encode('ascii'),
                                self.ipiv())
            excpt = rpclib.check_server_exception()
            if excpt["status"]:
                raise RuntimeError(excpt["info"])
            self.__mtype = None
            self.__ipiv_ptr = None
            self.__info = None

    def __del__(self):  # destructor
        if FrovedisServer.isUP():
            self.release()

    def mtype(self):
        """mtype"""
        return self.__mtype

    def ipiv(self):
        """ipiv"""
        return self.__ipiv_ptr

    def stat(self):
        """stat"""
        return self.__info


# A generic class for storing SVD results
# of the type colmajor_matrix<float/double> or blockcyclic_matrix<float/double>
class svdResult(object):
    """A python container for holding pointers of Frovedis server side
    gesvd results"""

    def __init__(self, dummy=None, dtype=None):  # constructor
        self.__umat = None
        self.__vmat = None
        self.__svec = None
        self.__k = None
        self.__info = None
        if dtype is None:
            raise ValueError("Nonetype matrix can't be handled!")
        else:
            self.__dtype = dtype
        if dummy is not None:
            self.load_dummy(dummy)

    def load_dummy(self, dummy):
        """load_dummy"""
        self.release()
        try:
            mtype = dummy['mtype']
            uptr = dummy['uptr']
            vptr = dummy['vptr']
            sptr = dummy['sptr']
            info = dummy['info']
            m_m = dummy['m']
            n_n = dummy['n']
            k_k = dummy['k']
            if uptr != 0:
                dmat = {'dptr': uptr, 'nrow': m_m, 'ncol': k_k}
                self.__umat = FrovedisDenseMatrix(mtype, dmat,
                                                  dtype=self.__dtype)
            if vptr != 0:
                dmat = {'dptr': vptr, 'nrow': n_n, 'ncol': k_k}
                self.__vmat = FrovedisDenseMatrix(mtype, dmat,
                                                  dtype=self.__dtype)
            dummy_vec = {'dptr': sptr, 'size': k_k}
            self.__svec = FrovedisVector(dummy_vec, dtype=self.__dtype)
            self.__k = k_k
            self.__info = info
        except KeyError:
            raise TypeError("[INTERNAL ERROR] Invalid input encountered.")

    def debug_print(self):
        """debug_print"""
        if self.__svec is not None:
            print("svec: ")
            self.__svec.debug_print()
            if self.__umat is not None:
                print("umat: ")
                self.__umat.get_rowmajor_view()
            if self.__vmat is not None:
                print("vmat: ")
                self.__vmat.get_rowmajor_view()

    def to_numpy_results(self):
        """to_numpy_results"""
        if self.__svec is not None:
            svec = self.__svec.to_numpy_array()
            if self.__umat is not None:
                umat = self.__umat.to_numpy_matrix()
            else:
                umat = None
            if self.__vmat is not None:
                mtype = self.__vmat.get_mtype()
                if (mtype == 'B'):
                    vmat = self.__vmat.transpose().to_numpy_matrix()
                elif (mtype == 'C'):
                    vmat = self.__vmat.to_frovedis_rowmatrix() \
                               .transpose().to_numpy_matrix()
                else:
                    raise ValueError(\
                        "SVD vmat: expected mtype is either B or C")
            else:
                vmat = None
            return umat, svec, vmat
        else:
            raise ValueError("Empty input matrix.")

    def save(self, sfl, ufl=None, vfl=None):
        """save"""
        if self.__svec is not None:
            if sfl is None:
                raise ValueError("s_filename can't be None")
            (host, port) = FrovedisServer.getServerInstance()
            if self.__dtype == np.float32:
                dtype = 'F'
            elif self.__dtype == np.float64:
                dtype = 'D'
            else:
                raise TypeError("Invalid dtype, expected double/float.")
            rpclib.save_as_diag_matrix(host, port, self.__svec.get(),
                                       sfl.encode('ascii'), False,
                                       dtype.encode('ascii'))
            excpt = rpclib.check_server_exception()
            if excpt["status"]:
                raise RuntimeError(excpt["info"])
            want_u = self.__umat is not None and ufl is not None
            want_v = self.__vmat is not None and vfl is not None
            if want_u:
                self.__umat.save(ufl)
            if want_v:
                self.__vmat.save(vfl)

    def save_binary(self, sfl, ufl=None, vfl=None):
        """save_binary"""
        if self.__svec is not None:
            if sfl is None:
                raise ValueError("s_filename can't be None")
            (host, port) = FrovedisServer.getServerInstance()
            if self.__dtype == np.float32:
                dtype = 'F'
            elif self.__dtype == np.float64:
                dtype = 'D'
            else:
                raise TypeError("Invalid dtype, expected double/float.")
            rpclib.save_as_diag_matrix(host, port, self.__svec.get(),
                                       sfl.encode('ascii'), True,
                                       dtype.encode('ascii'))
            excpt = rpclib.check_server_exception()
            if excpt["status"]:
                raise RuntimeError(excpt["info"])
            want_u = self.__umat is not None and ufl is not None
            want_v = self.__vmat is not None and vfl is not None
            if want_u:
                self.__umat.save_binary(ufl)
            if want_v:
                self.__vmat.save_binary(vfl)

    def load(self, sfl, ufl=None, vfl=None, mtype='B'):
        """load"""
        self.__load_impl(sfl, ufl, vfl, mtype, False)

    def load_binary(self, sfl, ufl=None, vfl=None, mtype='B'):
        """load_binary"""
        self.__load_impl(sfl, ufl, vfl, mtype, True)

    def __load_impl(self, sfl, ufl, vfl, mtype, bin1):
        """__load_impl"""
        if sfl is None:
            raise ValueError("s_filename can't be None")
        if mtype != 'B':
            if mtype != 'C':
                raise ValueError(
                    "Unknown matrix type. Expected: (B or C), Got: ",
                    mtype)
        if self.__dtype == np.float32:
            dtype = 'F'
        elif self.__dtype == np.float64:
            dtype = 'D'
        else:
            raise TypeError("Invalid dtype, expected double/float.")
        want_u = ufl is not None
        want_v = vfl is not None
        if not want_u:
            ufl = ''
        if not want_v:
            vfl = ''
        (host, port) = FrovedisServer.getServerInstance()
        dummy = rpclib.get_svd_results_from_file(host, port,
                                                 sfl.encode('ascii'),
                                                 ufl.encode('ascii'),
                                                 vfl.encode('ascii'),
                                                 bin1, want_u, want_v,
                                                 mtype.encode('ascii'),
                                                 dtype.encode('ascii'))
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        self.load_dummy(dummy)

    # developer's API
    def set_none(self):
        """this api sets the metadata to None.
           it does not release the actual result
           components in server side. this API should
           call when result components are
           copied in other objects and result object
           can be released in order to
           avoid auto release of memory objects for result components."""
        self.__umat = None
        self.__vmat = None
        self.__svec = None
        self.__k = None
        self.__info = None

    def release(self):
        """release"""
        if self.__umat is not None:
            self.__umat.release()
            self.__umat = None
        if self.__svec is not None:
            self.__svec.release()
            self.__svec = None
        if self.__vmat is not None:
            self.__vmat.release()
            self.__vmat = None
        self.__k = None
        self.__info = None

    def __del__(self):  # destructor
        if FrovedisServer.isUP():
            self.release()

    def stat(self):
        """stat"""
        return self.__info

    def get_k(self):
        """get_k"""
        return self.__k

    @property
    def umat_(self):
        """umat_ getter"""
        return self.__umat

    @umat_.setter
    def umat_(self, u):
        """umat_ setter"""
        raise AttributeError(\
            "attribute 'umat_' of svdResult object is not writable")

    @property
    def vmat_(self):
        """vmat_ getter"""
        return self.__vmat

    @vmat_.setter
    def vmat_(self, v):
        """vmat_ setter"""
        raise AttributeError(\
            "attribute 'vmat_' of svdResult object is not writable")

    @property
    def singular_values_(self):
        return self.__svec

    @singular_values_.setter
    def singular_values_(self, s):
        """singular_values_ setter"""
        raise AttributeError(\
        "attribute 'singular_values_' of svdResult object is not writable")

# A generic class for storing PCA results
class PcaResult(object):
    """A python container for holding pointers of Frovedis server side
    pca results"""

    def __init__(self, dummy=None, dtype=None):  # constructor
        self.__pc = None
        self.__score = None
        self.__var = None
        self.__n_components = None
        self.__var_ratio = None
        self.__singular = None
        self.__mean = None
        self.__noise = None
        if dtype is None:
            raise ValueError("Nonetype matrix can't be handled!")
        else:
            self.__dtype = dtype
        if dummy is not None:
            self.load_dummy(dummy)

    # modify this function according to update in client (utility.cc)
    def load_dummy(self, dummy):
        self.release()
        try:
            mtype = dummy['mtype']
            pc_ptr = dummy['pc_ptr']
            var_ptr = dummy['var_ptr']
            score_ptr = dummy['score_ptr']
            exp_var_ptr = dummy['exp_var_ptr']
            singular_val_ptr = dummy['singular_val_ptr']
            mean_ptr = dummy["mean_ptr"]
            n_components = dummy["n_components"]
            n_samples = dummy["n_samples"]
            n_features = dummy["n_features"]
            noise = dummy["noise"]

            # matrices
            pc_mat = {'dptr': pc_ptr, 'nrow': n_features, 'ncol': n_components}
            self.__pc = FrovedisDenseMatrix(mtype, pc_mat,
                                            dtype=self.__dtype)  # colmajor
            sc_mat = {'dptr': score_ptr, 'nrow': n_samples,
                      'ncol': n_components}
            self.__score = FrovedisDenseMatrix(mtype, sc_mat,
                                               dtype=self.__dtype)  # colmajor

            # vectors
            dummy_vec = {'dptr': exp_var_ptr, 'size': n_components}
            self.__var = FrovedisVector(dummy_vec, dtype=self.__dtype)

            dummy_vec = {'dptr': var_ptr, 'size': n_components}
            self.__var_ratio = FrovedisVector(dummy_vec, dtype=self.__dtype)

            dummy_vec = {'dptr': singular_val_ptr, 'size': n_components}
            self.__singular = FrovedisVector(dummy_vec, dtype=self.__dtype)

            dummy_vec = {'dptr': mean_ptr, 'size': n_features}
            self.__mean = FrovedisVector(dummy_vec, dtype=self.__dtype)

            # numeric values
            self.__n_components = n_components
            self.__noise = noise

            # for internal use
            self.__n_features = n_features

        except KeyError:
            raise TypeError("[pca: load_dummy()] Invalid input encountered.")

    def release(self):
        if self.__pc:
            self.__pc.release()
        if self.__score:
            self.__score.release()
        if self.__var:
            self.__var.release()
        if self.__var_ratio:
            self.__var_ratio.release()
        if self.__singular:
            self.__singular.release()
        if self.__mean:
            self.__mean.release()
        self.__pc = None
        self.__score = None
        self.__var = None
        self.__var_ratio = None
        self.__singular = None
        self.__n_components = None
        self.__n_features = None
        self.__mean = None
        self.__noise = None

    def debug_print(self):
        if self.__pc:
            print("principal components: ")
            self.__pc.get_rowmajor_view()
        if self.__score:
            print("score: ")
            self.__score.get_rowmajor_view()
        if self.__var:
            print("explained variance: ")
            self.__var.debug_print()
        if self.__var_ratio:
            print("explained variance ratio: ")
            self.__var_ratio.debug_print()
        if self.__singular:
            print("singular_values: ")
            self.__singular.debug_print()
        if self.__mean:
            print("mean: ")
            self.__mean.debug_print()
        if self.__noise:
            print("noise variance: ")
            print(self.__noise)

    def to_numpy_results(self):
        pca_components = None
        pca_scores = None
        explained_variance = None
        explained_variance_ratio = None
        singular_values = None
        mean = None

        if self.__pc: # colmajor matrix
            pca_components = self.__pc.to_frovedis_rowmatrix() \
                                 .transpose().to_numpy_matrix()

        if self.__score:
            pca_scores = self.__score.to_numpy_matrix()

        if self.__var_ratio:
            explained_variance_ratio = self.__var_ratio.to_numpy_array()

        if self.__var:
            explained_variance = self.__var.to_numpy_array()

        if self.__singular:
            singular_values = self.__singular.to_numpy_array()

        if self.__mean:
            mean = self.__mean.to_numpy_array()

        return (pca_components, pca_scores, explained_variance,
                explained_variance_ratio, singular_values, mean, self.__noise)

    def __del__(self):  # destructor
        if FrovedisServer.isUP():
            self.release()

    def get_dtype(self):
        if self.__pc is not None:
            return self.__pc.get_dtype()
        else:
            raise ValueError(\
            "get_dtype: pca_res_ object might have been released!")

    def save(self, path):
        if os.path.exists(path):
            raise ValueError(\
                "another model with %s name already exists!" % path)
        else:
            os.mkdir(path)
        self.__pc.save(path + "/components")
        self.__score.save(path + "/score")
        self.__var.save(path + "/variance")
        self.__var_ratio.save(path + "/variance_ratio")
        self.__singular.save(path + "/singular_values")
        self.__mean.save(path + "/mean")

    def save_binary(self, path):
        if os.path.exists(path):
            raise ValueError(\
                "another model with %s name already exists!" % path)
        else:
            os.mkdir(path)
        self.__pc.save_binary(path + "/components")
        self.__score.save_binary(path + "/score")
        self.__var.save_binary(path + "/variance")
        self.__var_ratio.save_binary(path + "/variance_ratio")
        self.__singular.save_binary(path + "/singular_values")
        self.__mean.save_binary(path + "/mean")

    def load(self, path, dtype):
        if not os.path.exists(path):
            raise ValueError(\
                "the model with name %s does not exist!" % path)
        self.release()
        self.__pc = \
        FrovedisDenseMatrix(mat=path+"/components", dtype=dtype, mtype='C')
        self.__score = \
        FrovedisDenseMatrix(mat=path+"/score", dtype=dtype, mtype='C')
        self.__var = FrovedisVector(vec=path+"variance", dtype=dtype)
        self.__var_ratio = \
        FrovedisVector(vec=path+"/variance_ratio", dtype=dtype)
        self.__singular = \
        FrovedisVector(vec=path+"/singular_values", dtype=dtype)
        self.__mean = FrovedisVector(vec=path+"/mean", dtype=dtype)

    def load_binary(self, path, dtype):
        if not os.path.exists(path):
            raise ValueError(\
                "the model with name %s does not exist!" % path)
        self.release()
        self.__pc = FrovedisDenseMatrix(dtype=dtype, mtype='C')
        self.__pc.load_binary(path + "/components")
        self.__score = FrovedisDenseMatrix(dtype=dtype, mtype='C')
        self.__score.load_binary(path + "/score")
        self.__var = FrovedisVector(dtype=dtype)
        self.__var.load_binary(path + "variance")
        self.__var_ratio = FrovedisVector(dtype=dtype)
        self.__var_ratio.load_binary(path + "/variance_ratio")
        self.__singular = FrovedisVector(dtype=dtype)
        self.__singular.load_binary(path + "/singular_values")
        self.__mean = FrovedisVector(dtype=dtype)
        self.__mean.load_binary(path + "/mean")

    @property
    def n_components(self):
        return self.__n_components

    @n_components.setter
    def n_components(self, val):
        raise AttributeError(\
        "attribute 'n_components' of PcaResult object is not writable")

    @property
    def _noise_variance(self):
        return self.__noise

    @_noise_variance.setter
    def _noise_variance(self, val):
        raise AttributeError(\
        "attribute '_noise_variance' of PcaResult object is not writable")

    @property
    def _components(self):
        return self.__pc

    @_components.setter
    def _components(self, val):
        raise AttributeError(\
        "attribute '_components' of PcaResult object is not writable")

    @property
    def _score(self):
        return self.__score

    @_score.setter
    def _score(self, val):
        raise AttributeError(\
        "attribute '_score' of PcaResult object is not writable")

    @property
    def _explained_variance(self):
        return self.__var

    @_explained_variance.setter
    def _explained_variance(self, val):
        raise AttributeError(\
        "attribute '_explained_variance' of PcaResult object is not writable")

    @property
    def _explained_variance_ratio(self):
        return self.__var_ratio

    @_explained_variance_ratio.setter
    def _explained_variance_ratio(self, val):
        raise AttributeError(\
            "attribute '_explained_variance_ratio' \
            of PcaResult object is not writable")

    @property
    def _mean(self):
        return self.__mean

    @_mean.setter
    def _mean(self, val):
        raise AttributeError(\
        "attribute '_mean' of PcaResult object is not writable")

    @property
    def _singular_values(self):
        return self.__singular

    @_singular_values.setter
    def _singular_values(self, val):
        raise AttributeError(\
        "attribute '_singular_values' of PcaResult object is not writable")

