% spectral_clustering_model

# NAME
`spectral_clustering_model<T>` - A data structure used in modeling the outputs of the frovedis spectral clustering algorithm.

# SYNOPSIS
`#include <frovedis/ml/clustering/spectral_clustering_model.hpp>`  

## Constructors
`spectral_clustering_model<T>()`  
`spectral_clustering_model<T>(rowmajor_matrix<T>& aff, std::vector<int>`& lbl, int ncluster)   

## Public Member Functions  
void save(const std::string& dir)  
void savebinary(const std::string& dir)  
void load(const std::string& dir)  
void loadbinary(const std::string& dir)  
void debug_print()  

## DESCRIPTION
`spectral_clustering_model<T>` models the output of the frovedis spectral clustering algorithms. This model has the below structure:  
`template <class T>`  
struct spectral_clustering_model {  
\  \  \  \  \  \  `rowmajor_matrix<T>` affinity_matrix,  
\  \  \  \  \  \  `std::vector<int>` labels,  
\  \  \  \  \  \   int nclusters) };  

This is a template based data structure, where "T" is supposed to be "float" (single-precision) or "double" (double-precision). Note this is a serialized data structure. The detailed description can be found in subsequent sections.

## Public Member Function Documentation

__void save(const std::string& dir)__   

It saves the target model in the specified path in simple text format. It will throw an exception, if any error occurs during the save operation.

__void savebinary(const std::string& dir)__   

It saves the target model in the specified path in (little-endian) binary data format. It will throw an exception, if any error occurs during the save operation.

__void load(const std::string& dir)__   

It loads the target linear regression model from the data in specified text file. It will throw an exception, if any error occurs during the load operation.

__void loadbinary(const std::string& dir)__   

It loads the target linear regression model from the data in specified (little-endian) binary file. It will throw an exception, if any error occurs during the load operation.

__void debug_print()__   

It prints the contents of the model on the user terminal. It is mainly useful for debugging purpose.

## Public Data Member Documentation
__affinity_matrix:__ `A rowmajor_matrix<T>` that return the values of distance co-related data values required for model building.

__labels:__ It contains the final cluster output obtained from the successful creation of the model.

__nclusters:__ An integer parameter containing the number of components for clusters.

# SEE ALSO
spectral_clustering, spectral_embedding_model
