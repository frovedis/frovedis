% Manual of Frovedis C++ API

# Introduction

This manual contains C++ API documantation.
If you are new to Frovedis, please read the tutorial first.

Frovedis is composed of following layers: core, matrix library, 
machine learning library, and data frame. Currently we only provide
part of core, matrix, and machine learning API documentation.
We are still updating the contents.

- Core
    + [frovedis::dvector\<T\>]
    + [frovedis::node_local\<T\>]
    + [frovedis::dunordered_map\<K,V\>]
- Matrix
    + [frovedis::rowmajor_matrix_local\<T\>]
    + [frovedis::rowmajor_matrix\<T\>]
    + [frovedis::colmajor_matrix_local\<T\>]
    + [frovedis::colmajor_matrix\<T\>]
    + [frovedis::sliced_colmajor_matrix_local\<T\>]
    + [frovedis::sliced_colmajor_vector_local\<T\>]
    + [blas_wrapper]
    + [lapack_wrapper]
    + [frovedis::blockcyclic_matrix\<T\>]
    + [frovedis::sliced_blockcyclic_matrix\<T\>]
    + [frovedis::sliced_blockcyclic_vector\<T\>]
    + [pblas_wrapper]
    + [scalapack_wrapper]
    + [frovedis::crs_matrix_local\<T,I,O\>]
    + [frovedis::crs_matrix\<T,I,O\>]
    + [frovedis::ccs_matrix_local\<T,I,O\>]
    + [frovedis::ccs_matrix\<T,I,O\>]
    + [frovedis::ell_matrix_local\<T,I\>]
    + [frovedis::ell_matrix\<T,I\>]
    + [frovedis::jds_matrix_local\<T,I,O,P\>]
    + [frovedis::jds_matrix\<T,I,O,P\>]
- Machine Learning
    + [frovedis::linear_regression_model\<T\>]
    + [Linear Regression]
    + [Lasso Regression]
    + [Ridge Regression]
    + [frovedis::logistic_regression_model\<T\>]
    + [Logistic Regression]
    + [frovedis::svm_model\<T\>]
    + [Linear SVM]
    + [frovedis::matrix_factorization_model\<T\>]
    + [Matrix Factorization using ALS]
    + [kmeans]
    + [spectral_clustering_model]
    + [spectral clustering]
    + [spectral_embedding_model]
    + [spectral embedding]
