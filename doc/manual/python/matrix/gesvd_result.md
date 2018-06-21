% gesvd_result 

# NAME
gesvd_result - a structure to model the output of frovedis singular value 
decomposition methods. 

# SYNOPSIS

import frovedis.matrix.results.GesvdResult   

## Public Member Functions
to_numpy_results()   
save(svec, umat=None, vmat=None)    
save_binary(svec, umat=None, vmat=None)    
load (svec, umat=None, vmat=None, mtype='B')   
load_binary (svec, umat=None, vmat=None, mtype='B')   
debug_print()   
release()   
stat()   
getK()    

# DESCRIPTION

GesvdResult is a python side pseudo result structure containing the 
proxies of the in-memory SVD results created at frovedis server side. It can be 
used to convert the frovedis side SVD result to python equivalent data structures.  

## Public Member Function Documentation
 
### to_numpy_results()
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

### save(svec, umat=None, vmat=None)   
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

### save_binary (svec, umat=None, vmat=None)   
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

### load(svec, umat=None, vmat=None, mtype='B')   
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

### load_binary(svec, umat=None, vmat=None, mtype='B')   
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

### debug_print()
__Purpose__   
This function can be used to print the result components at server side 
user terminal. This is useful in debugging purpose.

__Return Type__    
It returns nothing.   

### release()
__Purpose__   
This function can be used to release the in-memory result components at frovedis 
server.

__Return Type__    
It returns nothing.   

### stat()
__Purpose__   
This function returns the exit status of the scalapack native gesvd routine 
on calling of which the target result object was obtained. 

__Return Type__    
An integer value.   

### getK()
__Purpose__   
This function returns the number of singular values computed.

__Return Type__    
An integer value.   
