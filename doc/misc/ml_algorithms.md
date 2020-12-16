List of supported machine learning algorithms
===

All the algorithms support sparse and dense matrix with multi
card/node, unless otherwise specified.

|----------------|------------------------------------|----------|
|Category        |Algorithm                           |Comment   |
|----------------|------------------------------------|----------|
|Classification /|Logistic Regression                 |          |
|Regression      |Multinomial Logistic Regression     |          |
|                |Linear Regression                   |          |
|                |Ridge Regression                    |          |
|                |Lasso Regression                    |          |
|                |Linear Support Vector Classification|          |
|                |Linear Support Vector Regression    |          |
|                |Kernel Support Vector Classification|[*1]      |
|                |Decision Tree Classification        |[*3]      |
|                |Decision Tree Regression            |[*3]      |
|                |Random Forest Classification        |[*3]      |
|                |Random Forest Regression            |[*3]      |
|                |GBDT Classification                 |[*3]      |
|                |GBDT Regression                     |[*3]      |
|                |Nearest Neighbor Classification     |Only dense|
|                |Nearest Neighbor Regression         |Only dense|
|                |Unsupervised Nearest Neighbor       |Only dense|
|                |Multinomial Naive Bayes             |          |
|                |Bernoulli Naive Bayes               |          |
|----------------|------------------------------------|----------|
|Clustering      |K-means                             |          |
|                |Spectral Clustering                 |Only dense|
|                |Agglomerative Clustering            |Only dense|
|                |DBSCAN                              |Only dense|
|                |ART-2A                              |Only C++  |
|----------------|------------------------------------|----------|
|Preprocess      |Singular Value Decomposition        |          |
|                |Eigen Value Decomposition           |          |
|                |Principal Component Analysis        |Only dense|
|                |Spectral Embedding                  |Only dense|
|                |T-SNE                               |Only dense|
|----------------|------------------------------------|----------|
|Recommendation  |ALS                                 |          |
|                |Factorization Machines              |[*2]      |
|----------------|------------------------------------|----------|
|Basket Analysis |FP Growth                           |          |
|----------------|------------------------------------|----------|
|Natural Language|word2vec                            |[*2]      |
|                |Latent Dirichlet Allocation         |          |
|----------------|------------------------------------|----------|
|Neural Network  |Multi-layer Perceptron              |Only C++  |
|                |Convolutional Neural Network        |Only C++  |
|----------------|------------------------------------|----------|
|Graph           |PageRank                            |          |
|                |Connected Components                |          |
|                |Single-Source Shortest Path         |          |
|                |Breadth First Search                |          |
|----------------|------------------------------------|----------|

[*1] Only support OpenMP parallelization. 
     Run with VE_OMP_NUM_THREADS=8 and mpirun -np 1

[*2] Support OpenMP + MPI parallelization. 
     Better performance with VE_OMP_NUM_THREADS=8 and one MPI process per card.

[*3] Only dense matrix is supported, but category variables can be specified