"""scalapack.py"""
#!/usr/bin/env python

import numpy as np
from ..matrix.dense import FrovedisBlockcyclicMatrix
from ..matrix.wrapper import SCALAPACK

def handle_scalapack_input(a, overwrite_a, dtype):
    """
    This routine checks if a is a FrovedisBlockcyclicMatrix or a numpy matrix.
    In case of a FrovedisBlockcyclicMatrix, no copy is made,
    rather an assignment of reference is made to input_a. In case of a numpy
    matrix, it is converted to a FrovedisBlockcyclicMatrix of the same
    datatype.
    The routine returns input_a(which holds a copy if overwrite is
    not required else a direct reference in case of overwrite)
    and to_convert flag(whether we have to convert to BCM or not)
    """
    if isinstance(a, FrovedisBlockcyclicMatrix):
        isMatrix = False
        to_convert = False
    else:
        to_convert = True

    if to_convert:
        # 'a' is numpy matrix, so converting into frovedis bcm
        if type(a).__name__ == 'matrix':
            isMatrix = True
        else:
            isMatrix = False
        input_a = FrovedisBlockcyclicMatrix(a, dtype=dtype)
    else:
        if overwrite_a == 0:
            # 'a' is frovedis bcm and needs no overwrite, thus copying it
            input_a = FrovedisBlockcyclicMatrix(a, dtype=dtype)
        else:
            # 'a' is frovedis bcm and needs overwrite, thus no physical \
            #    copy here
            input_a = a  # just assignment of reference
    return (input_a, to_convert, isMatrix)

def handle_scalapack_output(input_b, to_convert, b, overwrite_b, \
                            stat, isMatrix):
    """ if 'b' is numpy matrix and overwrite_b is non-zero,
        'b' would be overwritten in-place. """
    if to_convert:
        if isMatrix:
            if overwrite_b == 0:
                output_b = input_b.to_numpy_matrix()
            else:
                input_b.to_numpy_matrix_inplace(b) # b will be inplace
                                                   #    updated (overwritten)
                output_b = b
        else:
            if overwrite_b == 0:
                output_b = input_b.to_numpy_array()
            else:
                input_b.to_numpy_array_inplace(b) # b will be inplace
                                                  #    updated (overwritten)
                output_b = b
    else:
        output_b = input_b
    return (output_b, stat)

def getrf(a, overwrite_a=0, dtype=np.float64):
    """getrf"""
    (input_a, to_convert, isMatrix) = \
        handle_scalapack_input(a, overwrite_a, dtype=dtype)
    res = SCALAPACK.getrf(input_a) # input_a will be overwritten with LU factor
    lu, stat = handle_scalapack_output(input_a, to_convert, a,\
                                       overwrite_a, res.stat(), isMatrix)
    return (lu, res, stat)

def dgetrf(a, overwrite_a=0):
    """ wrapper of native scalapack pdgetrf (double precision)
        input:
          a: numpy matrix (any type) or FrovedisBlockcyclicMatrix
              (np.float64 type)
          overwrite_a: optional integer with default 0. Input matrix a is
	               overwritten with 'LU' in case the value is not 0.
        output:
          tuple: (lu, res, stat)
            lu: if input 'a' is numpy matrix, output 'lu' is also numpy
                matrix containing LU factor of input 'a'.
                if input 'a' is FrovedisBlockcyclicMatrix, output 'lu' will be
                FrovedisBlockclyclicMatrix conatining LU factor of input 'a'.
            res: GetrfResult object containing server side pointer of pivot
                    array
            stat: returning status (info) of native scalapack dgetrf
    """
    return getrf(a, overwrite_a, dtype=np.float64)

def sgetrf(a, overwrite_a=0):
    """ wrapper of native scalapack psgetrf (single precision)
        input:
          a: numpy matrix (any type) or FrovedisBlockcyclicMatrix
              (np.float32 type)
          overwrite_a: optional integer with default 0. Input matrix a is
	               overwritten with 'LU' in case the value is not 0.
        output:
          tuple: (lu, res, stat)
            lu: if input 'a' is numpy matrix, output 'lu' is also numpy
                matrix containing LU factor of input 'a'.
                if input 'a' is FrovedisBlockcyclicMatrix, output 'lu' will be
                FrovedisBlockclyclicMatrix conatining LU factor of input 'a'.
            res: GetrfResult object containing server side pointer of
                 pivot array
            stat: returning status (info) of native scalapack dgetrf
    """
    return getrf(a, overwrite_a, dtype=np.float32)

def getri(lu, piv, lwork=0, overwrite_lu=0, dtype=np.float64):
    """getri"""
    (input_lu, to_convert, isMatrix) = \
        handle_scalapack_input(lu, overwrite_lu, dtype=dtype)
    stat = SCALAPACK.getri(input_lu, piv.ipiv()) # input_lu will be
                                                 #   overwritten with inverse_m
    return handle_scalapack_output(input_lu, to_convert, \
                lu, overwrite_lu, stat, isMatrix)

def dgetri(lu, piv, lwork=0, overwrite_lu=0):
    """ wrapper of native scalapack pdgetri (double precision)
        input:
          lu: numpy matrix (any type) or FrovedisBlockcyclicMatrix
              (np.float64 type)
          piv: GetrfResult object containing pivot information (ipiv)
          lwork: optional integer value without any effect (internally skipped)
          overwrite_lu: optional integer value, if ZERO 'lu' would remain
                         unchanged.
                        Otherwise 'lu' would be overwritten with inverse
                          matrix.
        output:
          tuple: (inv_a, stat)
            inv_a: if input 'lu' is numpy matrix, output 'inv_a' is also numpy
                matrix containing inverse matrix.
                if input 'lu' is FrovedisBlockcyclicMatrix, output 'inv_a'
                  will be
                FrovedisBlockclyclicMatrix conatining inverse matrix.
            stat: returning status (info) of native scalapack dgetri
    """
    return getri(lu, piv, lwork, overwrite_lu, dtype=np.float64)

def sgetri(lu, piv, lwork=0, overwrite_lu=0):
    """ wrapper of native scalapack psgetri (single precision)
        input:
          lu: numpy matrix (any type) or FrovedisBlockcyclicMatrix
            (np.float32 type)
          piv: GetrfResult object containing pivot information (ipiv)
          lwork: optional integer value without any effect (internally skipped)
          overwrite_lu: optional integer value, if ZERO 'lu' would remain
                        unchanged.
                        Otherwise 'lu' would be overwritten with inverse matrix.
        output:
          tuple: (inv_a, stat)
            inv_a: if input 'lu' is numpy matrix, output 'inv_a' is also numpy
                matrix containing inverse matrix.
                if input 'lu' is FrovedisBlockcyclicMatrix, output 'inv_a'
                will be
                FrovedisBlockclyclicMatrix conatining inverse matrix.
            stat: returning status (info) of native scalapack dgetri
    """
    return getri(lu, piv, lwork, overwrite_lu, dtype=np.float32)

def getrs(lu, piv, b, trans=0, overwrite_b=0, dtype=np.float64):
    """getrs"""
    (input_b, to_convert, isMatrix) = \
        handle_scalapack_input(b, overwrite_b, dtype=dtype)
    if trans == 0:
        is_trans = False
    elif trans == 1 or trans == 2:
        is_trans = True
    else: raise ValueError("trans: can be trans>=0 && trans <=2")
    stat = SCALAPACK.getrs(lu, input_b, piv.ipiv(), is_trans) # input_b will be
                                        # overwritten with solution matrix 'x'
    return handle_scalapack_output(input_b, to_convert, \
                b, overwrite_b, stat, isMatrix)

def dgetrs(lu, piv, b, trans=0, overwrite_b=0):
    """ wrapper of native scalapack pdgetrs (double precision)
        input:
          lu: numpy matrix (any type) or FrovedisBlockcyclicMatrix
              (np.float64 type)
              computed using dgetrf
          piv: GetrfResult object containing pivot information (ipiv)
          b: This is the right hand side matrix of the linear equation.
             It can be a numpy matrix(any type) or FrovedisBlockcyclicMatrix
             (np.float64 type).
          trans: optional integer variable indicating if transpose of LU needs
                 to be
                 computed before solving linear equation. If not ZERO,
                 transpose is computed.
          overwrite_b: optional integer value, if ZERO 'b' would remain
                       unchanged.
                       Otherwise 'b' would be overwritten with the solution
                       matrix.
        output:
          tuple: (x, stat)
            x: if input 'b' is numpy matrix, output 'x' is also numpy
                matrix containing the solution matrix.
                if input 'b' is FrovedisBlockcyclicMatrix, output 'x' will
                also be
                FrovedisBlockclyclicMatrix conatining the solution matrix.
            stat: returning status (info) of native scalapack dgetrs
    """
    return getrs(lu, piv, b, trans, overwrite_b, dtype=np.float64)

def sgetrs(lu, piv, b, trans=0, overwrite_b=0):
    """ wrapper of native scalapack psgetrs (single precision)
        input:
          lu: numpy matrix (any type) or FrovedisBlockcyclicMatrix
              (np.float32 type)
              computed using sgetrf.
          ipiv: GetrfResult object containing pivot information (ipiv)
          b: This is the right hand side matrix of the linear equation.
             It can be a numpy matrix(any type) or FrovedisBlockcyclicMatrix
             (np.float32 type).
          trans: optional integer variable indicating if transpose of LU needs
                 to be computed before solving linear equation. If not ZERO,
                 transpose is computed.
          overwrite_b: optional integer value, if ZERO 'b' would remain
                       unchanged.
                       Otherwise 'b' would be overwritten with the solution
                       matrix.
        output:
          tuple: (x, stat)
            x: if input 'b' is numpy matrix, output 'x' is also numpy
                matrix containing the solution matrix.
                if input 'b' is FrovedisBlockcyclicMatrix, output 'x' will
                also be FrovedisBlockclyclicMatrix conatining the solution
                matrix.
            stat: returning status (info) of native scalapack dgetrs
    """
    return getrs(lu, piv, b, trans, overwrite_b, dtype=np.float32)

def gesv(a, b, overwrite_a=0, overwrite_b=0, dtype=np.float64):
    """gesv"""
    lu, piv, rf_stat = getrf(a, overwrite_a, dtype=dtype)
    if rf_stat > 0:
        raise ValueError("LU factorization has been completed, \
                but the factor U is exactly singular")
    trans = 0
    x, rs_stat = getrs(lu, piv, b, trans, overwrite_b, dtype=dtype)
    return (lu, piv, x, rs_stat)

def dgesv(a, b, overwrite_a=0, overwrite_b=0):
    """
    wrapper of native scalapack pdgesv (double precision)
        input:
          a: This is the left hand side matrix of the linear equation. It can
             be a
             numpy matrix(any type) or FrovedisBlockcyclicMatrix
             (np.float64 type).
          b: This is the right hand side matrix of the linear equation.
             It can be a numpy matrix(any type) or FrovedisBlockcyclicMatrix
             (np.float64 type).
          overwrite_a: optional integer value, if ZERO 'a' would remain
                       unchanged.
                        Otherwise 'a' would be overwritten with the 'LU' factor.
          overwrite_b: optional integer value, if ZERO 'b' would remain
                       unchanged.
                        Otherwise 'b' would be overwritten with the solution
                        matrix.
        output:
          tuple: (lu, piv, x, stat)
            lu: if input 'a' is numpy matrix, output 'lu' is also numpy
                matrix containing LU factor of input 'a'.
                if input 'a' is FrovedisBlockcyclicMatrix, output 'lu' will be
                FrovedisBlockclyclicMatrix conatining LU factor of input 'a'.
            piv: GetrfResult object containing server side pointer of pivot
                 array
            x: if input 'b' is numpy matrix, output 'x' is also numpy
                matrix containing the solution matrix.
                if input 'b' is FrovedisBlockcyclicMatrix, output 'x' will
                also be
                FrovedisBlockclyclicMatrix conatining the solution matrix.
            rs_stat: returning status (info) of native scalapack dgetrs
    """
    return gesv(a, b, overwrite_a, overwrite_b, dtype=np.float64)

def sgesv(a, b, overwrite_a=0, overwrite_b=0):
    """
    wrapper of native scalapack psgesv (single precision)
        input:
          a: This is the left hand side matrix of the linear equation. It can
             be a numpy matrix(any type) or FrovedisBlockcyclicMatrix
             (np.float32 type).
          b: This is the right hand side matrix of the linear equation.
             It can be a numpy matrix(any type) or FrovedisBlockcyclicMatrix
             (np.float32 type).
          overwrite_a: optional integer value, if ZERO 'a' would remain
             unchanged.
                        Otherwise 'a' would be overwritten with the 'LU' factor.
          overwrite_b: optional integer value, if ZERO 'b' would remain
                        unchanged.
                        Otherwise 'b' would be overwritten with the solution
                        matrix.
        output:
        tuple: (lu, piv, x, stat)
            lu: if input 'a' is numpy matrix, output 'lu' is also numpy
                matrix containing LU factor of input 'a'.
                if input 'a' is FrovedisBlockcyclicMatrix, output 'lu' will be
                FrovedisBlockclyclicMatrix conatining LU factor of input 'a'.
            piv: GetrfResult object containing server side pointer of pivot
                  array
            x: if input 'b' is numpy matrix, output 'x' is also numpy
                matrix containing the solution matrix.
                if input 'b' is FrovedisBlockcyclicMatrix, output 'x' will
                 also be
                FrovedisBlockclyclicMatrix conatining the solution matrix.
            rs_stat: returning status (info) of native scalapack dgetrs
    """
    return gesv(a, b, overwrite_a, overwrite_b, dtype=np.float32)

def gels(a, b, trans='N', lwork=0, overwrite_a=0,
          overwrite_b=0, dtype=np.float64):
    """gels"""
    (input_a, to_convert_a, isMatrix) = \
        handle_scalapack_input(a, overwrite_a, dtype=dtype)
    (input_b, to_convert_b, isMatrix) = \
        handle_scalapack_input(b, overwrite_b, dtype=dtype)
    if trans == 'N':
        is_trans = False
    elif trans == 'T':
        is_trans = True
    else: raise ValueError("trans: can be either N or T")
    # input_a will be overwritten with lqr,
    #    input_b will be overwritten with solution
    stat = SCALAPACK.gels(input_a, input_b, is_trans)
    lqr, _ = handle_scalapack_output(input_a, to_convert_a, \
                a, overwrite_a, stat, isMatrix)
    x, _ = handle_scalapack_output(input_b, to_convert_b, b, \
                overwrite_b, stat, isMatrix)
    return (lqr, x, stat)

def dgels(a, b, trans='N', lwork=0, overwrite_a=0, overwrite_b=0):
    """ wrapper of native scalapack pdgels (double precision)
        input:
          a: LHS numpy matrix (any type) or FrovedisBlockcyclicMatrix
                (np.float64 type)
          b: RHS numpy matrix (any type) or FrovedisBlockcyclicMatrix
                (np.float64 type)
          lwork: optional integer value without any effect (internally skipped)
          trans: optional integer variable indicating if transpose of A needs
                 to be computed before solving linear equation. If not ZERO,
                 transpose is computed.
          overwrite_a: optional integer value, if ZERO 'a' would remain
                       unchanged.
                        Otherwise 'a' would be overwritten with QR or LQ factor.
          overwrite_b: optional integer value, if ZERO 'b' would remain
                        unchanged.
                        Otherwise 'b' would be overwritten with the solution
                        matrix.
        output:
          tuple: (lqr, x, stat)
            lqr: QR or LQ factor. If input 'a' is a numpy matrix, output 'lqr'
                 will be
                 a numpy matrix contaoing the QR or LQ factor, else it will be a
                 FrovedisBlockclyclic matrix.
            x: if input 'b' is numpy matrix, output 'x' is also numpy
                matrix containing the solution matrix.
                if input 'b' is FrovedisBlockcyclicMatrix, output 'x' will
                also be
                FrovedisBlockclyclicMatrix conatining the solution matrix.
            stat: returning status (info) of native scalapack dgetrs
    """
    return gels(a, b, trans, lwork, overwrite_a, overwrite_b, dtype=np.float64)

def sgels(a, b, trans='N', lwork=0, overwrite_a=0, overwrite_b=0):
    """
    wrapper of native scalapack psgels (single precision)
        input:
          a: LHS numpy matrix (any type) or FrovedisBlockcyclicMatrix
             (np.float32 type)
          b: RHS numpy matrix (any type) or FrovedisBlockcyclicMatrix
             (np.float32 type)
          lwork: optional integer value without any effect (internally skipped)
          trans: optional integer variable indicating if transpose of A needs
                 to be computed before solving linear equation. If not ZERO,
                 transpose is computed.
          overwrite_a: optional integer value, if ZERO 'b' would remain
                       unchanged.
                        Otherwise 'a' would be overwritten with QR or LQ factor.
          overwrite_b: optional integer value, if ZERO 'b' would remain
                       unchanged.
                        Otherwise 'b' would be overwritten with the solution
                        matrix.
        output:
          tuple: (lqr, x, stat)
            lqr: QR or LQ factor. If input 'a' is a numpy matrix, output
                       'lqr' will be
                 a numpy matrix contaoing the QR or LQ factor, else it will be a
                 FrovedisBlockclyclic matrix.
            x: if input 'b' is numpy matrix, output 'x' is also numpy
                matrix containing the solution matrix.
                if input 'b' is FrovedisBlockcyclicMatrix, output 'x' will
                  also be
                FrovedisBlockclyclicMatrix conatining the solution matrix.
            stat: returning status (info) of native scalapack dgetrs
    """
    return gels(a, b, trans, lwork, overwrite_a, overwrite_b, dtype=np.float32)

def gesvd(a, compute_uv=1, full_matrices=1, lwork=0,
          overwrite_a=0, dtype=np.float64):
    """gesvd"""
    (input_a, to_convert, isMatrix) = \
        handle_scalapack_input(a, overwrite_a, dtype=dtype)
    if compute_uv == 0:
        wantU = False
        wantV = False
    elif compute_uv == 1:
        wantU = True
        wantV = True
    else:
        raise ValueError("compute_uv: can be either 0 or 1")
    res = SCALAPACK.gesvd(input_a, wantU, wantV) # input_a will be destroyed
    _, stat = handle_scalapack_output(input_a, to_convert, a,
                                            overwrite_a, res.stat(), isMatrix)
    if to_convert:
        u, s, vt = res.to_numpy_results()
        if u is None:
            u = np.zeros((1, 1), dtype=dtype)
        if vt is None:
            vt = np.zeros((1, 1), dtype=dtype)
        return (u, s, vt, stat)
    else:
        umat = res.umat_ # FrovedisDenseMatrix
        vmat = res.vmat_ # FrovedisDenseMatrix
        s = res.singular_values_ # FrovedisVector
        if umat is None:
            u = FrovedisBlockcyclicMatrix.asBCM(np.zeros((1, 1), dtype=dtype))
        if vmat is None:
            vt = FrovedisBlockcyclicMatrix.asBCM(np.zeros((1, 1), dtype=dtype))
        if umat is not None and vmat is not None:
            u = FrovedisBlockcyclicMatrix.asBCM(umat)
            vt = FrovedisBlockcyclicMatrix.asBCM(vmat)
            umat.set_none() # setting none to avoid auto
                            #  release of dense matrix, umat
            vmat.set_none() # setting none to avoid auto
                            #  release of dense matrix, vmat
        res.set_none() # setting none to avoid auto
                       #  release of result components
        return (u, s, vt, stat)

def dgesvd(a, compute_uv=1, full_matrices=1, lwork=0, overwrite_a=0):
    """ wrapper of native scalapack pdgesvd (double precision)
        input:
          a: LHS numpy matrix (any type) or FrovedisBlockcyclicMatrix
               (np.float64 type)
	  compute_uv: optional integer value with default value ONE, indicating
	              that left singular matrix and right singular matrix will
                      be computed. If set as ZERO, a numpy matrix of shape
                      (1,1) and datatype same as A will be assigned to the
                      left and right singular matrix.
	  full_matrix: optional integer value with default value ONE.
          lwork: optional integer value without any effect (internally skipped)
          overwrite_a: optional integer value, if ZERO 'a' would remain
                        unchanged.
                        Otherwise 'a' would be consumed internally and its
			contents will be destroyed.
        output:
          tuple: (u, s, vt, stat)
            u: Left singular matrix, will be numpy matrix if a is a numpy
               matrix,or a FrovedisBlockcyclicMatrix if a is a
               FrovedisBlockcyclicMatrix.
            s: singular matrix.
	    vt: Left singular matrix, will be numpy matrix if a is a numpy
                matrix,	or a FrovedisBlockcyclicMatrix if a is a
                FrovedisBlockcyclicMatrix.
            stat: returning status (info) of native scalapack dgetrs
    """
    return gesvd(a, compute_uv, full_matrices, lwork,
                 overwrite_a, dtype=np.float64)


def sgesvd(a, compute_uv=1, full_matrices=1, lwork=0, overwrite_a=0):
    """ wrapper of native scalapack psgesvd (single precision)
        input:
          a: LHS numpy matrix (any type) or FrovedisBlockcyclicMatrix
              (np.float32 type)
	  compute_uv: optional integer value with default value ONE, indicating
	              that left singular matrix and right singular matrix will
                        be computed. If set as ZERO, a numpy matrix of shape
                        (1,1) and datatype same as A will be assigned to the
                        left and right singular matrix.
	  full_matrix: optional integer value with default value ONE.
          lwork: optional integer value without any effect (internally skipped)
          overwrite_a: optional integer value, if ZERO 'a' would remain
                         unchanged.
                        Otherwise 'a' would be consumed internally and its
			contents will be destroyed.
        output:
          tuple: (u, s, vt, stat)
            u: Left singular matrix, will be numpy matrix if a is a numpy
                 matrix, or a FrovedisBlockcyclicMatrix if a is a
                 FrovedisBlockcyclicMatrix.
            s: singular matrix.
	    vt: Left singular matrix, will be numpy matrix if a is a numpy
                matrix,	or a FrovedisBlockcyclicMatrix if a is a
                FrovedisBlockcyclicMatrix.
            stat: returning status (info) of native scalapack dgetrs
    """
    return gesvd(a, compute_uv, full_matrices, lwork,
                 overwrite_a, dtype=np.float32)
