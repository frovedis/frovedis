List of supported machine learning algorithms
===

All the algorithms support sparse and dense matrix with multi
card/node, unless otherwise specified.

<table>
<tr><th>Category</th> <th>Algorithm</th> <th>Comment</th> </tr>
<tr> <td rowspan="20">Classification / Regression</td>
<td>Logistic Regression</td> <td></td> </tr>
<tr><td>Multinomial Logistic Regression</td> <td></td> </tr>
<tr><td>Linear Regression</td> <td></td> </tr>
<tr><td>Ridge Regression</td> <td></td> </tr>
<tr><td>Lasso Regression</td> <td></td> </tr>
<tr><td>Linear Support Vector Classification</td> <td></td> </tr>
<tr><td>Linear Support Vector Regression</td> <td></td> </tr>
<tr><td>Kernel Support Vector Classification</td> <td>[*1]</td> </tr>
<tr><td>Decision Tree Classification</td> <td>[*3]</td> </tr>
<tr><td>Decision Tree Regression</td> <td>[*3]</td> </tr>
<tr><td>Random Forest Classification</td> <td>[*3]</td> </tr>
<tr><td>Random Forest Regression</td> <td>[*3]</td> </tr>
<tr><td>GBDT Classification</td> <td>[*3]</td> </tr>
<tr><td>GBDT Regression</td> <td>[*3]</td> </tr>
<tr><td>Nearest Neighbor Classification</td> <td>Only dense</td> </tr>
<tr><td>Nearest Neighbor Regression</td> <td>Only dense</td> </tr>
<tr><td>Unsupervised Nearest Neighbor</td> <td>Only dense</td> </tr>
<tr><td>Multinomial Naive Bayes</td> <td></td> </tr>
<tr><td>Bernoulli Naive Bayes</td> <td></td> </tr>
<tr><td>Factorization Machines</td> <td></td> </tr>
<tr><td rowspan="6">Clustering</td>
<td>K-means</td> <td></td> </tr>
<tr><td>Spectral Clustering</td> <td>Only dense</td> </tr>
<tr><td>Agglomerative Clustering</td> <td>Only dense</td> </tr>
<tr><td>DBSCAN</td> <td>Only dense</td> </tr>
<tr><td>Gaussian Mixture Model</td> <td>Only dense</td> </tr>
<tr><td>ART-2A</td> <td>Only C++</td> </tr>
<tr><td rowspan="5">Preprocess</td>
<td>Singular Value Decomposition</td> <td></td> </tr>
<tr><td>Eigen Value Decomposition</td> <td>Only C++</td> </tr>
<tr><td>Principal Component Analysis</td> <td>Only dense</td> </tr>
<tr><td>Spectral Embedding</td> <td>Only dense</td> </tr>
<tr><td>T-SNE</td> <td>Only dense</td> </tr>
<tr><td>Recommendation</td> <td>ALS</td> <td></td> </tr>
<tr><td>Basket Analysis</td> <td>FP Growth</td> <td></td> </tr>
<tr><td rowspan="2">Natural Language</td>
<td>word2vec</td> <td>[*2]</td> </tr>
<tr><td>Latent Dirichlet Allocation</td> <td></td> </tr>
<tr><td rowspan="2">Neural Network</td>
<td>Multi-layer Perceptron</td> <td>Only C++</td> </tr>
<tr><td>Convolutional Neural Network</td> <td>Only C++</td> </tr>
<tr><td rowspan="4">Graph</td>
<td>PageRank</td> <td></td> </tr>
<tr><td>Connected Components</td> <td></td> </tr>
<tr><td>Single-Source Shortest Path</td> <td></td> </tr>
<tr><td>Breadth First Search</td> <td></td> </tr>
</table>

[*1] Only support OpenMP parallelization. 
     Run with VE_OMP_NUM_THREADS=8 and mpirun -np 1

[*2] Support OpenMP + MPI parallelization. 
     Better performance with VE_OMP_NUM_THREADS=8 and one MPI process per card.

[*3] Only dense matrix is supported, but category variables can be specified