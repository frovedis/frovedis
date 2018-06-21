% arpack_wrapper

# NAME
arpack_wrapper - a frovedis module supports singular value decomposition on 
sparse data using arpack routines. 

# SYNOPSIS

import com.nec.frovedis.matrix.ARPACK    

## Public Member Functions
GesvdResult ARPACK.computeSVD (RowMatrix data, Int k)    
GesvdResult ARPACK.computeSVD (FrovedisSparseData data, Int k)    

# DESCRIPTION
This module provides interface to wrap spark computeSVD method of RowMatrix 
class using arpack native routines at frovedis server side. 

## Detailed Description 
### computeSVD (data, k)
__Parameters__  
_data_: A spark RowMatrix object or a FrovedisSparseData object   
_k_: An integer value to specify the number of singular values to compute

__Purpose__    
If "data" is a spark RowMatrix object, then internally it first converts
the RowMatrix to frovedis sparse data at frovedis server and then computes the 
singular value decomposition on the sparse data at frovedis side. Once done, 
it returns a GesvdResult object containing the proxy of the results at 
frovedis server and releases the server memory for the converted sparse data. 

If "data" is already a FrovedisSparseData object, then it directly computes the 
singular value decomposition at frovedis side and returns the GesvdResult 
containing proxy of server side results. In that case, the input sparse data 
needs to be released by the user. 

When required, the spark client can convert back the frovedis server side SVD 
result to spark equivalent result form.

For example,

    val res = ARPACK.computeSVD(data,2) // compute 2 singular values for the given data
    val r2 = res.to_spark_result(sc) // "sc" is the object of SparkContext
    
__Return Value__   
On success, it returns an object of GesvdResult type containing the proxy of 
SVD results at frovedis server side. If any error occurs, it throws an exception.   

