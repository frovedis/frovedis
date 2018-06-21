% arpack_wrapper

# NAME
arpack_wrapper - a frovedis module supports singular value decomposition on 
sparse data using arpack routines. 

# SYNOPSIS

import frovedis.matrix.wrapper.ARPACK    

## Public Member Functions
ARPACK.computeSVD (data, k)    

# DESCRIPTION
This module provides interface to compute singular value decomposition 
on sparse data using arpack native routines at frovedis server side. 

## Detailed Description 
### computeSVD (data, k)
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