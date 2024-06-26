% linalg

# NAME  

linalg - a frovedis module which provides user-friendly interfaces for commonly used linear algebra functions.  

# SYNOPSIS  

    import frovedis.linalg.linalg

# Public Member Functions

    1. dot(a, b, out=None)
    2. eigsh(A, M=None, k=6, sigma=None, which='LM', v0=None, ncv=None, maxiter=None, 
             tol=0., return_eigenvectors=True, Minv=None, OPinv=None, mode='normal')
    3. inv(a)
    4. matmul(x1, x2, out=None, casting='same_kind', order='K', dtype=None, 
              subok=True, signature=None, extobj=None)
    5. solve(a, b)
    6. svd(a, full_matrices=False, compute_uv=True)


# DESCRIPTION  
The frovedis linear algebra functions rely on PBLAS and ScaLAPACK wrappers in frovedis to provide efficient low level implementations of standard linear algebra algorithms. These functions are used to compute matrices and vector product, matrix decompostion, solving linear equations, inverting matrices, etc.  

This module provides a client-server implementation, where the client application is a normal python program. The frovedis interface for all linear algebra functions is almost same as numpy linear algebra function interfaces, but it doesn't have any dependency on numpy. It can be used simply even if the system doesn't have numpy installed. Thus, in this implementation, a python client can interact with a frovedis server sending the required python matrix data to frovedis server side. Python data is converted into frovedis compatible data (blockcyclic matrix) internally and then python client can request frovedis server for any of the supported linalg functions on that matrix. Once the operation is completed, the frovedis server sends back the resultant matrix as equivalent python data.  

## Detailed Description  

### 1. dot(a, b, out = None)  

__Parameters__  
_**a**_: It accepts scalar values and python array-like inputs (having dimensions <= 2) of int, float (float32) or double (float64) type values. It also accepts FrovedisBlockcyclicMatrix instance having float (float32) or double (float64) type values.  
_**b**_: It accepts scalar values, and python array-like inputs (having dimensions <= 2) of int, float (float32) or double (float64) type values. It also accepts FrovedisBlockcyclicMatrix instance having float (float32) or double (float64) type values.  
_**out**_: A numpy ndarray of int, float (float32) or double (float64) type values which is where the result is written. (Default: None)  
When it is None (not specified explicitly), a freshly-allocated array having double (float64) type values is returned. Otherwise if provided, it must have a shape that matches the signature (n,k),(k,m)->(n,m) and same datatype as input matrices.  

__Purpose__  
It computes dot product of two arrays.   

**1. If both inputs 'a' and 'b' are scalar values,**   

For example,  
    
    # dot() demo with scalar values as input
    from frovedis.linalg import dot
    prod = dot(3,4)
    print(prod)

Output  

    12

Here, result is also a scalar value.  

**2. If both 'a' and 'b' are 1D arrays,**   

For example,  
    
    import numpy as np
    from frovedis.linalg import dot
    vec1 = np.array([1., 9., 8.])
    vec2 = np.array([4., 1., 7.])    
    
    # dot() demo with both inputs as 1D array
    prod = dot(vec1,vec2)
    print(prod)

Output  

    69.0

Here, dot product is the inner product of vectors.  

**3. If either 'a' or 'b' is a scalar value,**  

For example,  
    
    import numpy as np
    from frovedis.linalg import dot
    vec1 = np.array([[1, 9, 8], [7, 1, 5], [1, 2, 4]])
    
    # dot() demo with 1D array and scalar value as input
    prod = dot(vec1,4)
    print(prod)

Output  

    [[ 4. 36. 32.]
     [28.  4. 20.]
     [ 4.  8. 16.]]

Here, result is multiplication of scalar over the vector.  

**4. If both a and b are 2D arrays,**  

For example,  

    import numpy as np
    from frovedis.linalg import dot
    mat1 = np.array([[1, 9, 8], [7, 1, 5], [1, 2, 4]])
    mat2 = np.array([[4, 1, 7], [2, 2, 9], [1, 2, 3]])
    
    # dot() demo with 2D arrays as input
    prod = dot(mat1, mat2)
    print(prod)

Output  

    [[ 30.  35. 112.]
     [ 35.  19.  73.]
     [ 12.  13.  37.]]

Here, result is simply matrix multiplication.  

**When both 'a' and 'b' inputs are matrices or instances of FrovedisBlockCyclicMatrix, then the column size of 'a' must be same as row size of 'b' before computing dot product as internally it performs matrix-matrix multiplication.**  

**5. If a is an ND array and b is a 1-D array,**  

Currenlty, frovedis supports only N <= 2, so that dot() between ND array and 1D array can be interpreted currently as:  
- dot product between both 1D arrays.  
- dot product between 2D array and 1D array.  

For example,  

    import numpy as np
    from frovedis.linalg import dot
    mat1 = np.array([[1, 9, 8], [7, 1, 5], [1, 2, 4]])
    mat2 = np.array([4, 2, 1])
    
    # dot() demo with 'a' as 2D array, 'b' as 1D array as input
    prod = dot(mat1, mat2)
    print(prod)

Output  

    [30. 35. 12.]

In above example, result will be the sum product over the last axis of 'a' and 'b'.  

**Currently, dot product between inputs 'a' (an N-D array) and 'b' (an M-D array (where M>=2) is not supported in frovedis.**  

For example,  

    import numpy as np
    from frovedis.linalg import dot
    mat1 = np.array([[1., 9., 8.], [7., 1., 5.], [1., 2., 4.]])
    mat2 = np.array([[4., 1., 7.], [2., 2., 9.], [1., 2., 3.]])
    
    # dot() demo with out != None
    prod = dot(mat1, mat2, out = np.array([[1., 1., 1.],[1., 1., 1.],[1., 1., 1.]]))
    print(prod)

Output  

    [[ 30.  35. 112.]
     [ 35.  19.  73.]
     [ 12.  13.  37.]]

Here, the result will be written in the user provided ndarray and shape and datatype same as input matrix.  

For example,  

    import numpy as np
    from frovedis.linalg import dot
    mat1 = np.array([[1, 9, 8], [7, 1, 5], [1, 2, 4]])
    mat2 = np.array([[4, 1, 7], [2, 2, 9], [1, 2, 3]])
    
    # creating FrovedisBlockcyclicMatrix instance
    from frovedis.matrix.dense import FrovedisBlockcyclicMatrix
    A_Bmat = FrovedisBlockcyclicMatrix(mat1, np.float64)
    B_Bmat = FrovedisBlockcyclicMatrix(mat2, np.float64)
    
    # dot() demo with FrovedisBlockcyclicMatrix instance 
    prod = dot(A_Bmat, B_Bmat)
    prod.debug_print()

Output  

    matrix:
    num_row = 3, num_col = 3
    node 0
    node = 0, local_num_row = 3, local_num_col = 3, type = 2, descriptor = 1 1 3 3 3 3 0 0 3 3 3
    val = 30 35 12 35 19 13 112 73 37

**This output will be only visible on server side terminal.**  

__Return Value__  
It returns the dot product of 'a' and 'b'.  
If 'a' and 'b' are both scalars or both 1-D arrays then a scalar is returned. Otherwise, an array is returned.  
If out is given, then it is returned.  
If either 'a' or 'b' is a FrovedisBlockCyclicMatrix instance, then it returns a FrovedisBlockCyclicMatrix instance.  

### 2. eigsh(A, M = None, k = 6, sigma = None, which = 'LM', v0 = None, ncv = None, maxiter = None, tol = 0., return_eigenvectors = True, Minv = None, OPinv = None, mode = 'normal')  

__Parameters__  
_**A**_: A numpy dense or scipy sparse matrix or any python array-like object or an instance of FrovedisCRSMatrix for sparse data and FrovedisRowmajorMatrix for dense data. The input matrix provided must be squared symmetric in nature.  
_**M**_: This is an unused parameter. (Default: None)  
_**k**_: A positive integer parameter that specifies the number of eigenvalues and eigenvectors desired. It must be in 
range 0 to N, where N is the number of rows in squared symmetric matrix.  (Default: 6)  
_**sigma**_: It accepts a float (float32) type parameter that is used to find eigenvalues near sigma using shift-invert mode. Currently, it is only supported for dense matrices. (Default: None)  
When it is None (not specified explicitly), it will be set as largest possible value of float (float32) type.  
_**which**_: A string object parameter that specifies which **k** eigenvectors and eigenvalues to find:  
- **'LM'** : Largest (in magnitude) eigenvalues. It is the default value.  
- **'SM'** : Smallest (in magnitude) eigenvalues.  
- **'LA'** : Largest (algebraic) eigenvalues.  
- **'SA'** : Smallest (algebraic) eigenvalues.  
- **'BE'** : Half (k/2) from each end of the spectrum.  

When sigma != None, 'which' refers to the shifted eigenvalues.  
_**v0**_: This is an unused parameter. (Default: None)  
_**ncv**_: This is an unused parameter. (Default: None)  
_**maxiter**_: A positive integer that specifies the maximum number of iterations allowed. The convergance may occur 
on or before the maximum iteration limit is reached. In case, the convergance does not occur before the iteration limit, then 
this method will not generate the resultant **'k'** eigenvalues and eigenvectors. (Default: None)  
When it is None (not specified explicitly), it will be set as 10 * N, where N is the number of rows in squared symmetric matrix.  
_**tol**_: A float (float32) type parameter that specifies the tolerance values used to find relative accuracy for 
eigenvalues (stopping criterion). (Default: 0.)  
_**return\_eigenvectors**_: A boolean parameter that specifies whether to return eigenvectors in addition to eigenvalues. (Default: True)  
When it is set to False, it will only return **k** eigenvalues.  
_**Minv**_: This is an unused parameter. (Default: None)  
_**OPinv**_: This is an unused parameter. (Default: None)  
_**mode**_: A string object parameter that specifies the strategy to use for shift-invert mode. This parameter applies 
only when sigma != None. This parameter can be **'normal'**, **'cayley'** or **'buckling' modes**. **Currently, frovedis supports only 
normal mode**. (Default: 'normal')  
The choice of mode will affect which eigenvalues are selected by the 'which' parameter.  

__Purpose__  
It finds **k** eigenvalues and eigenvectors of the symmetric square matrix A.  

The parameters: "M", "vc0", "ncv", "Minv" and "OPinv" are simply kept in to to make the interface uniform to the 
scipy.sparse.linalg.eigsh() module. They are not used anywhere within the frovedis implementation.  

For example,  

    # dense square symmetric matrix
    import numpy as np
    mat = np.asarray([[2.,-1.,0.,0.,-1.,0.], [-1.,3.,-1.,0.,-1.,0.], [0.,-1.,2.,-1.,0.,0.],
                      [0.,0.,-1.,3.,-1.,-1], [-1.,-1.,0.,-1.,3.,0], [0.,0.,0.,-1.,0.,1.]])

    # eigsh() demo
    from frovedis.linalg import eigsh
    eigen_vals, eigen_vecs = eigsh(mat, k = 3)
    print("frovedis computed eigen values:\n")
    print(eigen_vals)
    print("\nfrovedis computed eigen vectors:\n")
    print(eigen_vecs)

Output  

    frovedis computed eigen values:

    [3.         3.70462437 4.89121985]

    frovedis computed eigen vectors:

    [[-0.28867513  0.56702343  0.03232265]
     [-0.28867513 -0.65812747  0.46850031]
     [-0.28867513  0.20514371 -0.35640753]
     [ 0.57735027  0.30843449  0.56195221]
     [ 0.57735027 -0.30843449 -0.56195221]
     [-0.28867513 -0.11403968 -0.14441544]]

Since, input dense matrix has shape (3,3), so 0< k <=3.  

For example,

    # eigsh() demo when sigma != None
    eigen_vals, eigen_vecs = eigsh(mat, k = 3, sigma = -1)
    print("frovedis computed eigen values:\n")
    print(eigen_vals)
    print("\nfrovedis computed eigen vectors:\n")
    print(eigen_vecs)

Output  

    frovedis computed eigen values:

    [0.         0.72158639 1.68256939]

    frovedis computed eigen vectors:

    [[ 0.40824829  0.41486979 -0.50529469]
     [ 0.40824829  0.30944167  0.04026854]
     [ 0.40824829  0.0692328   0.75901025]
     [ 0.40824829 -0.22093352  0.20066454]
     [ 0.40824829  0.22093352 -0.20066454]
     [ 0.40824829 -0.79354426 -0.29398409]]

Here, the resultant eigenvalues and eigenvectors are generated using shift invert mode.  

For example,

    # eigsh() demo where which = 'SM'
    eigen_vals, eigen_vecs = eigsh(mat, k = 3, which = 'SM')
    print("frovedis computed eigen values:\n")
    print(eigen_vals)
    print("\nfrovedis computed eigen vectors:\n")
    print(eigen_vecs)

Output  

    frovedis computed eigen values:
    
    [2.70826973e-16 7.21586391e-01 1.68256939e+00]

    frovedis computed eigen vectors:

    [[ 0.40824829  0.41486979  0.50529469]
     [ 0.40824829  0.30944167 -0.04026854]
     [ 0.40824829  0.0692328  -0.75901025]
     [ 0.40824829 -0.22093352 -0.20066454]
     [ 0.40824829  0.22093352  0.20066454]
     [ 0.40824829 -0.79354426  0.29398409]]

Here, the resultant eigenvalues and eigenvectors are smallest in magnitute for the given dense input matrix.  

For example,

    # eigsh() demo where maxiter = 10
    eigen_vals, eigen_vecs = eigsh(mat, k = 3, maxiter = 10)
    print("frovedis computed eigen values:\n")
    print(eigen_vals)
    print("\nfrovedis computed eigen vectors:\n")
    print(eigen_vecs)

Output  

    frovedis computed eigen values:

    [3.         3.70462437 4.89121985]

    scipy computed eigen vectors:

    [[-0.28867513  0.56702343  0.03232265]
     [-0.28867513 -0.65812747  0.46850031]
     [-0.28867513  0.20514371 -0.35640753]
     [ 0.57735027  0.30843449  0.56195221]
     [ 0.57735027 -0.30843449 -0.56195221]
     [-0.28867513 -0.11403968 -0.14441544]]

The eigenvalues and eigenvectors will be generated only when convergence occurs.  
In above example, this will happen on or before maxiter = 10 is reached.  

For example,

    # eigsh() demo where return_eigenvectors = False
    eigen_vals = eigsh(mat, k = 3, return_eigenvectors = False)
    print("frovedis computed eigen values:\n")
    print(eigen_vals)

Output  

    frovedis computed eigen values:

    [3.         3.70462437 4.89121985]

Here, only eigenvalues are returned by this method.  

__Return Value__    
1. **If return_eigenvectors = True:**  
     - It returns eigenvectors in addition to eigenvalues as numpy ndarrays having double (float64) type values. Also, eigenvectors has shape **(k,1)** and eigenvalues has shape **(M,k)**, where **M** is the number of rows of input matrix 'A'.  
2. **If return_eigenvectors = False:**  
     - It returns only eigenvalues as numpy ndarray having shape **(k,1)** and double (float64) type values.  

### 3. inv(a)  

__Parameters__  
_**a**_: It accepts a python array-like input or numpy matrix (having dimensions <= 2) of int, float (float32) or double (float64) type values. It also accepts FrovedisBlockcyclicMatrix instance having float (float32) or double (float64) type values. It specifies the matrix to be inverted. It must be a square matrix of shape **(M, M)** , where **M** is the number of rows and columns of input squared matrix.  

__Purpose__  
This method computes the inverse matrix of a given matrix. 

Internally it uses **getrf()** and **getri()** methods present in **frovedis.linalg.scalapack** module.  

For example,  

    import numpy as np
    from frovedis.linalg import inv

    # input array contains integer values
    mat = np.array([[1, 2], [3, 4]])
    
    # inv(0 demo with numpy array as input
    matinv = inv(mat)
    print(matinv)

Output  

    [[-2.   1. ]
     [ 1.5 -0.5]]
 
It returns an ndarray as the inverse of given matrix having double (float64) type values.

For example,  

    import numpy as np
    from frovedis.linalg import inv
    
    # input array contains float (float320 type values
    mat = np.array([[1., 2.], [3., 4.]], dtype = np.float32)
    
    # inv() demo with numpy array as input
    matinv = inv(mat)
    print(matinv)

Output  

    [[-2.0000002   1.0000001 ]
     [ 1.5000001  -0.50000006]]
 
It returns an ndarray as the inverse of given matrix having float (float32) type values.

For example,  

    import numpy as np
    from frovedis.linalg import inv

    # input array contains double (float64) type values
    mat = np.array([[1., 2.], [3., 4.]])
    
    # inv(0 demo with numpy array as input
    matinv = inv(mat)
    print(matinv)

Output  

    [[-2.   1. ]
     [ 1.5 -0.5]]
 
It returns an ndarray as the inverse of given matrix having double (float64) type values.

For example,  

    import numpy as np
    from frovedis.linalg import inv
    
    # creating FrovedisBlockcyclicMatrix instance 
    from frovedis.matrix.dense import FrovedisBlockcyclicMatrix
    mat = np.array([[1., 2.], [3., 4.]])
    A_Bmat = FrovedisBlockcyclicMatrix(mat)
    
    # inv() demo FrovedisBlockcyclicMatrix instance
    matinv = inv(A_Bmat)
    matinv.debug_print()

Output  

    matrix:
    num_row = 2, num_col = 2
    node 0
    node = 0, local_num_row = 2, local_num_col = 2, type = 2, descriptor = 1 1 2 2 2 2 0 0 2 2 2
    val = -2 1.5 1 -0.5

**This output will be only visible on server side terminal.**  

__Return Value__  
1. **When numpy array-like input is used**:  
     - It returns a numpy ndarray having float (float32) or double (float64) type values and has shape **(M, M)**, where **M** is number of rows and columns of input ndarray.  
2. **When input is a numpy matrix**:  
     - It returns a numpy matrix having float (float32) or double (float64) type values and has shape **(M, M)**, where **M** is number of rows and columns of input matrix.  
3. **When input is a FrovedisBlockCyclicMatrix instance**:  
     - It returns an instance of FrovedisBlockCyclicMatrix.  

### 4. matmul(x1, x2, out = None, casting = 'same_kind', order = 'K', dtype = None, subok = True, signature = None, extobj = None)  

__Parameters__  
_**x1**_: It accepts a python array-like input or numpy maxtrix (having dimensions <= 2) of int, float (float32) or double (float64) type values. It also accepts FrovedisBlockcyclicMatrix instance having float (float32) or double (float64) type values. It must not be scalar values.  
_**x2**_: It accepts scalar values, python array-like inputs or numpy maxtrix (having dimensions <= 2) of int, float (float32) or double (float64) type values. It also accepts FrovedisBlockcyclicMatrix instance having float (float32) or double (float64) type values. It must not be scalar values.  
_**out**_: A numpy ndarray of int, float or double (float64) type values which is where the result is written. (Default: None)  
When it is None (not specified explicitly), a freshly-allocated array having double (float64) type values is returned. Otherwise if provided, it must have a shape that matches the signature (n,k),(k,m)->(n,m) and same datatype as input matrices.  
_**casting**_: This is an unused parameter. (Default: 'same_kind')  
_**order**_: This is an unused parameter. (Default: 'K')  
_**dtype**_: This is an unused parameter. (Default: None)  
_**subok**_: This is an unused parameter. (Default: True)  
_**signature**_: This is an unused parameter. (Default: None)  
_**extobj**_: This is an unused parameter. (Default: None)  

__Purpose__  
It computes matrix product of two arrays.  

The parameters: "casting", "order", "dtype", "subok" and "signature" and "extobj" are simply kept in to to make the 
interface uniform to the numpy.matmul() module. They are not used anywhere within the frovedis implementation.  

For example,  

    import numpy as np
    from frovedis.linalg import matmul
    
    # inputs are having integer type values
    mat1 = np.array([[1, 9, 8], [7, 1, 5], [1, 2, 4]])
    mat2 = np.array([[4, 1, 7], [2, 2, 9], [1, 2, 3]])
    
    # matmul() demo with numpy array as input
    prod = matmul(mat1, mat2)
    print(prod)

Output  

    [[ 30.  35. 112.]
     [ 35.  19.  73.]
     [ 12.  13.  37.]]

Here, the result will be written in a newly generated array having double (float64) type values.  

**For inputs 'mat1' and 'mat2', the column size of 'mat1' must be same as row size of 'mat2' before computing matrix multiplication.**  

For example,  

    import numpy as np
    from frovedis.linalg import matmul
    
    # inputs are having double (float64) type values
    mat1 = np.array([[1.5, 9, 8], [7, 1, 5.5], [1, 2.5, 4]])
    mat2 = np.array([[4, 1, 7.5], [2.5, 2, 9], [1, 2, 3.5]])
    
    # matmul() demo with numpy array as input
    prod = matmul(mat1, mat2)
    print(prod)

Output  

    [[ 36.5   35.5  120.25]
     [ 36.    20.    80.75]
     [ 14.25  14.    44.  ]]

Here, the result will be written in a newly generated array having double (float64) type values.  

**For inputs having float (float32) type values, the newly generated array will also have float (float32) type values.**

For example,  

    import numpy as np
    from frovedis.linalg import matmul
    
    mat1 = np.array([[1., 9., 8.], [7., 1., 5.], [1., 2., 4.]])
    mat2 = np.array([[4., 1., 7.], [2., 2., 9.], [1., 2., 3.]])
    
    # matmul() demo with out != None
    prod = matmul(mat1, mat2, out = np.array([[1.,1.,1.],[1.,1.,1.],[1.,1.,1.]]))
    print(prod)

Output  

    [[ 30.  35. 112.]
     [ 35.  19.  73.]
     [ 12.  13.  37.]]

Here, the result will be written in the user provided ndarray.  

For example,  

    import numpy as np
    from frovedis.linalg import matmul
    mat1 = np.array([[1, 9, 8], [7, 1, 5], [1, 2, 4]])
    mat2 = np.array([[4, 1, 7], [2, 2, 9], [1, 2, 3]])
    
    # creating FrovedisBlockcyclicMatrix instance
    from frovedis.matrix.dense import FrovedisBlockcyclicMatrix
    A_Bmat = FrovedisBlockcyclicMatrix(mat1, np.float64)
    B_Bmat = FrovedisBlockcyclicMatrix(mat2, np.float64)
    
    # matmul() demo with FrovedisBlockcyclicMatrix instance 
    prod = matmul(A_Bmat, B_Bmat)
    prod.debug_print()

Output  

    matrix:
    num_row = 3, num_col = 3
    node 0
    node = 0, local_num_row = 3, local_num_col = 3, type = 2, descriptor = 1 1 3 3 3 3 0 0 3 3 3
    val = 30 35 12 35 19 13 112 73 37

**This output will be only visible on server side terminal.**  

__Return Value__  
1. **If any python array like input (having dimensions <= 2) and out = None:**  
     -  It returns a numpy ndarray as output and having float (float32) or double (float64) type values.  
2. **If any python array like input (having dimensions <= 2) and out != None:**  
     -  It returns a numpy ndarray as output. Datatype for output values will be same as datatype of input 'x1' such as int, float (float32) or double (float64) type.  
3. **If numpy matrix (having dimensions <= 2) and out = None:**  
     -  It returns a numpy matrix as output and having float (float32) or double (float64) type values.  
4. **If numpy matrix (having dimensions <= 2) and out != None:**  
     -  It returns a numpy matrix as output. Datatype for output values will be same as datatype of input 'x1' such as int, float (float32) or double (float64) type.  
5. **If inputs are FrovedisBlockCyclicMatrix instances:**  
     - It returns a FrovedisBlockCyclicMatrix instance.  

### 5. solve(a, b)  

__Parameters__  
_**a**_: It accepts a python array-like input or numpy maxtrix (having dimensions <= 2) of int, float (float32) or double (float64) type values. It also accepts FrovedisBlockcyclicMatrix instance having float (float32) or double (float64) type values. It specifies a coefficient matrix. It must be a square matrix of shape **(nRows, nCols)** , where **nRows = nCols**.  
_**b**_: It accepts a python array-like inputs or numpy matrix (having dimensions <= 2) of int, float (float32) or double (float64) type values. It also accepts FrovedisBlockcyclicMatrix instance having float (float32) or double (float64) type values. It specifies the variables to be determined while solving the equation 'Ax = B'.   

__Purpose__  
This function solves a linear matrix equation Ax = B, or system of linear scalar equations.  

Internally it uses **gesv()** method present in **frovedis.linalg.scalapack** module.  

For example,  

    import numpy as np
    from frovedis.linalg import solve
    
    # inputs have integer type values
    A = np.array([[3,4,3],[1,2,3],[4,2,1]])
    B = np.array([[1,1,1], [3,0,2], [8,2,5]])
    
    #solve() demo with numpy array input
    X = solve(A,B)
    print(X)

Output  

    [[ 3.57142857  0.57142857  2.14285714]
     [-4.57142857 -0.07142857 -2.64285714]
     [ 2.85714286 -0.14285714  1.71428571]]

Here, it returns an array with solution matrix having double (float64) type values.  

For example,  

    import numpy as np
    from frovedis.linalg import solve
    
    # inputs have float (float32) type values
    A = np.array([[3,4,3],[1,2,3],[4,2,1]], np.float32)
    B = np.array([[1,1,1], [3,0,2], [8,2,5]], np.float32)
    
    #solve() demo with numpy array input
    X = solve(A,B)
    print(X)

Output  

    [[ 3.5714288   0.57142854  2.142857  ]
     [-4.571429   -0.07142858 -2.6428573 ]
     [ 2.857143   -0.14285713  1.7142859 ]]

Here, it returns an array with solution matrix having float (float32) type values.  

For example,  

    import numpy as np
    from frovedis.linalg import solve
    
    # inputs have integer type values
    A = np.array([[3.,4.,3.],[1.,2.,3.],[4.,2.,1.]])
    B = np.array([[1.,1.,1.], [3.,0.,2.], [8.,2.,5.]])
    
    #solve() demo with numpy array input
    X = solve(A,B)
    print(X)

Output  

    [[ 3.57142857  0.57142857  2.14285714]
     [-4.57142857 -0.07142857 -2.64285714]
     [ 2.85714286 -0.14285714  1.71428571]]

Here, it returns an array with solution matrix having double (float64) type values.  

For example,  

    import numpy as np
    from frovedis.linalg import solve
    A = np.array([[3,4,3],[1,2,3],[4,2,1]])
    B = np.array([[1,1,1], [3,0,2], [8,2,5]])
    
    #creating FrovedisBlockCyclicMatrix instance
    from frovedis.matrix.dense import FrovedisBlockcyclicMatrix
    A_Bmat = FrovedisBlockcyclicMatrix(A, np.float64)
    B_Bmat = FrovedisBlockcyclicMatrix(B, np.float64)
    
    #solve() demo with FrovedisBlockCyclicMatrix input
    X = solve(A_Bmat,B_Bmat)
    X.debug_print()

Output  

    matrix:
    num_row = 3, num_col = 3
    node 0
    node = 0, local_num_row = 3, local_num_col = 3, type = 2, descriptor = 1 1 3 3 3 3 0 0 3 3 3
    val = 3.57143 -4.57143 2.85714 0.571429 -0.0714286 -0.142857 2.14286 -2.64286 1.71429

It stores result as a FrovedisBlockCyclicMatrix instance.  

**Also, this output will be only visible on server side terminal.**  

__Return Value__  
1. **When python array-like input is used**:  
     - It returns a numpy ndarray having float (float32) or double (float64) type values and shape **(nRows, nCols)**, where **nRows** is number of rows and **nCols** is number of columns of resultant array.  
2. **When input is numpy matrix**:  
     - It returns a numpy matrix having float (float32) or double (float64) type values and shape **(nRows, nCols)**, where **nRows** is number of rows and **nCols** is number of columns of resultant matrix.  
3. **When input is a FrovedisBlockCyclicMatrix instance**:  
     - It returns an instance of FrovedisBlockCyclicMatrix.  

### 6. svd(a, full_matrices = False, compute_uv = True)  

__Parameters__  
_**a**_: It accepts python array-like inputs or numpy matrix (having dimensions <= 2) containing int, float (float32) or double (float64) type values. It also accepts FrovedisBlockcyclicMatrix instance having float (float32) or double (float64) type values.  
_**full\_matrices**_: It is a boolean parameter. Currently, it can only be set as False. Also, the unitary 2D arrays 'u' and 'vh' 
shapes are (M, K) and (K, N), respectively, where **K = min(M, N)**, **M** is the number of rows and **N** is the number of columns of 
input matrix. (Default: False)  
_**compute\_uv**_: It is a boolean parameter that specifies whether or not to compute unitary arrays 'u' and 'vh' in 
addition to 1D array of singular values 's'. (Default: True)  

__Purpose__  
It computes the factor of an array by Singular Value Decomposition.  

In a nutshell, this method performs factorization of matrix into three matrices 'u', 'vh' and 's'.  
Here,  
- **u** and **vh** are orthogonal matrices.  
- **s** is a diagonal matrix of singular values.  

This method internally **gesvd()** method present in **frovedis.linalg.scalapack** module.  

For example,  

    import numpy as np
    from frovedis.linalg import svd
    
    # input is having integer values
    mat1 = np.array([[3,4,3],[1,2,3],[4,2,1]])
    
    # svd() demo with numpy 2D array
    u, s, vh = svd(mat1)
    print(u)
    print(s)
    print(vh)

Output  

    [[-0.73553325 -0.18392937 -0.65204358]
     [-0.42657919 -0.62196982  0.65664582]
     [-0.52632788  0.76113306  0.37901904]]

    [7.87764972 2.54031671 0.69958986]

    [[-0.60151068  0.73643349  0.30959751]
     [-0.61540527 -0.18005275 -0.76737042]
     [-0.5093734  -0.65210944  0.5615087 ]]

Here it returns 'u', 's' and 'vh' as arrays having double (float64) dtype values.  

For example,  

    import numpy as np
    from frovedis.linalg import svd

    # input is having float (float32) type values
    mat1 = np.array([[3.,4.,3.],[1.,2.,3.],[4.,2.,1.]], dtype = np.float32)
    
    # svd() demo with numpy 2D array
    u, s, vh = svd(mat1)
    print(u)
    print(s)
    print(vh)

Output  

    [[-0.7355336  -0.18392955 -0.65204364]
     [-0.4265793  -0.62197     0.656646  ]
     [-0.5263281   0.7611333   0.3790189 ]]
    [7.8776484 2.540317  0.69959  ]
    [[-0.60151064  0.73643357  0.30959752]
     [-0.6154053  -0.1800527  -0.76737046]
     [-0.5093734  -0.6521094   0.5615088 ]]

Here it returns 'u', 's' and 'vh' as arrays having float (float32) dtype values.  

For example,  

    import numpy as np
    from frovedis.linalg import svd

    # input is having double (float64) type values
    mat1 = np.array([[3.,4.,3.],[1.,2.,3.],[4.,2.,1.]])
    
    # svd() demo with numpy 2D array
    u, s, vh = svd(mat1)
    print(u)
    print(s)
    print(vh)

Output  

    [[-0.73553325 -0.18392937 -0.65204358]
     [-0.42657919 -0.62196982  0.65664582]
     [-0.52632788  0.76113306  0.37901904]]

    [7.87764972 2.54031671 0.69958986]

    [[-0.60151068  0.73643349  0.30959751]
     [-0.61540527 -0.18005275 -0.76737042]
     [-0.5093734  -0.65210944  0.5615087 ]]

Here it returns 'u', 's' and 'vh' as arrays having double (float64) dtype values.  

For example,  

    import numpy as np
    from frovedis.linalg import svd
    mat1 = np.array([[3,4,3],[1,2,3],[4,2,1]])
    
    # svd() demo with compute_uv = False
    s = svd(mat1, compute_uv = False)
    print(s)

Output  

    [7.87764972 2.54031671 0.69958986]

Here, it returns only 's' as an array having double (float64) type values.  

For example,  

    import numpy as np
    from frovedis.linalg import svd
    mat1 = np.array([[3,4,3],[1,2,3],[4,2,1]])
    
    # create FrovedisBlockcyclicMatrix instance
    from frovedis.matrix.dense import FrovedisBlockcyclicMatrix
    mat2 = FrovedisBlockcyclicMatrix(mat1, np.float64)
    
    # svd() demo with FrovedisBlockcyclicMatrix object as input
    u, s, vh = svd(mat2)
    u.debug_print()
    s.debug_print()
    vh.debug_print()

Output  

    matrix:
    num_row = 3, num_col = 3
    node 0
    node = 0, local_num_row = 3, local_num_col = 3, type = 2, descriptor = 1 1 3 3 3 3 0 0 3 3 3
    val = -0.735533 -0.426579 -0.526328 -0.183929 -0.62197 0.761133 -0.652044 0.656646 0.379019

    vector:
    [7.87764972 2.54031671 0.69958986]

    matrix:
    num_row = 3, num_col = 3
    node 0
    node = 0, local_num_row = 3, local_num_col = 3, type = 2, descriptor = 1 1 3 3 3 3 0 0 3 3 3
    val = -0.601511 -0.615405 -0.509373 0.736433 -0.180053 -0.652109 0.309598 -0.76737 0.561509

**This output will be only visible on server side terminal.**  

For example,  

    import numpy as np
    from frovedis.linalg import svd
    mat1 = np.array([[3,4,3],[1,2,3],[4,2,1]])
    
    # create FrovedisBlockcyclicMatrix instance
    from frovedis.matrix.dense import FrovedisBlockcyclicMatrix
    mat2 = FrovedisBlockcyclicMatrix(mat1, np.float64)
    
    # svd() demo with FrovedisBlockcyclicMatrix object as input and compute_cv = False
    s = svd(mat2, compute_uv = False)
    s.debug_print()

Output  

    vector:
    [7.87764972 2.54031671 0.69958986]

**This output will be only visible on server side terminal.**  

__Return Value__  
1. **When input is numpy array**:  
     - If compute_cv = True, then it returns a tuple **(u, s, vh)**, where **u**, **s** and **vh** are numpy ndarrays having float (float32) or double (float64) type values.  
     - If compute_cv = False, then it returns only array of singular values **s** of float (float32) or double (float64) type.  
2. **When input is numpy matrix**:  
     - If compute_cv = True, then it returns a tuple **(u, s, vh)**, where **u**, **vh** are matrices and **s** is a numpy ndarray having float (float32) or double (float64) type values.  
     - If compute_cv = False, then it returns only array of singular values **s** of float (float32) or double (float64) type.  
3. **When input is a FrovedisBlockcyclicMatrix instance**:  
     - If compute_cv = True, then it returns a tuple **(u, s, vh)**, where **u**, **vh** are FrovedisBlockcyclicMatrix instances and **s** is FrovedisVector instance.  
     - If compute_cv = False, then it returns **s** is an instance of FrovedisVector.  

# SEE ALSO  
- **[Scalapack Functions](./scalapack.md)**