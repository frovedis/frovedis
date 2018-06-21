% FrovedisDvector

# NAME

FrovedisDvector -  A data structure used in modeling the in-memory 
dvector data of frovedis server side at client python side. 

# SYNOPSIS

class frovedis.matrix.dvector.FrovedisDvector(vec=None)    

## Public Member Functions
load (vec)   
load_numpy_array (vec)     
debug_print()   
release()   

# DESCRIPTION

FrovedisDvector is a pseudo data structure at client python side 
which aims to model the frovedis server side `dvector<double>` 
(see manual of frovedis dvector for details).   

Note that the actual vector data is created at frovedis server side only. 
Python side FrovedisDvector contains a proxy handle of the in-memory vector 
data created at frovedis server, along with its size.   

## Constructor Documentation

### FrovedisDvector (vec=None)   
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

## Pubic Member Function Documentation

### load (vec)   
__Parameters__    
_vec_: It can be any python array-like object (but not None). 

__Purpose__    

This function works similar to the constructor. 
It can be used to load a FrovedisDvector instance, as follows:

    v = FrovedisDvector().load([1,2,3,4]) # will load data from the given list   

__Return Type__    

It simply returns "self" reference.   


### load_numpy_array (vec)   
__Parameters__    
_vec_:  Any numpy array with values to be loaded in.   

__Purpose__    
This function can be used to load a python side numpy array data into 
frovedis server side dvector. It accepts a python numpy array object 
and converts it into the frovedis server side dvector whose proxy 
along size information are stored in the target FrovedisDvector object.

__Return Type__    
It simply returns "self" reference.   

### size()
__Purpose__   
It returns the size of the dvector

__Return Type__   
An integer value containing size of the target dvector.   
 
### debug_print()  
__Purpose__    
It prints the contents of the server side distributed vector data on the server 
side user terminal. It is mainly useful for debugging purpose.

__Return Type__   
It returns nothing.    

### release()  
__Purpose__    
This function can be used to release the existing in-memory data at frovedis 
server side.

__Return Type__   
It returns nothing.    

### FrovedisDvector.asDvec(vec)
__Parameters__    
_vec_: A numpy array or python array like object or an instance of FrovedisDvector.    

__Purpose__   
This static function is used in order to convert a given array to a dvector.
If the input is already an instance of FrovedisDvector, then the same will be 
returned. 

__Return Type__   

An instance of FrovedisDvector.
