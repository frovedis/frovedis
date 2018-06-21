% getrf_result 

# NAME
getrf_result - a structure to model the output of frovedis wrapper of scalapack 
getrf routine. 

# SYNOPSIS

import com.nec.frovedis.matrix.GetrfResult   

## Public Member Functions
Unit release()  
Long ipiv()     
Int stat()   

# DESCRIPTION

GetrfResult is a client spark side pseudo result structure containing the 
proxy of the in-memory scalapack getrf result (node local ipiv vector) created 
at frovedis server side. 

## Public Member Function Documentation
 
### Unit release()
This function can be used to release the in-memory result component (ipiv 
vector) at frovedis server.

### Long ipiv()
This function returns the proxy of the node_local "ipiv" vector computed 
during getrf calculation. This value will be required in other scalapack 
routine calculation, like getri, getrs etc.

### Int stat()
This function returns the exit status of the scalapack native getrf routine 
on calling of which the target result object was obtained. 
