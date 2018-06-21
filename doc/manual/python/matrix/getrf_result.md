% getrf_result 

# NAME
getrf_result - a structure to model the output of frovedis wrapper of scalapack 
getrf routine. 

# SYNOPSIS

import frovedis.matrix.results.GetrfResult   

## Public Member Functions
release()  
ipiv()     
stat()   

# DESCRIPTION

GetrfResult is a client python side pseudo result structure containing the 
proxy of the in-memory scalapack getrf result (node local ipiv vector) created 
at frovedis server side. 

## Public Member Function Documentation
 
### release()
__Purpose__   
This function can be used to release the in-memory result component (ipiv 
vector) at frovedis server.

__Return Type__   
It returns nothing. 

### ipiv()
__Purpose__    
This function returns the proxy of the node_local "ipiv" vector computed 
during getrf calculation. This value will be required in other scalapack 
routine calculation, like getri, getrs etc.

__Return Type__   
A long value containing the proxy of ipiv vector.   

### stat()   
__Purpose__    
This function returns the exit status of the scalapack native getrf routine 
on calling of which the target result object was obtained. 

__Return Type__   
It returns an integer value.   