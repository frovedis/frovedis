% spectral embedding

# NAME
spectral embedding - Spectral embedding is useful for reducing the dimensionality of data that is expected to lie on a low-dimensional manifold contained within a high-dimensional space, it yields a low-dimensional representation of the data that best preserves the structure of the original manifold in the sense that points that are close to each other on the original manifold will also be close after embedding. At the same time, the embedding emphasizes clusters in the original data.

<br>

# SYNOPSIS
`#include <frovedis/ml/clustering/spectral_embedding.hpp>`  

`spectral_embedding_model<T>`   
frovedis::spectral_embedding(`rowmajor_matrix<T>`& mat,  
\  \  \  \  \  \  \  \  \  \ int n_comp = 2,  
\  \  \  \  \  \  \  \  \  \ bool norm_laplacian = true,  
\  \  \  \  \  \  \  \  \  \ bool precomputed = false,  
\  \  \  \  \  \  \  \  \  \ bool drop_first = true,  
\  \  \  \  \  \  \  \  \  \ double gamma = 1.0,  
\  \  \  \  \  \  \  \  \  \ int mode = 1)

`spectral_embedding_model<T>`  
frovedis::spectral_embedding(`rowmajor_matrix<T>`&& mat,  
\  \  \  \  \  \  \  \  \  \ int n_comp = 2,  
\  \  \  \  \  \  \  \  \  \ bool norm_laplacian = true,  
\  \  \  \  \  \  \  \  \  \ bool precomputed = false,  
\  \  \  \  \  \  \  \  \  \ bool drop_first = true,  
\  \  \  \  \  \  \  \  \  \ double gamma = 1.0,  
\  \  \  \  \  \  \  \  \  \ int mode = 1)

# DESCRIPTION
Spectral embedding is the accurate method for extraction of meaningful patterns in high dimensional data. It forms an affinity matrix given by the specified function and applies spectral decomposition to the corresponding graph laplacian. The resulting transformation is given by the value of the eigenvectors for each data point.

## Detailed Description  
### Public Global Function Documentation  

`spectral_embedding_model<T>`  
spectral_embedding(mat,n_comp,norm_laplacian,precomputed,drop_first,gamma,mode)

__Parameters__   

_mat_: A rowmajor_matrix of type "T"(where T can be either float or double) containing n-dimensional data points. It can be treated as either input data matrix or precomputed affinity matrix, based on the boolean parameter "precomputed". The input can be passed as an lvalue or rvalue. The matrix is internally cleared to save computation memory during the algorithm, in case the input matrix is a rvalue.   
_n\_comp_: An integer parameter containing the number of components for clusters(Default: 2)  
_norm\_laplacian_: A boolean parameter if set True, then compute normalized Laplacian else not(Default: true)  
_precomputed_: A boolean parameter if set False, then internally affinity matrix is computed based on input matrix otherwise input matrix is treated as precomputed affinity matrix and internal affinity computation is skipped(Default: false)  
_drop\_first_: A boolean parameter if set True, then drops the first eigenvector. The first eigenvector of a normalized laplacian is full of constants, thus if drop_first is set true, compute (n_comp+1) eigenvectors and will drop the first vector. Otherwise it will calculate n_comp number of eigenvectors(Default: true)  
_gamma_: The value required for computing nearby relational meaningful eigenvalues(Default: 1.0)  
_mode_: A parameter required to set the eigen computation method. It can be either 1 or 3, 1 for generic and 3 for shift-invert mode(Default: 1)  


__Purpose__  

After getting the affinity matrix by computing distance co-relation, this is used to extract meaningful patterns using normalized eigenvectors.

__Return Value__  
It returns a `spectral_embedding_model<T>` containing the values of the largest eigenvectors obtained from the normalized laplacian.

# SEE ALSO
spectral_clustering, spectral_embedding_model

