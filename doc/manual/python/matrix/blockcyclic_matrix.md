% FrovedisBlockcyclicMatrix 

# NAME

FrovedisBlockcyclicMatrix -  A data structure used in modeling the in-memory 
blockcyclic matrix data of frovedis server side at client python side. 

# SYNOPSIS

class frovedis.matrix.dense.FrovedisBlockcyclicMatrix(mat=None)    

## Overloaded Operators   
operator= (mat)   
operator+ (mat)   
operator- (mat)   
operator* (mat)   
operator~ (mat)   

## Public Member Functions
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

# DESCRIPTION

FrovedisBlockcyclicMatrix is a pseudo matrix structure at client python side 
which aims to model the frovedis server side `blockcyclic_matrix<double>` 
(see manual of frovedis blockcyclic_matrix for details).   

Note that the actual matrix data is created at frovedis server side only. 
Python side FrovedisBlockcyclicMatrix contains a proxy handle of the in-memory matrix 
data created at frovedis server, along with number of rows and number of columns 
information.

## Constructor Documentation
### FrovedisBlockcyclicMatrix (mat=None)   
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

## Overloaded Operators Documentation

### operator= (mat)

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

### operator+ (mat)

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

### operator- (mat)

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

### operator\* (mat)

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

### operator\~ ()

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

## Pubic Member Function Documentation

### load (mat)   
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

### load_numpy_matrix (mat)   
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
 
### load_text (filename)   
__Parameters__    
_filename_:  A string object containing the text file name to be loaded.   

__Purpose__    
This function can be used to load the data from a text file into the target 
matrix. Note that the file must be placed at server side 
at the given path. 

__Return Type__    
It simply returns "self" reference.   

### load_binary (dirname)   
__Parameters__    
_dirname_:  A string object containing the directory name having the binary 
data to be loaded.   

__Purpose__    
This function can be used to load the data from the specified directory with 
binary data file into the target matrix. Note that the file must be placed at server side 
at the given path. 

__Return Type__    
It simply returns "self" reference.   

### save_text (filename)   
__Parameters__    
_filename_:  A string object containing the text file name in which the data 
is to be saved.   

__Purpose__    

This function is used to save the target matrix as text file with the filename 
at the given path. Note that the file will be saved at server side 
at the given path.  

__Return Type__    
It returns nothing.    

### save_binary (dirname) 
__Parameters__    
_dirname_:  A string object containing the directory name in which the data 
is to be saved as little-endian binary form.   

__Purpose__    

This function is used to save the target matrix as little-endian binary file 
with the filename at the given path. Note that the file will be saved at 
server side at the given path.  

__Return Type__    
It returns nothing.    

### transpose () 
__Purpose__    

This function will compute the transpose of the given matrix. 

__Return Type__    
It returns the transposed blockcyclic matrix of the type FrovedisBlockcyclicMatrix.

### to_numpy_matrix ()   

__Purpose__   
This function is used to convert the target blockcyclic matrix into numpy matrix.  
Note that this function will request frovedis server to gather the distributed data, 
and send back that data in the rowmajor array form and the python client
will then convert the received numpy array from frovedis server to python 
numpy matrix. 

__Return Type__    
It returns a two-dimensional dense numpy matrix    

### numRows()
__Purpose__   
It returns the number of rows in the matrix

__Return Type__   
An integer value containing rows count in the target matrix.      

### numCols()
__Purpose__   
It returns the number of columns in the matrix

__Return Type__   
An integer value containing columns count in the target matrix.      

### debug_print()  
__Purpose__   
It prints the contents of the server side distributed matrix data on the server 
side user terminal. It is mainly useful for debugging purpose.

__Return Type__    
It returns nothing.    

### release()  
__Purpose__   
This function can be used to release the existing in-memory data at frovedis 
server side.

__Return Type__   
It returns nothing.    

### FrovedisBlockcyclicMatrix.asBCM(mat)
__Parameters__      
_mat_: An instance of FrovedisBlockcyclicMatrix or any python array-like structure.   

__Purpose__   
This static function is used in order to convert a given matrix to a blockcyclic 
matrix. If the input is already an instance of FrovedisBlockcyclicMatrix, then 
the same will be returned. 

__Return Type__   

An instance of FrovedisBlockcyclicMatrix.   
