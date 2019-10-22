"""wrapper.py"""
#!/usr/bin/env python

#from scipy.sparse import csr_matrix
from ..exrpc import rpclib
from ..exrpc.server import FrovedisServer
from .dense import FrovedisBlockcyclicMatrix
from .ml_data import FrovedisFeatureData
from .results import svdResult, GetrfResult
from .dtype import DTYPE, TypeUtil


class WrapperUtil:
    """A python class to check dtype equality of wrapper arguments"""

    @staticmethod
    def type_check(arg1, arg2, name):
        """type_check"""
        dt1 = arg1.get_dtype()
        dt2 = arg2.get_dtype()
        if dt1 != dt2:
            raise TypeError(name, ": input argument dtypes are not matched!")
        return dt1  # returns the type

# Input matrix/vector can be either (user created) FrovedisBlockcyclicMatrix
# or any array-like object
#   (FrovedisBlockcyclicMatrix auto creation and deletion)
class PBLAS:
    """A python wrapper to call pblas routines at Frovedis server"""

    @staticmethod
    def swap(vec1, vec2):
        """swap"""
        (host, port) = FrovedisServer.getServerInstance()
        dt = WrapperUtil.type_check(vec1, vec2, "swap")
        rpclib.pswap(host, port, vec1.get(), vec2.get(), dt)
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])

    @staticmethod
    def copy(vec1, vec2):
        """copy"""
        (host, port) = FrovedisServer.getServerInstance()
        dt = WrapperUtil.type_check(vec1, vec2, "copy")
        rpclib.pcopy(host, port, vec1.get(), vec2.get(), dt)
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])

    @staticmethod
    def scal(vec, al_):
        """scal"""
        (host, port) = FrovedisServer.getServerInstance()
        rpclib.pscal(host, port, vec.get(), al_, vec.get_dtype())
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])

    @staticmethod
    def axpy(vec1, vec2, al_=1.0):
        """axpy"""
        (host, port) = FrovedisServer.getServerInstance()
        dt = WrapperUtil.type_check(vec1, vec2, "axpy")
        rpclib.paxpy(host, port, vec1.get(), vec2.get(), al_, dt)
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])

    @staticmethod
    def dot(vec1, vec2):
        """dot"""
        vv1 = FrovedisBlockcyclicMatrix.asBCM(vec1)
        vv2 = FrovedisBlockcyclicMatrix.asBCM(vec2)
        (host, port) = FrovedisServer.getServerInstance()
        dt = WrapperUtil.type_check(vv1, vv2, "dot")
        ret = rpclib.pdot(host, port, vv1.get(), vv2.get(), dt)
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        return ret

    @staticmethod
    def nrm2(vec1):
        """nrm2"""
        vec = FrovedisBlockcyclicMatrix.asBCM(vec1)
        (host, port) = FrovedisServer.getServerInstance()
        ret = rpclib.pnrm2(host, port, vec.get(), vec.get_dtype())
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        return ret

    @staticmethod
    def gemv(mat, vec, trans=False, al_=1.0, be_=0.0):
        """gemv"""
        mat = FrovedisBlockcyclicMatrix.asBCM(mat)
        vec = FrovedisBlockcyclicMatrix.asBCM(vec)
        dt = WrapperUtil.type_check(mat, vec, "gemv")
        (host, port) = FrovedisServer.getServerInstance()
        dmat = rpclib.pgemv(host, port, mat.get(), vec.get(), trans,
                            al_, be_, dt)
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        data_type = TypeUtil.to_numpy_dtype(dt)
        return FrovedisBlockcyclicMatrix(mat=dmat, dtype=data_type)

    @staticmethod
    def ger(vec1, vec2, al_=1.0):
        """ger"""
        vv1 = FrovedisBlockcyclicMatrix.asBCM(vec1)
        vv2 = FrovedisBlockcyclicMatrix.asBCM(vec2)
        (host, port) = FrovedisServer.getServerInstance()
        dt = WrapperUtil.type_check(vv1, vv2, "ger")
        dmat = rpclib.pger(host, port, vv1.get(), vv2.get(), al_, dt)
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        data_type = TypeUtil.to_numpy_dtype(dt)
        return FrovedisBlockcyclicMatrix(mat=dmat, dtype=data_type)

    @staticmethod
    def gemm(mat1, mat2, trans_m1=False, trans_m2=False, al_=1.0, be_=0.0):
        """gemm"""
        mm1 = FrovedisBlockcyclicMatrix.asBCM(mat1)
        mm2 = FrovedisBlockcyclicMatrix.asBCM(mat2)
        (host, port) = FrovedisServer.getServerInstance()
        dt = WrapperUtil.type_check(mm1, mm2, "gemm")
        dmat = rpclib.pgemm(host, port, mm1.get(), mm2.get(),
                            trans_m1, trans_m2, al_, be_, dt)
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        data_type = TypeUtil.to_numpy_dtype(dt)
        return FrovedisBlockcyclicMatrix(mat=dmat, dtype=data_type)

    # performs m2 = m1 + m2
    @staticmethod
    def geadd(mat1, mat2, trans=False, al_=1.0, be_=1.0):
        """geadd"""
        mm1 = FrovedisBlockcyclicMatrix.asBCM(mat1)
        mm2, conv = FrovedisBlockcyclicMatrix.asBCM(mat2, retIsConverted=True)
        (host, port) = FrovedisServer.getServerInstance()
        dt = WrapperUtil.type_check(mm1, mm2, "geadd")
        rpclib.pgeadd(host, port, mm1.get(), mm2.get(), trans,
                      al_, be_, dt)
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        if conv:
            mm2.to_numpy_matrix_inplace(mat2)


# Input matrix can be either (user created) FrovedisBlockcyclicMatrix
# or any array-like object (FrovedisBlockcyclicMatrix
#   auto creation and deletion)
class SCALAPACK:
    """A python wrapper to call scalapack routines at Frovedis server"""

    @staticmethod
    def getrf(mat):
        """ wrapper of scalapack pdgetrf (computes LU factor of input matrix)
            input
              mat: can be either numpy matrix or FrovedisBlockcyclicMatrix.
                   On entry, it should contain the data matrix.
                   On exit, it would be overwritten with LU factor of
                   input matrix.
            output:
              res: GetrfResult object containing the pivot information
                   (res.ipiv())
                   and return status (info) of native scalapack routine
                   (res.stat()).
        """
        m_m, conv = FrovedisBlockcyclicMatrix.asBCM(mat, retIsConverted=True)
        (host, port) = FrovedisServer.getServerInstance()
        res = rpclib.pgetrf(host, port, m_m.get(), m_m.get_dtype())
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        if conv:
            m_m.to_numpy_matrix_inplace(mat) # overwrites mat in-place
        return GetrfResult(res)

    @staticmethod
    def getri(mat, ipiv_ptr):
        """wrapper of scalapack pdgetri (computes inverse, dependent on getrf())
            input:
              mat: can be either numpy matrix or FrovedisBlockcyclicMatrix.
                   On entry, it should contain the LU factor matrix (must be
                   precomputed using getrf().
                   On exit, it would be overwritten with inverse matrix.
              ipiv_ptr: A long value containing the pointer of server
                        side pivot array (must be precomputed using getrf()).
            output:
              return status (info) of native scalapack routine."""
        m_m, conv = FrovedisBlockcyclicMatrix.asBCM(mat,
                                                    retIsConverted=True)
        (host, port) = FrovedisServer.getServerInstance()
        ret = rpclib.pgetri(host, port, m_m.get(), ipiv_ptr, m_m.get_dtype())
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        if conv:
            m_m.to_numpy_matrix_inplace(mat) # overwrites mat in-place
        return ret

    @staticmethod
    def getrs(mat_a, mat_b, ipiv_ptr, is_trans=False):
        """ wrapper of scalapack pdgetrs (solves Ax=B, dependent on getrf())
            input:
              mat_a: can be either numpy matrix or FrovedisBlockcyclicMatrix.
                     It should contain LU factor (must be precomputed using
                     getrf()
                     of RHS matrix A.
              mat_b: can be either numpy matrix or FrovedisBlockcyclicMatrix.
                     On entry, it should contain the RHS matrix B.
                     On exit, it would be overwritten with solution matrix x.
              ipiv_ptr: A long value containing the pointer of server
                        side pivot array (must be precomputed using getrf()).
              is_trans: A boolean parameter, if true solves "transpose(A)x = B".
                        Otherwise, it solves Ax = B.
            output:
              return status (info) of native scalapack routine.
        """
        mm_a = FrovedisBlockcyclicMatrix.asBCM(mat_a)
        mm_b, conv = FrovedisBlockcyclicMatrix.asBCM(mat_b,
                                                     retIsConverted=True)
        (host, port) = FrovedisServer.getServerInstance()
        dt = WrapperUtil.type_check(mm_a, mm_b, "getrs")
        ret = rpclib.pgetrs(host, port, mm_a.get(), # no change on mm_a
                            mm_b.get(), ipiv_ptr, is_trans, dt)
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        if conv:
            mm_b.to_numpy_matrix_inplace(mat_b)
        return ret

    @staticmethod
    def gesv(mat_a, mat_b):
        """ wrapper of scalapack pdgesv (solves Ax=B)
            this can be considererd as (combination of getrf() + getrs()):
               -> res = getrf(mat_a)
               -> getrs(mat_a, mat_b, res.ipiv())
            input:
              mat_a: can be either numpy matrix or FrovedisBlockcyclicMatrix.
                     On entry, it should contain the LHS matrix, A.
                     On exit, it would be overwritten with LU factor of A.
              mat_b: can be either numpy matrix or FrovedisBlockcyclicMatrix.
                     On entry, it should contain the RHS matrix B.
                     On exit, it would be overwritten with solution matrix x.
            output:
              return status (info) of native scalapack routine.
        """
        mm_a, conv1 = FrovedisBlockcyclicMatrix.asBCM(mat_a,
                                                      retIsConverted=True)
        mm_b, conv2 = FrovedisBlockcyclicMatrix.asBCM(mat_b,
                                                      retIsConverted=True)
        (host, port) = FrovedisServer.getServerInstance()
        dt = WrapperUtil.type_check(mm_a, mm_b, "gesv")
        ret = rpclib.pgesv(host, port, mm_a.get(), mm_b.get(), dt)
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        if conv1:
            mm_a.to_numpy_matrix_inplace(mat_a)
        if conv2:
            mm_b.to_numpy_matrix_inplace(mat_b)
        return ret

    @staticmethod
    def gels(mat_a, mat_b, is_trans=False):
        """ wrapper of scalapack pdgels (solves Ax=B)
            input:
              mat_a: can be either numpy matrix or FrovedisBlockcyclicMatrix.
                     On entry, it should contain the LHS matrix, A.
                     On exit, it would be overwritten with QR or LQ factor of A.
              mat_b: can be either numpy matrix or FrovedisBlockcyclicMatrix.
                     On entry, it should contain the RHS matrix B.
                     On exit, it would be overwritten with solution matrix x.
              is_trans: A boolean parameter, if true solves "transpose(A)x = B".
                        Otherwise, it solves Ax = B.
            output:
              return status (info) of native scalapack routine.
        """
        mm_a, conv1 = FrovedisBlockcyclicMatrix.asBCM(mat_a,
                                                      retIsConverted=True)
        mm_b, conv2 = FrovedisBlockcyclicMatrix.asBCM(mat_b,
                                                      retIsConverted=True)
        (host, port) = FrovedisServer.getServerInstance()
        dt = WrapperUtil.type_check(mm_a, mm_b, "gels")
        ret = rpclib.pgels(host, port, mm_a.get(), mm_b.get(), is_trans, dt)
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        if conv1:
            mm_a.to_numpy_matrix_inplace(mat_a)
        if conv2:
            mm_b.to_numpy_matrix_inplace(mat_b)
        return ret

    @staticmethod
    def gesvd(mat, want_u=True, want_v=True):
        """ wrapper of scalapack pdgesvd (computes Singular Value Decomposition)
            input:
              mat:    can be either numpy matrix or FrovedisBlockcyclicMatrix.
                      On entry, it should contain the input matrix.
                      On exit, no change if input is numpy matrix. Otherwise, it
                      would be destroyed (internally used as workspace in
                      server side).
              want_u: A boolean parameter, if true computes left singular
                      vector along with singular values. Otherwise left-singular
                      vector is not computed.
              want_v: A boolean parameter, if true computes iright singular
                      vector along with singular values. Otherwise
                      right-singular
                      vector is not computed.
            output:
              res: svdResult object containing U,s,V
                   U and V are FrovedisColmajorMatrix objects.
                   s: A long value containing the pointer of server side vector
                      for singular values
        """
        m_m = FrovedisBlockcyclicMatrix.asBCM(mat)
        (host, port) = FrovedisServer.getServerInstance()
        res = rpclib.pgesvd(host, port, m_m.get(), want_u, want_v,
                            m_m.get_dtype())
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        return svdResult(res, TypeUtil.to_numpy_dtype(m_m.get_dtype()))

class ARPACK:
    """A python wrapper to call arpack routines to compute
       truncated svd at Frovedis server"""

    @staticmethod
    def compute_svd(X, k):
        inp_data = FrovedisFeatureData(X, dense_kind='rowmajor')
        X = inp_data.get()
        dtype = inp_data.get_dtype()
        itype = inp_data.get_itype()
        dense = inp_data.is_dense()
        (host, port) = FrovedisServer.getServerInstance()
        res = rpclib.compute_truncated_svd(host, port, X.get(),
                                           k, dtype, itype, dense)
        excpt = check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        return svdResult(res, TypeUtil.to_numpy_dtype(dtype))
