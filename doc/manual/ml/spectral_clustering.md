% spectral clustering

# NAME
spectral clustering - A clustering algorithm commonly used in EDA (exploratory data analysis), using the spectrum (eigenvalues) of the similarity matrix of the data to perform clustering.

<br>

# SYNOPSIS
`#include <frovedis/ml/clustering/spectral_clustering.hpp>`

`spectral_clustering_model<T>`  
frovedis::spectral_clustering(`rowmajor_matrix<T>`& mat,  
\  \  \  \  \  \  \  \  \  \ int ncluster = 2,  
\  \  \  \  \  \  \  \  \  \ int n_comp = 2,  
\  \  \  \  \  \  \  \  \  \ int niter = 100,  
\  \  \  \  \  \  \  \  \  \ double eps = 0.01,  
\  \  \  \  \  \  \  \  \  \ bool norm_laplacian = true,  
\  \  \  \  \  \  \  \  \  \ bool precomputed = false,  
\  \  \  \  \  \  \  \  \  \ bool drop_first = false,  
\  \  \  \  \  \  \  \  \  \ double gamma = 1.0,  
\  \  \  \  \  \  \  \  \  \ int mode = 1)

`spectral_clustering_model<T>`  
frovedis::spectral_clustering(`rowmajor_matrix<T>`&& mat,  
\  \  \  \  \  \  \  \  \  \ int ncluster = 2,  
\  \  \  \  \  \  \  \  \  \ int n_comp = 2,  
\  \  \  \  \  \  \  \  \  \ int niter = 100,  
\  \  \  \  \  \  \  \  \  \ double eps = 0.01,  
\  \  \  \  \  \  \  \  \  \ bool norm_laplacian = true,  
\  \  \  \  \  \  \  \  \  \ bool precomputed = false,  
\  \  \  \  \  \  \  \  \  \ bool drop_first = false,  
\  \  \  \  \  \  \  \  \  \ double gamma = 1.0,  
\  \  \  \  \  \  \  \  \  \ int mode = 1)

# DESCRIPTION
Clustering is an unsupervised learning problem whereby we aim to group subsets of entities with one another based on some notion of similarity. In spectral clustering, the data points are treated as nodes of a graph. Thus, clustering is treated as a graph partitioning problem. The user can provide input in form of co-ordinate values for graphical representation or as affinity matrix. The components or features are identified as per column order in matrix data. The nodes are then mapped to a low-dimensional space that can be easily segregated to form clusters.

## Detailed Description
### Public Global Function Documentation  

`spectral_clustering_model<T>`  
spectral_clustering(mat,ncluster,n_comp,niter,eps,norm_laplacian,precomputed,drop_first,gamma,mode)  


__Parameters__  
 
_mat_: A rowmajor_matrix of type "T"(where T can be either float or double) containing n-dimensional data points. It can be treated as either input data matrix or precomputed affinity matrix, based on the boolean parameter "precomputed". The input can be passed as an lvalue or rvalue. The matrix is internally cleared to save computation memory during the algorithm, in case the input matrix is a rvalue.  
_ncluster_: An integer parameter containing the number of required clusters(Default: 2)  
_n\_comp_: An integer parameter containing the number of components for clusters(Default: 2)  
_niter_: An integer parameter containing the maximum number of iteration count for kmeans(Default: 300)  
_eps_: A parameter of double type containing the epsilon value for kmeans(Default: 0.1)  
_norm\_laplacian_: A boolean parameter if set True, then compute normalized Laplacian else not(Default: true)   
_precomputed_: A boolean parameter if set False, then internally affinity matrix is computed based on input matrix otherwise input matrix is treated as precomputed affinity matrix and internal affinity computation is skipped(Default: false)  
_drop\_first_: A boolean parameter if set True, then drops the first eigenvector. The first eigenvector of a normalized laplacian is full of constants, thus if drop_first is set true, compute (n_comp+1) eigenvectors and will drop the first vector. Otherwise it will calculate n_comp number of eigenvectors(Default: false)  
_gamma_: The value required for computing nearby relational meaningful eigenvalues(Default: 1.0)  
_mode_: A parameter required to set the eigen computation method. It can be either 1 or 3, 1 for generic and 3 for shift-invert mode(Default: 1)  

__Purpose__  
It computes the clusters with respect to the relational distance between the given data points, using normalized eigenvectors.  
After the successful clustering, it returns `spectral_clustering_model<T>` containing the computed cluster values.  
  
__Return Value__  
After the successful clustering it returns the `spectral_clustering_model<T>` from the computed labels containing the centroids from kmeans.

# SEE ALSO
spectral_clustering_model, spectral_embedding
