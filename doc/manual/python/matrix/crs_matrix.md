% FrovedisCRSMatrix 

# NAME

FrovedisCRSMatrix -  A data structure used in modeling the in-memory 
crs matrix data of frovedis server side at client python side. 

# SYNOPSIS

class frovedis.matrix.sparse.FrovedisCRSMatrix(mat=None)    

## Public Member Functions
load (mat)    
load_scipy_matrix (mat)   
load_text (filename)   
load_binary (dirname)   
save_text (filename)    
save_binary (dirname)    
debug_print()   
release()   

# DESCRIPTION

FrovedisCRSMatrix is a pseudo matrix structure at client python side 
which aims to model the frovedis server side `crs_matrix<double>` 
(see manual of frovedis crs_matrix for details).   

Note that the actual matrix data is created at frovedis server side only. 
Python side FrovedisCRSMatrix contains a proxy handle of the in-memory matrix 
data created at frovedis server, along with number of rows and number of columns 
information.

## Constructor Documentation

### FrovedisCRSMatrix (mat=None)   
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

## Pubic Member Function Documentation

### load (mat)   
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


### load_scipy_matrix (mat)   
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
 
### load_text (filename)   
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

### load_binary (dirname)   
__Parameters__    
_dirname_:  A string object containing the directory name having the binary 
data to be loaded.   

__Purpose__    
This function can be used to load the data from the specified directory with 
binary data file into the target matrix. Note that the file must be placed at 
server side at the given path. 

__Return Type__    
It simply returns "self" reference.   

### save_text (filename)   
__Parameters__    
_filename_:  A string object containing the text file name in which the data 
is to be saveed.   

__Purpose__    

This function is used to save the target matrix as text file with the filename 
at the given path. Note that the file will be saved at server side 
at the given path.  

__Return Type__    
It returns nothing.    

### save_binary (dirname) 
__Parameters__    
_dirname_:  A string object containing the directory name in which the data 
is to be saveed as little-endian binary form.   

__Purpose__    

This function is used to save the target matrix as little-endian binary file 
with the filename at the given path. Note that the file will be saved at 
server side at the given path.  

__Return Type__    
It returns nothing.    

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

### FrovedisCRSMatrix.asCRS(mat)
__Parameters__    
_mat_: A scipy matrix, an instance of FrovedisCRSMatrix or any python array-like data.   

__Purpose__   
This static function is used in order to convert a given matrix to a crs matrix.
If the input is already an instance of FrovedisCRSMatrix, then the same will be 
returned. 

__Return Type__   

An instance of FrovedisCRSMatrix.
