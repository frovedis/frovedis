% Manual of Frovedis Python API

# Introduction

This manual contains Python API documantation.
If you are new to Frovedis, please read the tutorial_python first.

Currently we only provide part of the API documentation.
We are still updating the contents.

- Matrix
    + [FrovedisDvector] 
    + [FrovedisCRSMatrix]
    + [FrovedisBlockcyclicMatrix]
    + [pblas_wrapper]
    + [scalapack_wrapper]
	+ [arpack_wrapper]
    + [getrf_result]
    + [gesvd_result]
- Machine Learning
    + [Linear Regression]
    + [Lasso Regression]
    + [Ridge Regression]
    + [Logistic Regression]
    + [Linear SVM]
    + [Matrix Factorization using ALS]
    + [kmeans]


# FrovedisDvector

## NAME

FrovedisDvector -  A data structure used in modeling the in-memory 
dvector data of frovedis server side at client python side. 

## SYNOPSIS

class frovedis.matrix.dvector.FrovedisDvector(vec=None)    

### Public Member Functions
load (vec)   
load_numpy_array (vec)     
debug_print()   
release()   

## DESCRIPTION

FrovedisDvector is a pseudo data structure at client python side 
which aims to model the frovedis server side `dvector<double>` 
(see manual of frovedis dvector for details).   

Note that the actual vector data is created at frovedis server side only. 
Python side FrovedisDvector contains a proxy handle of the in-memory vector 
data created at frovedis server, along with its size.   

### Constructor Documentation

#### FrovedisDvector (vec=None)   
__Parameters__    
_vec_: It can be any python array-like object or None. 
In case of None (Default), it does not make any request to server.   

__Purpose__    

This constructor can be used to construct a FrovedisDvector instance, 
as follows:

    v1 = FrovedisDvector()          # empty dvector, no server request is made   
    v2 = FrovedisDvector([1,2,3,4]) # will load data from the given list   

__Return Type__    

It simply returns "self" reference.   

### Pubic Member Function Documentation

#### load (vec)   
__Parameters__    
_vec_: It can be any python array-like object (but not None). 

__Purpose__    

This function works similar to the constructor. 
It can be used to load a FrovedisDvector instance, as follows:

    v = FrovedisDvector().load([1,2,3,4]) # will load data from the given list   

__Return Type__    

It simply returns "self" reference.   


#### load_numpy_array (vec)   
__Parameters__    
_vec_:  Any numpy array with values to be loaded in.   

__Purpose__    
This function can be used to load a python side numpy array data into 
frovedis server side dvector. It accepts a python numpy array object 
and converts it into the frovedis server side dvector whose proxy 
along size information are stored in the target FrovedisDvector object.

__Return Type__    
It simply returns "self" reference.   

#### size()
__Purpose__   
It returns the size of the dvector

__Return Type__   
An integer value containing size of the target dvector.   
 
#### debug_print()  
__Purpose__    
It prints the contents of the server side distributed vector data on the server 
side user terminal. It is mainly useful for debugging purpose.

__Return Type__   
It returns nothing.    

#### release()  
__Purpose__    
This function can be used to release the existing in-memory data at frovedis 
server side.

__Return Type__   
It returns nothing.    

#### FrovedisDvector.asDvec(vec)
__Parameters__    
_vec_: A numpy array or python array like object or an instance of FrovedisDvector.    

__Purpose__   
This static function is used in order to convert a given array to a dvector.
If the input is already an instance of FrovedisDvector, then the same will be 
returned. 

__Return Type__   

An instance of FrovedisDvector.


# FrovedisCRSMatrix 

## NAME

FrovedisCRSMatrix -  A data structure used in modeling the in-memory 
crs matrix data of frovedis server side at client python side. 

## SYNOPSIS

class frovedis.matrix.sparse.FrovedisCRSMatrix(mat=None)    

### Public Member Functions
load (mat)    
load_scipy_matrix (mat)   
load_text (filename)   
load_binary (dirname)   
save_text (filename)    
save_binary (dirname)    
debug_print()   
release()   

## DESCRIPTION

FrovedisCRSMatrix is a pseudo matrix structure at client python side 
which aims to model the frovedis server side `crs_matrix<double>` 
(see manual of frovedis crs_matrix for details).   

Note that the actual matrix data is created at frovedis server side only. 
Python side FrovedisCRSMatrix contains a proxy handle of the in-memory matrix 
data created at frovedis server, along with number of rows and number of columns 
information.

### Constructor Documentation

#### FrovedisCRSMatrix (mat=None)   
__Parameters__    
_mat_: It can be a string containing filename having text data to be loaded, 
or any scipy sparse matrix or any python array-like object or None. 
In case of None (Default), it does not make any request to server.   

__Purpose__    

This constructor can be used to construct a FrovedisCRSMatrix instance, 
as follows:

    mat1 = FrovedisCRSMatrix() # empty matrix, no server request is made   
    mat2 = FrovedisCRSMatrix("./data") # will load data from given text file   
    mat3 = FrovedisCRSMatrix([1,2,3,4]) # will load data from the given list   

__Return Type__    

It simply returns "self" reference.   

### Pubic Member Function Documentation

#### load (mat)   
__Parameters__    
_mat_: It can be a string containing filename having text data to be loaded, 
or any scipy sparse matrix or any python array-like object (but it can not 
be None). 

__Purpose__    

This works similar to the constructor.  
It can be used to load a FrovedisCRSMatrix instance, as follows:

    mat1 = FrovedisCRSMatrix().load("./data")  # will load data from given text file   
    mat2 = FrovedisCRSMatrix().load([1,2,3,4]) # will load data from the given list   

__Return Type__    

It simply returns "self" reference.   


#### load_scipy_matrix (mat)   
__Parameters__    
_mat_:  Any scipy matrix with values to be loaded in.   

__Purpose__    
This function can be used to load a python side scipy sparse data matrix into 
frovedis server side crs matrix. It accepts a scipy sparse matrix object 
and converts it into the frovedis server side crs matrix whose proxy 
along with number of rows and number of columns information are stored in 
the target FrovedisCRSMatrix object.

__Return Type__    
It simply returns "self" reference.   
 
#### load_text (filename)   
__Parameters__    
_filename_:  A string object containing the text file name to be loaded.   

__Purpose__    
This function can be used to load the data from a text file into the target 
matrix. Note that the file must be placed at server side 
at the given path and it should have contents stored in libSVM format, i.e., 
"column_index:value" at each row (see frovedis manual of make_crs_matrix_load() 
for more details).   

__Return Type__    
It simply returns "self" reference.   

#### load_binary (dirname)   
__Parameters__    
_dirname_:  A string object containing the directory name having the binary 
data to be loaded.   

__Purpose__    
This function can be used to load the data from the specified directory with 
binary data file into the target matrix. Note that the file must be placed at 
server side at the given path. 

__Return Type__    
It simply returns "self" reference.   

#### save_text (filename)   
__Parameters__    
_filename_:  A string object containing the text file name in which the data 
is to be saveed.   

__Purpose__    

This function is used to save the target matrix as text file with the filename 
at the given path. Note that the file will be saved at server side 
at the given path.  

__Return Type__    
It returns nothing.    

#### save_binary (dirname) 
__Parameters__    
_dirname_:  A string object containing the directory name in which the data 
is to be saveed as little-endian binary form.   

__Purpose__    

This function is used to save the target matrix as little-endian binary file 
with the filename at the given path. Note that the file will be saved at 
server side at the given path.  

__Return Type__    
It returns nothing.    

#### numRows()
__Purpose__   
It returns the number of rows in the matrix

__Return Type__   
An integer value containing rows count in the target matrix.      

#### numCols()
__Purpose__   
It returns the number of columns in the matrix

__Return Type__   
An integer value containing columns count in the target matrix.      


#### debug_print()  
__Purpose__    
It prints the contents of the server side distributed matrix data on the server 
side user terminal. It is mainly useful for debugging purpose.

__Return Type__   
It returns nothing.    

#### release()  
__Purpose__    
This function can be used to release the existing in-memory data at frovedis 
server side.

__Return Type__   
It returns nothing.    

#### FrovedisCRSMatrix.asCRS(mat)
__Parameters__    
_mat_: A scipy matrix, an instance of FrovedisCRSMatrix or any python array-like data.   

__Purpose__   
This static function is used in order to convert a given matrix to a crs matrix.
If the input is already an instance of FrovedisCRSMatrix, then the same will be 
returned. 

__Return Type__   

An instance of FrovedisCRSMatrix.


# FrovedisBlockcyclicMatrix 

## NAME

FrovedisBlockcyclicMatrix -  A data structure used in modeling the in-memory 
blockcyclic matrix data of frovedis server side at client python side. 

## SYNOPSIS

class frovedis.matrix.dense.FrovedisBlockcyclicMatrix(mat=None)    

### Overloaded Operators   
operator= (mat)   
operator+ (mat)   
operator- (mat)   
operator* (mat)   
operator~ (mat)   

### Public Member Functions
load (mat)   
load_numpy_matrix (mat)   
load_text (filename)   
load_binary (dirname)   
save_text (filename)    
save_binary (dirname)    
transpose()   
to_numpy_matrix ()   
debug_print()   
release()   

## DESCRIPTION

FrovedisBlockcyclicMatrix is a pseudo matrix structure at client python side 
which aims to model the frovedis server side `blockcyclic_matrix<double>` 
(see manual of frovedis blockcyclic_matrix for details).   

Note that the actual matrix data is created at frovedis server side only. 
Python side FrovedisBlockcyclicMatrix contains a proxy handle of the in-memory matrix 
data created at frovedis server, along with number of rows and number of columns 
information.

### Constructor Documentation
#### FrovedisBlockcyclicMatrix (mat=None)   
__Parameters__    
_mat_: It can be a string containing filename having text data to be loaded, 
or another FrovedisBlockcyclicMatrix instance for copy or any python array-like 
object or None. In case of None (Default), it does not make any request to server.   

__Purpose__    

This constructor can be used to construct a FrovedisBlockcyclicMatrix instance, 
as follows:

    mat1 = FrovedisBlockcyclicMatrix() # empty matrix, no server request is made   
    mat2 = FrovedisBlockcyclicMatrix("./data") # will load data from given text file   
    mat3 = FrovedisBlockcyclicMatrix(mat2) # copy constructor   
    mat4 = FrovedisBlockcyclicMatrix([1,2,3,4]) # will load data from the given list   

__Return Type__	    

It simply returns "self" reference.   

### Overloaded Operators Documentation

#### operator= (mat)

__Parameters__    
_mat_: An existing FrovedisBlockcyclicMatrix instance to be copied.    

__Purpose__    
It can be used to copy the input matrix in the target matrix.
It returns a self reference to support operator chaining.  

For example,

    m1 = FrovedisBlockcyclicMatrix([1,2,3,4])    
    m2 = m1 (copy operatror)
    m3 = m2 = m1   

__Return Type__    
It returns "self" reference.

#### operator+ (mat)

__Parameters__    
_mat_: An instance of FrovedisBlockcyclicMatrix or an array-like structure.   

__Purpose__    
It can be used to perform addition between two blockcyclic matrices. 
If the input data is not a FrovedisBlockcyclicMatrix instance, internally it 
will get converted into a FrovedisBlockcyclicMatrix instance first and then 
that will be added with the source matrix. 

For example,

    m1 = FrovedisBlockcyclicMatrix([1,2,3,4])    
    m2 = FrovedisBlockcyclicMatrix([1,2,3,4]) 
    m3 = m2 + m1 

__Return Type__    
It returns the resultant matrix of the type FrovedisBlockcyclicMatrix. 

#### operator- (mat)

__Parameters__    
_mat_: An instance of FrovedisBlockcyclicMatrix or an array-like structure.   

__Purpose__    
It can be used to perform subtraction between two blockcyclic matrices. 
If the input data is not a FrovedisBlockcyclicMatrix instance, internally it 
will get converted into a FrovedisBlockcyclicMatrix instance first and then 
that will be subtracted from the source matrix. 

For example,

    m1 = FrovedisBlockcyclicMatrix([1,2,3,4])    
    m2 = FrovedisBlockcyclicMatrix([1,2,3,4]) 
    m3 = m2 - m1 

__Return Type__    
It returns the resultant matrix of the type FrovedisBlockcyclicMatrix. 

#### operator\* (mat)

__Parameters__    
_mat_: An instance of FrovedisBlockcyclicMatrix or an array-like structure.   

__Purpose__    
It can be used to perform multiplication between two blockcyclic matrices. 
If the input data is not a FrovedisBlockcyclicMatrix instance, internally it 
will get converted into a FrovedisBlockcyclicMatrix instance first and then 
that will be multiplied with the source matrix. 

For example,

    m1 = FrovedisBlockcyclicMatrix([1,2,3,4])    
    m2 = FrovedisBlockcyclicMatrix([1,2,3,4]) 
    m3 = m2 * m1 

__Return Type__    
It returns the resultant matrix of the type FrovedisBlockcyclicMatrix. 

#### operator\~ ()

__Purpose__    
It can be used to obtain transpose of the target matrix.
If the input data is not a FrovedisBlockcyclicMatrix instance, internally it 
will get converted into a FrovedisBlockcyclicMatrix instance first and then 
the transpose will get computed. 

For example,

    m1 = FrovedisBlockcyclicMatrix([1,2,3,4])    
    m2 = ~m1 

__Return Type__    
It returns the resultant matrix of the type FrovedisBlockcyclicMatrix. 

### Pubic Member Function Documentation

#### load (mat)   
__Parameters__    
_mat_: It can be a string containing filename having text data to be loaded, 
or another FrovedisBlockcyclicMatrix instance for copy or any python array-like 
object (but it can not be None).

__Purpose__    

This function works similar to the constructor. 
It can be used to load a FrovedisBlockcyclicMatrix instance, as follows:

    mat1 = FrovedisBlockcyclicMatrix().load("./data")  # will load data from given text file   
    mat2 = FrovedisBlockcyclicMatrix().load(mat1)      # copy operation   
    mat3 = FrovedisBlockcyclicMatrix().load([1,2,3,4]) # will load data from the given list   

__Return Type__	    

It simply returns "self" reference.   

#### load_numpy_matrix (mat)   
__Parameters__    
_mat_:  A numpy matrix with values to be loaded in.   

__Purpose__    
This function can be used to load a python side dense data matrix into a frovedis 
server side blockcyclic matrix. It accepts a numpy matrix object 
and converts it into the frovedis server side blockcyclic matrix whose proxy 
along with number of rows and number of columns information are stored in 
the target FrovedisBlockcyclicMatrix object.

__Return Type__    
It simply returns "self" reference.   
 
#### load_text (filename)   
__Parameters__    
_filename_:  A string object containing the text file name to be loaded.   

__Purpose__    
This function can be used to load the data from a text file into the target 
matrix. Note that the file must be placed at server side 
at the given path. 

__Return Type__    
It simply returns "self" reference.   

#### load_binary (dirname)   
__Parameters__    
_dirname_:  A string object containing the directory name having the binary 
data to be loaded.   

__Purpose__    
This function can be used to load the data from the specified directory with 
binary data file into the target matrix. Note that the file must be placed at server side 
at the given path. 

__Return Type__    
It simply returns "self" reference.   

#### save_text (filename)   
__Parameters__    
_filename_:  A string object containing the text file name in which the data 
is to be saved.   

__Purpose__    

This function is used to save the target matrix as text file with the filename 
at the given path. Note that the file will be saved at server side 
at the given path.  

__Return Type__    
It returns nothing.    

#### save_binary (dirname) 
__Parameters__    
_dirname_:  A string object containing the directory name in which the data 
is to be saved as little-endian binary form.   

__Purpose__    

This function is used to save the target matrix as little-endian binary file 
with the filename at the given path. Note that the file will be saved at 
server side at the given path.  

__Return Type__    
It returns nothing.    

#### transpose () 
__Purpose__    

This function will compute the transpose of the given matrix. 

__Return Type__    
It returns the transposed blockcyclic matrix of the type FrovedisBlockcyclicMatrix.

#### to_numpy_matrix ()   

__Purpose__   
This function is used to convert the target blockcyclic matrix into numpy matrix.  
Note that this function will request frovedis server to gather the distributed data, 
and send back that data in the rowmajor array form and the python client
will then convert the received numpy array from frovedis server to python 
numpy matrix. 

__Return Type__    
It returns a two-dimensional dense numpy matrix    

#### numRows()
__Purpose__   
It returns the number of rows in the matrix

__Return Type__   
An integer value containing rows count in the target matrix.      

#### numCols()
__Purpose__   
It returns the number of columns in the matrix

__Return Type__   
An integer value containing columns count in the target matrix.      

#### debug_print()  
__Purpose__   
It prints the contents of the server side distributed matrix data on the server 
side user terminal. It is mainly useful for debugging purpose.

__Return Type__    
It returns nothing.    

#### release()  
__Purpose__   
This function can be used to release the existing in-memory data at frovedis 
server side.

__Return Type__   
It returns nothing.    

#### FrovedisBlockcyclicMatrix.asBCM(mat)
__Parameters__      
_mat_: An instance of FrovedisBlockcyclicMatrix or any python array-like structure.   

__Purpose__   
This static function is used in order to convert a given matrix to a blockcyclic 
matrix. If the input is already an instance of FrovedisBlockcyclicMatrix, then 
the same will be returned. 

__Return Type__   

An instance of FrovedisBlockcyclicMatrix.   


# pblas_wrapper

## NAME
pblas_wrapper - a frovedis module provides user-friendly interfaces for commonly 
used pblas routines in scientific applications like machine learning algorithms.

## SYNOPSIS

import frovedis.matrix.wrapper.PBLAS    

### Public Member Functions
PBLAS.swap (v1, v2)  
PBLAS.copy (v1, v2)  
PBLAS.scal (v,  al)     
PBLAS.axpy (v1, v2, al=1.0)       
PBLAS.dot  (v1, v2)  
PBLAS.nrm2 (v)  
PBLAS.gemv (m, v1, v2, trans=False, al=1.0, b2=0.0)     
PBLAS.ger  (v1, v2, m, al=1.0)  
PBLAS.gemm (m1, m2, m3, trans_m1=False, trans_m2=False, al=1.0, be=0.0)      
PBLAS.geadd (m1, m2, trans=False, al=1.0, be=1.0)   

## DESCRIPTION
PBLAS is a high-performance scientific library written in Fortran language. It 
provides rich set of functionalities on vectors and matrices. The computation 
loads of these functionalities are parallelized over the available processes 
in a system and the user interfaces of this library is very detailed and complex 
in nature. It requires a strong understanding on each of the input parameters, 
along with some distribution concepts. 

Frovedis provides a wrapper module for some commonly used PBLAS subroutines in 
scientific applications like machine learning algorithms. These wrapper 
interfaces are very simple and user needs not to consider all the detailed 
distribution parameters. Only specifying the target vectors or matrices with 
some other parameters (depending upon need) are fine. At the same time, all the 
use cases of a PBLAS routine can also be performed using Frovedis PBLAS wrapper 
of that routine.

This python module implements a client-server application, where the python client 
can send the python matrix data to frovedis server side in order to create 
blockcyclic matrix at frovedis server and then python client can request frovedis 
server for any of the supported PBLAS operation on that matrix. When required, 
python client can request frovedis server to send back the resultant matrix 
and it can then create equivalent python data.

The individual detailed descriptions can be found in the subsequent sections. 
Please note that the term "inout", used in the below section indicates a 
function argument as both "input" and "output".

### Detailed Description 
#### swap (v1, v2)  
__Parameters__  
_v1_: A FrovedisBlockcyclicMatrix with single column (inout)   
_v2_: A FrovedisBlockcyclicMatrix with single column (inout)   

__Purpose__    
It will swap the contents of v1 and v2, 
if they are semantically valid and are of same length.

__Return Value__   
On success, it returns nothing. If any error occurs, it throws an exception.   

#### copy (v1, v2)
__Parameters__   
_v1_: A FrovedisBlockcyclicMatrix with single column (input)   
_v2_: A FrovedisBlockcyclicMatrix with single column (output)   

__Purpose__   
It will copy the contents of v1 in v2 (v2 = v1), 
if they are semantically valid and are of same length.   

__Return Value__   
On success, it returns nothing. If any error occurs, it throws an exception.   

#### scal (v, al)
__Parameters__   
_v_: A FrovedisBlockcyclicMatrix with single column (inout)   
_al_: A double parameter to specify the value to which the 
input vector needs to be scaled. (input)    

__Purpose__   
It will scale the input vector with the provided "al" value, 
if it is semantically valid. On success, input vector "v" 
would be updated (in-place scaling).   

__Return Value__   
On success, it returns nothing. If any error occurs, it throws an exception.   

#### axpy (v1, v2, al=1.0)
__Parameters__   
_v1_: A FrovedisBlockcyclicMatrix with single column (input)   
_v2_: A FrovedisBlockcyclicMatrix with single column (inout)   
_al_: A double parameter to specify the value to which "v1" 
needs to be scaled (not in-place scaling) [Default: 1.0] (input/optional)   

__Purpose__   
It will solve the expression v2 = al*v1 + v2, if the input vectors are 
semantically valid and are of same length. On success, "v2" will be updated 
with desired result, but "v1" would remain unchanged.   

__Return Value__   
On success, it returns nothing. If any error occurs, it throws an exception.

#### dot (v1, v2)
__Parameters__   
_v1_: A FrovedisBlockcyclicMatrix with single column (input)   
_v2_: A FrovedisBlockcyclicMatrix with single column (input)   

__Purpose__   
It will perform dot product of the input vectors, if they are 
semantically valid and are of same length. Input vectors would not get 
modified during the operation.   

__Return Value__   
On success, it returns the dot product result of the type double.
If any error occurs, it throws an exception.   

#### nrm2 (v)
__Parameters__   
_v_: A FrovedisBlockcyclicMatrix with single column (input)  

__Purpose__   
It will calculate the norm of the input vector, if it is semantically valid. 
Input vector would not get modified during the operation.   

__Return Value__   
On success, it returns the norm value of the type double.
If any error occurs, it throws an exception.

#### gemv (m, v1, v2, trans=False, al=1.0, be=0.0)
__Parameters__   
_m_: A FrovedisBlockcyclicMatrix (input)   
_v1_: A FrovedisBlockcyclicMatrix with single column (input)   
_v2_: A FrovedisBlockcyclicMatrix with single column (inout)   
_trans_: A boolean value to specify whether to transpose "m" or not 
[Default: False] (input/optional)   
_al_: A double type value [Default: 1.0] (input/optional)   
_be_: A double type value [Default: 0.0] (input/optional)   

__Purpose__   
The primary aim of this routine is to perform simple matrix-vector multiplication.   
But it can also be used to perform any of the below operations:  

    (1) v2 = al*m*v1 + be*v2   
    (2) v2 = al*transpose(m)*v1 + be*v2

If trans=False, then expression (1) is solved.
In that case, the size of "v1" must be at least the number of columns in "m" 
and the size of "v2" must be at least the number of rows in "m".   
If trans=True, then expression (2) is solved.
In that case, the size of "v1" must be at least the number of rows in "m" 
and the size of "v2" must be at least the number of columns in "m".   

Since "v2" is used as input-output both, memory must be allocated for this 
vector before calling this routine, even if simple matrix-vector multiplication 
is required. Otherwise, this routine will throw an exception.

For simple matrix-vector multiplication, no need to specify values for the 
input parameters "trans", "al" and "be" (leave them at their default values).

On success, "v2" will be overwritten with the desired output. 
But "m" and "v1" would remain unchanged.   

__Return Value__   
On success, it returns nothing. If any error occurs, it throws an exception.   

#### ger (v1, v2, m, al=1.0)
__Parameters__   
_v1_: A FrovedisBlockcyclicMatrix with single column (input)   
_v2_: A FrovedisBlockcyclicMatrix with single column (input)   
_m_: A FrovedisBlockcyclicMatrix (inout)   
_al_: A double type value [Default: 1.0] (input/optional)   

__Purpose__   
The primary aim of this routine is to perform simple vector-vector multiplication 
of the sizes "a" and "b" respectively to form an axb matrix. But it can also be 
used to perform the below operations:   

    m = al*v1*v2' + m

This operation can only be performed if the inputs are semantically valid and 
the size of "v1" is at least the number of rows in matrix "m" and 
the size of "v2" is at least the number of columns in matrix "m".

Since "m" is used as input-output both, memory must be allocated for this 
matrix before calling this routine, even if simple vector-vector multiplication 
is required. Otherwise it will throw an exception.

For simple vector-vector multiplication, no need to specify the value for the
input parameter "al" (leave it at its default value).

On success, "m" will be overwritten with the desired output. 
But "v1" and "v2" will remain unchanged.   

__Return Value__   
On success, it returns nothing. If any error occurs, it throws an exception.   

#### gemm (m1, m2, m3, trans_m1=False, trans_m2=False, al=1.0, be=0.0)
__Parameters__  
_m1_: A FrovedisBlockcyclicMatrix (input)   
_m2_: A FrovedisBlockcyclicMatrix (input)   
_m3_: A FrovedisBlockcyclicMatrix (inout)   
_trans\_m1_: A boolean value to specify whether to transpose "m1" or not 
[Default: False] (input/optional)   
_trans\_m2_: A boolean value to specify whether to transpose "m2" or not 
[Default: False] (input/optional)   
_al_: A double type value [Default: 1.0] (input/optional)   
_be_: A double type value [Default: 0.0] (input/optional)   

__Purpose__   
The primary aim of this routine is to perform simple matrix-matrix multiplication.   
But it can also be used to perform any of the below operations:

    (1) m3 = al*m1*m2 + be*m3
    (2) m3 = al*transpose(m1)*m2 + be*m3
    (3) m3 = al*m1*transpose(m2) + be*m3
    (4) m3 = al*transpose(m1)*transpose(m2) + be*m3  

(1) will be performed, if both "trans_m1" and "trans_m2" are False.   
(2) will be performed, if trans_m1=True and trans_m2 = False.   
(3) will be performed, if trans_m1=False and trans_m2 = True.   
(4) will be performed, if both "trans_m1" and "trans_m2" are True.   

If we have four variables nrowa, nrowb, ncola, ncolb defined as follows:   

    if(trans_m1) {
      nrowa = number of columns in m1
      ncola = number of rows in m1
    }
    else {
      nrowa = number of rows in m1
      ncola = number of columns in m1
    }

    if(trans_m2) {
      nrowb = number of columns in m2
      ncolb = number of rows in m2
    }
    else {
      nrowb = number of rows in m2
      ncolb = number of columns in m2
    }

Then this function can be executed successfully, if the below conditions are 
all true:

    (a) "ncola" is equal to "nrowb"
    (b) number of rows in "m3" is equal to or greater than "nrowa"
    (b) number of columns in "m3" is equal to or greater than "ncolb"

Since "m3" is used as input-output both, memory must be allocated for this
matrix before calling this routine, even if simple matrix-matrix multiplication
is required. Otherwise it will throw an exception.

For simple matrix-matrix multiplication, no need to specify the value for the
input parameters "trans_m1", "trans_m2", "al", "be" (leave them at their 
default values).

On success, "m3" will be overwritten with the desired output.
But "m1" and "m2" will remain unchanged.

__Return Value__   
On success, it returns nothing. If any error occurs, it throws an exception.

#### geadd (m1, m2, trans=False, al=1.0, be=1.0)
__Parameters__  
_m1_: A FrovedisBlockcyclicMatrix (input)  
_m2_: A FrovedisBlockcyclicMatrix (inout)  
_trans_: A boolean value to specify whether to transpose "m1" or not 
[Default: False] (input/optional)  
_al_: A double type value [Default: 1.0] (input/optional)  
_be_: A double type value [Default: 1.0] (input/optional)  

__Purpose__   
The primary aim of this routine is to perform simple matrix-matrix addition.
But it can also be used to perform any of the below operations:

    (1) m2 = al*m1 + be*m2
    (2) m2 = al*transpose(m1) + be*m2

If trans=False, then expression (1) is solved.
In that case, the number of rows and the number of columns in "m1" should be 
equal to the number of rows and the number of columns in "m2" respectively.  
If trans=True, then expression (2) is solved.
In that case, the number of columns and the number of rows in "m1" should be 
equal to the number of rows and the number of columns in "m2" respectively.  

If it is needed to scale the input matrices before the addition, corresponding 
"al" and "be" values can be provided. But for simple matrix-matrix addition, 
no need to specify values for the input parameters "trans", "al" and "be" 
(leave them at their default values).  

On success, "m2" will be overwritten with the desired output.
But "m1" would remain unchanged.

__Return Value__  
On success, it returns nothing. If any error occurs, it throws an exception.

## SEE ALSO
scalapack_wrapper, blockcyclic_matrix 


# scalapack_wrapper

## NAME
scalapack_wrapper - a frovedis module provides user-friendly interfaces for commonly 
used scalapack routines in scientific applications like machine learning algorithms.

## SYNOPSIS

import frovedis.matrix.wrapper.SCALAPACK   

## WRAPPER FUNCTIONS
SCALAPACK.getrf (m)      
SCALAPACK.getri (m, ipivPtr)   
SCALAPACK.getrs (m1, m2, ipivPtr, trans=False)    
SCALAPACK.gesv  (m1, m2)    
SCALAPACK.gels  (m1, m2, trans=False)   
SCALAPACK.gesvd (m, wantU=False, wantV=False)      

## DESCRIPTION
ScaLAPACK is a high-performance scientific library written in Fortran language. 
It provides rich set of linear algebra functionalities whose computation 
loads are parallelized over the available processes in a system and 
the user interfaces of this library is very detailed and complex 
in nature. It requires a strong understanding on each of the input parameters, 
along with some distribution concepts. 

Frovedis provides a wrapper module for some commonly used ScaLAPACK subroutines in 
scientific applications like machine learning algorithms. These wrapper 
interfaces are very simple and user needs not to consider all the detailed 
distribution parameters. Only specifying the target vectors or matrices with 
some other parameters (depending upon need) are fine. At the same time, all the 
use cases of a ScaLAPACK routine can also be performed using Frovedis ScaLAPACK 
wrapper of that routine.   

This python module implements a client-server application, where the python client 
can send the python matrix data to frovedis server side in order to create 
blockcyclic matrix at frovedis server and then python client can request frovedis 
server for any of the supported ScaLAPACK operation on that matrix. When required, 
python client can request frovedis server to send back the resultant matrix 
and it can then create equivalent python data (see manuals for 
FrovedisBlockcyclicMatrix to python data conversion).

The individual detailed descriptions can be found in the subsequent sections. 
Please note that the term "inout", used in the below section indicates a function 
argument as both "input" and "output".

### Detailed Description 
#### getrf (m)
__Parameters__  
_m_: A FrovedisBlockcyclicMatrix (inout)   

__Purpose__    
It computes an LU factorization of a general M-by-N distributed matrix, "m" 
using partial pivoting with row interchanges. 

On successful factorization, matrix "m" is overwritten with the computed 
L and U factors. Along with the return status of native scalapack routine, 
it also returns the proxy address of the node local vector "ipiv" containing 
the pivoting information associated with input matrix "m" in the form of 
GetrfResult. The "ipiv" information will be useful in computation of some 
other routines (like getri, getrs etc.)

__Return Value__   
On success, it returns the object of the type GetrfResult as explained above. 
If any error occurs, it throws an exception explaining cause of the error.   

#### getri (m, ipivPtr)
__Parameters__  
_m_: A FrovedisBlockcyclicMatrix  (inout)   
_ipiv_: A long object containing the proxy of the ipiv vector 
(from GetrfResult) (input)     

__Purpose__   
It computes the inverse of a distributed square matrix using the LU 
factorization computed by getrf(). So in order to compute inverse of a matrix, 
first compute it's LU factor (and ipiv information) using getrf() and then 
pass the factored matrix, "m" along with the "ipiv" information to this 
function.

On success, factored matrix "m" is overwritten with the inverse (of the 
matrix which was passed to getrf()) matrix. "ipiv" will be internally 
used by this function and will remain unchanged.

For example,

    res = SCALAPACK.getrf(m)      // getting LU factorization of "m"
    SCALAPACK.getri(m,res.ipiv()) // "m" will have inverse of the initial value

__Return Value__  
On success, it returns the exit status of the scalapack routine itself.
If any error occurs, it throws an exception explaining cause of the error.

#### getrs (m1, m2, ipiv, trans=False)
__Parameters__   
_m1_: A FrovedisBlockcyclicMatrix  (input)   
_m2_: A FrovedisBlockcyclicMatrix  (inout)   
_ipiv_: A long object containing the proxy of the ipiv vector 
(from GetrfResult) (input)    
_trans_: A boolean value to specify whether to transpose "m1" 
[Default: False] (input/optional)   

__Purpose__   
It solves a real system of distributed linear equations, AX=B with a general 
distributed square matrix (A) using the LU factorization computed by getrf(). 
Thus before calling this function, it is required to obtain the factored matrix 
"m1" (along with "ipiv" information) by calling getrf().

For example,    

    res = SCALAPACK.getrf(m1) // getting LU factorization of "m1"
    SCALAPACK.getrs(m1,m2,res.ipiv()) 

If trans=False, the linear equation AX=B is solved.   
If trans=True, the linear equation transpose(A)X=B (A'X=B) is solved.
 
The matrix "m2" should have number of rows >= the number of rows in "m1" and 
at least 1 column in it.   

On entry, "m2" contains the distributed right-hand-side (B) of the equation and 
on successful exit it is overwritten with the distributed solution matrix (X). 

__Return Value__  
On success, it returns the exit status of the scalapack routine itself.
If any error occurs, it throws an exception explaining cause of the error.  

#### gesv (m1, m2)
__Parameters__   
_m1_: A FrovedisBlockcyclicMatrix (inout)   
_m2_: A FrovedisBlockcyclicMatrix (inout)   

__Purpose__   
It solves a real system of distributed linear equations, AX=B with a general
distributed square matrix, "m1" by computing it's LU factors internally. This 
function internally computes the LU factors and ipiv information using getrf() 
and then solves the equation using getrs(). 

The matrix "m2" should have number of rows >= the number of rows in "m1" and 
at least 1 column in it.   
 
On entry, "m1" contains the distributed left-hand-side square matrix (A), 
"m2" contains the distributed right-hand-side matrix (B) and
on successful exit "m1" is overwritten with it's LU factors, 
"m2" is overwritten with the distributed solution matrix (X). 

__Return Value__   
On success, it returns the exit status of the scalapack routine itself.
If any error occurs, it throws an exception explaining cause of the error.

#### gels (m1, m2, trans=False)
__Parameters__  
_m1_: A FrovedisBlockcyclicMatrix (input)  
_m2_: A FrovedisBlockcyclicMatrix (inout)  
_trans_: A boolean value to specify whether to transpose "m1" 
[Default: False] (input/optional)  

__Purpose__  
It solves overdetermined or underdetermined real linear systems involving an 
M-by-N distributed matrix (A) or its transpose, using a QR or LQ factorization 
of (A). It is assumed that distributed matrix (A) has full rank. 

If trans=False and M >= N: 
it finds the least squares solution of an overdetermined system.   
If trans=False and M < N: 
it finds the minimum norm solution of an underdetermined system.   
If trans=True and M >= N: 
it finds the minimum norm solution of an underdetermined system.   
If trans=True and M < N: 
it finds the least squares solution of an overdetermined system.   

The matrix "m2" should have number of rows >= max(M,N) and at least 1 column.    

On entry, "m1" contains the distributed left-hand-side matrix (A) and "m2" 
contains the distributed right-hand-side matrix (B). On successful exit, 
"m1" is overwritten with the QR or LQ factors and "m2" is overwritten with 
the distributed solution matrix (X).  

__Return Value__   
On success, it returns the exit status of the scalapack routine itself.
If any error occurs, it throws an exception explaining cause of the error.   

#### gesvd (m, wantU=False, wantV=False)  
__Parameters__  
_m_: A FrovedisBlockcyclicMatrix (inout)  
_wantU_: A boolean value to specify whether to compute U matrix 
[Default: False] (input)  
_wantV_: A boolean value to specify whether to compute V matrix 
[Default: False] (input)  

__Purpose__  
It computes the singular value decomposition (SVD) of an M-by-N distributed 
matrix. 

On entry "m" contains the distributed matrix whose singular values 
are to be computed.   

If wantU = wantV = False, then it computes only the singular values in 
sorted oder, so that sval(i) >= sval(i+1). Otherwise it also computes U 
and/or V (left and right singular vectors respectively) matrices.

On successful exit, the contents of "m" is destroyed (internally used as 
workspace). 

__Return Value__  
On success, it returns the object of the type GesvdResult containing 
the singular values and U and V components (based on the requirement) along 
with the exit status of the native scalapack routine.
If any error occurs, it throws an exception explaining cause of the error.

## SEE ALSO
blockcyclic_matrix, pblas_wrapper, arpack_wrapper, getrf_result, gesvd_result     


# arpack_wrapper

## NAME
arpack_wrapper - a frovedis module supports singular value decomposition on 
sparse data using arpack routines. 

## SYNOPSIS

import frovedis.matrix.wrapper.ARPACK    

### Public Member Functions
ARPACK.computeSVD (data, k)    

## DESCRIPTION
This module provides interface to compute singular value decomposition 
on sparse data using arpack native routines at frovedis server side. 

### Detailed Description 
#### computeSVD (data, k)
__Parameters__  
_data_: Any scipy sparse matrix or python array-like structure or an 
instance of FrovedisCRSMatrix.     
_k_: An integer value to specify the number of singular values to compute.   

__Purpose__    
It computes the singular value decomposition on the sparse data at 
frovedis side. Once done, it returns a GesvdResult object containing the 
proxy of the results at frovedis server. 

When required, the spark client can convert back the frovedis server side SVD 
result to numpy data by calling to_numpy_results() function on GesvdResult 
structure. 

For example,

    res = ARPACK.computeSVD(data,2) // compute 2 singular values for the given data
    p_res = res.to_numpy_results() 
    
__Return Value__   
On success, it returns an object of GesvdResult type containing the proxy of 
SVD results at frovedis server side. If any error occurs, it throws an exception.   

# getrf_result 

## NAME
getrf_result - a structure to model the output of frovedis wrapper of scalapack 
getrf routine. 

## SYNOPSIS

import frovedis.matrix.results.GetrfResult   

### Public Member Functions
release()  
ipiv()     
stat()   

## DESCRIPTION

GetrfResult is a client python side pseudo result structure containing the 
proxy of the in-memory scalapack getrf result (node local ipiv vector) created 
at frovedis server side. 

### Public Member Function Documentation
 
#### release()
__Purpose__   
This function can be used to release the in-memory result component (ipiv 
vector) at frovedis server.

__Return Type__   
It returns nothing. 

#### ipiv()
__Purpose__    
This function returns the proxy of the node_local "ipiv" vector computed 
during getrf calculation. This value will be required in other scalapack 
routine calculation, like getri, getrs etc.

__Return Type__   
A long value containing the proxy of ipiv vector.   

#### stat()   
__Purpose__    
This function returns the exit status of the scalapack native getrf routine 
on calling of which the target result object was obtained. 

__Return Type__   
It returns an integer value.   

# gesvd_result 

## NAME
gesvd_result - a structure to model the output of frovedis singular value 
decomposition methods. 

## SYNOPSIS

import frovedis.matrix.results.GesvdResult   

### Public Member Functions
to_numpy_results()   
save(svec, umat=None, vmat=None)    
save_binary(svec, umat=None, vmat=None)    
load (svec, umat=None, vmat=None, mtype='B')   
load_binary (svec, umat=None, vmat=None, mtype='B')   
debug_print()   
release()   
stat()   
getK()    

## DESCRIPTION

GesvdResult is a python side pseudo result structure containing the 
proxies of the in-memory SVD results created at frovedis server side. It can be 
used to convert the frovedis side SVD result to python equivalent data structures.  

### Public Member Function Documentation
 
#### to_numpy_results()
__Purpose__   
This function can be used to convert the frovedis side SVD results to python numpy
result structures.

If U and V both are computed, it returns: (numpy matrix, numpy array, numpy matrix)    
indicating (umatrix, singular vector, vmatrix).   

When U is calculated, but not V, it returns: (numpy matrix, numpy array, None)   
When V is calculated, but not U, it returns: (None, numpy array, numpy matrix)   
When neither U nor V is calculated, it returns: (None, numpy array, None)   

__Return Type__     
It returns a tuple as explained above.   

#### save(svec, umat=None, vmat=None)   
__Parameters__   
_svec_: A string object containing name of the file to save singular vectors as 
text data. (mandatory)     
_umat_: A string object containing name of the file to save umatrix as 
text data. (optional)     
_vmat_: A string object containing name of the file to save vmatrix as 
text data. (ptional)     

__Purpose__   
This function can be used to save the result values in different text files at 
server side. If saving of U and V components are not required, "umat" and "vmat" 
can be None, but "svec" should have a valid filename.

__Return Type__  
It returns nothing.   

#### save_binary (svec, umat=None, vmat=None)   
__Parameters__   
_svec_: A string object containing name of the file to save singular vectors as 
binary data. (mandatory)     
_umat_: A string object containing name of the file to save umatrix as 
binary data. (optional)     
_vmat_: A string object containing name of the file to save vmatrix as 
binary data. (optional)     

__Purpose__    
This function can be used to save the result values in different files as 
little-endian binary data at server side. If saving of U and V components are not 
required, "umat" and "vmat" can be None, but "svec" should have a valid filename.   

__Return Type__    
It returns nothing.   

#### load(svec, umat=None, vmat=None, mtype='B')   
__Parameters__   
_svec_: A string object containing name of the file from which to load 
singular vectors as text data for the target result. (mandatory)     
_umat_: A string object containing name of the file from which to load 
umatrix as text data for the target result. (optional)     
_vmat_: A string object containing name of the file from which to load 
vmatrix as text data for the target result. (optional)     
_mtype_: A character value, can be either 'B' or 'C'. (optional)   

__Purpose__   
This function can be used to load the result values in different text files at 
server side. If loading of U and V components are not required, "umat" and "vmat" 
can be None, but "svec" should have a valid filename.

If mtype = 'B' and umat/vmat is to be loaded, then they will be loaded 
as blockcyclic matrices at server side.   

If mtype = 'C' and umat/vmat is to be loaded, then they will be loaded 
as colmajor matrices at server side.   

__Return Type__  
It returns nothing.   

#### load_binary(svec, umat=None, vmat=None, mtype='B')   
__Parameters__   
_svec_: A string object containing name of the file from which to load 
singular vectors as binary data for the target result. (mandatory)     
_umat_: A string object containing name of the file from which to load 
umatrix as binary data for the target result. (optional)     
_vmat_: A string object containing name of the file from which to load 
vmatrix as binary data for the target result. (optional)     
_mtype_: A character value, can be either 'B' or 'C'. (optional)   

__Purpose__   
This function can be used to load the result values in different little-endian 
binary files at server side. If loading of U and V components are not required, 
"umat" and "vmat" can be None, but "svec" should have a valid filename.

If mtype = 'B' and umat/vmat is to be loaded, then they will be loaded 
as blockcyclic matrices at server side.   

If mtype = 'C' and umat/vmat is to be loaded, then they will be loaded 
as colmajor matrices at server side.   

__Return Type__  
It returns nothing. 

#### debug_print()
__Purpose__   
This function can be used to print the result components at server side 
user terminal. This is useful in debugging purpose.

__Return Type__    
It returns nothing.   

#### release()
__Purpose__   
This function can be used to release the in-memory result components at frovedis 
server.

__Return Type__    
It returns nothing.   

#### stat()
__Purpose__   
This function returns the exit status of the scalapack native gesvd routine 
on calling of which the target result object was obtained. 

__Return Type__    
An integer value.   

#### getK()
__Purpose__   
This function returns the number of singular values computed.

__Return Type__    
An integer value.   


# Linear Regression

## NAME

Linear Regression - A regression algorithm to predict 
the continuous output without any regularization.

## SYNOPSIS

class frovedis.mllib.linear_model.LinearRegression (fit_intercept=True, normalize=False,    
\  \ \  \  \  \ copy_X=True, n_jobs=1, solver='sag', verbose=0) 

### Public Member Functions

fit(X, y, sample_weight=None)   
predict(X)   
save(filename)   
load(filename)   
debug_print()   
release()   

## DESCRIPTION
Linear least squares is the most common formulation for regression problems. 
It is a linear method with the loss function given by the **squared loss**:

    L(w;x,y) := 1/2(wTx-y)^2

Where the vectors x are the training data examples and y are their corresponding 
labels which we want to predict. w is the linear model (also known as weight) 
which uses a single weighted sum of features to make a prediction. The method 
is called linear since it can be expressed as a function of wTx and y. Linear 
regression does not use any regularizer. 

The gradient of the squared loss is: (wTx-y).x   

Frovedis provides implementation of linear regression with two different 
optimizers: (1) stochastic gradient descent with minibatch and (2) LBFGS 
optimizer.

The simplest method to solve optimization problems of the form **min f(w)** 
is gradient descent. Such first-order optimization methods well-suited for 
large-scale and distributed computation. Whereas, L-BFGS is an optimization 
algorithm in the family of quasi-Newton methods to solve the optimization 
problems of the similar form. 

Like the original BFGS, L-BFGS (Limited Memory BFGS) uses an estimation to 
the inverse Hessian matrix to steer its search through feature space, 
but where BFGS stores a dense nxn approximation to the inverse Hessian 
(n being the number of features in the problem), L-BFGS stores only a few 
vectors that represent the approximation implicitly. L-BFGS often achieves 
rapider convergence compared with other first-order optimization.

This module provides a client-server implementation, where the client 
application is a normal python scikit-learn program. Scikit-learn has its own 
linear_model providing the Linear Regression support. But that algorithm is 
non-distributed in nature. Hence it is slower when comparing with 
the equivalent Frovedis algorithm (see frovedis manual for ml/linear_regression) with 
big dataset. Thus in this implementation, a scikit-learn client can interact with 
a frovedis server sending the required python data for training at frovedis side. 
Python data is converted into frovedis compatible data internally and the 
scikit-learn ML call is linked with the respective frovedis ML call to get the 
job done at frovedis server. 

Scikit-learn side call for Linear Regression quickly returns, 
right after submitting the training request to the frovedis server with a unique 
model ID for the submitted training request. 

When operations like prediction will be required on the trained model, scikit-learn 
client sends the same request to frovedis server on the same model 
(containing the unique ID) and the request is served at frovedis server and output 
is sent back to the scikit-learn client. 

### Detailed Description  

#### LinearRegression()

__Parameters__   
_fit\_intercept_: A boolean parameter specifying whether a constant (intercept) 
should be added to the decision function. (Default: True)    
_normalize_: A boolean parameter. (unused)     
_copy\_X_: A boolean parameter. (unsed)      
_n\_jobs_: An integer parameter. (unused)    
_solver_: A string parameter specifying the solver to use. (Default: 'sag')    
_verbose_: A integer parameter specifying the log level to use. (Default: 0)   

__Purpose__    
It initialized a LinearRegression object with the given parameters.   

The parameters: "normalize", "copy_X" and "n_jobs" are not yet supported at 
frovedis side. Thus they don't have any significance when calling the frovedis 
linear regression algorithm. They are simply provided for the compatibility 
with scikit-learn application.    

"solver" can be either 'sag' for frovedis side stochastic gradient descent or 
'lbfgs' for frovedis side LBFGS optimizer when optimizing the linear regression 
model. 

"verbose" value is set at 0 by default.
But it can be set to 1 (for DEBUG mode) or 2 (for TRACE mode) for getting 
training time logs from frovedis server.   

__Return Value__    
It simply returns "self" reference. 

#### fit(X, y, sample_weight=None)
__Parameters__   
_X_: A scipy sparse matrix or any python array-like object or an instance 
of FrovedisCRSMatrix.    
_y_: Any python array-like object or an instance of FrovedisDvector.     
_sample\_weight_: Python array-like optional parameter. (unused)   

__Purpose__    
It accepts the training feature matrix (X) and corresponding output labels (y) 
as inputs from the user and trains a linear regression model with those data 
at frovedis server. 

It doesn't support any initial weight to be passed as input at this moment. 
Thus the "sample_weight" parameter will simply be ignored. It starts with an 
initial guess of zeros for the model vector and keeps updating the model to 
minimize the cost function until convergence is achieved (default convergence 
tolerance value is 0.001) or maximum iteration count is reached (default 1000, 
is not configurable at this moment).  

For example,   

    # loading sample CRS data file
    mat = FrovedisCRSMatrix().load("./sample")
    lbl = FrovedisDvector([1.1,0.2,1.3,1.4,1.5,0.6,1.7,1.8])
    
    # fitting input matrix and label on linear regression object
    lr = LinearRegression(solver='sgd', verbose=2).fit(mat,lbl)

__Return Value__  
It simply returns "self" reference.   
Note that the call will return quickly, right after submitting the fit request 
at frovedis server side with a unique model ID for the fit request. It may be 
possible that the training is not completed at the frovedis server side even 
though the client scikit-learn side fit() returns. 

#### predict(X)
__Parameters__   
_X_: A scipy sparse matrix or any python array-like object or an instance 
of FrovedisCRSMatrix.    

__Purpose__    
It accepts the test feature matrix (X) in order to make prediction on the 
trained model at frovedis server. 

__Return Value__  
It returns a numpy array of double (float64) type containing the predicted 
outputs. 

#### save(filename)
__Parameters__   
_filename_: A string object containing the name of the file on which the target 
model is to be saved.    

__Purpose__    
On success, it writes the model information (weight values etc.) in the 
specified file as little-endian binary data. Otherwise, it throws an exception. 

__Return Value__  
It returns nothing.   

#### load(filename)
__Parameters__   
_filename_: A string object containing the name of the file having model 
information to be loaded.   

__Purpose__    
It loads the model from the specified file (having little-endian binary data).

__Return Value__  
It simply returns "self" instance.   

#### debug_print()

__Purpose__    
It shows the target model information (weight values etc.) on the server side 
user terminal. It is mainly used for debugging purpose.   

__Return Value__  
It returns nothing.   

#### release()

__Purpose__    
It can be used to release the in-memory model at frovedis server. 

__Return Value__  
It returns nothing.   

## SEE ALSO  
lasso_regression, ridge_regression, dvector, crs_matrix      

# Lasso Regression

## NAME

Lasso Regression - A regression algorithm to predict 
the continuous output with L1 regularization.   

## SYNOPSIS

class frovedis.mllib.linear_model.Lasso (alpha=0.01, fit_intercept=True, normalize=False,    
\  \ \  \  \  \ precompute=False, copy_X=True, max_iter=1000,    
\  \ \  \  \  \ tol=1e-4, warm_start=False, positive=False,    
\  \ \  \  \  \ random_state=None, selection='cyclic',    
\  \ \  \  \  \ verbose=0, solver='sag')     

### Public Member Functions

fit(X, y, sample_weight=None)   
predict(X)   
save(filename)   
load(filename)   
debug_print()   
release()   

## DESCRIPTION
Linear least squares is the most common formulation for regression problems. 
It is a linear method with the loss function given by the **squared loss**:

    L(w;x,y) := 1/2(wTx-y)^2

Where the vectors x are the training data examples and y are their corresponding 
labels which we want to predict. w is the linear model (also known as weight) 
which uses a single weighted sum of features to make a prediction. The method 
is called linear since it can be expressed as a function of wTx and y. Lasso 
regression uses L1 regularization to address the overfit problem.    

The gradient of the squared loss is: (wTx-y).x   
The gradient of the regularizer is: sign(w)   

Frovedis provides implementation of lasso regression with two different 
optimizers: (1) stochastic gradient descent with minibatch and (2) LBFGS 
optimizer. 

The simplest method to solve optimization problems of the form **min f(w)** 
is gradient descent. Such first-order optimization methods well-suited for 
large-scale and distributed computation. Whereas, L-BFGS is an optimization 
algorithm in the family of quasi-Newton methods to solve the optimization 
problems of the similar form. 

Like the original BFGS, L-BFGS (Limited Memory BFGS) uses an estimation to 
the inverse Hessian matrix to steer its search through feature space, 
but where BFGS stores a dense nxn approximation to the inverse Hessian 
(n being the number of features in the problem), L-BFGS stores only a few 
vectors that represent the approximation implicitly. L-BFGS often achieves 
rapider convergence compared with other first-order optimization.

This module provides a client-server implementation, where the client 
application is a normal python scikit-learn program. Scikit-learn has its own 
linear_model providing the Lasso Regression support. But that algorithm is 
non-distributed in nature. Hence it is slower when comparing with 
the equivalent Frovedis algorithm (see frovedis manual for ml/lasso_regression) with 
big dataset. Thus in this implementation, a scikit-learn client can interact with 
a frovedis server sending the required python data for training at frovedis side. 
Python data is converted into frovedis compatible data internally and the 
scikit-learn ML call is linked with the respective frovedis ML call to get the 
job done at frovedis server. 

Scikit-learn side call for Lasso Regression quickly returns, 
right after submitting the training request to the frovedis server with a unique 
model ID for the submitted training request. 

When operations like prediction will be required on the trained model, scikit-learn 
client sends the same request to frovedis server on the same model 
(containing the unique ID) and the request is served at frovedis server and output 
is sent back to the scikit-learn client. 

### Detailed Description  

#### Lasso()

__Parameters__   
_alpha_: A double parameter containing the learning rate. (Default: 0.01)   
_fit\_intercept_: A boolean parameter specifying whether a constant (intercept) 
should be added to the decision function. (Default: True)    
_normalize_: A boolean parameter (unused)     
_precompute_: A boolean parameter (unused)     
_copy\_X_: A boolean parameter (unsed)      
_max\_iter_: An integer parameter specifying maximum iteration count. (Default: 1000)    
_tol_: A double parameter specifying the convergence tolerance value, (Default: 1e-4)    
_warm\_start_: A boolean parameter (unused)     
_positive_: A boolean parameter (unused)     
_random\_state_: An integer, None or RandomState instance. (unused)   
_selection_: A string object. (unused)   
_verbose_: An integer object specifying the log level to use. (Default: 0)   
_solver_: A string object specifying the solver to use. (Default: 'sag')   

__Purpose__    
It initialized a Lasso object with the given parameters.   

The parameters: "normalize", "precompute", "copy_X", "warm_start", 
"positive", "random_state" and "selection" are not yet supported at frovedis side. 
Thus they don't have any significance in this call. They are simply provided 
for the compatibility with scikit-learn application.    

"solver" can be either 'sag' for frovedis side stochastic gradient descent or 
'lbfgs' for frovedis side LBFGS optimizer when optimizing the linear regression 
model. 

"verbose" value is set at 0 by default.
But it can be set to 1 (for DEBUG mode) or 2 (for TRACE mode) for getting 
training time logs from frovedis server.   

__Return Value__    
It simply returns "self" reference. 

#### fit(X, y, sample_weight=None)
__Parameters__   
_X_: A scipy sparse matrix or any python array-like object or an instance 
of FrovedisCRSMatrix.    
_y_: Any python array-like object or an instance of FrovedisDvector.     
_sample\_weight_: Python array-like optional parameter. (unused)   

__Purpose__    
It accepts the training feature matrix (X) and corresponding output labels (y) 
as inputs from the user and trains a linear regression model with L1 regularization 
with those data at frovedis server. 

It doesn't support any initial weight to be passed as input at this moment. 
Thus the "sample_weight" parameter will simply be ignored. It starts with an 
initial guess of zeros for the model vector and keeps updating the model to 
minimize the cost function until convergence is achieved or maximum iteration count 
is reached.  

For example,   

    # loading sample CRS data file
    mat = FrovedisCRSMatrix().load("./sample")
    lbl = FrovedisDvector([1.1,0.2,1.3,1.4,1.5,0.6,1.7,1.8])
    
    # fitting input matrix and label on lasso object
    lr = Lasso(solver='sgd', verbose=2).fit(mat,lbl)

__Return Value__  
It simply returns "self" reference.   
Note that the call will return quickly, right after submitting the fit request 
at frovedis server side with a unique model ID for the fit request. It may be 
possible that the training is not completed at the frovedis server side even 
though the client scikit-learn side fit() returns. 

#### predict(X)
__Parameters__   
_X_: A scipy sparse matrix or any python array-like object or an instance 
of FrovedisCRSMatrix.    

__Purpose__    
It accepts the test feature matrix (X) in order to make prediction on the 
trained model at frovedis server. 

__Return Value__  
It returns a numpy array of double (float64) type containing the predicted 
outputs. 

#### save(filename)
__Parameters__   
_filename_: A string object containing the name of the file on which the target 
model is to be saved.    

__Purpose__    
On success, it writes the model information (weight values etc.) in the 
specified file as little-endian binary data. Otherwise, it throws an exception. 

__Return Value__  
It returns nothing.   

#### load(filename)
__Parameters__   
_filename_: A string object containing the name of the file having model 
information to be loaded.    

__Purpose__    
It loads the model from the specified file (having little-endian binary data).

__Return Value__  
It simply returns "self" instance.   

#### debug_print()

__Purpose__    
It shows the target model information (weight values etc.) on the server side 
user terminal. It is mainly used for debugging purpose.   

__Return Value__  
It returns nothing.   

#### release()

__Purpose__    
It can be used to release the in-memory model at frovedis server. 

__Return Value__  
It returns nothing.   

## SEE ALSO  
linear_regression, ridge_regression, dvector, crs_matrix      

# Ridge Regression

## NAME

Ridge Regression - A regression algorithm to predict 
the continuous output with L2 regularization.   

## SYNOPSIS

class frovedis.mllib.linear_model.Ridge (alpha=0.01, fit_intercept=True, normalize=False,    
\  \ \  \  \  \ copy_X=True, max_iter=1000,    
\  \ \  \  \  \ tol=1e-3, solver='sag',    
\  \ \  \  \  \ random_state=None, verbose=0)        

### Public Member Functions

fit(X, y, sample_weight=None)   
predict(X)   
save(filename)   
load(filename)   
debug_print()   
release()   

## DESCRIPTION
Linear least squares is the most common formulation for regression problems. 
It is a linear method with the loss function given by the **squared loss**:

    L(w;x,y) := 1/2(wTx-y)^2

Where the vectors x are the training data examples and y are their corresponding 
labels which we want to predict. w is the linear model (also known as weight) 
which uses a single weighted sum of features to make a prediction. The method 
is called linear since it can be expressed as a function of wTx and y. Ridge 
regression uses L2 regularization to address the overfit problem.    

The gradient of the squared loss is: (wTx-y).x   
The gradient of the regularizer is: w   

Frovedis provides implementation of ridge regression with two different 
optimizers: (1) stochastic gradient descent with minibatch and (2) LBFGS 
optimizer. 

The simplest method to solve optimization problems of the form **min f(w)** 
is gradient descent. Such first-order optimization methods well-suited for 
large-scale and distributed computation. Whereas, L-BFGS is an optimization 
algorithm in the family of quasi-Newton methods to solve the optimization 
problems of the similar form. 

Like the original BFGS, L-BFGS (Limited Memory BFGS) uses an estimation to 
the inverse Hessian matrix to steer its search through feature space, 
but where BFGS stores a dense nxn approximation to the inverse Hessian 
(n being the number of features in the problem), L-BFGS stores only a few 
vectors that represent the approximation implicitly. L-BFGS often achieves 
rapider convergence compared with other first-order optimization.

This module provides a client-server implementation, where the client 
application is a normal python scikit-learn program. Scikit-learn has its own 
linear_model providing the Ridge Regression support. But that algorithm is 
non-distributed in nature. Hence it is slower when comparing with 
the equivalent Frovedis algorithm (see frovedis manual for ml/ridge_regression) with 
big dataset. Thus in this implementation, a scikit-learn client can interact with 
a frovedis server sending the required python data for training at frovedis side. 
Python data is converted into frovedis compatible data internally and the 
scikit-learn ML call is linked with the respective frovedis ML call to get the 
job done at frovedis server. 

Scikit-learn side call for Ridge Regression quickly returns, 
right after submitting the training request to the frovedis server with a unique 
model ID for the submitted training request. 

When operations like prediction will be required on the trained model, scikit-learn 
client sends the same request to frovedis server on the same model 
(containing the unique ID) and the request is served at frovedis server and output 
is sent back to the scikit-learn client. 

### Detailed Description  

#### Ridge()

__Parameters__   
_alpha_: A double parameter containing the learning rate. (Default: 0.01)   
_fit\_intercept_: A boolean parameter specifying whether a constant (intercept) 
should be added to the decision function. (Default: True)    
_normalize_: A boolean parameter (unused)     
_copy\_X_: A boolean parameter (unsed)      
_max\_iter_: An integer parameter specifying maximum iteration count. (Default: 1000)    
_tol_: A double parameter specifying the convergence tolerance value, (Default: 1e-3)    
_solver_: A string object specifying the solver to use. (Default: 'sag')   
_random\_state_: An integer, None or RandomState instance. (unused)   
_verbose_: An integer object specifying the log level to use. (Default: 0)   

__Purpose__    
It initialized a Ridge object with the given parameters.   

The parameters: "normalize", "copy_X" and "random_state" are not yet supported 
at frovedis side. Thus they don't have any significance in this call. They are 
simply provided for the compatibility with scikit-learn application.    

"solver" can be either 'sag' for frovedis side stochastic gradient descent or 
'lbfgs' for frovedis side LBFGS optimizer when optimizing the linear regression 
model. 

"verbose" value is set at 0 by default.
But it can be set to 1 (for DEBUG mode) or 2 (for TRACE mode) for getting 
training time logs from frovedis server.   

__Return Value__    
It simply returns "self" reference. 

#### fit(X, y, sample_weight=None)
__Parameters__   
_X_: A scipy sparse matrix or any python array-like object or an instance 
of FrovedisCRSMatrix.    
_y_: Any python array-like object or an instance of FrovedisDvector.     
_sample\_weight_: Python array-like optional parameter. (unused)   

__Purpose__    
It accepts the training feature matrix (X) and corresponding output labels (y) 
as inputs from the user and trains a linear regression model with L2 regularization 
with those data at frovedis server. 

It doesn't support any initial weight to be passed as input at this moment. 
Thus the "sample_weight" parameter will simply be ignored. It starts with an 
initial guess of zeros for the model vector and keeps updating the model to 
minimize the cost function until convergence is achieved or maximum iteration count 
is reached.  

For example,   

    # loading sample CRS data file
    mat = FrovedisCRSMatrix().load("./sample")
    lbl = FrovedisDvector([1.1,0.2,1.3,1.4,1.5,0.6,1.7,1.8])
    
    # fitting input matrix and label on ridge regression object
    lr = Ridge(solver='sgd', verbose=2).fit(mat,lbl)

__Return Value__  
It simply returns "self" reference.   
Note that the call will return quickly, right after submitting the fit request 
at frovedis server side with a unique model ID for the fit request. It may be 
possible that the training is not completed at the frovedis server side even 
though the client scikit-learn side fit() returns. 

#### predict(X)
__Parameters__   
_X_: A scipy sparse matrix or any python array-like object or an instance 
of FrovedisCRSMatrix.    

__Purpose__    
It accepts the test feature matrix (X) in order to make prediction on the 
trained model at frovedis server. 

__Return Value__  
It returns a numpy array of double (float64) type containing the predicted 
outputs. 

#### save(filename)
__Parameters__   
_filename_: A string object containing the name of the file on which the target 
model is to be saved.    

__Purpose__    
On success, it writes the model information (weight values etc.) in the 
specified file as little-endian binary data. Otherwise, it throws an exception. 

__Return Value__  
It returns nothing.   

#### load(filename)
__Parameters__   
_filename_: A string object containing the name of the file having model 
information to be loaded.   

__Purpose__    
It loads the model from the specified file (having little-endian binary data).

__Return Value__  
It simply returns "self" instance.   

#### debug_print()

__Purpose__    
It shows the target model information (weight values etc.) on the server side 
user terminal. It is mainly used for debugging purpose.   

__Return Value__  
It returns nothing.   

#### release()

__Purpose__    
It can be used to release the in-memory model at frovedis server. 

__Return Value__  
It returns nothing.   

## SEE ALSO  
linear_regression, lasso_regression, dvector, crs_matrix    


# Logistic Regression

## NAME

Logistic Regression - A classification algorithm to predict 
the binary output with logistic loss.   

## SYNOPSIS

class frovedis.mllib.linear_model.LogisticRegression (penalty='l2', dual=False,    
\  \ \  \  \  \ tol=1e-4, C=0.01, fit_intercept=True, intercept_scaling=1,    
\  \ \  \  \  \ class_weight=None, random_state=None, solver='sag',    
\  \ \  \  \  \ max_iter=1000, multi_class='ovr', verbose=0, warm_start=False,    
\  \ \  \  \  \ n_jobs=1)      

### Public Member Functions

fit(X, y, sample_weight=None)   
predict(X)  
predict_proba (X)
save(filename)   
load(filename)   
debug_print()   
release()   

## DESCRIPTION
Classification aims to divide the items into categories. 
The most common classification type is binary classification, where there are 
two categories, usually named positive and negative. Frovedis supports binary 
classification algorithm only. 

Logistic regression is widely used to predict a binary response. 
It is a linear method with the loss function given by the **logistic loss**:  

    L(w;x,y) := log(1 + exp(-ywTx))    

Where the vectors x are the training data examples and y are their corresponding 
labels (Frovedis considers negative response as -1 and positive response as 1, but 
when calling from scikit-learn interface, user should pass 0 for negative response and 
1 for positive response according to the scikit-learn requirement) which we want to 
predict. w is the linear model (also called as weight) which uses a single weighted 
sum of features to make a prediction. Frovedis Logistic Regression supports ZERO, 
L1 and L2 regularization to address the overfit problem.     

The gradient of the logistic loss is: -y( 1 - 1 / (1 + exp(-ywTx))).x    
The gradient of the L1 regularizer is: sign(w)     
And The gradient of the L2 regularizer is: w     

For binary classification problems, the algorithm outputs a binary logistic 
regression model. Given a new data point, denoted by x, the model makes 
predictions by applying the logistic function:   

    f(z) := 1 / 1 + exp(-z)  

Where z = wTx. By default (threshold=0.5), if f(wTx) > 0.5, the response is 
positive (1), else the response is negative (0).   

Frovedis provides implementation of logistic regression with two different 
optimizers: (1) stochastic gradient descent with minibatch and (2) LBFGS 
optimizer. 

The simplest method to solve optimization problems of the form **min f(w)** 
is gradient descent. Such first-order optimization methods well-suited for 
large-scale and distributed computation. Whereas, L-BFGS is an optimization 
algorithm in the family of quasi-Newton methods to solve the optimization 
problems of the similar form. 

Like the original BFGS, L-BFGS (Limited Memory BFGS) uses an estimation to 
the inverse Hessian matrix to steer its search through feature space, 
but where BFGS stores a dense nxn approximation to the inverse Hessian 
(n being the number of features in the problem), L-BFGS stores only a few 
vectors that represent the approximation implicitly. L-BFGS often achieves 
rapider convergence compared with other first-order optimization.

This module provides a client-server implementation, where the client 
application is a normal python scikit-learn program. Scikit-learn has its own 
linear_model providing the Logistic Regression support. But that algorithm is 
non-distributed in nature. Hence it is slower when comparing with 
the equivalent Frovedis algorithm (see frovedis manual for ml/logistic_regression) with 
big dataset. Thus in this implementation, a scikit-learn client can interact with 
a frovedis server sending the required python data for training at frovedis side. 
Python data is converted into frovedis compatible data internally and the 
scikit-learn ML call is linked with the respective frovedis ML call to get the 
job done at frovedis server. 

Scikit-learn side call for Logistic Regression quickly returns, 
right after submitting the training request to the frovedis server with a unique 
model ID for the submitted training request. 

When operations like prediction will be required on the trained model, scikit-learn 
client sends the same request to frovedis server on the same model 
(containing the unique ID) and the request is served at frovedis server and output 
is sent back to the scikit-learn client. 

### Detailed Description  

#### LogisticRegression()   

__Parameters__ 
_penalty_: A string object containing the regularizer type to use. (Default: 'l2')    
_dual_: A boolean parameter (unused)      
_tol_: A double parameter specifying the convergence tolerance value, (Default: 1e-4)     
_C_: A double parameter containing the learning rate. (Default: 0.01)    
_fit\_intercept_: A boolean parameter specifying whether a constant (intercept) 
should be added to the decision function. (Default: True)     
_intercept\_scaling_: An integer parameter. (unused)    
_class_weight_: A python dictionary or a string object. (unused)    
_random\_state_: An integer, None or RandomState instance. (unused)   
_solver_: A string object specifying the solver to use. (Default: 'sag')   
_max\_iter_: An integer parameter specifying maximum iteration count. (Default: 1000)    
_multi\_class_: A string object specifying type of classification. (Default: 'ovr')   
_verbose_: An integer object specifying the log level to use. (Default: 0)   
_warm\_start_: A boolean parameter. (unused)     
_n\_jobs_: An integer parameter. (unused)

__Purpose__    
It initialized a Lasso object with the given parameters.   

The parameters: "dual", "intercept_scaling", "class_weight", "warm_start", 
"random_state" and "n_jobs" are not yet supported at frovedis side. 
Thus they don't have any significance in this call. They are simply provided 
for the compatibility with scikit-learn application.    

"penalty" can be either 'l1' or 'l2' (Default: 'l2').    

"solver" can be either 'sag' for frovedis side stochastic gradient descent or 
'lbfgs' for frovedis side LBFGS optimizer when optimizing the linear regression 
model. 

"multi_class" can only be 'ovr' as frovedis suppots binary classification 
algorithms only at this moment.   

"verbose" value is set at 0 by default.
But it can be set to 1 (for DEBUG mode) or 2 (for TRACE mode) for getting 
training time logs from frovedis server.   

__Return Value__    
It simply returns "self" reference. 

#### fit(X, y, sample_weight=None)
__Parameters__   
_X_: A scipy sparse matrix or any python array-like object or an instance 
of FrovedisCRSMatrix.    
_y_: Any python array-like object or an instance of FrovedisDvector.     
_sample\_weight_: Python array-like optional parameter. (unused)   

__Purpose__    
It accepts the training feature matrix (X) and corresponding output labels (y) 
as inputs from the user and trains a linear regression model with specifed 
regularization with those data at frovedis server. 

It doesn't support any initial weight to be passed as input at this moment. 
Thus the "sample_weight" parameter will simply be ignored. It starts with an 
initial guess of zeros for the model vector and keeps updating the model to 
minimize the cost function until convergence is achieved or maximum iteration count 
is reached.  

For example,   

    # loading sample CRS data file
    mat = FrovedisCRSMatrix().load("./sample")
    lbl = FrovedisDvector([1,0,1,1,1,0,1,1])
    
    # fitting input matrix and label on logistic regression object
    lr = LogisticRegression(solver='sgd', verbose=2).fit(mat,lbl)

__Return Value__  
It simply returns "self" reference.   
Note that the call will return quickly, right after submitting the fit request 
at frovedis server side with a unique model ID for the fit request. It may be 
possible that the training is not completed at the frovedis server side even 
though the client scikit-learn side fit() returns. 

#### predict(X)
__Parameters__   
_X_: A scipy sparse matrix or any python array-like object or an instance 
of FrovedisCRSMatrix.    

__Purpose__    
It accepts the test feature matrix (X) in order to make prediction on the 
trained model at frovedis server. 

__Return Value__  
It returns a numpy array of double (float64) type containing the predicted 
outputs. 

#### predict_proba(X)
__Parameters__   
_X_: A scipy sparse matrix or any python array-like object or an instance 
of FrovedisCRSMatrix.    

__Purpose__    
It accepts the test feature matrix (X) in order to make prediction on the 
trained model at frovedis server. But unlike predict(), it returns the 
probability values against each input sample to be positive.   

__Return Value__  
It returns a numpy array of double (float64) type containing the prediction 
probability values. 


#### save(filename)
__Parameters__   
_filename_: A string object containing the name of the file on which the target 
model is to be saved.    

__Purpose__    
On success, it writes the model information (weight values etc.) in the 
specified file as little-endian binary data. Otherwise, it throws an exception. 

__Return Value__  
It returns nothing.   

#### load(filename)
__Parameters__   
_filename_: A string object containing the name of the file having model 
information to be loaded.    

__Purpose__    
It loads the model from the specified file (having little-endian binary data).

__Return Value__  
It simply returns "self" instance.   

#### debug_print()

__Purpose__    
It shows the target model information (weight values etc.) on the server side 
user terminal. It is mainly used for debugging purpose.   

__Return Value__  
It returns nothing.   

#### release()

__Purpose__    
It can be used to release the in-memory model at frovedis server. 

__Return Value__  
It returns nothing.   

## SEE ALSO  
linear_svm, dvector, crs_matrix   


# Linear SVM  

## NAME

Linear SVM (Support Vector Machines) - A classification algorithm 
to predict the binary output with hinge loss.  

## SYNOPSIS

class frovedis.mllib.svm.LinearSVC (penalty='l2', loss='hinge', dual=True, tol=1e-4,   
\  \ \  \  \  \ C=0.01, multi_class='ovr', fit_intercept=True,    
\  \ \  \  \  \ intercept_scaling=1, class_weight=None, verbose=0,    
\  \ \  \  \  \ random_state=None, max_iter=1000, solver='sag')      

### Public Member Functions

fit(X, y, sample_weight=None)   
predict(X)  
predict_proba (X)
save(filename)   
load(filename)   
debug_print()   
release()   

## DESCRIPTION
Classification aims to divide items into categories. 
The most common classification type is binary classification, where there are 
two categories, usually named positive and negative. Frovedis supports binary 
classification algorithms only. 

The Linear SVM is a standard method for large-scale classification tasks. 
It is a linear method with the loss function given by the **hinge loss**:   

    L(w;x,y) := max{0, 1-ywTx}

Where the vectors x are the training data examples and y are their corresponding 
labels (Frovedis considers negative response as -1 and positive response as 1, but 
when calling from scikit-learn interface, user should pass 0 for negative response 
and 1 for positive response according to the scikit-learn requirement) which we 
want to predict. w is the linear model (also known as weight) which uses a 
single weighted sum of features to make a prediction. Linear SVM supports ZERO, 
L1 and L2 regularization to address the overfit problem.     

The gradient of the hinge loss is: -y.x, if ywTx < 1, 0 otherwise.    
The gradient of the L1 regularizer is: sign(w)     
And The gradient of the L2 regularizer is: w     

For binary classification problems, the algorithm outputs a binary svm 
model. Given a new data point, denoted by x, the model makes 
predictions based on the value of wTx. 

By default (threshold=0), if wTx >= 0, then the response is positive (1), 
else the response is negative (0).

Frovedis provides implementation of linear SVM with two different 
optimizers: (1) stochastic gradient descent with minibatch and (2) LBFGS 
optimizer. 

The simplest method to solve optimization problems of the form **min f(w)** 
is gradient descent. Such first-order optimization methods well-suited for 
large-scale and distributed computation. Whereas, L-BFGS is an optimization 
algorithm in the family of quasi-Newton methods to solve the optimization 
problems of the similar form. 

Like the original BFGS, L-BFGS (Limited Memory BFGS) uses an estimation to 
the inverse Hessian matrix to steer its search through feature space, 
but where BFGS stores a dense nxn approximation to the inverse Hessian 
(n being the number of features in the problem), L-BFGS stores only a few 
vectors that represent the approximation implicitly. L-BFGS often achieves 
rapider convergence compared with other first-order optimization.

This module provides a client-server implementation, where the client 
application is a normal python scikit-learn program. Scikit-learn has its own 
svm module providing the LinearSVC (Support Vector Classification) support. 
But that algorithm is non-distributed in nature. Hence it is slower when comparing 
with the equivalent Frovedis algorithm (see frovedis manual for ml/linear_svm) with 
big dataset. Thus in this implementation, a scikit-learn client can interact with 
a frovedis server sending the required python data for training at frovedis side. 
Python data is converted into frovedis compatible data internally and the 
scikit-learn ML call is linked with the respective frovedis ML call to get the 
job done at frovedis server. 

Scikit-learn side call for Linear SVC quickly returns, 
right after submitting the training request to the frovedis server with a unique 
model ID for the submitted training request. 

When operations like prediction will be required on the trained model, scikit-learn 
client sends the same request to frovedis server on the same model 
(containing the unique ID) and the request is served at frovedis server and output 
is sent back to the scikit-learn client. 

### Detailed Description  

#### LinearSVC()   

__Parameters__       
_penalty_: A string object containing the regularizer type to use. (Default: 'l2')    
_loss_: A string object containing the loss function type to use. (Default: 'hinge')    
_dual_: A boolean parameter (unused)      
_tol_: A double parameter specifying the convergence tolerance value, (Default: 1e-4)     
_C_: A double parameter containing the learning rate. (Default: 0.01)    
_multi\_class_: A string object specifying type of classification. (Default: 'ovr')   
_fit\_intercept_: A boolean parameter specifying whether a constant (intercept) 
should be added to the decision function. (Default: True)     
_intercept\_scaling_: An integer parameter. (unused)    
_class\_weight_: A python dictionary or a string object. (unused)    
_verbose_: An integer object specifying the log level to use. (Default: 0)   
_random\_state_: An integer, None or RandomState instance. (unused)   
_max\_iter_: An integer parameter specifying maximum iteration count. (Default: 1000)    
_solver_: A string object specifying the solver to use. (Default: 'sag')   

__Purpose__    
It initialized a Lasso object with the given parameters.   

The parameters: "dual", "intercept_scaling", "class_weight", and 
"random_state" are not yet supported at frovedis side. 
Thus they don't have any significance in this call. They are simply provided 
for the compatibility with scikit-learn application.    

"penalty" can be either 'l1' or 'l2' (Default: 'l2').    
"loss" value can only be 'hinge'.   

"solver" can be either 'sag' for frovedis side stochastic gradient descent or 
'lbfgs' for frovedis side LBFGS optimizer when optimizing the linear regression 
model. 

"multi_class" can only be 'ovr' as frovedis suppots binary classification 
algorithms only at this moment.   

"verbose" value is set at 0 by default.
But it can be set to 1 (for DEBUG mode) or 2 (for TRACE mode) for getting 
training time logs from frovedis server.   

__Return Value__    
It simply returns "self" reference. 

#### fit(X, y, sample_weight=None)
__Parameters__   
_X_: A scipy sparse matrix or any python array-like object or an instance 
of FrovedisCRSMatrix.    
_y_: Any python array-like object or an instance of FrovedisDvector.     
_sample\_weight_: Python array-like optional parameter. (unused)   

__Purpose__    
It accepts the training feature matrix (X) and corresponding output labels (y) 
as inputs from the user and trains a linear regression model with specifed 
regularization with those data at frovedis server. 

It doesn't support any initial weight to be passed as input at this moment. 
Thus the "sample_weight" parameter will simply be ignored. It starts with an 
initial guess of zeros for the model vector and keeps updating the model to 
minimize the cost function until convergence is achieved or maximum iteration count 
is reached.  

For example,   

    # loading sample CRS data file
    mat = FrovedisCRSMatrix().load("./sample")
    lbl = FrovedisDvector([1,0,1,1,1,0,1,1])
    
    # fitting input matrix and label on linear SVC object
    lr = LinearSVC(solver='sgd', verbose=2).fit(mat,lbl)

__Return Value__  
It simply returns "self" reference.   
Note that the call will return quickly, right after submitting the fit request 
at frovedis server side with a unique model ID for the fit request. It may be 
possible that the training is not completed at the frovedis server side even 
though the client scikit-learn side fit() returns. 

#### predict(X)
__Parameters__   
_X_: A scipy sparse matrix or any python array-like object or an instance 
of FrovedisCRSMatrix.    

__Purpose__    
It accepts the test feature matrix (X) in order to make prediction on the 
trained model at frovedis server. 

__Return Value__  
It returns a numpy array of double (float64) type containing the predicted 
outputs. 

#### predict_proba(X)
__Parameters__   
_X_: A scipy sparse matrix or any python array-like object or an instance 
of FrovedisCRSMatrix.    

__Purpose__    
It accepts the test feature matrix (X) in order to make prediction on the 
trained model at frovedis server. But unlike predict(), it returns the 
probability values against each input sample to be positive.   

__Return Value__  
It returns a numpy array of double (float64) type containing the prediction 
probability values. 

#### save(filename)
__Parameters__   
_filename_: A string object containing the name of the file on which the target 
model is to be saved.    

__Purpose__    
On success, it writes the model information (weight values etc.) in the 
specified file as little-endian binary data. Otherwise, it throws an exception. 

__Return Value__  
It returns nothing.   

#### load(filename)
__Parameters__   
_filename_: A string object containing the name of the file having model 
information to be loaded.    

__Purpose__    
It loads the model from the specified file (having little-endian binary data).

__Return Value__  
It simply returns "self" instance.   

#### debug_print()

__Purpose__    
It shows the target model information (weight values etc.) on the server side 
user terminal. It is mainly used for debugging purpose.   

__Return Value__  
It returns nothing.   

#### release()

__Purpose__    
It can be used to release the in-memory model at frovedis server. 

__Return Value__  
It returns nothing.   

## SEE ALSO  
logistic_regression, dvector, crs_matrix    


# Matrix Factorization using ALS 

## NAME

Matrix Factorization using ALS - A matrix factorization algorithm commonly 
used for recommender systems.   

## SYNOPSIS

class frovedis.mllib.recommendation.ALS (max_iter=100, alpha=0.01, regParam=0.01,     
\  \ \  \  \  \ seed=0, verbose=0)     

### Public Member Functions

fit(X, rank)   
predict(id)  
recommend_users (pid, k)   
recommend_products (uid, k)    
save(filename)   
load(filename)   
debug_print()   
release()   
 
## DESCRIPTION
Collaborative filtering is commonly used for recommender systems. 
These techniques aim to fill in the missing entries of a user-item 
association matrix. Frovedis currently supports model-based collaborative 
filtering, in which users and products are described by a small set of 
latent factors that can be used to predict missing entries. 

Frovedis uses the alternating least squares (ALS) algorithm to learn these 
latent factors. The algorithm is based on a paper “Collaborative Filtering 
for Implicit Feedback Datasets” by Hu, et al.

This module provides a client-server implementation, where the client 
application is a normal python scikit-learn program. Scikit-learn does not have
any collaborative filtering recommender algorithms like ALS. In this 
implementation, scikit-learn side recommender interfaces are provided, where 
a scikit-learn client can interact with a frovedis server sending the required 
python data for training at frovedis side. Python data is converted into frovedis 
compatible data internally and the scikit-learn ALS call is linked with the 
frovedis ALS call to get the job done at frovedis server. 

Scikit-learn side call for ALS quickly returns, 
right after submitting the training request to the frovedis server with a unique 
model ID for the submitted training request. 

When operations like recommendation will be required on the trained model, 
scikit-learn client sends the same request to frovedis server on the same model 
(containing the unique ID) and the request is served at frovedis server and output 
is sent back to the scikit-learn client. 

### Detailed Description 
 
#### ALS ()   
__Parameters__   
_max\_iter_: An integer parameter specifying maximum iteration count. (Default: 100)    
_alpha_: A double parameter containing the learning rate (Default: 0.01)    
_regParam_: A double parameter containing the regularization parameter (Default: 0.01)     
_seed_: A long parameter containing the seed value to initialize the 
model structures with random values. (Default: 0)        
_verbose_: An integer parameter specifying the log level to use. (Default: 0)    

__Purpose__    

It initialized an ALS object with the given parameters.   

"verbose" value is set at 0 by default.
But it can be set to 1 (for DEBUG mode) or 2 (for TRACE mode) for getting 
training time logs from frovedis server.   

__Return Value__    
It simply returns "self" reference. 

#### fit(X, rank)
__Parameters__   
_X_: A scipy sparse matrix or any python array-like object or an instance 
of FrovedisCRSMatrix.    
_rank_: An integer parameter containing the user given rank for the input matrix.   
 
__Purpose__    
It accepts the training matrix (X) and trains a matrix factorization model 
on that at frovedis server. 

It starts with initializing the model structures of the size MxF 
and NxF (where M is the number of users and N is the products in the given 
rating matrix and F is the given rank) with random values and keeps 
updating them until maximum iteration count is reached. 

For example,   

    # loading sample CRS data file
    mat = FrovedisCRSMatrix().load("./sample")
    
    # fitting input matrix on ALS object
    als = ALS().fit(mat,rank=4)

__Return Value__  
It simply returns "self" reference.   
Note that the call will return quickly, right after submitting the fit request 
at frovedis server side with a unique model ID for the fit request. It may be 
possible that the training is not completed at the frovedis server side even 
though the client scikit-learn side fit() returns. 

#### predict(ids)
__Parameters__   
_ids_: A python tuple or list object containing the pairs of user id and product 
id to predict.       

__Purpose__    
It accepts a list of pair of user ids and product ids (0-based ID) in order to make 
prediction for their ratings from the trained model at frovedis server. 

For example,

    # this will print the predicted ratings for the given list of id pairs
    print als.predict([(1,1), (0,1), (2,3), (3,1)])   

__Return Value__  
It returns a numpy array of double (float64) type containing the predicted ratings.   

#### recommend_users(pid, k)
__Parameters__   
_pid_: An integer parameter specifying the product ID (0-based) for which
to recommend users.   
_k_: An integer parameter specifying the number of users to be recommended.   

__Purpose__    
It recommends the best "k" users with highest rating confidence in sorted 
order for the given product.    

If k > number of rows (number of users in the given matrix when training the 
model), then it resets the k as "number of rows in the given matrix" in order 
to recommend all the users with rating confidence values in sorted order.   

__Return Value__  
It returns a python list containing the pairs of recommended users and 
their corresponding rating confidence values in sorted order.    

#### recommend_products(uid, k)
__Parameters__   
_uid_: An integer parameter specifying the user ID (0-based) for which
to recommend products.   
_k_: An integer parameter specifying the number of products to be recommended.   

__Purpose__    
It recommends the best "k" products with highest rating confidence in sorted 
order for the given user.    

If k > number of columns (number of products in the given matrix when training the 
model), then it resets the k as "number of columns in the given matrix" in order 
to recommend all the products with rating confidence values in sorted order.   

__Return Value__  
It returns a python list containing the pairs of recommended products and 
their corresponding rating confidence values in sorted order.    

#### save(filename)
__Parameters__   
_filename_: A string object containing the name of the file on which the target 
model is to be saved.    

__Purpose__    
On success, it writes the model information (user-product features etc.) in the 
specified file as little-endian binary data. Otherwise, it throws an exception. 

__Return Value__  
It returns nothing.   

#### load(filename)
__Parameters__   
_filename_: A string object containing the name of the file having model 
information to be loaded.    

__Purpose__    
It loads the model from the specified file (having little-endian binary data).

__Return Value__  
It simply returns "self" instance.   

#### debug_print()

__Purpose__    
It shows the target model information on the server side user terminal. 
It is mainly used for debugging purpose.   

__Return Value__  
It returns nothing.   

#### release()

__Purpose__    
It can be used to release the in-memory model at frovedis server. 

__Return Value__  
It returns nothing.   


# kmeans 

## NAME

kmeans - A clustering algorithm commonly used in EDA 
(exploratory data analysis).    

## SYNOPSIS

class frovedis.mllib.cluster.KMeans (n_clusters=8, init='k-means++',     
\  \ \  \  \  \ n_init=10, max_iter=300, tol=1e-4, precompute_distances='auto',    
\  \ \  \  \  \ verbose=0, random_state=None, copy_x=True,    
\  \ \  \  \  \ n_jobs=1, algorithm='auto')         

### Public Member Functions

fit(X, y=None)   
predict(X)  
save(filename)   
load(filename)   
debug_print()   
release()   

## DESCRIPTION
Clustering is an unsupervised learning problem whereby we aim to group subsets 
of entities with one another based on some notion of similarity. 
K-means is one of the most commonly used clustering algorithms that clusters 
the data points into a predefined number of clusters (K).  

This module provides a client-server implementation, where the client 
application is a normal python scikit-learn program. Scikit-learn has its own 
cluster module providing kmeans support. But that algorithm is 
non-distributed in nature. Hence it is slower when comparing with 
the equivalent Frovedis algorithm (see frovedis manual for ml/kmeans) with 
big dataset. Thus in this implementation, a scikit-learn client can interact with 
a frovedis server sending the required python data for training at frovedis side. 
Python data is converted into frovedis compatible data internally and the 
scikit-learn ML call is linked with the respective frovedis ML call to get the 
job done at frovedis server. 

Scikit-learn side call for kmeans quickly returns, 
right after submitting the training request to the frovedis server with a unique 
model ID for the submitted training request. 

When operations like prediction will be required on the trained model, scikit-learn 
client sends the same request to frovedis server on the same model 
(containing the unique ID) and the request is served at frovedis server and output 
is sent back to the scikit-learn client. 

### Detailed Description  

#### KMeans()   

__Parameters__ 
_n\_clusters_: An integer parameter specifying the number of clusters. (Default: 8)   
_init_: A string object. (unused)    
_n\_init_: An integer parameter. (unused)    
_max\_iter_: An integer parameter specifying the maximum iteration count. (Default: 300)   
_tol_: A double parameter specifying the convergence tolerance. (Default: 1e-4)    
_precompute\_distances_: A string object. (unused)   
_verbose_: An integer object specifying the log level to use. (Default: 0)   
_random\_state_: An integer, None or RandomState instance. (unused)   
_copy\_X_: A boolean parameter. (unused)     
_n\_jobs_: An integer parameter. (unused)   
_algorithm_: A string object. (unused)   

__Purpose__    
It initialized a KMeans object with the given parameters.   

The parameters: "init", "n_init", "precompute_distances", "random_state", 
"copy_X", "n_jobs" and "algorithms" are not yet supported at frovedis side. 
Thus they don't have any significance in this call. They are simply provided 
for the compatibility with scikit-learn application.    

"verbose" value is set at 0 by default.
But it can be set to 1 (for DEBUG mode) or 2 (for TRACE mode) for getting 
training time logs from frovedis server.   

__Return Value__    
It simply returns "self" reference. 

#### fit(X, y=None)
__Parameters__   
_X_: A scipy sparse matrix or any python array-like object or an instance 
of FrovedisCRSMatrix.    
_y_: None (simply ignored in scikit-learn as well).    

__Purpose__    

It clusters the given data points (X) into a predefined number (k) of clusters.   

For example,   

    # loading sample CRS data file
    mat = FrovedisCRSMatrix().load("./sample")
    
    # fitting input matrix on kmeans object
    kmeans = KMeans(n_clusters=2, verbose=2).fit(mat)   

__Return Value__  
It simply returns "self" reference.   
Note that the call will return quickly, right after submitting the fit request 
at frovedis server side with a unique model ID for the fit request. It may be 
possible that the training is not completed at the frovedis server side even 
though the client scikit-learn side fit() returns. 

#### predict(X)    
__Parameters__   
_X_: A scipy sparse matrix or any python array-like object or an instance 
of FrovedisCRSMatrix.    

__Purpose__    
It accepts the test data points (X) and returns the centroid information.  

__Return Value__  
It returns a numpy array of integer (int32) type containing the centroid values.   

#### save(filename)
__Parameters__   
_filename_: A string object containing the name of the file on which the target 
model is to be saved.    

__Purpose__    
On success, it writes the model information in the specified file as little-endian 
binary data. Otherwise, it throws an exception. 

__Return Value__  
It returns nothing.   

#### load(filename)
__Parameters__   
_filename_: A string object containing the name of the file having model 
information to be loaded.    

__Purpose__    
It loads the model from the specified file (having little-endian binary data).

__Return Value__  
It simply returns "self" instance.   

#### debug_print()

__Purpose__    
It shows the target model information on the server side user terminal. 
It is mainly used for debugging purpose.   

__Return Value__  
It returns nothing.   

#### release()

__Purpose__    
It can be used to release the in-memory model at frovedis server. 

__Return Value__  
It returns nothing.   
