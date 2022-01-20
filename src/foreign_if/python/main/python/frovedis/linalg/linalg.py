"""linalg.py"""
#!/usr/bin/env python

import numpy as np
from ..exrpc import rpclib
from ..exrpc.server import FrovedisServer
from ..matrix.dense import FrovedisBlockcyclicMatrix, FrovedisDenseMatrix
from ..matrix.vector import FrovedisVector
from ..matrix.ml_data import FrovedisFeatureData
from ..matrix.dtype import DTYPE, TypeUtil
from ..matrix.wrapper import PBLAS
from .scalapack import *

def check_if_vec(a):
    """checking if the input mat is col or row vec"""
    isRowvec = False
    isNmat = False
    if not isinstance(a, FrovedisBlockcyclicMatrix):
        if a.ndim == 1:
            isRowvec = True
        else:
            isNmat = True
    return (isRowvec, isNmat)


def get_computation_matrix(a, copy_bcm=False,
                           check_col_vector=False,
                           return_ndim=False,
                           isVector=False):
    """get_computation_matrix"""
    if not isinstance(a, FrovedisBlockcyclicMatrix):
        if type(a).__name__ == 'matrix':
            isMatrix = True
        else:
            isMatrix = False
        arr = np.asarray(a)
        conv = True
        ndim = arr.ndim
        if ndim > 2:
            raise ValueError("get_computation_matrix: input data has more \
                than two dimensions!")
        if arr.dtype == np.float32 or arr.dtype == np.float64:
            t_dt = arr.dtype
        else:
            t_dt = np.float64 # default: double
        # scaling of a row vector is not possible using frovedis
        if check_col_vector and ndim == 1:
            if not isVector:
                arr = arr.reshape(-1, 1)
        ret = FrovedisBlockcyclicMatrix(arr, dtype=t_dt)
    else:
        conv = False
        isMatrix = False # output is not a numpy matrix
        if a.numCols() == 1: # for column vector
            ndim = 1 # always two, since input is frovedis bcm
        else:
            ndim = 2
        if copy_bcm:
            ret = FrovedisBlockcyclicMatrix(a)
        else:
            ret = a

    if return_ndim:
        return (ret, conv, isMatrix, ndim)
    else:
        return (ret, conv, isMatrix)

def handle_scal_output(y, cv, out, isMatrix, toFlatten=False):
    """handle_scal_output"""
    if not cv:
        return y # when input is a bcm, output will be bcm
                 #   (no change on out param)

    elif cv == True and out is not None:
        if not isinstance(out, np.ndarray):
            raise TypeError("dot: out parameter should be an ndarray")

        if out.size != y.size:
            raise ValueError("dot: size of out parameter is incompatible \
                with input size!")

        if isMatrix:
            y.to_numpy_matrix_inplace(out)
        else:
            y.to_numpy_array_inplace(out)
        return out

    elif cv == True and out is None:
        if isMatrix:
            return y.to_numpy_matrix()
        else:
            return y.to_numpy_array().flatten()

def handle_dot_output(x1, x2, y, cv1, cv2, out, isMatrix, toFlatten=False):
    """handle_dot_output"""
    if cv1 == False or cv2 == False:
        return y # when any of the inputs is a bcm,
                 #   output will be bcm (no change on out param)

    elif cv1 == cv2 == True and out is not None:
        if not isinstance(out, np.ndarray):
            raise TypeError("dot: out parameter should be an ndarray")

        nrow1, _ = x1.shape
        _, ncol2 = x2.shape
        odtype = out.dtype
        nrow3, ncol3 = out.shape #(nrow1, ncol2)
        if not (nrow1 == nrow3 and ncol2 == ncol3):
            raise ValueError("dot: shape of out parameter is incompatible \
                with input shapes!")
        if isMatrix:
            y.to_numpy_matrix_inplace(out)
        else:
            y.to_numpy_array_inplace(out)
        return out

    elif cv1 == cv2 == True and out is None:
        if isMatrix:
            return y.to_numpy_matrix()
        else:
            if toFlatten:
                return y.to_numpy_array().flatten()
            else:
                return y.to_numpy_array()

def svd(a, full_matrices=False, compute_uv=True):
    #-> gesvd
    """
    This function computes the singular value decomposition.
    input parameters:
          a: float32/float64 array with dim == 2.
             If dimension is not equal 2, ValueError is raised.
          full_matrices: False, since ScaLapack p?gesvd supports only 
                         partial, first min(M,N) no. of U/V to be computed.
          compute_uv: optional boolean argument to specify whether
          or not to compute u and vh in addition to s.
    output parameters:
          tuple: (u, s, vh)
                u: left singular matrix returned from gesvd call.
                s: singular matrix returned from gesvd call.
                vh: right singular matrix returned from gesvd call.
    """
    a, cv, isMatrix = get_computation_matrix(a)
    t_dtype = TypeUtil.to_numpy_dtype(a.get_dtype())
    (u, s, vt, _) = gesvd(a, compute_uv=compute_uv, \
                          full_matrices=full_matrices, lwork=0,\
                          overwrite_a=1, dtype=t_dtype)
    if not compute_uv:
        if cv:
            return s.to_numpy_array() # ndarray
        else:
            return s # FrovedisVector
    else:
        if cv and isMatrix:
            return (u.to_numpy_matrix(), s.to_numpy_array(),\
                                        vt.to_numpy_matrix())
        elif cv and not isMatrix:
            return (u.to_numpy_array(), s.to_numpy_array(),\
                                        vt.to_numpy_array())
        else:
            return (u, s, vt)

def solve(a, b):
    #-> getrf + getrs
    """
    This function solve a linear matrix equation, or system of
    linear scalar equations.
    input parameters:
        a: The coefficient matrix of float32/float64 type.
        b: The variables to be determined, float32/float64 type.
    output parameters:
        x: Solution to the system a x = b. Returned shape is identical to b.
           Exception is raise if size of a[0] doesn't match with size of b or
           if a is not a square matrix.
    """
    a, _, _ = get_computation_matrix(a)
    b, cv2, isM2 = get_computation_matrix(b)
    if a.get_dtype() != b.get_dtype():
        raise TypeError("solve: dtype of a and b are not compatible!")
    if a.numRows() != a.numCols():
        raise ValueError("solve: input a is not a square matrix!")
    t_dtype = TypeUtil.to_numpy_dtype(a.get_dtype())
    (_, _, x, _) = gesv(a, b, overwrite_a=1, overwrite_b=1, dtype=t_dtype)

    if cv2:
        if isM2:
            return x.to_numpy_matrix()
        else:
            return x.to_numpy_array()
    else:
        return x

def inv(a):
    """
    This function computes the inverse matrix of a given matrix
    using getrf and getri functions of scalapack.
    input parameter:
       a: input matrix of float32/float64 type.
    output parameter:
       ainv: inverse matrix of a.
    """
    a, cv, isMatrix = get_computation_matrix(a)
    t_dtype = TypeUtil.to_numpy_dtype(a.get_dtype())
    if a.numRows() != a.numCols():
        raise ValueError("inv: input a is not a square matrix!")
    #compute LU using getrf
    (lu, piv, _) = getrf(a, overwrite_a=1, dtype=t_dtype)
    (ainv, _) = getri(lu, piv, lwork=0, overwrite_lu=1, dtype=t_dtype)
    if cv:
        if isMatrix:
            return ainv.to_numpy_matrix()
        else:
            return ainv.to_numpy_array()
    else:
        return ainv

def matmul(x1, x2, out=None, casting='same_kind',
           order='K', dtype=None, subok=True,
           signature=None, extobj=None):
    """
    This function computes the matrix product of two arrays.
    input parameters:
       x1, x2: input matrix of float32/float64 type. These can't be scalar.
       out: Optional input parameter of float32/float64 type. This can only be
            of float32/float64 type. The shape of out should be conforming
            with x1 and x2.
       other input parameters have no impact.
    output parameters:
       y: the result matrix is the product of the two input matrices.
          If the input matrices are scalar, then error is raised.
          If the input matrices are 1-D vectors, then vdot is called, which
          returns a scalar.
    """
    if np.isscalar(x1) or np.isscalar(x2):
        raise ValueError("matmul: input can not be scalar")
    return dot(x1, x2, out)

def dot(a, b, out=None):
    """
    This function computes the dot product of two arrays.
    input parameter:
        a, b: input matrix of float32/float64 type. These can only have a
              maximum of 2 dimensions.
        out: optional input parameter, must be conforming to the shape of a.b
    output parameter: output: Returns the dot product of a and b.
         If a and b are both scalars or both 1-D arrays then a
         scalar is returned; otherwise an array is returned.
    """
    #if out is specified, check its type with input mat
    if (not isinstance(a, FrovedisBlockcyclicMatrix)) and\
       out is not None:
        if np.asarray(a).dtype != np.asarray(out).dtype:
            raise TypeError("dot: dtype of out is not compatible with input")
    # when both a and b are scalar
    if np.isscalar(a) and np.isscalar(b):
        return a * b
    # when either a or b is scalar
    elif np.isscalar(a) and not np.isscalar(b):
        b_isRowvec, b_isNmat = check_if_vec(b)
        b, cv, isM, _ = get_computation_matrix(b, copy_bcm=False, \
                                        check_col_vector=True, \
		                        return_ndim=True)
        PBLAS.scal(b, a)
        if b_isRowvec:
            return handle_scal_output(b, cv, out, isM, toFlatten=True)
        else:
            return handle_scal_output(b, cv, out, isM)
    elif not np.isscalar(a) and np.isscalar(b):
        a_isRowvec, a_isNmat = check_if_vec(a)
        a, cv, isM, _ = get_computation_matrix(a, copy_bcm=False, \
                                        check_col_vector=True,\
		                        return_ndim=True)
        PBLAS.scal(a, b)
        if a_isRowvec:
            return handle_scal_output(a, cv, out, isM, toFlatten=True)
        else:
            return handle_scal_output(a, cv, out, isM)
    # when neither a nor b is scalar
    else:
        a_isRowvec, a_isNmat = check_if_vec(a)
        b_isRowvec, b_isNmat = check_if_vec(b)
        if a_isRowvec and b_isNmat:
            a, cv1, isM1, a_ndim = get_computation_matrix(a, copy_bcm=False, \
                                                      check_col_vector=True, \
                                                      return_ndim=True,      \
                                                      isVector=True)
            b, cv2, isM2, b_ndim = get_computation_matrix(b, copy_bcm=False, \
                                                      check_col_vector=True, \
                                                      return_ndim=True)
        else:
            a, cv1, isM1, a_ndim = get_computation_matrix(a, copy_bcm=False, \
                                                      check_col_vector=True, \
                                                      return_ndim=True)
            b, cv2, isM2, b_ndim = get_computation_matrix(b, copy_bcm=False, \
                                                      check_col_vector=True, \
                                                      return_ndim=True)
        if a.get_dtype() != b.get_dtype():
            raise TypeError("dot: dtype of a and b are not compatible!")

        if a_ndim == b_ndim == 1: # both a and b are 1D vector
                                    #    (column vector)
            return PBLAS.dot(a, b)
        elif a_ndim == 2 and b_ndim == 1:
            if a.numCols() != b.numRows():
                raise ValueError("dot: input dimensions does not comply \
                    with matrix-vector multiplication rule!")
            c = PBLAS.gemv(a, b)
            if a_isNmat and b_isRowvec:
                return handle_dot_output(a, b, c, cv1, cv2, out, isM1 or isM2,\
                                         toFlatten=True)
            else:
                return handle_dot_output(a, b, c, cv1, cv2, out, isM1 or isM2)
        elif a_ndim == 2 and b_ndim == 2:
            if a.numCols() != b.numRows():
                raise ValueError("dot: input dimensions does not comply \
                    with matrix-matrix multiplication rule!")
            c = PBLAS.gemm(a, b)
            return handle_dot_output(a, b, c, cv1, cv2, out, isM1 or isM2)
        else:
            if a_isRowvec and not b_isNmat:
                print(b_isNmat)
                c = PBLAS.gemv(a, b)
                return handle_dot_output(a, b, c, cv1, cv2, out, isM1 or isM2,\
                                         toFlatten=True)
            else:
                c = PBLAS.gemm(a, b)
                return handle_dot_output(a, b, c, cv1, cv2, out, isM1 or isM2,\
                                         toFlatten=True)
            #raise ValueError("dot: vector-matrix multiplication is not \
            #    supported!")

def eigsh(A, M = None, k = 6, sigma = None, which = 'LM', v0=None,
          ncv = None, maxiter = None, tol = 0., return_eigenvectors = True,
          Minv = None, OPinv = None, mode = 'normal'):
    """
    Find k eigenvalues and eigenvectors of the real symmetric square matrix A.
    """
    if M is not None:
        raise NotImplementedError("M is not currently supported!")
    if v0 is not None:
        raise NotImplementedError("v0 is not currently supported!")
    if ncv is not None:
        raise NotImplementedError("ncv is not currently supported!")
    if Minv is not None:
        raise NotImplementedError("Minv is not currently supported!")
    if OPinv is not None:
        raise NotImplementedError("OPinv is not currently supported!")
    inp_data = FrovedisFeatureData(A, dense_kind='rowmajor')
    X = inp_data.get()
    x_dtype = inp_data.get_dtype()
    x_itype = inp_data.get_itype()
    dense = inp_data.is_dense()
    nrows = inp_data.numRows()
    ncols = inp_data.numCols()

    if nrows != ncols:
        raise ValueError('expected squared symmetric matrix (shape=%s)' % (inp_data.shape,))
    if k <= 0:
        raise ValueError('k must be greater than 0.')
    if k >= nrows:
        raise ValueError('k must be less than or equal to N for N * N square matrix.')
    if sigma is not None and not dense:
        raise ValueError('currently sigma is only supported for dense matrices.')
    if sigma is None:
        sigma =  np.finfo(np.float32).max

    if which not in ['LM', 'SM', 'LA', 'SA', 'BE']:
        raise ValueError('which must be one of LM, SM, LA, SA, or BE')
    if mode in ['buckling', 'cayley']:
        raise ValueError('currenly normal mode is only supported!')
    if maxiter is None:
        maxiter = 10 * nrows
    wantEv = return_eigenvectors
    (host, port) = FrovedisServer.getServerInstance()
    res = rpclib.compute_eigsh(host, port, X.get(),
                       k, which.encode('ascii'),
                       sigma, maxiter, wantEv,
                       tol, x_dtype,
                       x_itype, dense)
    excpt = rpclib.check_server_exception()
    if excpt["status"]:
        raise RuntimeError(excpt["info"])
    sptr = res["eigenval"]
    uptr = res["eigenvec"]
    m_m = res['m']
    k_k = res['k']
    eigval = FrovedisVector({'dptr' : sptr, 'size' : k_k},
                            dtype = TypeUtil.to_numpy_dtype(x_dtype)).to_numpy_array()
    if wantEv:
        eigvec = FrovedisDenseMatrix('C', {'dptr' : uptr, 'nrow' : m_m, 'ncol' : k_k},
                                     dtype = TypeUtil.to_numpy_dtype(x_dtype)).to_numpy_array()
        return eigval, eigvec
    else:
        return eigval
