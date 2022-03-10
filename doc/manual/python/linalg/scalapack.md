% scalapack

# NAME  
scalapack - it contains wrapper functions created for low level functions from ScaLAPACK library (present in frovedis).  

# SYNOPSIS  

    import frovedis.linalg.scalapack  

# Public Member Functions  

    1. dgels(a, b, trans='N', lwork=0, overwrite_a=0, overwrite_b=0)
    2. gels(a, b, trans='N', lwork=0, overwrite_a=0, overwrite_b=0, dtype=np.float64)
    3. sgels(a, b, trans='N', lwork=0, overwrite_a=0, overwrite_b=0)
    4. dgesv(a, b, overwrite_a=0, overwrite_b=0)
    5. gesv(a, b, overwrite_a=0, overwrite_b=0, dtype=np.float64)
    6. sgesv(a, b, overwrite_a=0, overwrite_b=0) 
    7. dgesvd(a, compute_uv=1, full_matrices=0, lwork=0, overwrite_a=0) 
    8. gesvd(a, compute_uv=1, full_matrices=0, lwork=0, overwrite_a=0, dtype=np.float64)
    9. sgesvd(a, compute_uv=1, full_matrices=0, lwork=0, overwrite_a=0)    
    10. dgetrf(a, overwrite_a=0)
    11. getrf(a, overwrite_a=0, dtype=np.float64)
    12. sgetrf(a, overwrite_a=0)
    13. dgetri(lu, piv, lwork=0, overwrite_lu=0)
    14. getri(lu, piv, lwork=0, overwrite_lu=0, dtype=np.float64)
    15. sgetri(lu, piv, lwork=0, overwrite_lu=0)    
    16. dgetrs(lu, piv, b, trans=0, overwrite_b=0)
    17. getrs(lu, piv, b, trans=0, overwrite_b=0, dtype=np.float64)
    18. sgetrs(lu, piv, b, trans=0, overwrite_b=0)

# DESCRIPTION  

In Frovedis, the linalg module contains wrapper for ScaLAPACK (Scalable LAPACK: the LAPACK routines distributed in nature) routines, whereas scipy.linalg contains wrapper for LAPACK routines. Here, the method names, arguments, purpose etc. in frovedis.linalg wrapper routines are similar to scipy.linalg wrapper routines.  
In order to use dgetrf() wrapper present in linalg module of both frovedis and scipy:  

**In frovedis:**  
  
    from frovedis.linalg.scalapack import dgetrf  

**In scipy:**  
    
    from scipy.linalg.lapack import dgetrf

This python module implements a client-server application, where the python client can send the python matrix data to 
frovedis server side in order to create blockcyclic matrix at frovedis server and then perform the supported ScaLAPACK operation requested by the python client on that matrix. After request is completed, frovedis server sends back the resultant matrix and it can then create equivalent python data.  

**We have supported 'overwrite' option in frovedis routine wrappers.**  

**However, unlike scipy.linalg, if 'overwrite' option is enabled and input is a valid ndarray or an instance of FrovedisBlockcyclicMatrix, we would always overwrite (copy-back) to the same (irrespective of its dtype).**  

**This will slow down the overall computation. This should be enabled only when it's wanted to check the intermediate results (like LU factor etc.)**  

## Detailed Description  

### 1. dgels(a, b, trans = 'N', lwork = 0, overwrite_a = 0, overwrite_b = 0)  

__Parameters__  
_**a**_: It accepts a python array-like input or left hand side numpy matrix of the linear equation having int, float (float32) or double (float64) type values. It also accepts FrovedisBlockcyclicMatrix instance having double (float64) type values.  
_**b**_: It accepts a python array-like input or right hand side numpy matrix of the linear equation having int, float (float32) or double (float64) type values. It also accepts FrovedisBlockcyclicMatrix instance having double (float64) type values. It should have number of rows >= max(M,N) and at least 1 column.  
_**trans**_: It accepts a string object parameter like 'N' or 'T' which specifies if transpose of 'a' is needed to be computed before solving linear equation. If set to 'T', then transpose is computed. (Default: 'N')  
_**lwork**_: This is an ununsed parameter. (Default: 0)  
_**overwrite\_a**_: It accepts an integer parameter, if set to 0, then 'a' will remain unchanged. Otherwise, 'a' would be overwritten 
with QR or LQ factor. (Default: 0)  
_**overwrite\_b**_: It accepts an integer parameter, if set to 0, then 'b' will remain unchanged. Otherwise, 'b' would be overwritten 
with the solution matrix. (Default: 0)  

__Purpose__  
This method solves overdetermined or underdetermined real linear systems involving a left hand side matrix 'a' or its transpose, using a QR or LQ factorization of 'a'. It is assumed that the matrix 'a' has full rank. It returns the LQ or QR factor, solution matrix with double (float64) precision.  

The parameter: "lwork" is simply kept in to to make the interface uniform to the scipy.linalg.lapack.dgels() module. They 
are not used anywhere within the frovedis implementation.  

This method internally uses Scalapack.gels() **(present in frovedis.matrix module)**.  

For example,  

    import numpy as np
    mat1 = np.array([[1.,0.,2.],[-1.,5.,0.],[0.,3.,-9.]])
    mat2 = np.array([[-9.,3.,-1.],[0.,3.,1.],[2.,2.,0.]])
    
    # dgels() demo
    from frovedis.linalg.scalapack import dgels
    rf = dgels(mat1,mat2)
    print(rf)
    
Output  

    (array([[-1.41421356,  3.53553391, -1.41421356],
       [-0.41421356, -4.63680925,  4.74464202],
       [ 0.        ,  0.36709178, -7.77742709]]), array([[-7.54901961,  2.68627451, -1.        ],
       [-1.50980392,  1.1372549 ,  0.        ],
       [-0.7254902 ,  0.15686275,  0.        ]]), 0)


For example,  
    
    # dgels() demo and using transpose of a
    from frovedis.linalg.scalapack import dgels
    rf = dgels(mat1,mat2,trans='T')
    print(rf)
    
Output  

    (array([[-1.41421356,  3.53553391, -1.41421356],
       [-0.41421356, -4.63680925,  4.74464202],
       [ 0.        ,  0.36709178, -7.77742709]]), array([[-7.82352941,  3.29411765, -0.70588235],
       [ 1.17647059,  0.29411765,  0.29411765],
       [-1.96078431,  0.50980392, -0.15686275]]), 0)

For example,  
    
    # dgels() demo and overwriting of a with QR or LQ factor
    print('original matrix mat1: ')
    print(mat1)
    from frovedis.linalg.scalapack import dgels
    rf = dgels(mat1,mat2,overwrite_a=1)
    print('overwritten matrix with LQ or QF factor: ')
    print(mat1)
    
Output  

    original matrix mat1:  
    [[ 1.  0.  2.]
     [-1.  5.  0.]
     [ 0.  3. -9.]]
    
    overwritten matrix with LQ or QF factor: 
     [[-1.41421356  3.53553391 -1.41421356]
     [-0.41421356 -4.63680925  4.74464202]
     [ 0.          0.36709178 -7.77742709]]

**Here, if the input 'mat1' is double (float64) type and overwite is enabled, then LQ or QR factor would 
also be double (float64) type.**  

**Same applies for other types (int, float (float32)) when input is a numpy matrix/array and overwrite is enabled.**  

For example,  
    
    # dgels() demo and overwriting of b with solution matrix
    print('original matrix mat2: ')
    print(mat2)
    from frovedis.linalg.scalapack import dgels
    rf = dgels(mat1,mat2,overwrite_b=1)
    print('overwritten matrix with solution matrix: ')
    print(mat2)
    
Output  

    original matrix mat2: 
    [[-9.  3. -1.]
     [ 0.  3.  1.]
     [ 2.  2.  0.]]
    
    overwritten matrix with solution matrix: 
    [[-7.54901961  2.68627451 -1.        ]
     [-1.50980392  1.1372549   0.        ]
     [-0.7254902   0.15686275  0.        ]]

**Here, if the input 'mat2' is double (float64) type and overwite is enabled, then solution matrix 
would also be double (float64) type.**  

**Same applies for other types (int, float (float32)) when input is a numpy matrix/array and overwrite is enabled.**  


For example,  
    
    from frovedis.matrix.dense import FrovedisBlockcyclicMatrix
    mat1 = np.array([[1.,0.,2.],[-1.,5.,0.],[0.,3.,-9.]])
    mat2 = np.array([[-9.,3.,-1.],[0.,3.,1.],[2.,2.,0.]])
    bcm1 = FrovedisBlockcyclicMatrix(mat1)
    bcm2 = FrovedisBlockcyclicMatrix(mat2)

    # dgels() demo and a and b as an instance of FrovedisBlockcyclicMatrix
    from frovedis.linalg.scalapack import dgels
    rf = dgels(bcm1,bcm2)
    
    # Unpacking the tuple
    rf[0].debug_print()
    rf[1].debug_print()
    print(rf[2])
    
Output  

    0
    matrix:
    num_row = 3, num_col = 3
    node 0
    node = 0, local_num_row = 3, local_num_col = 3, type = 2, descriptor = 1 1 3 3 3 3 0 0 3 3 3
    val = -1.41421 -0.414214 0 3.53553 -4.63681 0.367092 -1.41421 4.74464 -7.77743
    matrix:
    num_row = 3, num_col = 3
    node 0
    node = 0, local_num_row = 3, local_num_col = 3, type = 2, descriptor = 1 1 3 3 3 3 0 0 3 3 3
    val = -7.54902 -1.5098 -0.72549 2.68627 1.13725 0.156863 -1 0 0

__Return Value__   
1. **If 'a' and 'b' are python inputs such as numpy matrices**:  
     - It returns a tuple **(lqr, x, stat)** where,  
       - **lqr**: It is a numpy matrix having double (float64) type values (by default) and containing the QR or LQ factor of input matrix 'a'. In case 'overwrite_a' is enabled, then dtype for the matrix will depend on input 'a' dtype.  
       - **x**: It is also a numpy matrix having double (float64) type values (by default) and containing the solution matrix. In case 'overwrite_b' is enabled, then dtype for the matrix will depend on input 'b' dtype.  
       - **stat**: It returns an integer containing status (info) of native scalapack dgetrs.  
2. **If 'a' and 'b' are instances of FrovedisBlockcyclicMatrix:**  
     - It returns a tuple **(lqr, x, stat)** where,  
       - **lqr**: It returns instance of FrovedisBlockcyclicMatrix containing the QR or LQ factor of input matrix 'a'.  
       - **x**: It returns an instance of FrovedisBlockcyclicMatrix containing the solution matrix.  
       - **stat**: It returns an integer containing status (info) of native scalapack dgetrs.  
     Here, the original inputs 'a' and 'b' are not overwritten even when overwrite_a/overwrite_b is enabled.  


### 2. gels(a, b, trans = 'N', lwork = 0, overwrite_a = 0, overwrite_b = 0, dtype = np.float64)  

__Parameters__  
_**a**_: It accepts a python array-like input or left hand side numpy matrix of the linear equation having int, float (float32) or double (float64) type values. It also accepts FrovedisBlockcyclicMatrix instance having float (float32) or double (float64) type values.  
_**b**_: It accepts a python array-like input or right hand side numpy matrix of the linear equation having int, float (float32) or double (float64) type values. It also accepts FrovedisBlockcyclicMatrix instance having float (float32) or double (float64) type values. It should have number of rows >= max(M,N) and at least 1 column.  
_**trans**_: It accepts a string object parameter like 'N' or 'T' which specifies if transpose of 'a' is needed to be computed before solving linear equation. If set to 'T', then transpose is computed. (Default: 'N')  
_**lwork**_: This is an ununsed parameter. (Default: 0)  
_**overwrite\_a**_: It accepts an integer parameter, if set to 0, then 'a' will remain unchanged. Otherwise, 'a' would be overwritten 
with QR or LQ factor. (Default: 0)  
_**overwrite\_b**_: It accepts an integer parameter, if set to 0, then 'b' will remain unchanged. Otherwise, 'b' would be overwritten 
with the solution matrix. (Default: 0)  
_**dtype**_: It specifies the datatype to be used for setting the precision level (single for float32 / double for float64) for the values returned by this method. (Default: np.float64)  
**Currently, it supports float (float32) or double (float64) datatypes.**  

__Purpose__  
This method solves overdetermined or underdetermined real linear systems involving a left hand side matrix 'a' or its transpose, using a QR or LQ factorization of 'a'. It is assumed that the matrix 'a' has full rank. It returns the LQ or QR factor, solution matrix for the system of linear equations with single (float32) or double (float64) precision depending on the 'dtype' parameter provided by user.  

The parameter: "lwork" is simply kept in to to make the interface uniform with other modules in frovedis. They are not used anywhere within the frovedis implementation.  

**This method is present only in frovedis**.  

This method internally uses Scalapack.gels() **(present in frovedis.matrix module)**.  

For example,  

    import numpy as np
    mat1 = np.array([[1.,0.,2.],[-1.,5.,0.],[0.,3.,-9.]])
    mat2 = np.array([[-9.,3.,-1.],[0.,3.,1.],[2.,2.,0.]])
    
    # gels() demo
    from frovedis.linalg.scalapack import gels
    rf = gels(mat1,mat2)
    print(rf)
    
Output  

    (array([[-1.41421356,  3.53553391, -1.41421356],
       [-0.41421356, -4.63680925,  4.74464202],
       [ 0.        ,  0.36709178, -7.77742709]]), array([[-7.54901961,  2.68627451, -1.        ],
       [-1.50980392,  1.1372549 ,  0.        ],
       [-0.7254902 ,  0.15686275,  0.        ]]), 0)


For example,  
    
    # gels() demo and using transpose of a
    from frovedis.linalg.scalapack import gels
    rf = gels(mat1,mat2,trans='T')
    print(rf)
    
Output  

    (array([[-1.41421356,  3.53553391, -1.41421356],
       [-0.41421356, -4.63680925,  4.74464202],
       [ 0.        ,  0.36709178, -7.77742709]]), array([[-7.82352941,  3.29411765, -0.70588235],
       [ 1.17647059,  0.29411765,  0.29411765],
       [-1.96078431,  0.50980392, -0.15686275]]), 0)

For example,  
    
    # gels() demo and overwriting of a with QR or LQ factor
    print('original matrix mat1: ')
    print(mat1)
    from frovedis.linalg.scalapack import gels
    rf = gels(mat1,mat2,overwrite_a=1)
    print('overwritten matrix with LQ or QF factor: ')
    print(mat1)
    
Output  

    original matrix mat1:  
    [[ 1.  0.  2.]
     [-1.  5.  0.]
     [ 0.  3. -9.]]
    
    overwritten matrix with LQ or QF factor: 
     [[-1.41421356  3.53553391 -1.41421356]
     [-0.41421356 -4.63680925  4.74464202]
     [ 0.          0.36709178 -7.77742709]]

**Here, if the input 'mat1' is double (float64) type and overwite is enabled, then LQ or QR factor 
would also be double (float64) type.**  

**Same applies for other types (int, float (float32)) when input is a numpy matrix/array and overwrite is enabled.**  

For example,  
    
    # gels() demo and overwriting of b with solution matrix
    print('original matrix mat2: ')
    print(mat2)
    from frovedis.linalg.scalapack import gels
    rf = gels(mat1,mat2,overwrite_b=1)
    print('overwritten matrix with solution matrix: ')
    print(mat2)
    
Output  

    original matrix mat2: 
    [[-9.  3. -1.]
     [ 0.  3.  1.]
     [ 2.  2.  0.]]
    
    overwritten matrix with solution matrix: 
    [[-7.54901961  2.68627451 -1.        ]
     [-1.50980392  1.1372549   0.        ]
     [-0.7254902   0.15686275  0.        ]]

**Here, if the input 'mat2' is double (float64) type and overwite is enabled, then solution matrix 
would also be double (float64) type.**  

**Same applies for other types (int, float (float32)) when input is a numpy matrix/array and overwrite is enabled.**  

For example,  
        
    # gels() demo and specifying the dtypes of output matrices LQ or QR factor and solution matrix
    from frovedis.linalg.scalapack import gels
    rf = gels(mat1,mat2,dtype=np.float32)
    print(rf)
    
Output  

    (array([[-1.4142135 ,  3.535534  , -1.4142137 ],
           [-0.41421354, -4.6368093 ,  4.744642  ],
           [ 0.        ,  0.36709177, -7.777427  ]], dtype=float32), 
           array([[-7.5490184 ,  2.686274  , -1.0000001 ],
           [-1.5098035 ,  1.1372546 ,  0.        ],
           [-0.7254901 ,  0.15686269,  0.        ]], dtype=float32), 0)

For example,  
    
    from frovedis.matrix.dense import FrovedisBlockcyclicMatrix
    mat1 = np.array([[1.,0.,2.],[-1.,5.,0.],[0.,3.,-9.]])
    mat2 = np.array([[-9.,3.,-1.],[0.,3.,1.],[2.,2.,0.]])
    bcm1 = FrovedisBlockcyclicMatrix(mat1)
    bcm2 = FrovedisBlockcyclicMatrix(mat2)

    # gels() demo and a and b as an instance of FrovedisBlockcyclicMatrix
    from frovedis.linalg.scalapack import gels
    rf = gels(bcm1,bcm2)
    
    # Unpacking the tuple
    rf[0].debug_print()
    rf[1].debug_print()
    print(rf[2])
    
Output  

    0
    matrix:
    num_row = 3, num_col = 3
    node 0
    node = 0, local_num_row = 3, local_num_col = 3, type = 2, descriptor = 1 1 3 3 3 3 0 0 3 3 3
    val = -1.41421 -0.414214 0 3.53553 -4.63681 0.367092 -1.41421 4.74464 -7.77743
    matrix:
    num_row = 3, num_col = 3
    node 0
    node = 0, local_num_row = 3, local_num_col = 3, type = 2, descriptor = 1 1 3 3 3 3 0 0 3 3 3
    val = -7.54902 -1.5098 -0.72549 2.68627 1.13725 0.156863 -1 0 0

**Note:- 'dtype' for the wrapper function and FrovedisBlockcyclicMatrix instance must be same during computation. Otherwise, it 
will raise an excpetion.**  

__Return Value__   
1. **If 'a' and 'b' are python inputs such as numpy matrices and dtype = np.float64**:  
     - It returns a tuple **(lqr, x, stat)** where,  
       - **lqr**: It is a numpy matrix having double (float64) type values (by default) and containing the QR or LQ factor of input matrix 'a'. In case 'overwrite_a' is enabled, then dtype for the matrix will depend on input 'a' dtype.  
       - **x**: It is also a numpy matrix having double (float64) type values (by default) and containing the solution matrix. In case 'overwrite_b' is enabled, then dtype for the matrix will depend on input 'b' dtype.  
       - **stat**: It returns an integer containing status (info) of native scalapack dgetrs.  
2. **If 'a' and 'b' are python inputs such as numpy matrices and dtype = np.float32**:  
     - It returns a tuple **(lqr, x, stat)** where,  
       - **lqr**: It is a numpy matrix having float (float32) type values (by default) and containing the QR or LQ factor of input matrix 'a'. In case 'overwrite_a' is enabled, then dtype for the matrix will depend on input 'a' dtype.  
       - **x**: It is also a numpy matrix having float (float32) type values (by default) and containing the solution matrix. In case 'overwrite_b' is enabled, then dtype for the matrix will depend on input 'b' dtype.  
       - **stat**: It returns an integer containing status (info) of native scalapack dgetrs.  
3. **If 'a' and 'b' are instances of FrovedisBlockcyclicMatrix:**  
     - It returns a tuple **(lqr, x, stat)** where,  
       - **lqr**: It returns instance of FrovedisBlockcyclicMatrix containing the QR or LQ factor of input matrix 'a'.  
       - **x**: It returns an instance of FrovedisBlockcyclicMatrix containing the solution matrix.  
       - **stat**: It returns an integer containing status (info) of native scalapack dgetrs.  

### 3. sgels(a, b, trans = 'N', lwork = 0, overwrite_a = 0, overwrite_b = 0)  

__Parameters__  
_**a**_: It accepts a python array-like input or left hand side numpy matrix of the linear equation having int, float (float32) or double (float64) type values. It also accepts FrovedisBlockcyclicMatrix instance having float (float32) type values.  
_**b**_: It accepts a python array-like input or right hand side numpy matrix of the linear equation having int, float (float32) or double (float64) type values. It also accepts FrovedisBlockcyclicMatrix instance having float (float32) type values. It should have number of rows >= max(M,N) and at least 1 column.  
_**trans**_: It accepts a string object parameter like 'N' or 'T' which specifies if transpose of 'a' is needed to be computed before solving linear equation. If set to 'T', then transpose is computed. (Default: 'N')  
_**lwork**_: This is an ununsed parameter. (Default: 0)  
_**overwrite\_a**_: It accepts an integer parameter, if set to 0, then 'a' will remain unchanged. Otherwise, 'a' would be overwritten 
with QR or LQ factor. (Default: 0)  
_**overwrite\_b**_: It accepts an integer parameter, if set to 0, then 'b' will remain unchanged. Otherwise, 'b' would be overwritten 
with the solution matrix. (Default: 0)  

__Purpose__  
This method solves overdetermined or underdetermined linear systems involving a left hand side matrix 'a' or its transpose, using a QR or LQ factorization of 'a'. It is assumed that the matrix 'a' has full rank. It returns the LQ or QR factor, solution matrix for the system of linear equations with single (float32) precision.  

The parameter: "lwork" is simply kept in to to make the interface uniform to the scipy.linalg.lapack.sgels() module. They 
are not used anywhere within the frovedis implementation.  

This method internally uses Scalapack.gels() **(present in frovedis.matrix module)**.  

For example,  

    import numpy as np
    mat1 = np.array([[1.,0.,2.],[-1.,5.,0.],[0.,3.,-9.]])
    mat2 = np.array([[-9.,3.,-1.],[0.,3.,1.],[2.,2.,0.]])
    
    # sgels() demo
    from frovedis.linalg.scalapack import sgels
    rf = sgels(mat1,mat2)
    print(rf)
    
Output  

    (array([[-1.4142135 ,  3.535534  , -1.4142137 ],
       [-0.41421354, -4.6368093 ,  4.744642  ],
       [ 0.        ,  0.36709177, -7.777427  ]], dtype=float32), 
       array([[-7.5490184 ,  2.686274  , -1.0000001 ],
       [-1.5098035 ,  1.1372546 ,  0.        ],
       [-0.7254901 ,  0.15686269,  0.        ]], dtype=float32), 0)

For example,  
    
    # sgels() demo and using transpose of a
    from frovedis.linalg.scalapack import sgels
    rf = sgels(mat1,mat2,trans='T')
    print(rf)
    
Output  

    (array([[-1.4142135 ,  3.535534  , -1.4142137 ],
       [-0.41421354, -4.6368093 ,  4.744642  ],
       [ 0.        ,  0.36709177, -7.777427  ]], dtype=float32), 
       array([[-7.8235283 ,  3.2941177 , -0.7058824 ],
       [ 1.1764708 ,  0.29411745,  0.29411763],
       [-1.9607842 ,  0.509804  , -0.15686275]], dtype=float32), 0)

For example,  
    
    # sgels() demo and overwriting of a with QR or LQ factor
    print('original matrix mat1: ', mat1)
    from frovedis.linalg.scalapack import sgels
    rf = sgels(mat1,mat2,overwrite_a=1)
    print('overwritten matrix with LQ or QF factor: ', mat1)
    
Output  

    original matrix mat1:
    [[ 1.  0.  2.]
     [-1.  5.  0.]
     [ 0.  3. -9.]]
    overwritten matrix with LQ or QF factor:
    [[-1.41421354  3.53553391 -1.41421366]
     [-0.41421354 -4.63680935  4.74464178]
     [ 0.          0.36709177 -7.7774272 ]]

**Here, if the input 'mat1' is double (float64) type and overwite is enabled, then LQ or QR factor 
would also be double (float64) type.**  

**Same applies for other types (int, float (float32)) when input is a numpy matrix/array and overwrite is enabled.**  

For example,  
    
    # sgels() demo and overwriting of b with solution matrix
    print('original matrix mat2: ')
    print(mat2)
    from frovedis.linalg.scalapack import sgels
    rf = sgels(mat1,mat2,overwrite_b=1)
    print('overwritten matrix with solution matrix: ')
    print(mat2)
    
Output  

    original matrix mat2: 
    [[-9.  3. -1.]
     [ 0.  3.  1.]
     [ 2.  2.  0.]]
     
    overwritten matrix with solution matrix: 
    [[-7.54901838  2.68627405 -1.00000012]
     [-1.50980353  1.1372546   0.        ]
     [-0.72549009  0.15686269  0.        ]]

**Here, if the input 'mat2' is double (float64) type and overwite is enabled, then solution matrix 
would also be double (float64) type.**  

**Same applies for other types (int, float (float32)) when input is a numpy matrix/array and overwrite is enabled.**  

For example,  
    
    from frovedis.matrix.dense import FrovedisBlockcyclicMatrix
    mat1 = np.array([[1.,0.,2.],[-1.,5.,0.],[0.,3.,-9.]])
    mat2 = np.array([[-9.,3.,-1.],[0.,3.,1.],[2.,2.,0.]])
    bcm1 = FrovedisBlockcyclicMatrix(mat1)
    bcm2 = FrovedisBlockcyclicMatrix(mat2)

    # sgels() demo and a and b as an instance of FrovedisBlockcyclicMatrix
    from frovedis.linalg.scalapack import sgels
    rf = sgels(bcm1,bcm2)
    
    # Unpacking the tuple
    rf[0].debug_print()
    rf[1].debug_print()
    print(rf[2])
    
Output  

    0
    matrix:
    num_row = 3, num_col = 3
    node 0
    node = 0, local_num_row = 3, local_num_col = 3, type = 2, descriptor = 1 1 3 3 3 3 0 0 3 3 3
    val = -1.41421 -0.414214 0 3.53553 -4.63681 0.367092 -1.41421 4.74464 -7.77743
    matrix:
    num_row = 3, num_col = 3
    node 0
    node = 0, local_num_row = 3, local_num_col = 3, type = 2, descriptor = 1 1 3 3 3 3 0 0 3 3 3
    val = -7.54902 -1.5098 -0.72549 2.68627 1.13725 0.156863 -1 0 0

__Return Value__   
1. **If 'a' and 'b' are python inputs such as numpy matrices**:  
     - It returns a tuple **(lqr, x, stat)** where,  
       - **lqr**: It is a numpy matrix having float (float32) type values (by default) and containing the QR or LQ factor of input matrix 'a'. In case 'overwrite_a' is enabled, then dtype for the matrix will depend on input 'a' dtype.    
       - **x**: It is also a numpy matrix having float (float32) (by default) type values and containing the solution matrix. In case 'overwrite_b' is enabled then dtype for the matrix will depend on input 'b' dtype.  
       - **stat**: It returns an integer containing status (info) of native scalapack dgetrs.  
2. **If 'a' and 'b' are instances of FrovedisBlockcyclicMatrix:**  
     - It returns a tuple **(lqr, x, stat)** where,  
       - **lqr**: It returns an instance of FrovedisBlockcyclicMatrix containing the QR or LQ factor of input matrix 'a'.  
       - **x**: It returns an instance of FrovedisBlockcyclicMatrix containing the solution matrix.  
       - **stat**: It returns an integer containing status (info) of native scalapack dgetrs.  

### 4. dgesv(a, b, overwrite_a  = 0, overwrite_b = 0)  

__Parameters__  
_**a**_: It accepts a python array-like input or left hand side numpy matrix of the linear equation having int, float (float32) or double (float64) type values. It also accepts FrovedisBlockcyclicMatrix instance having double (float64) type values.  
_**b**_: It accepts a python array-like input or right hand side numpy matrix of the linear equation having int, float (float32) or double (float64) type values. It also accepts FrovedisBlockcyclicMatrix instance having double (float64) type values. It should have number of rows >= the number of rows in 'a' and at least 1 column in it.  
_**overwrite\_a**_: It accepts an integer parameter, if set to 0, then 'a' will remain unchanged. Otherwise, 'a' would be overwritten 
with LU factor. (Default: 0)  
_**overwrite\_b**_: It accepts an integer parameter, if set to 0, then 'b' will remain unchanged. Otherwise, 'b' would be overwritten 
with the solution matrix. (Default: 0)  

__Purpose__   
It solves a system of linear equations, AX = B with a left hand side square matrix, 'a' by computing it's LU 
factors internally. It returns the LU factor, solution matrix for the system of linear equations with double (float64) precision.  

This method internally uses Scalapack.gesv() **(present in frovedis.matrix module)**.  

For example,  

    import numpy as np
    mat1 = np.array([[1.,0.,2.],[-1.,5.,0.],[0.,3.,-9.]])
    mat2 = np.array([[-9.,3.,-1.],[0.,3.,1.],[2.,2.,0.]])
    
    # dgesv() demo
    from frovedis.linalg.scalapack import dgesv
    rf = dgesv(mat1,mat2)
    print(rf)
    
Output  

    (array([[  1. ,   0. ,   2. ],
       [ -1. ,   5. ,   2. ],
       [  0. ,   0.6, -10.2]]), <frovedis.matrix.results.GetrfResult object at 0x7f30f7e25208>, 
       array([[-7.54901961,  2.68627451, -1.        ],
       [-1.50980392,  1.1372549 ,  0.        ],
       [-0.7254902 ,  0.15686275,  0.        ]]), 0)

**This resultant tuple of dgesv() wrapper function contains GetrfResult instance in frovedis. Whereas in scipy, it contains an array.**  
**Both here are responsible to hold pivot array information.**  

For example,  
    
    # dgesv() demo and overwriting of a with LU factor
    print('original matrix mat1: ')
    print(mat1)
    from frovedis.linalg.scalapack import dgesv
    rf = dgesv(mat1,mat2,overwrite_a=1)
    print('overwritten matrix with LU factor: ')
    print(mat1)
    
Output  

    original matrix mat1:  
     [[ 1.  0.  2.]
     [-1.  5.  0.]
     [ 0.  3. -9.]]

    overwritten matrix with LU factor: 
    [[  1.    0.    2. ]
     [ -1.    5.    2. ]
     [  0.    0.6 -10.2]]    

**Here, if the input 'mat1' is double (float64) type and overwite is enabled, then LU factor 
would also be double (float64) type.**  

**Same applies for other types (int, float (float32)) when input is a numpy matrix/array and overwrite is enabled.**  

For example,  
    
    # dgesv() demo and overwriting of b with solution matrix
    print('original matrix mat2: ')
    print(mat2)
    from frovedis.linalg.scalapack import dgesv
    rf = dgesv(mat1,mat2,overwrite_b=1)
    print('overwritten matrix with solution matrix: ')
    print(mat2)
    
Output  

    original matrix mat2: 
     [[-9.  3. -1.]
     [ 0.  3.  1.]
     [ 2.  2.  0.]]
    
    overwritten matrix with solution matrix: 
    [[-7.54901961  2.68627451 -1.        ]
     [-1.50980392  1.1372549   0.        ]
     [-0.7254902   0.15686275  0.        ]]    

**Here, if the input 'mat2' is double (float64) type and overwite is enabled, then solution matrix 
would also be double (float64) type.**  

**Same applies for other types (int, float (float32)) when input is a numpy matrix/array and overwrite is enabled.**  

For example,  
    
    from frovedis.matrix.dense import FrovedisBlockcyclicMatrix
    mat1 = np.array([[1.,0.,2.],[-1.,5.,0.],[0.,3.,-9.]])
    mat2 = np.array([[-9.,3.,-1.],[0.,3.,1.],[2.,2.,0.]])
    bcm1 = FrovedisBlockcyclicMatrix(mat1)
    bcm2 = FrovedisBlockcyclicMatrix(mat2)

    # dgesv() demo and a and b as an instance of FrovedisBlockcyclicMatrix
    from frovedis.linalg.scalapack import dgesv
    rf = dgesv(bcm1,bcm2)
    
    # Unpacking the tuple
    rf[0].debug_print()
    print(rf[1])
    rf[2].debug_print()
    print(rf[3])
    
Output  

    <frovedis.matrix.results.GetrfResult object at 0x7f7ce5798cf8>
    matrix:
    num_row = 3, num_col = 3
    node 0
    node = 0, local_num_row = 3, local_num_col = 3, type = 2, descriptor = 1 1 3 3 3 3 0 0 3 3 3
    val = 1 -1 0 0 5 0.6 2 2 -10.2
    0
    matrix:
    num_row = 3, num_col = 3
    node 0
    node = 0, local_num_row = 3, local_num_col = 3, type = 2, descriptor = 1 1 3 3 3 3 0 0 3 3 3
    val = -7.54902 -1.5098 -0.72549 2.68627 1.13725 0.156863 -1 0 0

__Return Value__   
1. **If 'a' and 'b' are python inputs such as numpy matrices**:  
     - It returns a tuple **(lu, piv, x, rs_stat)** where,  
       - **lu**: It is a numpy matrix having double (float64) type values (by default) and containing the LU factor of input 'a'. In case 'overwrite_a' is enabled, then dtype for the matrix will depend on input 'a' dtype.  
       - **piv**: It returns an instance of GetrfResult containing server side pointer of pivot array.  
       - **x**: It is also a numpy matrix having double (float64) type values (by default) and containing the solution matrix. In case 'overwrite_b' is enabled, then dtype for the matrix will depend on input 'b' dtype.  
       - **rs_stat**: It returns an integer containing status (info) of native scalapack dgetrs.  
2. **If 'a' and 'b' are instances of FrovedisBlockcyclicMatrix:**  
     - It returns a tuple **(lu, piv, x, rs_stat)** where,  
       - **lu**: It returns an instance of FrovedisBlockcyclicMatrix containing the LU factor of input 'a'.  
       - **piv**: It returns an instance of GetrfResult containing server side pointer of pivot array.  
       - **x**: It returns an instance of FrovedisBlockcyclicMatrix containing the solution matrix.  
       - **rs_stat**: It returns an integer containing status (info) of native scalapack dgetrs.  

### 5. gesv(a, b, overwrite_a  = 0, overwrite_b = 0, dtype = np.float64)  

__Parameters__  
_**a**_: It accepts a python array-like input or left hand side numpy matrix of the linear equation having int, float (float32) or double (float64) type values. It also accepts FrovedisBlockcyclicMatrix instance having float (float32) or double (float64) type values.  
_**b**_: It accepts a python array-like input or right hand side numpy matrix of the linear equation having int, float (float32) or double (float64) type values. It also accepts FrovedisBlockcyclicMatrix instance having float (float32) or double (float64) type values. It should have number of rows >= the number of rows in 'a' and at least 1 column in it.  
_**overwrite\_a**_: It accepts an integer parameter, if set to 0, then 'a' will remain unchanged. Otherwise, 'a' would be overwritten 
with LU factor. (Default: 0)  
_**overwrite\_b**_: It accepts an integer parameter, if set to 0, then 'b' will remain unchanged. Otherwise, 'b' would be overwritten 
with the solution matrix. (Default: 0)  
_**dtype**_: It specifies the datatype to be used for setting the precision level (single for float32 / double for float64) for the values returned by this method. (Default: np.float64)  
**Currently, it supports float (float32) or double (float64) datatypes.**  

__Purpose__  
It solves a system of linear equations, AX = B with a left hand side square matrix, 'a' by computing it's LU 
factors internally. It returns the LU factor computed using getrf() and solution matrix computed using getrf() for the system of linear equations with float (float32) or double (float64) precision depending on the 'dtype' parameter provided by user.  

**This method is present only in frovedis**.  

This method internally uses Scalapack.gesv() **(present in frovedis.matrix module)**.  

For example,  

    import numpy as np
    mat1 = np.array([[1.,0.,2.],[-1.,5.,0.],[0.,3.,-9.]])
    mat2 = np.array([[-9.,3.,-1.],[0.,3.,1.],[2.,2.,0.]])
    
    # gesv() demo
    from frovedis.linalg.scalapack import gesv
    rf = gesv(mat1,mat2)
    print(rf)
    
Output  

    (array([[  1. ,   0. ,   2. ],
       [ -1. ,   5. ,   2. ],
       [  0. ,   0.6, -10.2]]), <frovedis.matrix.results.GetrfResult object at 0x7f30f7e25208>, 
       array([[-7.54901961,  2.68627451, -1.        ],
       [-1.50980392,  1.1372549 ,  0.        ],
       [-0.7254902 ,  0.15686275,  0.        ]]), 0)

For example,  
    
    # dgesv() demo and overwriting of a with LU factor
    print('original matrix mat1: ')
    print(mat1)
    from frovedis.linalg.scalapack import dgesv
    rf = dgesv(mat1,mat2,overwrite_a=1)
    print('overwritten matrix with LU factor: ')
    print(mat1)
    
Output  

    original matrix mat1:  
     [[ 1.  0.  2.]
     [-1.  5.  0.]
     [ 0.  3. -9.]]

    overwritten matrix with LU factor: 
    [[  1.    0.    2. ]
     [ -1.    5.    2. ]
     [  0.    0.6 -10.2]]    

**Here, if the input 'mat1' is double (float64) type and overwite is enabled, then LU factor 
would also be double (float64) type.**  

**Same applies for other types (int, float (float32)) when input is a numpy matrix/array and overwrite is enabled.**  

For example,  
    
    # dgesv() demo and overwriting of b with solution matrix
    print('original matrix mat2: ')
    print(mat2)
    from frovedis.linalg.scalapack import dgesv
    rf = dgesv(mat1,mat2,overwrite_b=1)
    print('overwritten matrix with solution matrix: ')
    print(mat2)
    
Output  

    original matrix mat2: 
     [[-9.  3. -1.]
     [ 0.  3.  1.]
     [ 2.  2.  0.]]
    
    overwritten matrix with solution matrix: 
    [[-7.54901961  2.68627451 -1.        ]
     [-1.50980392  1.1372549   0.        ]
     [-0.7254902   0.15686275  0.        ]]    

**Here, if the input 'mat2' is double (float64) type and overwite is enabled, then solution matrix 
would also be double (float64) type.**  

**Same applies for other types (int, float (float32)) when input is a numpy matrix/array and overwrite is enabled.**  

For example,  
        
    # gesv() demo and specifying the dtypes of output matrices LU factor and solution matrix
    from frovedis.linalg.scalapack import gesv
    rf = gesv(mat1,mat2,dtype=np.float32)
    print(rf)
    
Output  

    (array([[  1. ,   0. ,   2. ],
       [ -1. ,   5. ,   2. ],
       [  0. ,   0.6, -10.2]], dtype=float32), 
       <frovedis.matrix.results.GetrfResult object at 0x7f03d450ad30>, 
       array([[-7.54902   ,  2.6862745 , -1.        ],
       [-1.509804  ,  1.137255  ,  0.        ],
       [-0.7254902 ,  0.15686277,  0.        ]], dtype=float32), 0)

For example,  
    
    from frovedis.matrix.dense import FrovedisBlockcyclicMatrix
    mat1 = np.array([[1.,0.,2.],[-1.,5.,0.],[0.,3.,-9.]])
    mat2 = np.array([[-9.,3.,-1.],[0.,3.,1.],[2.,2.,0.]])
    bcm1 = FrovedisBlockcyclicMatrix(mat1)
    bcm2 = FrovedisBlockcyclicMatrix(mat2)

    # dgesv() demo and a and b as an instance of FrovedisBlockcyclicMatrix
    from frovedis.linalg.scalapack import dgesv
    rf = dgesv(bcm1,bcm2)
    
    # Unpacking the tuple
    rf[0].debug_print()
    print(rf[1])
    rf[2].debug_print()
    print(rf[3])
    
Output  

    <frovedis.matrix.results.GetrfResult object at 0x7f7ce5798cf8>
    matrix:
    num_row = 3, num_col = 3
    node 0
    node = 0, local_num_row = 3, local_num_col = 3, type = 2, descriptor = 1 1 3 3 3 3 0 0 3 3 3
    val = 1 -1 0 0 5 0.6 2 2 -10.2
    0
    matrix:
    num_row = 3, num_col = 3
    node 0
    node = 0, local_num_row = 3, local_num_col = 3, type = 2, descriptor = 1 1 3 3 3 3 0 0 3 3 3
    val = -7.54902 -1.5098 -0.72549 2.68627 1.13725 0.156863 -1 0 0

**Note:- 'dtype' for the wrapper function and FrovedisBlockcyclicMatrix instance must be same during computation. Otherwise, it 
will raise an excpetion.**  

__Return Value__  
1. **If 'a' and 'b' are python inputs such as numpy matrices and dtype = np.float32**:  
     - It returns a tuple **(lu, piv, x, rs_stat)** where,  
       - **lu**: It is a numpy matrix having float (float32) type values (by default) and containing the LU factor of input 'a'. In case 'overwrite_a' is enabled, then dtype for the matrix will depend on input 'a' dtype.  
       - **piv**: It returns an instance of GetrfResult containing server side pointer of pivot array.  
       - **x**: It is also a numpy matrix having float (float32) type values (by default) and containing the solution matrix. In case 'overwrite_b' is enabled, then dtype for the matrix will depend on input 'b' dtype.  
       - **rs_stat**: It returns an integer containing status (info) of native scalapack dgetrs.  
2. **If 'a' and 'b' are python inputs such as numpy matrices and dtype = np.float64**:  
     - It returns a tuple **(lu, piv, x, rs_stat)** where,  
       - **lu**: It is a numpy matrix having double (float64) type values (by default) and containing the LU factor of input 'a'. In case 'overwrite_a' is enabled, then dtype for the matrix will depend on input 'a' dtype.  
       - **piv**: It returns an instance of GetrfResult containing server side pointer of pivot array.  
       - **x**: It is also a numpy matrix having double (float64) type values (by default) and containing the solution matrix. In case 'overwrite_b' is enabled, then dtype for the matrix will depend on input 'b' dtype.  
       - **rs_stat**: It returns an integer containing status (info) of native scalapack dgetrs.  
3. **If 'a' and 'b' are instances of FrovedisBlockcyclicMatrix:**  
     - It returns a tuple **(lu, piv, x, rs_stat)** where,  
       - **lu**: It returns an instance of FrovedisBlockcyclicMatrix containing the LU factor of input 'a'.  
       - **piv**: It returns an instance of GetrfResult containing server side pointer of pivot array.  
       - **x**: It returns an instance of FrovedisBlockcyclicMatrix containing the solution matrix.  
       - **rs_stat**: It returns an integer containing status (info) of native scalapack dgetrs.  

### 6. sgesv(a, b, overwrite_a  = 0, overwrite_b = 0)  

__Parameters__   
_**a**_: It accepts a python array-like input or left hand side numpy matrix of the linear equation having int, float (float32) or double (float64) type values. It also accepts FrovedisBlockcyclicMatrix instance having float (float32) type values.  
_**b**_: It accepts a python array-like input or right hand side numpy matrix of the linear equation having int, float (float32) or double (float64) type values. It also accepts FrovedisBlockcyclicMatrix instance having float (float32) type values. It should have number of rows >= the number of rows in 'a' and at least 1 column in it.  
_**overwrite\_a**_: It accepts an integer parameter, if set to 0, then 'a' will remain unchanged. Otherwise, 'a' would be overwritten 
with LU factor. (Default: 0)  
_**overwrite\_b**_: It accepts an integer parameter, if set to 0, then 'b' will remain unchanged. Otherwise, 'b' would be overwritten 
with the solution matrix. (Default: 0)  

__Purpose__  
It solves a system of linear equations, AX = B with a left hand side square matrix, 'a' by computing it's LU 
factors internally. It returns the LU factor, solution matrix for the system of linear equations with single (float32) precision.  

This method internally uses Scalapack.gesv() **(present in frovedis.matrix module)**.  

For example,  

    import numpy as np
    mat1 = np.array([[1.,0.,2.],[-1.,5.,0.],[0.,3.,-9.]],dtype = np.float32)
    mat2 = np.array([[-9.,3.,-1.],[0.,3.,1.],[2.,2.,0.]],dtype = np.float32)
    
    # sgesv() demo
    from frovedis.linalg.scalapack import sgesv
    rf = sgesv(mat1,mat2)
    print(rf)
    
Output  

    (array([[  1. ,   0. ,   2. ],
       [ -1. ,   5. ,   2. ],
       [  0. ,   0.6, -10.2]], dtype=float32), 
       <frovedis.matrix.results.GetrfResult object at 0x7f49657f3cf8>, 
       array([[-7.54902   ,  2.6862745 , -1.        ],
       [-1.509804  ,  1.137255  ,  0.        ],
       [-0.7254902 ,  0.15686277,  0.        ]], dtype=float32), 0)

**This resultant tuple of sgesv() wrapper function contains GetrfResult instance in frovedis. Whereas in scipy, it contains an array.**  
**Both here are responsible to hold pivot array information.**  

For example,  
    
    # sgesv() demo and overwriting of a with LU factor
    print('original matrix mat1: ')
    print(mat1)
    from frovedis.linalg.scalapack import sgesv
    rf = sgesv(mat1,mat2,overwrite_a=1)
    print('overwritten matrix with LU factor: ')
    print(mat1)
    
Output  

    original matrix mat1:  
    [[ 1.  0.  2.]
     [-1.  5.  0.]
     [ 0.  3. -9.]]

    overwritten matrix with LU factor: 
    [[  1.    0.    2. ]
     [ -1.    5.    2. ]
     [  0.    0.6 -10.2]]

**Here, if the input 'mat1' is double (float64) type and overwite is enabled, then LU factor 
would also be double (float64) type.**  

**Same applies for other types (int, float (float32)) when input is a numpy matrix/array and overwrite is enabled.**  

For example,  
    
    # sgesv() demo and overwriting of b with solution matrix
    print('original matrix mat2: ')
    print(mat2)
    from frovedis.linalg.scalapack import sgesv
    rf = sgesv(mat1,mat2,overwrite_b=1)
    print('overwritten matrix with solution matrix: ')
    print(mat2)
    
Output  

    original matrix mat2: 
    [[-9.  3. -1.]
     [ 0.  3.  1.]
     [ 2.  2.  0.]]

    overwritten matrix with solution matrix: 
    [[-7.54901981  2.68627453 -1.        ]
     [-1.50980401  1.13725495  0.        ]
     [-0.72549021  0.15686277  0.        ]]

**Here, if the input 'mat2' is double (float64) type and overwite is enabled, then solution matrix 
would also be double (float64) type.**  

**Same applies for other types (int, float (float32)) when input is a numpy matrix/array and overwrite is enabled.**  

For example,  
    
    from frovedis.matrix.dense import FrovedisBlockcyclicMatrix
    mat1 = np.array([[1.,0.,2.],[-1.,5.,0.],[0.,3.,-9.]])
    mat2 = np.array([[-9.,3.,-1.],[0.,3.,1.],[2.,2.,0.]])
    bcm1 = FrovedisBlockcyclicMatrix(mat1,dtype=np.float32)
    bcm2 = FrovedisBlockcyclicMatrix(mat2,dtype=np.float32)

    # sgesv() demo and a and b as an instance of FrovedisBlockcyclicMatrix
    from frovedis.linalg.scalapack import sgesv
    rf = sgesv(bcm1,bcm2)
    
    # Unpacking the tuple
    rf[0].debug_print()
    print(rf[1])
    rf[2].debug_print()
    print(rf[3])
    
Output  

    <frovedis.matrix.results.GetrfResult object at 0x7f67ddeccd30>
    0
    matrix:
    num_row = 3, num_col = 3
    node 0
    node = 0, local_num_row = 3, local_num_col = 3, type = 2, descriptor = 1 1 3 3 3 3 0 0 3 3 3
    val = 1 -1 0 0 5 0.6 2 2 -10.2
    matrix:
    num_row = 3, num_col = 3
    node 0
    node = 0, local_num_row = 3, local_num_col = 3, type = 2, descriptor = 1 1 3 3 3 3 0 0 3 3 3
    val = -7.54902 -1.5098 -0.72549 2.68627 1.13725 0.156863 -1 0 0    

__Return Value__   
1. **If 'a' and 'b' are python inputs such as numpy matrices**:  
     - It returns a tuple **(lu, piv, x, rs_stat)** where,  
       - **lu**: It is a numpy matrix having float (float32) type values (by default) and containing the LU factor of input 'a'. In case 'overwrite_a' is enabled, then dtype for the matrix will depend on input 'a' dtype.  
       - **piv**: It returns an instance of GetrfResult containing server side pointer of pivot array.  
       - **x**: It is also a numpy matrix having float (float32) type values (by default) and containing the solution matrix. In case 'overwrite_b' is enabled, then dtype for the matrix will depend on input 'b' dtype.  
       - **rs_stat**: It returns an integer containing status (info) of native scalapack dgetrs.  
2. **If 'a' and 'b' are instances of FrovedisBlockcyclicMatrix:**  
     - It returns a tuple **(lu, piv, x, rs_stat)** where,  
       - **lu**: It returns an instance of FrovedisBlockcyclicMatrix containing the LU factor of input 'a'.  
       - **piv**: It returns an instance of GetrfResult containing server side pointer of pivot array.  
       - **x**: It returns an instance of FrovedisBlockcyclicMatrix containing the solution matrix.  
       - **rs_stat**: It returns an integer containing status (info) of native scalapack dgetrs.  

### 7. dgesvd(a, compute_uv = 1, full_matrices = 0, lwork = 0, overwrite_a = 0)  

__Parameters__  
_**a**_: It accepts a python array-like input or left hand side numpy matrix of the linear equation having int, float (float32) or double (float64) type values. It also accepts FrovedisBlockcyclicMatrix instance having double (float64) type values.  
_**compute\_uv**_: It accepts an integer parameter that specifies whether left singular matrix and right singular 
matrix will be computed. If it is set as '0', a numpy matrix of shape **(1,1)** and datatype same as 'a' will be assigned 
to the left and right singular matrix. (Default: 1)  
_**full\_matrices**_: It accepts an integer parameter. Currently, it can only be set as 0. Also, the left singular 
matrix and right singular matrix shapes are **(M, K)** and **(K, N)**, respectively, where **K = min(M, N)**, **M** is the 
number of rows and **N** is the number of columns of input matrix. (Default: 0)  
_**lwork**_: This is an ununsed parameter. (Default: 0)  
_**overwrite\_a**_: It accepts an integer parameter, if set to 0, then 'a' will remain unchanged. Otherwise 'a' would be 
consumed internally and its contents will be destroyed. (Default: 0)  

__Purpose__  
It computes the singular value decomposition (SVD) of matrix 'a' with double (float64) precision.  

The parameter: "lwork" is simply kept in to to make the interface uniform to the scipy.linalg.lapack.dgesvd() module. They 
are not used anywhere within the frovedis implementation.  

This method internally uses Scalapack.gesvd() **(present in frovedis.matrix module)**.  

For example,  

    import numpy as np
    mat1 = np.array([[1.,0.,2.],[-1.,5.,0.],[0.,3.,-9.]])
    
    # dgesvd() demo
    from frovedis.linalg.scalapack import dgesvd
    rf = dgesvd(mat1)
    print(rf)
    
Output  

    (array([[ 0.19046167, -0.12914878, -0.97316234],
       [-0.20606538, -0.97448299,  0.08899413],
       [-0.95982364,  0.18358509, -0.21221475]]), array([9.83830146, 4.80016509, 1.0799257 ]), 
       array([[ 0.04030442, -0.39740577,  0.91675744],
       [ 0.17610524, -0.9003148 , -0.39802035],
       [-0.98354588, -0.17748777, -0.03369857]]), 0)

For example,  
    
    # dgesvd() demo and compute_uv = 0
    from frovedis.linalg.scalapack import dgesvd
    rf = dgesvd(mat1,compute_uv=0)
    print(rf)
    
Output  

    (array([[0.]]), array([9.83830146, 4.80016509, 1.0799257 ]), array([[0.]]), 0)

For example,  
    
    # dgesvd() demo and overwriting of a
    print('original matrix mat1: ')
    print(mat1)
    from frovedis.linalg.scalapack import dgesvd
    rf = dgesvd(mat1,overwrite_a=1)
    print('overwritten matrix: ')
    print(mat1)
    
Output  

    original matrix mat1:  
    [[ 1.  0.  2.]
     [-1.  5.  0.]
     [ 0.  3. -9.]]

    overwritten matrix: 
    [[-1.41421356 -3.80788655 -0.1925824 ]
     [-0.41421356  6.71975985  5.52667534]
     [ 0.          0.64659915 -5.36662718]]

**Here, if the input 'mat1' is double (float64) type and overwite is enabled, it's overwitten with double (float64) type values.**  

**Same applies for other types (int, float (float32)) when input is a numpy matrix/array and overwrite is enabled.**  

For example,  
    
    from frovedis.matrix.dense import FrovedisBlockcyclicMatrix
    mat1 = np.array([[1.,0.,2.],[-1.,5.,0.],[0.,3.,-9.]])
    bcm1 = FrovedisBlockcyclicMatrix(mat1)

    # dgesvd() demo and a is an instance of FrovedisBlockcyclicMatrix
    from frovedis.linalg.scalapack import dgesvd
    rf = dgesvd(bcm1)
    
    # Unpacking the tuple
    rf[0].debug_print()
    rf[1].debug_print()
    rf[2].debug_print()
    
Output  

    vector:
    matrix:
    num_row = 3, num_col = 3
    node 0
    node = 0, local_num_row = 3, local_num_col = 3, type = 2, descriptor = 1 1 3 3 3 3 0 0 3 3 3
    val = 0.190462 -0.206065 -0.959824 -0.129149 -0.974483 0.183585 -0.973162 0.0889941 -0.212215
    [9.83830146 4.80016509 1.0799257 ]
    matrix:
    num_row = 3, num_col = 3
    node 0
    node = 0, local_num_row = 3, local_num_col = 3, type = 2, descriptor = 1 1 3 3 3 3 0 0 3 3 3
    val = 0.0403044 -0.397406 0.916757 0.176105 -0.900315 -0.39802 -0.983546 -0.177488 -0.0336986

__Return Value__  
1. **If 'a' is a python input such as numpy matrix**:  
     - It returns a tuple **(u, s, vt, stat)** where,  
       - **u**: It is a numpy matrix having double (float64) type values and containing the left singular matrix.  
       - **s**: It is a numpy matrix having double (float64) type values and containing the singular matrix.  
       - **vt**: It is a numpy matrix having double (float64) type values and containing the right singular matrix.  
       - **stat**: It returns an integer containing status (info) of native scalapack dgetrs.  
2. **If 'a' is an instance of FrovedisBlockcyclicMatrix:**  
     - It returns a tuple **(u, s, vt, stat)** where,  
       - **u**: It returns an instance of FrovedisBlockcyclicMatrix containing the left singular matrix.  
       - **s**: It is a numpy matrix having double (float64) type values and containing the singular matrix.  
       - **vt**: It returns an instance of FrovedisBlockcyclicMatrix containing the right singular matrix.  
       - **stat**: It returns an integer containing status (info) of native scalapack dgetrs.  

### 8. gesvd(a, compute_uv = 1, full_matrices = 0, lwork = 0, overwrite_a = 0, dtype = np.float64)  

__Parameters__  
_**a**_: It accepts a python array-like input or left hand side numpy matrix of the linear equation having int, float (float32) or double (float64) type values. It also accepts FrovedisBlockcyclicMatrix instance having float (float32) or double (float64) type values.  
_**compute\_uv**_: It accepts an integer parameter that specifies whether left singular matrix and right singular 
matrix will be computed. If it is set as '0', a numpy matrix of shape **(1,1)** and datatype same as 'a' will be assigned 
to the left and right singular matrix. (Default: 1)  
_**full\_matrices**_: It accepts an integer parameter. Currently, it can only be set as 0. Also, the left singular 
matrix and right singular matrix shapes are **(M, K)** and **(K, N)**, respectively, where **K = min(M, N)**, **M** is the 
number of rows and **N** is the number of columns of input matrix. (Default: 0)  
_**lwork**_: This is an ununsed parameter. (Default: 0)  
_**overwrite\_a**_: It accepts an integer parameter, if set to 0, then 'a' will remain unchanged. Otherwise 'a' would be 
consumed internally and its contents will be destroyed. (Default: 0)  
_**dtype**_: It specifies the datatype to be used for setting the precision level (single for float32 / double for float64) for the values returned by this method. (Default: np.float64)  
**Currently, it supports float (float32) or double (float64) datatypes.**  

__Purpose__  
It computes the singular value decomposition (SVD) of matrix 'a' with single (float32) or double (float64) precision depending on the 'dtype' parameter provided by user.  

The parameter: "lwork" is simply kept in to to make the interface uniform to the scipy.linalg.lapack.dgesvd() module. They 
are not used anywhere within the frovedis implementation.  

**This method is present only in frovedis**.  

This method internally uses Scalapack.gesvd() **(present in frovedis.matrix module)**.  

For example,  

    import numpy as np
    mat1 = np.array([[1.,0.,2.],[-1.,5.,0.],[0.,3.,-9.]])
    
    # gesvd() demo
    from frovedis.linalg.scalapack import gesvd
    rf = gesvd(mat1)
    print(rf)
    
Output  

    (array([[ 0.19046167, -0.12914878, -0.97316234],
       [-0.20606538, -0.97448299,  0.08899413],
       [-0.95982364,  0.18358509, -0.21221475]]), 
       array([9.83830146, 4.80016509, 1.0799257 ]), 
       array([[ 0.04030442, -0.39740577,  0.91675744],
       [ 0.17610524, -0.9003148 , -0.39802035],
       [-0.98354588, -0.17748777, -0.03369857]]), 0)

For example,  
    
    # gesvd() demo and compute_uv = 0
    from frovedis.linalg.scalapack import gesvd
    rf = gesvd(mat1,compute_uv=0)
    print(rf)
    
Output  

    (array([[0.]]), array([9.83830146, 4.80016509, 1.0799257 ]), array([[0.]]), 0)

For example,  
    
    # gesvd() demo and overwriting of a
    print('original matrix mat1: ')
    print(mat1)
    from frovedis.linalg.scalapack import gesvd
    rf = gesvd(mat1,overwrite_a=1)
    print('overwritten matrix: ')
    print(mat1)
    
Output  

    original matrix mat1:  
    [[ 1.  0.  2.]
     [-1.  5.  0.]
     [ 0.  3. -9.]]

    overwritten matrix: 
    [[-1.41421356 -3.80788655 -0.1925824 ]
     [-0.41421356  6.71975985  5.52667534]
     [ 0.          0.64659915 -5.36662718]]

**Here, if the input 'mat1' is double (float64) type and overwite is enabled, it's overwitten with double (float64) type values.**  

**Same applies for other types (int, float (float32)) when input is a numpy matrix/array and overwrite is enabled.**  

For example,  
        
    # gesvd() demo and specifying the dtypes of output matrices 
    from frovedis.linalg.scalapack import gesvd
    rf = gesvd(mat1,dtype=np.float32)
    print(rf)
    
Output  

    (array([[ 0.19046175, -0.12914878, -0.9731621 ],
           [-0.20606534, -0.97448295,  0.08899409],
           [-0.9598237 ,  0.1835851 , -0.2122148 ]], dtype=float32), 
           array([9.838303 , 4.800165 , 1.0799255], dtype=float32), 
           array([[ 0.04030442, -0.3974058 ,  0.91675735],
           [ 0.17610519, -0.90031475, -0.39802024],
           [-0.98354584, -0.17748773, -0.03369856]], dtype=float32), 0)

For example,  
    
    from frovedis.matrix.dense import FrovedisBlockcyclicMatrix
    mat1 = np.array([[1.,0.,2.],[-1.,5.,0.],[0.,3.,-9.]])
    bcm1 = FrovedisBlockcyclicMatrix(mat1)

    # gesvd() demo and a is an instance of FrovedisBlockcyclicMatrix
    from frovedis.linalg.scalapack import gesvd
    rf = gesvd(bcm1)
    
    # Unpacking the tuple
    rf[0].debug_print()
    rf[1].debug_print()
    rf[2].debug_print()
    
Output  

    vector:
    matrix:
    num_row = 3, num_col = 3
    node 0
    node = 0, local_num_row = 3, local_num_col = 3, type = 2, descriptor = 1 1 3 3 3 3 0 0 3 3 3
    val = 0.190462 -0.206065 -0.959824 -0.129149 -0.974483 0.183585 -0.973162 0.0889941 -0.212215
    [9.83830146 4.80016509 1.0799257 ]
    matrix:
    num_row = 3, num_col = 3
    node 0
    node = 0, local_num_row = 3, local_num_col = 3, type = 2, descriptor = 1 1 3 3 3 3 0 0 3 3 3
    val = 0.0403044 -0.397406 0.916757 0.176105 -0.900315 -0.39802 -0.983546 -0.177488 -0.0336986

**Note:- 'dtype' for the wrapper function and FrovedisBlockcyclicMatrix instance must be same during computation. Otherwise, it 
will raise an excpetion.**  

__Return Value__  
1. **If 'a' is a python input such as numpy matrix and dtype = np.float32**:  
     - It returns a tuple **(u, s, vt, stat)** where,  
       - **u**: It is a numpy matrix having float (float32) type values and containing the left singular matrix.  
       - **s**: It is a numpy matrix having float (float32) type values and containing the singular matrix.  
       - **vt**: It is a numpy matrix having float (float32) type values and containing the right singular matrix.  
       - **stat**: It returns an integer containing status (info) of native scalapack dgetrs.  
2. **If 'a' is a python input such as numpy matrix and dtype = np.float64**:  
     - It returns a tuple **(u, s, vt, stat)** where,  
       - **u**: It is a numpy matrix having double (float64) type values and containing the left singular matrix.  
       - **s**: It is a numpy matrix having double (float64) type values and containing the singular matrix.  
       - **vt**: It is a numpy matrix having double (float64) type values and containing the right singular matrix.  
       - **stat**: It returns an integer containing status (info) of native scalapack dgetrs.  
3. **If 'a' is an instance of FrovedisBlockcyclicMatrix:**  
     - It returns a tuple **(u, s, vt, stat)** where,  
       - **u**: It returns an instance of FrovedisBlockcyclicMatrix containing the left singular matrix.  
       - **s**: It is a numpy matrix having double (float64) type values and containing the singular matrix.  
       - **vt**: It returns an instance of FrovedisBlockcyclicMatrix containing the right singular matrix.  
       - **stat**: It returns an integer containing status (info) of native scalapack dgetrs.  

### 9. sgesvd(a, compute_uv = 1, full_matrices = 0, lwork = 0, overwrite_a = 0)  

__Parameters__  
_**a**_: It accepts a python array-like input or left hand side numpy matrix of the linear equation having int, float (float32) or double (float64) type values. It also accepts FrovedisBlockcyclicMatrix instance having float (float32) type values.  
_**compute\_uv**_: It accepts an integer parameter that specifies whether left singular matrix and right singular 
matrix will be computed. If it is set as '0', a numpy matrix of shape **(1,1)** and datatype same as 'a' will be assigned 
to the left and right singular matrix. (Default: 1)  
_**full\_matrices**_: It accepts an integer parameter. Currently, it can only be set as 0. Also, the left singular 
matrix and right singular matrix shapes are **(M, K)** and **(K, N)**, respectively, where **K = min(M, N)**, **M** is the 
number of rows and **N** is the number of columns of input matrix. (Default: 0)  
_**lwork**_: This is an ununsed parameter. (Default: 0)  
_**overwrite\_a**_: It accepts an integer parameter, if set to 0, then 'a' will remain unchanged. Otherwise 'a' would be 
consumed internally and its contents will be destroyed. (Default: 0)  

__Purpose__  
It computes the singular value decomposition (SVD) of matrix 'a' with single (float32) precision.  

The parameter: "lwork" is simply kept in to to make the interface uniform to the scipy.linalg.lapack.sgesvd() module. They 
are not used anywhere within the frovedis implementation.  

This method internally uses Scalapack.gesvd() **(present in frovedis.matrix module)**.  

For example,  

    import numpy as np
    mat1 = np.array([[1.,0.,2.],[-1.,5.,0.],[0.,3.,-9.]])
    
    # sgesvd() demo
    from frovedis.linalg.scalapack import sgesvd
    rf = sgesvd(mat1)
    print(rf)
    
Output  

    (array([[ 0.19046175, -0.12914878, -0.9731621 ],
       [-0.20606534, -0.97448295,  0.08899409],
       [-0.9598237 ,  0.1835851 , -0.2122148 ]], dtype=float32), 
       array([9.838303 , 4.800165 , 1.0799255], dtype=float32), 
       array([[ 0.04030442, -0.3974058 ,  0.91675735],
       [ 0.17610519, -0.90031475, -0.39802024],
       [-0.98354584, -0.17748773, -0.03369856]], dtype=float32), 0)

For example,  
    
    # sgesvd() demo and compute_uv = 0
    from frovedis.linalg.scalapack import sgesvd
    rf = sgesvd(mat1,compute_uv=0)
    print(rf)
    
Output  

    (array([[0.]], dtype=float32), array([9.838302 , 4.800165 , 1.0799258], dtype=float32), 
    array([[0.]], dtype=float32), 0)

For example,  
    
    # sgesvd() demo and overwriting of a
    print('original matrix mat1: ')
    print(mat1)
    from frovedis.linalg.scalapack import sgesvd
    rf = sgesvd(mat1,overwrite_a=1)
    print('overwritten matrix: ')
    print(mat1)
    
Output  

    original matrix mat1:  
    [[ 1.  0.  2.]
     [-1.  5.  0.]
     [ 0.  3. -9.]]

    overwritten matrix: 
    [[-1.41421354 -3.80788684 -0.19258241]
     [-0.41421354  6.71975994  5.52667427]
     [ 0.          0.64659911 -5.36662769]]

**Here, if the input 'mat1' is double (float64) type and overwite is enabled, it's overwitten with double (float64) type values.**  

**Same applies for other types (int, float (float32)) when input is a numpy matrix/array and overwrite is enabled.**  

For example,  
    
    from frovedis.matrix.dense import FrovedisBlockcyclicMatrix
    mat1 = np.array([[1.,0.,2.],[-1.,5.,0.],[0.,3.,-9.]])
    bcm1 = FrovedisBlockcyclicMatrix(mat1,dtype = np.float32)

    # sgesvd() demo and a is an instance of FrovedisBlockcyclicMatrix
    from frovedis.linalg.scalapack import sgesvd
    rf = sgesvd(bcm1)
    
    # Unpacking the tuple
    rf[0].debug_print()
    rf[1].debug_print()
    rf[2].debug_print()
    
Output  

    vector:
    matrix:
    num_row = 3, num_col = 3
    node 0
    node = 0, local_num_row = 3, local_num_col = 3, type = 2, descriptor = 1 1 3 3 3 3 0 0 3 3 3
    val = 0.190462 -0.206065 -0.959824 -0.129149 -0.974483 0.183585 -0.973162 0.0889941 -0.212215
    [9.838303  4.800165  1.0799255]
    matrix:
    num_row = 3, num_col = 3
    node 0
    node = 0, local_num_row = 3, local_num_col = 3, type = 2, descriptor = 1 1 3 3 3 3 0 0 3 3 3
    val = 0.0403044 -0.397406 0.916757 0.176105 -0.900315 -0.39802 -0.983546 -0.177488 -0.0336986

__Return Value__  
1. **If 'a' is a python input such as numpy matrices**:  
     - It returns a tuple **(u, s, vt, stat)** where,  
       - **u**: It is a numpy matrix having float (float32) type values and containing the left singular matrix.  
       - **s**: It is a numpy matrix having float (float32) type values and containing the singular matrix.  
       - **vt**: It is a numpy matrix having float (float32) type values and containing the right singular matrix.  
       - **stat**: It returns an integer containing status (info) of native scalapack dgetrs.  
2. **If 'a' is an instance of FrovedisBlockcyclicMatrix:**  
     - It returns a tuple **(u, s, vt, stat)** where,  
       - **u**: It returns an instance of FrovedisBlockcyclicMatrix containing the left singular matrix.  
       - **s**: It is a numpy matrix having float (float32) type values and containing the singular matrix.  
       - **vt**: It returns an instance of FrovedisBlockcyclicMatrix containing the right singular matrix.  
       - **stat**: It returns an integer containing status (info) of native scalapack dgetrs.  

### 10. dgetrf(a, overwrite_a = 0)  

__Parameters__  
_**a**_: It accepts a python array-like input or left hand side numpy matrix of the linear equation having int, float (float32) or double (float64) type values. It also accepts FrovedisBlockcyclicMatrix instance having float (float32) or double (float64) type values.  
_**overwrite\_a**_: It accepts an integer parameter, if set to 0, then 'a' will remain unchanged. Otherwise 'a' will be 
overwritten with 'L' and 'U' factors. (Default: 0)  

__Purpose__  
It computes LU factorization of matrix 'a' with double (float64) precision.  

It computes an LU factorization of matrix 'a', using partial pivoting with row interchanges.  

This method internally uses Scalapack.getrf() **(present in frovedis.matrix module)**.  

For example,  

    import numpy as np
    mat1 = np.array([[1.,0.,2.],[-1.,5.,0.],[0.,3.,-9.]])
    
    # dgetrf() demo
    from frovedis.linalg.scalapack import dgetrf
    rf = dgetrf(mat1)
    print(rf)
    
Output  

    (array([[  1. ,   0. ,   2. ],
           [ -1. ,   5. ,   2. ],
           [  0. ,   0.6, -10.2]]), 
           <frovedis.matrix.results.GetrfResult object at 0x7f80614aacf8>, 0)

**This resultant tuple of dgetrf() wrapper function contains GetrfResult instance in frovedis. Whereas in scipy, it contains an array.**  
**Both here are responsible to hold pivot array information.**  

For example,  
    
    # dgetrf() demo and overwriting of a
    print('original matrix mat1: ')
    print(mat1)
    from frovedis.linalg.scalapack import dgetrf
    rf = dgetrf(mat1,overwrite_a=1)
    print('overwritten matrix: ')
    print(mat1)
    
Output  

    original matrix mat1: 
    [[ 1.  0.  2.]
     [-1.  5.  0.]
     [ 0.  3. -9.]]

    overwritten matrix: 
    [[  1.    0.    2. ]
     [ -1.    5.    2. ]
     [  0.    0.6 -10.2]]

**Here, if the input 'mat1' is double (float64) type and overwite is enabled, it's overwitten with double (float64) type values.**  

**Same applies for other types (int, float (float32)) when input is a numpy matrix/array and overwrite is enabled.**  

For example,  

    from frovedis.matrix.dense import FrovedisBlockcyclicMatrix
    mat1 = np.array([[1.,0.,2.],[-1.,5.,0.],[0.,3.,-9.]])
    bcm1 = FrovedisBlockcyclicMatrix(mat1,dtype = np.float64)
    
    # dgetrf() demo and bcm1 is an instance of FrovedisBlockcyclicMatrix
    from frovedis.linalg.scalapack import dgetrf
    rf = dgetrf(bcm1)
    
    # Unpacking the tuple
    rf[0].debug_print()
    print(rf[1])
    print(rf[2])

Output  

    <frovedis.matrix.results.GetrfResult object at 0x7f785d244978>
    0
    matrix:
    num_row = 3, num_col = 3
    node 0
    node = 0, local_num_row = 3, local_num_col = 3, type = 2, descriptor = 1 1 3 3 3 3 0 0 3 3 3
    val = 1 -1 0 0 5 0.6 2 2 -10.2

__Return Value__  
1. **If 'a' is a python inputs such as numpy matrix**:  
     - It returns a tuple **(lu, res, stat)** where,  
       - **lu**: It is a numpy matrix having double (float64) type values (by default) and containing the LU factor of input matrix 'a'. In case 'overwrite_a' is enabled, then dtype for the matrix will depend on input 'a' dtype.  
       - **res**: It returns an instance of GetrfResult containing server side pointer of pivot array.  
       - **stat**: It returns an integer containing status (info) of native scalapack dgetrf.  
2. **If 'a' is an instance of FrovedisBlockcyclicMatrix:**  
     - It returns a tuple **(lu, res, stat)** where,  
       - **lu**: It returns an instance of FrovedisBlockcyclicMatrix containing the LU factor of input matrix 'a'.  
       - **res**: It returns an instance of GetrfResult containing server side pointer of pivot array.  
       - **stat**: It returns an integer containing status (info) of native scalapack dgetrf.  

### 11. getrf(a, overwrite_a = 0, dtype = np.float64)  

__Parameters__  
_**a**_: It accepts a python array-like input or left hand side numpy matrix of the linear equation having int, float (float32) or double (float64) type values. It also accepts FrovedisBlockcyclicMatrix instance having float (float32) or double (float64) type values.  
_**overwrite\_a**_: It accepts an integer parameter, if set to 0, then 'a' will remain unchanged. Otherwise 'a' will be 
overwritten with 'LU'. (Default: 0)  
_**dtype**_: It specifies the datatype to be used for setting the precision level (single for float32 / double for float64) for the values returned by this method. (Default: np.float64)  
**Currently, it supports float (float32) or double (float64) datatypes.**  

__Purpose__  
It computes the LU factorization of matrix 'a' with single (float32) or double (float64) precision depending on 'dtype' parameter provided by user.  

It computes an LU factorization of matrix 'a', using partial pivoting with row interchanges.  

**This method is present only in frovedis**.  

This method internally uses Scalapack.getrf() **(present in frovedis.matrix module)**.  

For example,  

    import numpy as np
    mat1 = np.array([[1.,0.,2.],[-1.,5.,0.],[0.,3.,-9.]])
    
    # getrf() demo
    from frovedis.linalg.scalapack import getrf
    rf = getrf(mat1)
    print(rf)
    
Output  

    (array([[  1. ,   0. ,   2. ],
           [ -1. ,   5. ,   2. ],
           [  0. ,   0.6, -10.2]]), 
           <frovedis.matrix.results.GetrfResult object at 0x7f80614aacf8>, 0)

For example,  
    
    # getrf() demo and overwriting of a
    print('original matrix mat1: ')
    print(mat1)
    from frovedis.linalg.scalapack import getrf
    rf = getrf(mat1,overwrite_a=1)
    print('overwritten matrix: ')
    print(mat1)
    
Output  

    original matrix mat1: 
    [[ 1.  0.  2.]
     [-1.  5.  0.]
     [ 0.  3. -9.]]

    overwritten matrix: 
    [[  1.    0.    2. ]
     [ -1.    5.    2. ]
     [  0.    0.6 -10.2]]

**Here, if the input 'mat1' is double (float64) type and overwite is enabled, it's overwitten with double (float64) type values.**  

**Same applies for other types (int, float (float32)) when input is a numpy matrix/array and overwrite is enabled.**  

For example,  
        
    # getrf() demo and specifying the dtype of output matrix LU factor 
    from frovedis.linalg.scalapack import getrf
    rf = getrf(mat1,dtype=np.float32)
    print(rf)
    
Output  

    (array([[  1. ,   0. ,   2. ],
           [ -1. ,   5. ,   2. ],
           [  0. ,   0.6, -10.2]], dtype=float32), 
           <frovedis.matrix.results.GetrfResult object at 0x7f7c0bdbbd30>, 0)

For example,  

    from frovedis.matrix.dense import FrovedisBlockcyclicMatrix
    mat1 = np.array([[1.,0.,2.],[-1.,5.,0.],[0.,3.,-9.]])
    bcm1 = FrovedisBlockcyclicMatrix(mat1,dtype = np.float32)

    # getrf() demo and bcm1 is an instance of FrovedisBlockcyclicMatrix
    from frovedis.linalg.scalapack import getrf
    rf = getrf(bcm1,dtype = np.float32)

    # Unpacking the tuple
    rf[0].debug_print()
    print(rf[1])
    print(rf[2])
    
Output  

    <frovedis.matrix.results.GetrfResult object at 0x7f56091379b0>
    0
    matrix:
    num_row = 3, num_col = 3
    node 0
    node = 0, local_num_row = 3, local_num_col = 3, type = 2, descriptor = 1 1 3 3 3 3 0 0 3 3 3
    val = 1 -1 0 0 5 0.6 2 2 -10.2

**Note:- 'dtype' for the wrapper function and FrovedisBlockcyclicMatrix instance must be same during computation. Otherwise, it 
will raise an excpetion.**  

__Return Value__  
1. **If 'a' is a python input such as numpy matrix and dtype = np.float32**:  
     - It returns a tuple **(lu, res, stat)** where,  
       - **lu**: It is a numpy matrix having float (float32) type values (by default) and containing the LU factor of input matrix 'a'. In case 'overwrite_a' is enabled, then dtype for the matrix will depend on input 'a' dtype.  
       - **res**: It returns an instance of GetrfResult containing server side pointer of pivot array.  
       - **stat**: It returns an integer containing status (info) of native scalapack dgetrf.  
1. **If 'a' is a python input such as numpy matrix and dtype = np.float64**:  
     - It returns a tuple **(lu, res, stat)** where,  
       - **lu**: It is a numpy matrix having double (float64) type values (by default) and containing the LU factor of input matrix 'a'. In case 'overwrite_a' is enabled, then dtype for the matrix will depend on input 'a' dtype.  
       - **res**: It returns an instance of GetrfResult containing server side pointer of pivot array.  
       - **stat**: It returns an integer containing status (info) of native scalapack dgetrf.  
3. **If 'a' is an instance of FrovedisBlockcyclicMatrix:**  
     - It returns a tuple **(lu, res, stat)** where,  
       - **lu**: It returns an instance of FrovedisBlockcyclicMatrix containing the LU factor of input matrix 'a'.  
       - **res**: It returns an instance of GetrfResult containing server side pointer of pivot array.  
       - **stat**: It returns an integer containing status (info) of native scalapack dgetrf.  

### 12. sgetrf(a, overwrite_a = 0)  

__Parameters__  
_**a**_: It accepts a python array-like input or left hand side numpy matrix of the linear equation having int, float (float32) or double (float64) type values. It also accepts FrovedisBlockcyclicMatrix instance having float (float32) type values.  
_**overwrite\_a**_: It accepts an integer parameter, if set to 0, then 'a' will remain unchanged. Otherwise 'a' will be 
overwritten with 'LU'. (Default: 0)  

__Purpose__  
It computes the LU factorization of matrix 'a' with single (float32) precision.  

This method internally uses Scalapack.getrf() **(present in frovedis.matrix module)**.  

It computes an LU factorization of matrix 'a', using partial pivoting with row interchanges.  

For example,  

    import numpy as np
    mat1 = np.array([[1.,0.,2.],[-1.,5.,0.],[0.,3.,-9.]])
    
    # sgetrf() demo
    from frovedis.linalg.scalapack import sgetrf
    rf = sgetrf(mat1)
    print(rf)
    
Output  

    (array([[  1. ,   0. ,   2. ],
           [ -1. ,   5. ,   2. ],
           [  0. ,   0.6, -10.2]], dtype=float32), 
           <frovedis.matrix.results.GetrfResult object at 0x7f1498750cf8>, 0)

**This resultant tuple of sgetrf() wrapper function contains GetrfResult instance in frovedis. Whereas in scipy, it contains an array.**  
**Both here are responsible to hold pivot array information.**  

For example,  
    
    # sgetrf() demo and overwriting of a
    print('original matrix mat1: ')
    print(mat1)
    from frovedis.linalg.scalapack import sgetrf
    rf = sgetrf(mat1,overwrite_a=1)
    print('overwritten matrix: ')
    print(mat1)
    
Output  

    original matrix mat1: 
    [[ 1.  0.  2.]
     [-1.  5.  0.]
     [ 0.  3. -9.]]
    overwritten matrix: 
    [[  1.           0.           2.        ]
     [ -1.           5.           2.        ]
     [  0.           0.60000002 -10.19999981]]

**Here, if the input 'mat1' is double (float64) type and overwite is enabled, it's overwitten with double (float64) type values.**  

**Same applies for other types (int, float (float32)) when input is a numpy matrix/array and overwrite is enabled.**  

For example,  

    from frovedis.matrix.dense import FrovedisBlockcyclicMatrix
    mat1 = np.array([[1.,0.,2.],[-1.,5.,0.],[0.,3.,-9.]])
    bcm1 = FrovedisBlockcyclicMatrix(mat1,dtype = np.float32)

    # sgetrf() demo and bcm1 is an instance of FrovedisBlockcyclicMatrix
    from frovedis.linalg.scalapack import getrf
    rf = getrf(bcm1,dtype = np.float32)

    # Unpacking the tuple
    rf[0].debug_print()
    print(rf[1])
    print(rf[2])
    
Output

    <frovedis.matrix.results.GetrfResult object at 0x7f6199f28908>
    0
    matrix:
    num_row = 3, num_col = 3
    node 0
    node = 0, local_num_row = 3, local_num_col = 3, type = 2, descriptor = 1 1 3 3 3 3 0 0 3 3 3
    val = 1 -1 0 0 5 0.6 2 2 -10.2

__Return Value__  
1. **If 'a' is a python input such as numpy matrix**:  
     - It returns a tuple **(lu, res, stat)** where,  
       - **lu**: It is a numpy matrix having float (float32) type values (by default) and containing the LU factor of input matrix 'a'. In case 'overwrite_a' is enabled, then dtype for the matrix will depend on input 'a' dtype.  
       - **res**: It returns an instance of GetrfResult containing server side pointer of pivot array.  
       - **stat**: It returns an integer containing status (info) of native scalapack dgetrf.  
2. **If 'a' is an instance of FrovedisBlockcyclicMatrix:**  
     - It returns a tuple **(lu, res, stat)** where,  
       - **lu**: It returns an instance of FrovedisBlockcyclicMatrix containing the LU factor of input matrix 'a'.  
       - **res**: It returns an instance of GetrfResult containing server side pointer of pivot array.  
       - **stat**: It returns an integer containing status (info) of native scalapack dgetrf.  

### 13. dgetri(lu, piv, lwork = 0, overwrite_lu = 0)  
__Parameters__  
_**lu**_: It accepts a python array-like input or numpy matrix having int, float (float32) or double (float64) type values. It also accepts FrovedisBlockcyclicMatrix instance having double (float64) type values computed using dgetrf().  
_**piv**_: It accepts an instance of GetrfResult containing server side pointer of pivot array computed using dgetrf().  
_**lwork**_: This is an unused parameter. (Default: 0)  
_**overwrite\_lu**_: It accepts an integer parameter, if set to 0, then 'lu' will remain unchanged. Otherwise 'lu' will be 
overwritten with inverse matrix. (Default: 0)  

__Purpose__  
It computes the inverse matrix with double (float64) precision.  

The parameter: "lwork" is simply kept in to to make the interface uniform to the scipy.linalg.lapack.dgetri() module. They 
are not used anywhere within the frovedis implementation.  

This method internally uses Scalapack.getri() **(present in frovedis.matrix module)**.  

For example,  

    import numpy as np
    mat1 = np.array([[1.,0.,2.],[-1.,5.,0.],[0.,3.,-9.]])
    
    # dgetrf() demo
    from frovedis.linalg.scalapack import dgetrf,dgetri
    
    # compute lu and piv using dgetrf()
    rf = dgetrf(mat1)
    
    # dgetri() demo
    ri = dgetri(rf[0],rf[1])
    print(ri)
    
Output  

    (array([[ 0.88235294, -0.11764706,  0.19607843],
       [ 0.17647059,  0.17647059,  0.03921569],
       [ 0.05882353,  0.05882353, -0.09803922]]), 0)

For example,  

    import numpy as np
    mat1 = np.array([[1.,0.,2.],[-1.,5.,0.],[0.,3.,-9.]])
    
    # dgetrf() demo
    from frovedis.linalg.scalapack import dgetrf,dgetri
    
    # compute lu and piv using dgetrf()
    rf = dgetrf(mat1,overwrite_a = 1)
    
    # dgetri() demo and overwriting of lu
    print('original matrix mat1: ')
    print(mat1)
    ri = dgetri(rf[0],rf[1], overwrite_lu = 1)
    print('overwritten matrix: ')
    print(mat1)
    
Output  

    original matrix mat1:
    [[ 1.  0.  2.]
     [-1.  5.  0.]
     [ 0.  3. -9.]]
    overwritten matrix:
    [[ 0.88235294 -0.11764706  0.19607843]
     [ 0.17647059  0.17647059  0.03921569]
     [ 0.05882353  0.05882353 -0.09803922]]

The input matrix is only processed by dgetrf() first to generate LU factor. The LU factor and ipiv information is then 
used with dgetri().  

**Here, if the input 'mat1' is double (float64) type and overwite is enabled, it's overwitten with inverse matrix having double (float64) type values.**  

**Same applies for other types (int, float (float32)) when input is a numpy matrix/array and overwrite is enabled.**  

For example,  

    from frovedis.matrix.dense import FrovedisBlockcyclicMatrix
    mat1 = np.array([[1.,0.,2.],[-1.,5.,0.],[0.,3.,-9.]])
    mat2 = np.array([[-9.,3.,-1.],[0.,3.,1.],[2.,2.,0.]])
    bcm1 = FrovedisBlockcyclicMatrix(mat1)
    bcm2 = FrovedisBlockcyclicMatrix(mat2)

    # dgetri() demo and bcm1 and bcm2 are an instance of FrovedisBlockcyclicMatrix
    from frovedis.linalg.scalapack import dgetrf,dgetri
    rf = dgetrf(bcm1)
    ri = dgetri(rf[0],rf[1],bcm2)

    # Unpacking the tuple
    ri[0].debug_print()
    print(ri[1])

Output  

    0
    matrix:
    num_row = 3, num_col = 3
    node 0
    node = 0, local_num_row = 3, local_num_col = 3, type = 2, descriptor = 1 1 3 3 3 3 0 0 3 3 3
    val = 0.882353 0.176471 0.0588235 -0.117647 0.176471 0.0588235 0.196078 0.0392157 -0.0980392

__Return Value__  
1. **If 'lu' is python input such as numpy matrix**:  
     - It returns a tuple **(inv_a, stat)** where,  
       - **inv_a**: It is a numpy matrix having double (float64) type values (by default) and containing the inverse matrix. In case 'overwrite_lu' is enabled, then dtype for the matrix will depend on intermediate input 'LU' factor dtype.  
       - **stat**: It returns an integer containing status (info) of native scalapack dgetri.  
2. **If 'lu' is an instance of FrovedisBlockcyclicMatrix:**  
     - It returns a tuple **(inv_a, stat)** where,  
       - **inv_a**: It returns an instance of FrovedisBlockcyclicMatrix containing the inverse matrix.  
       - **stat**: It returns an integer containing status (info) of native scalapack dgetri.  

### 14. getri(lu, piv, lwork = 0, overwrite_lu = 0, dtype = np.float64)
__Parameters__  
_**lu**_: It accepts a python array-like input or numpy matrix having int, float (float32) or double (float64) type values. It also accepts FrovedisBlockcyclicMatrix instance having float (float32) or double (float64) type values.  
_**piv**_: It accepts an instance of GetrfResult containing server side pointer of pivot array.  
_**lwork**_: This is an unused parameter. (Default: 0)  
_**overwrite\_lu**_: It accepts an integer parameter, if set to 0, then 'lu' will remain unchanged. Otherwise 'lu' will be 
overwritten with inverse matrix. (Default: 0)  
_**dtype**_: It specifies the datatype to be used for setting the precision level (single for float32 / double for float64) for the values returned by this method. (Default: np.float64)  
**Currently, it supports float (float32) or double (float64) datatypes.**  

__Purpose__  
It computes the inverse matrix with single (float32) or double (float64) precision depending on 'dtype' parameter provided by user.  

The parameter: "lwork" is simply kept in to to make the interface uniform to the scipy.linalg.lapack.dgetri() module. They 
are not used anywhere within the frovedis implementation.  

**This method is present only in frovedis**.  

This method internally uses Scalapack.getri() **(present in frovedis.matrix module)**.  

For example,  

    import numpy as np
    mat1 = np.array([[1.,0.,2.],[-1.,5.,0.],[0.,3.,-9.]])
    
    # getrf() demo
    from frovedis.linalg.scalapack import getrf,getri
    
    # compute lu and piv using getrf()
    rf = getrf(mat1)
    
    # getri() demo
    ri = getri(rf[0],rf[1])
    print(ri)
    
Output  

    (array([[ 0.88235294, -0.11764706,  0.19607843],
           [ 0.17647059,  0.17647059,  0.03921569],
           [ 0.05882353,  0.05882353, -0.09803922]]), 0)

For example,  

    import numpy as np
    mat1 = np.array([[1.,0.,2.],[-1.,5.,0.],[0.,3.,-9.]])
    
    # getrf() demo
    from frovedis.linalg.scalapack import getrf,getri
    
    # compute lu and piv using getrf()
    rf = getrf(mat1, overwrite_a = 1)
    
    # getri() demo and overwriting of lu
    print('original matrix mat1: ')
    print(mat1)
    ri = getri(rf[0],rf[1], overwrite_lu = 1)
    print('overwritten matrix: ')
    print(mat1)
    
Output  

    original matrix mat1:
    [[ 1.  0.  2.]
     [-1.  5.  0.]
     [ 0.  3. -9.]]
    overwritten matrix:
    [[ 0.88235294 -0.11764706  0.19607843]
     [ 0.17647059  0.17647059  0.03921569]
     [ 0.05882353  0.05882353 -0.09803922]]

The input matrix is only processed by getrf() first to generate LU factor. The LU factor and ipiv information is then used with getri().  

**Here, if the input 'mat1' is double (float64) type and overwite is enabled, it's overwitten with inverse matrix having double (float64) type values.**  

**Same applies for other types (int, float (float32)) when input is a numpy matrix/array and overwrite is enabled.**  

For example,  
        
    import numpy as np
    mat1 = np.array([[1.,0.,2.],[-1.,5.,0.],[0.,3.,-9.]])
    
    # getrf() demo
    from frovedis.linalg.scalapack import getrf,getri
    
    # compute lu and piv using getrf()
    rf = getrf(mat1)
    
    # getri() demo and specifying the dtype of output LU matrix  
    from frovedis.linalg.scalapack import getri
    rf = getri(rf[0],rf[1],dtype=np.float32)
    print(rf)
    
Output  

    (array([[ 0.88235295, -0.11764707,  0.19607843],
       [ 0.1764706 ,  0.1764706 ,  0.03921569],
       [ 0.05882353,  0.05882353, -0.09803922]], dtype=float32), 0)

For example,  

    from frovedis.matrix.dense import FrovedisBlockcyclicMatrix
    mat1 = np.array([[1.,0.,2.],[-1.,5.,0.],[0.,3.,-9.]])
    mat2 = np.array([[-9.,3.,-1.],[0.,3.,1.],[2.,2.,0.]])
    bcm1 = FrovedisBlockcyclicMatrix(mat1,dtype = np.float32)
    bcm2 = FrovedisBlockcyclicMatrix(mat2,dtype = np.float32)

    # sgetri() demo and bcm1 and bcm2 are an instance of FrovedisBlockcyclicMatrix
    from frovedis.linalg.scalapack import getrf,getri
    rf = getrf(bcm1,dtype = np.float32)
    ri = getri(rf[0],rf[1],bcm2, dtype = np.float32)

    # Unpacking the tuple
    ri[0].debug_print()
    print(ri[1])

Output  

    0
    matrix:
    num_row = 3, num_col = 3
    node 0
    node = 0, local_num_row = 3, local_num_col = 3, type = 2, descriptor = 1 1 3 3 3 3 0 0 3 3 3
    val = 0.882353 0.176471 0.0588235 -0.117647 0.176471 0.0588235 0.196078 0.0392157 -0.0980392

**Note:- 'dtype' for the wrapper function and FrovedisBlockcyclicMatrix instance must be same during computation. Otherwise, it 
will raise an excpetion.**  

__Return Value__  
1. **If 'lu' is python input such as numpy matrix and dtype = np.float32**:  
     - It returns a tuple **(inv_a, stat)** where,  
       - **inv_a**: It is a numpy matrix having float (float32) type values (by default) and containing the inverse matrix. In case 'overwrite_lu' is enabled, then dtype for the matrix will depend on intermediate input 'LU' factor dtype.  
       - **stat**: It returns an integer containing status (info) of native scalapack dgetri.  
2. **If 'lu' is python input such as numpy matrix and dtype = np.float64**:  
     - It returns a tuple **(inv_a, stat)** where,  
       - **inv_a**: It is a numpy matrix having double (float64) type values (by default) and containing the inverse matrix. In case 'overwrite_lu' is enabled, then dtype for the matrix will depend on intermediate input 'LU' factor dtype.  
       - **stat**: It returns an integer containing status (info) of native scalapack dgetri.        
3. **If 'lu' is an instance of FrovedisBlockcyclicMatrix:**  
     - It returns a tuple **(inv_a, stat)** where,  
       - **inv_a**: It returns an instance of FrovedisBlockcyclicMatrix containing the inverse matrix.  
       - **stat**: It returns an integer containing status (info) of native scalapack dgetri.  

### 15. sgetri(lu, piv, lwork = 0, overwrite_lu = 0)  
__Parameters__  
_**lu**_: It accepts a python array-like input or numpy matrix having int, float (float32) or double (float64) type values. It also accepts FrovedisBlockcyclicMatrix instance having float (float32) type values.  
_**piv**_: It accepts an instance of GetrfResult containing server side pointer of pivot array.  
_**lwork**_: This is an unused parameter. (Default: 0)  
_**overwrite\_lu**_: It accepts an integer parameter, if set to 0, then 'lu' will remain unchanged. Otherwise 'lu' will be 
overwritten with inverse matrix. (Default: 0)  

__Purpose__  
It computes the inverse matrix with single (float32) precision.  

The parameter: "lwork" is simply kept in to to make the interface uniform to the scipy.linalg.lapack.dgetri() module. They 
are not used anywhere within the frovedis implementation.  

This method internally uses Scalapack.getri() **(present in frovedis.matrix module)**.  

For example,  

    import numpy as np
    mat1 = np.array([[1.,0.,2.],[-1.,5.,0.],[0.,3.,-9.]])
    
    # sgetrf() demo
    from frovedis.linalg.scalapack import sgetrf,sgetri
    
    # compute lu and piv using sgetrf()
    rf = sgetrf(mat1)
    
    # sgetri() demo
    ri = sgetri(rf[0],rf[1])
    print(ri)
    
Output  

    (array([[ 0.88235295, -0.11764707,  0.19607843],
       [ 0.1764706 ,  0.1764706 ,  0.03921569],
       [ 0.05882353,  0.05882353, -0.09803922]], dtype=float32), 0)

For example,  

    import numpy as np
    mat1 = np.array([[1.,0.,2.],[-1.,5.,0.],[0.,3.,-9.]])
    
    # sgetrf() demo
    from frovedis.linalg.scalapack import sgetrf,sgetri
    
    # compute lu and piv using sgetrf()
    rf = sgetrf(mat1, overwrite_a = 1)
    
    # sgetri() demo and overwriting of lu
    print('original matrix mat1: ')
    print(mat1)
    ri = sgetri(rf[0],rf[1], overwrite_lu = 1)
    print('overwritten matrix: ')
    print(mat1)
    
Output  

    original matrix mat1:
    [[ 1.  0.  2.]
     [-1.  5.  0.]
     [ 0.  3. -9.]]
     
    overwritten matrix:
    [[ 0.88235295 -0.11764707  0.19607843]
     [ 0.17647059  0.17647059  0.03921569]
     [ 0.05882353  0.05882353 -0.09803922]]

The input matrix is only processed by sgetrf() first to generate LU factor. The LU factor and ipiv information 
is then used with sgetri().  

**Here, if the input 'mat1' is double (float64) type and overwite is enabled, it's overwitten with inverse matrix having double (float64) type values.**  

**Same applies for other types (int, float (float32)) when input is a numpy matrix/array and overwrite is enabled.**  

For example,  

    from frovedis.matrix.dense import FrovedisBlockcyclicMatrix
    mat1 = np.array([[1.,0.,2.],[-1.,5.,0.],[0.,3.,-9.]])
    mat2 = np.array([[-9.,3.,-1.],[0.,3.,1.],[2.,2.,0.]])
    bcm1 = FrovedisBlockcyclicMatrix(mat1,dtype = np.float32)
    bcm2 = FrovedisBlockcyclicMatrix(mat2,dtype = np.float32)

    # sgetri() demo and bcm1 and bcm2 are an instance of FrovedisBlockcyclicMatrix
    from frovedis.linalg.scalapack import sgetrf,sgetri
    rf = sgetrf(bcm1)
    ri = sgetri(rf[0],rf[1],bcm2)

    # Unpacking the tuple
    ri[0].debug_print()
    print(ri[1])

Output  

    0
    matrix:
    num_row = 3, num_col = 3
    node 0
    node = 0, local_num_row = 3, local_num_col = 3, type = 2, descriptor = 1 1 3 3 3 3 0 0 3 3 3
    val = 0.882353 0.176471 0.0588235 -0.117647 0.176471 0.0588235 0.196078 0.0392157 -0.0980392

__Return Value__  
1. **If 'lu' is python input such as numpy matrix**:  
     - It returns a tuple **(inv_a, stat)** where,  
       - **inv_a**: It is a numpy matrix having float (float32) type values (by default) and containing the inverse matrix. In case 'overwrite_lu' is enabled, then dtype for the matrix will depend on intermediate input 'LU' factor dtype.  
       - **stat**: It returns an integer containing status (info) of native scalapack dgetri.  
2. **If 'lu' is an instance of FrovedisBlockcyclicMatrix:**  
     - It returns a tuple **(inv_a, stat)** where,  
       - **inv_a**: It returns an instance of FrovedisBlockcyclicMatrix containing the inverse matrix.  
       - **stat**: It returns an integer containing status (info) of native scalapack dgetri.  

### 16. dgetrs(lu, piv, b, trans = 0, overwrite_b = 0)  
__Parameters__  
_**lu**_: It accepts a python array-like input or numpy matrix having int, float (float32) or double (float64) type values. It also accepts FrovedisBlockcyclicMatrix instance having double (float64) type values computed using dgetrf.  
_**piv**_: It accepts an instance of GetrfResult containing server side pointer of pivot array computed using dgetrf().  
_**b**_: It accepts a python array-like input or right hand side matrix of the linear equation having int, float (float32) or double (float64) type values. It also accepts FrovedisBlockcyclicMatrix instance having double (float64) type values. It should have number of rows >= the number of rows in 'a' and at least 1 column in it.    
_**trans**_: It accepts an integer parameter indicating if transpose of 'lu' needs to be computed before solving linear equation. If 
it is not 0, then the transpose is computed. (Default: 0)  
_**overwrite\_b**_: It accepts an integer parameter, if set to 0, then 'b' will remain unchanged. Otherwise 'b' will be 
overwritten with the solution matrix. (Default: 0)  

__Purpose__   
It solves a system of linear equations, AX = B with matrix 'a' using the LU factorization computed by getrf(). Thus before calling this function, it is required to obtain the factored matrix 'lu' (along with piv information) by calling getrf().  

It computes the solution matrix for the system of linear equations with double (float64) precision.  

This method internally uses Scalapack.getrs() **(present in frovedis.matrix module)**.  

For example,  

    import numpy as np
    mat1 = np.array([[1.,0.,2.],[-1.,5.,0.],[0.,3.,-9.]])
    mat2 = np.array([[-9.,3.,-1.],[0.,3.,1.],[2.,2.,0.]])
    
    # dgetrf() demo
    from frovedis.linalg.scalapack import dgetrf,dgetrs
    
    # compute lu and piv using dgetrf()
    rf = dgetrf(mat1)
    
    # dgetrs() demo
    ri = dgetrs(rf[0],rf[1],mat2)
    print(ri)
    
Output  

    (array([[-7.54901961,  2.68627451, -1.        ],
       [-1.50980392,  1.1372549 ,  0.        ],
       [-0.7254902 ,  0.15686275,  0.        ]]), 0)

For example,  

    import numpy as np
    mat1 = np.array([[1.,0.,2.],[-1.,5.,0.],[0.,3.,-9.]])
    mat2 = np.array([[-9.,3.,-1.],[0.,3.,1.],[2.,2.,0.]])
    
    # dgetrf() demo
    from frovedis.linalg.scalapack import dgetrf,dgetrs
    
    # compute lu and piv using dgetrf()
    rf = dgetrf(mat1, overwrite_a = 1)
    
    # dgetrs() demo and overwriting of b
    print('original matrix mat2: ')
    print(mat2)
    ri = dgetrs(rf[0],rf[1],mat2,overwrite_b = 1)
    print('overwritten matrix: ')
    print(mat1)
    
Output  

    original matrix mat2:
    [[-9.  3. -1.]
     [ 0.  3.  1.]
     [ 2.  2.  0.]]
    overwritten matrix:
    [[-7.54901961  2.68627451 -1.        ]
     [-1.50980392  1.1372549   0.        ]
     [-0.7254902   0.15686275  0.        ]]

**Here, if the inputs 'mat2' is double (float64) type and overwite is enabled in dgetrs(), it's overwitten with solution having double (float64) type values.**  

**Same applies for other types (int, float (float32)) when input is a numpy matrix/array and overwrite is enabled.**  

For example,  

    from frovedis.matrix.dense import FrovedisBlockcyclicMatrix
    mat1 = np.array([[1.,0.,2.],[-1.,5.,0.],[0.,3.,-9.]])
    mat2 = np.array([[-9.,3.,-1.],[0.,3.,1.],[2.,2.,0.]])
    bcm1 = FrovedisBlockcyclicMatrix(mat1)
    bcm2 = FrovedisBlockcyclicMatrix(mat2)

    # dgetrs() demo and bcm1 and bcm2 are an instance of FrovedisBlockcyclicMatrix
    from frovedis.linalg.scalapack import dgetrf,dgetrs
    rf = dgetrf(bcm1)
    ri = dgetrs(rf[0],rf[1],bcm2)

    # Unpacking the tuple
    ri[0].debug_print()
    print(ri[1])

Output  

    0
    matrix:
    num_row = 3, num_col = 3
    node 0
    node = 0, local_num_row = 3, local_num_col = 3, type = 2, descriptor = 1 1 3 3 3 3 0 0 3 3 3
    val = -7.54902 -1.5098 -0.72549 2.68627 1.13725 0.156863 -1 0 0

__Return Value__  
1. **If 'b' is python input such as numpy matrix**:  
     - It returns a tuple **(x, stat)** where,  
       - **x**: It is a numpy matrix having double (float64) type values (by default) and containing the solution matrix. In case 'overwrite_b' is enabled, then dtype for the matrix will depend on input 'b' dtype.  
       - **stat**: It returns an integer containing status (info) of native scalapack dgetrs.  
2. **If 'b' is an instance of FrovedisBlockcyclicMatrix:**  
     - It returns a tuple **(x, stat)** where,  
       - **x**: It returns an instance of FrovedisBlockcyclicMatrix containing the solution matrix.  
       - **stat**: It returns an integer containing status (info) of native scalapack dgetri.  

### 17. getrs(lu, piv, b, trans = 0, overwrite_b = 0, dtype = np.float64)  
__Parameters__  
_**lu**_: It accepts a python array-like input or numpy matrix having int, float (float32) or double (float64) type values. It also accepts FrovedisBlockcyclicMatrix instance having float (float32) or double (float64) type values computed using dgetrf.  
_**piv**_: It accepts an instance of GetrfResult containing server side pointer of pivot array computed using dgetrf().  
_**b**_: It accepts a python array-like input or right hand side matrix of the linear equation having int, float (float32) or double (float64) type values. It also accepts FrovedisBlockcyclicMatrix instance having float (float32) or double (float64) type values. It should have number of rows >= the number of rows in 'a' and at least 1 column in it.    
_**trans**_: It accepts an integer parameter indicating if transpose of 'lu' needs to be computed before solving linear equation. If 
it is not 0, then the transpose is computed. (Default: 0)  
_**overwrite\_b**_: It accepts an integer parameter, if set to 0, then 'b' will remain unchanged. Otherwise 'b' will be 
overwritten with the solution matrix. (Default: 0)  
_**dtype**_: It specifies the datatype to be used for setting the precision level (single for float32 / double for float64) for the values returned by this method. (Default: np.float64)  
**Currently, it supports float (float32) or double (float64) datatypes.**  

__Purpose__  
It solves a system of linear equations, AX = B with matrix 'a' using the LU factorization computed by getrf(). Thus before calling this function, it is required to obtain the factored matrix 'lu' by calling getrf().  

It computes the solution matrix for the system of linear equations with float or double (float64) precision depending on 'dtype' parameter provided by user.  

**This method is present only in frovedis**.  

This method internally uses Scalapack.getrs() **(present in frovedis.matrix module)**.  

For example,  

    import numpy as np
    mat1 = np.array([[1.,0.,2.],[-1.,5.,0.],[0.,3.,-9.]])
    mat2 = np.array([[-9.,3.,-1.],[0.,3.,1.],[2.,2.,0.]])
    
    # getrf() demo
    from frovedis.linalg.scalapack import getrf,getrs
    
    # compute lu and piv using getrf()
    rf = getrf(mat1)
    
    # getrs() demo
    ri = getrs(rf[0],rf[1],mat2)
    print(ri)
    
Output  

    (array([[-7.54901961,  2.68627451, -1.        ],
       [-1.50980392,  1.1372549 ,  0.        ],
       [-0.7254902 ,  0.15686275,  0.        ]]), 0)

For example,  

    import numpy as np
    mat1 = np.array([[1.,0.,2.],[-1.,5.,0.],[0.,3.,-9.]])
    mat2 = np.array([[-9.,3.,-1.],[0.,3.,1.],[2.,2.,0.]])
    
    # getrf() demo
    from frovedis.linalg.scalapack import getrf,getrs
    
    # compute lu and piv using getrf()
    rf = getrf(mat1, overwrite_a = 1)
    
    # getrs() demo and overwriting of b
    print('original matrix mat2: ')
    print(mat2)
    ri = getrs(rf[0],rf[1], mat2, overwrite_b = 1)
    print('overwritten matrix: ')
    print(mat2)
    
Output  

    original matrix mat2:
    [[-9.  3. -1.]
     [ 0.  3.  1.]
     [ 2.  2.  0.]]
    overwritten matrix:
    [[-7.54901961  2.68627451 -1.        ]
     [-1.50980392  1.1372549   0.        ]
     [-0.7254902   0.15686275  0.        ]]

**Here, if the inputs 'mat2' is double (float64) type and overwite is enabled in dgetrs(), it's overwitten with solution having double (float64) type values.**  

**Same applies for other types (int, float (float32)) when input is a numpy matrix/array and overwrite is enabled.**  

For example,  
        
    import numpy as np
    mat1 = np.array([[1.,0.,2.],[-1.,5.,0.],[0.,3.,-9.]])
    
    # getrf() demo
    from frovedis.linalg.scalapack import getrf,getrs
    
    # compute lu and piv using getrf()
    rf = getrf(mat1,dtype=np.float32)
    
    # getrs() demo and specifying the dtype of output LU matrix  
    from frovedis.linalg.scalapack import getrs
    rf = getrs(rf[0],rf[1],mat2,dtype=np.float32)
    print(rf)
    
Output  

    (array([[-7.54902   ,  2.6862745 , -1.        ],
       [-1.509804  ,  1.137255  ,  0.        ],
       [-0.7254902 ,  0.15686277,  0.        ]], dtype=float32), 0)

For example,  

    from frovedis.matrix.dense import FrovedisBlockcyclicMatrix
    mat1 = np.array([[1.,0.,2.],[-1.,5.,0.],[0.,3.,-9.]])
    mat2 = np.array([[-9.,3.,-1.],[0.,3.,1.],[2.,2.,0.]])
    bcm1 = FrovedisBlockcyclicMatrix(mat1,dtype = np.float32)
    bcm2 = FrovedisBlockcyclicMatrix(mat2, dtype = np.float32)

    # getrs() demo and bcm1 and bcm2 are an instance of FrovedisBlockcyclicMatrix
    from frovedis.linalg.scalapack import getrf,getrs
    rf = getrf(bcm1,dtype = np.float32)
    ri = getrs(rf[0],rf[1],bcm2,dtype = np.float32)

    # Unpacking the tuple
    ri[0].debug_print()
    print(ri[1])

Output  

    0
    matrix:
    num_row = 3, num_col = 3
    node 0
    node = 0, local_num_row = 3, local_num_col = 3, type = 2, descriptor = 1 1 3 3 3 3 0 0 3 3 3
    val = -7.54902 -1.5098 -0.72549 2.68627 1.13725 0.156863 -1 0 0

**Note:- 'dtype' for the wrapper function and FrovedisBlockcyclicMatrix instance must be same during computation. Otherwise, it 
will raise an excpetion.**  

__Return Value__  
1. **If 'b' is python input such as numpy matrix and dtype = np.float32**:  
     - It returns a tuple **(x, stat)** where,  
       - **x**: It is a numpy matrix having float (float32) type values (by default) containing the solution matrix. In case 'overwrite_b' is enabled, then dtype for the matrix will depend on input 'b' dtype.  
       - **stat**: It returns an integer containing status (info) of native scalapack dgetrs.  
2. **If 'b' is python input such as numpy matrix and dtype = np.float64**:  
     - It returns a tuple **(x, stat)** where,  
       - **x**: It is a numpy matrix having double (float64) type values (by default) containing the solution matrix. In case 'overwrite_b' is enabled, then dtype for the matrix will depend on input 'b' dtype.  
       - **stat**: It returns an integer containing status (info) of native scalapack dgetrs.  
3. **If 'b' is an instance of FrovedisBlockcyclicMatrix:**  
     - It returns a tuple **(x, stat)** where,  
       - **x**: It returns an instance of FrovedisBlockcyclicMatrix containing the solution matrix.  
       - **stat**: It returns an integer containing status (info) of native scalapack dgetri.  

### 18. sgetrs(lu, piv, b, trans = 0, overwrite_b = 0)  
__Parameters__  
_**lu**_: It accepts a python array-like input or numpy matrix having int, float (float32) or double (float64) type values. It also accepts FrovedisBlockcyclicMatrix instance having float (float32) type values computed using dgetrf.  
_**piv**_: It accepts an instance of GetrfResult containing server side pointer of pivot array computed using dgetrf().  
_**b**_: It accepts a python array-like input or right hand side matrix of the linear equation having int, float (float32) or double (float64) type values. It also accepts FrovedisBlockcyclicMatrix instance having float (float32) type values. It should have number of rows >= the number of rows in 'a' and at least 1 column in it.  
_**trans**_: It accepts an integer parameter indicating if transpose of 'lu' needs to be computed before solving linear equation. If 
it is not 0, then the transpose is computed. (Default: 0)  
_**overwrite\_b**_: It accepts an integer parameter, if set to 0, then 'b' will remain unchanged. Otherwise 'b' will be 
overwritten with the solution matrix. (Default: 0)  

__Purpose__  
It solves a system of linear equations, AX = B with the matrix 'a' using the LU factorization computed by sgetrf().  
Thus before calling this function, it is required to obtain the factored matrix 'lu' by calling sgetrf().  

It computes the solution matrix for the system of linear equations with float precision.  

This method internally uses Scalapack.getrs() **(present in frovedis.matrix module)**.  

For example,  

    import numpy as np
    mat1 = np.array([[1.,0.,2.],[-1.,5.,0.],[0.,3.,-9.]])
    mat2 = np.array([[-9.,3.,-1.],[0.,3.,1.],[2.,2.,0.]])
    
    # sgetrf() demo
    from frovedis.linalg.scalapack import sgetrf,sgetrs
    
    # compute lu and piv using sgetrf()
    rf = sgetrf(mat1)
    
    # sgetrs() demo
    ri = sgetrs(rf[0]rf[1],mat2)
    print(ri)
    
Output  

    (array([[-7.54902   ,  2.6862745 , -1.        ],
       [-1.509804  ,  1.137255  ,  0.        ],
       [-0.7254902 ,  0.15686277,  0.        ]], dtype=float32), 0)

For example,  

    import numpy as np
    mat1 = np.array([[1.,0.,2.],[-1.,5.,0.],[0.,3.,-9.]])
    
    # sgetrf() demo
    from frovedis.linalg.scalapack import sgetrf,sgetrs
    
    # compute lu and piv using sgetrf()
    rf = sgetrf(mat1)
    
    # sgetrs() demo and overwriting of b
    print('original matrix mat2: ')
    print(mat2)
    ri = sgetrs(rf[0],rf[1], mat2, overwrite_b = 1)
    print('overwritten matrix: ')
    print(mat2)
    
Output  
    
    original matrix mat2:
    [[-9.  3. -1.]
     [ 0.  3.  1.]
     [ 2.  2.  0.]]
    overwritten matrix:
    [[-7.54901981  2.68627453 -1.        ]
     [-1.50980401  1.13725495  0.        ]
     [-0.72549021  0.15686277  0.        ]]

**Here, if the inputs 'mat2' is double (float64) type and overwite is enabled in dgetrs(), it's overwitten with solution having double (float64) type values.**  

**Same applies for other types (int, float (float32)) when input is a numpy matrix/array and overwrite is enabled.**  

For example,  

    from frovedis.matrix.dense import FrovedisBlockcyclicMatrix
    mat1 = np.array([[1.,0.,2.],[-1.,5.,0.],[0.,3.,-9.]])
    mat2 = np.array([[-9.,3.,-1.],[0.,3.,1.],[2.,2.,0.]])
    bcm1 = FrovedisBlockcyclicMatrix(mat1,dtype = np.float32)
    bcm2 = FrovedisBlockcyclicMatrix(mat2,dtype = np.float32)

    # sgetrs() demo and bcm1 and bcm2 are an instance of FrovedisBlockcyclicMatrix
    from frovedis.linalg.scalapack import sgetrf,sgetrs
    rf = sgetrf(bcm1)
    ri = sgetrs(rf[0],rf[1],bcm2)

    # Unpacking the tuple
    ri[0].debug_print()
    print(ri[1])

Output  

    0
    matrix:
    num_row = 3, num_col = 3
    node 0
    node = 0, local_num_row = 3, local_num_col = 3, type = 2, descriptor = 1 1 3 3 3 3 0 0 3 3 3
    val = -7.54902 -1.5098 -0.72549 2.68627 1.13725 0.156863 -1 0 0    

__Return Value__  
1. **If 'b' is python input such as numpy matrix**:  
     - It returns a tuple **(x, stat)** where,  
       - **x**: It is a numpy matrix having float (float32) type value and containing the solution matrix. In case 'overwrite_b' is enabled, then dtype for the matrix will depend on input 'b' dtype.  
       - **stat**: It returns an integer containing status (info) of native scalapack dgetrs.  
2. **If 'b' is an instance of FrovedisBlockcyclicMatrix:**  
     - It returns a tuple **(x, stat)** where,  
       - **x**: It returns an instance of FrovedisBlockcyclicMatrix containing the solution matrix.  
       - **stat**: It returns an integer containing status (info) of native scalapack dgetri.  

# SEE ALSO  
- **[Linalg Functions](./linalg.md)**  
