% Latent Dirichlet Allocation 

# NAME

Latent Dirichlet Allocation(LDA) - It is a Natural Language Processing algorithm that allows a 
set of observations to be explained by unobserved groups. These are used to explain why some parts 
of the data are similar.  

# SYNOPSIS

class frovedis.mllib.decomposition.LatentDirichletAllocation(n_components=10, doc_topic_prior=None,  
\ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ 
\ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ topic_word_prior=None,  
\ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ 
\ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ learning_method='batch',  
\ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ 
\ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ learning_decay=.7, learning_offset=10.,  
\ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ 
\ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ max_iter=10, batch_size=128,  
\ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ 
\ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ evaluate_every=-1, total_samples=1e6,  
\ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ 
\ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ perp_tol=1e-1, mean_change_tol=1e-3,  
\ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ 
\ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ max_doc_update_iter=100,  
\ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ 
\ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ n_jobs=None, verbose=0,  
\ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ 
\ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ random_state=None, algorithm="original",  
\ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ 
\ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ explore_iter=0)  

## Public Member Functions  

fit(X, y = None)  
transform(X)  
perplexity(X, sub_sampling = False)  
score(X, y = None)  
get_params(deep = True)  
set_params(\*\*params)  
fit_transform(X, y = None)  
load(fname, dtype = None)  
save(fname)  
release()  
is_fitted()  

# DESCRIPTION  
Latent Dirichlet Allocation (LDA) is a widely used machine learning technique for topic 
modeling. The input is a corpus of documents and the output is per-document topic 
distribution and per-topic word distribution. 

This implementation provides python wrapper interface to VLDA, which accelerates LDA training 
by exploiting the data-level, and the thread-level parallelism using vector processors. The 
priority-aware scheduling approach is proposed to address the high memory requirement and 
workload imbalance issues with existing works. 

This module provides a client-server implementation, where the client application 
is a normal python program. The frovedis interface is almost same as Scikit-learn 
LatentDirichletAllocation interface, but it doesn't have any dependency with Scikit-learn. It 
can be used simply even if the system doesn't have Scikit-learn installed. Thus in this 
implementation, a python client can interact with a frovedis server sending the required python 
data for training at frovedis side. Python data is converted into frovedis compatible data 
internally and the python ML call is linked with the respective frovedis ML call to get the job 
done at frovedis server.  

Python side calls for LatentDirichletAllocation on the frovedis server. Once the training 
is completed with the input data at the frovedis server, it returns an abstract model with 
a unique model ID to the client python program.  

When transform-like request would be made on the trained model, python program 
will send the same request to the frovedis server. After the request is served at 
the frovedis server, the output would be sent back to the python client.  

## Detailed Description  

### 1. LatentDirichletAllocation()  

__Parameters__  
**_n\_components_**: A positive integer parameter specifying the number of topics. (Default: 10)  
**_doc\_topic\_prior_**: A float parameter which specifies the prior of document topic 
distribution theta. It is also called alpha. (Default: None)  
If it is None (not specified explicitly), it will be set as (1 / n_components).  
**_topic\_word\_prior_**: A float parameter which specifies the prior of topic word 
distribution beta. It is also called beta. (Default: None)  
If it is None (not specified explicitly), it will be set as (1 / n_components).  
**_learning\_method_**: An unused parameter which specifies the method used to 
update 'components_'.  
Although, this parameter is unused in frovedis but 
it must be 'batch' or 'online' update. This is simply done to keep the behavior 
consistent with Scikit-learn. (Default: batch)  
**_learning\_decay_**: An unused parameter. (Default: 0.7)  
**_learning\_offset_**: An unused parameter. Although, this parameter is unused in 
frovedis but it must be zero or a positive float value. This is simply done to 
keep the behavior consistent with Scikit-learn. (Default: 10.0)  
**_max\_iter_**: An integer parameter which specifies the maximum number of passes 
over the training data. (Default: 10)  
**_batch\_size_**: An unused parameter. (Default: 128)  
**_evaluate\_every_**: An integer parameter that specifies how often to evaluate 
perplexity. Perplexity is not evaluated in training by default. If this parameter is 
greater than 0, perplexity will be evaluated, which can help in checking convergence 
in training process, but total training time will be increased. (Default: -1)  

For example, when evaluate_every < 0  

    # loading sample csr matrix data representing the input corpus-vocabulary count 
    corpus = [
    'This is the first document.',
    'This document is the second document.',
    'And this is the third one.',
    'Is this the first document?',
    ]

    # feature transformers CountVectorizer can be used for converting text to word count vectors.
    from sklearn.feature_extraction.text import CountVectorizer
    vectorizer = CountVectorizer()
    csr_mat = vectorizer.fit_transform(corpus)
    
    # fitting input matrix on lda object  
    from frovedis.mllib.decomposition import LatentDirichletAllocation
    lda = LatentDirichletAllocation(n_components = 3, evaluate_every = -1)
    lda.fit(csr_mat)

LDA training time: 0.0007402896881103516  

For example, when evaluate_every > 0  

    # loading sample csr matrix data representing the input corpus-vocabulary count 
    corpus = [
    'This is the first document.',
    'This document is the second document.',
    'And this is the third one.',
    'Is this the first document?',
    ]

    # feature transformers CountVectorizer can be used for converting text to word count vectors.
    from sklearn.feature_extraction.text import CountVectorizer
    vectorizer = CountVectorizer()
    csr_mat = vectorizer.fit_transform(corpus)
    
    # fitting input matrix on lda object  
    from frovedis.mllib.decomposition import LatentDirichletAllocation
    lda = LatentDirichletAllocation(n_components = 3, evaluate_every = 2)
    lda.fit(csr_mat)

LDA training time: 0.0008301734924316406  

**_total\_samples_**: An unused positive integer parameter which is unused in 
frovedis. This is simply kept to make the behavior consistent with Scikit-learn. (Default: 1e6)  
**_perp\_tol_**: An unused parameter. (Default: 1e-1)  
**_mean\_change\_tol_**: An unused parameter. (Default: 1e-3)  
**_max\_doc\_update\_iter_**: An unused parameter. (Default: 100)  
**_n\_jobs_**: An unused parameter. (Default: None)  
**_verbose_**: An integer parameter specifying the log level to use. Its value is set as 0 
by default(for INFO mode). But it can be set to 1(for DEBUG mode) or 2(for TRACE mode) for 
getting training time logs from frovedis server.  
**_algorithm_**: A string object parameter which specifies the sampling technique to be 
used. (Default: 'original')  
Frovedis LatentDirichletAllocation(LDA) supports either of two sampling techniques:  
**1. Collapsed Gibbs Sampling**  
**2. Metropolis Hastings**  
The default sampling algorithm is CGS (Default: 'original')  
If Metropolis Hastings is to be used, then it is required to set the proposal types namely:  
\ \ **2a. document proposal: "dp" algo,**  
\ \ **2b. word proposal:     "wp" algo,**  
\ \ **2c. cycle proposal:    "cp" algo,**  
\ \ **2d. sparse lda:        "sparse"**  
**_explore\_iter_**: An integer parameter that specifies the number of iterations to explore 
optimal hyperparams. (Default: 0)  

__Attributes__  
**components\_**: A numpy ndarray of double (float64) type values and has shape 
**(n_components, n_features)**.  

__Purpose__   
It initializes a LatentDirichletAllocation object with the given parameters.   

The parameters: "learning_method", "learning_decay", "learning_offset", "batch_size", 
"total_samples", "perp_tol", "mean_change_tol", "max_doc_update_iter" and "n_jobs" are simply 
kept in to make the interface uniform to the Scikit-learn LatentDirichletAllocation module. 
They are not used anywhere within frovedis implementation.  

__Return Value__   
It simply returns "self" reference.   

### 2. fit(X, y = None)  
__Parameters__  
**_X_**: A scipy sparse matrix or an instance of FrovedisCRSMatrix for sparse data of int, float, 
double(float64) type. It has shape **(n_samples, n_features)**.  
**_y_**:  None or any python array-like object (any shape). It is simply ignored in frovedis 
implementation, like in Scikit-learn.  

__Purpose__  
Fit LatentDirichletAllocation model on training data X.  

For example,  

    # loading sample csr matrix data representing the input corpus-vocabulary count 
    corpus = [
    'This is the first document.',
    'This document is the second document.',
    'And this is the third one.',
    'Is this the first document?',
    ]
    
    # feature transformers CountVectorizer can be used for converting text to word count vectors.
    from sklearn.feature_extraction.text import CountVectorizer
    vectorizer = CountVectorizer()
    csr_mat = vectorizer.fit_transform(corpus)
    
    # fitting input matrix on lda object  
    from frovedis.mllib.decomposition import LatentDirichletAllocation
    lda = LatentDirichletAllocation(n_components = 3)
    lda.fit(csr_mat)

When native python data is provided, it is converted to frovedis-like inputs and 
sent to frovedis server which consumes some data transfer time. Pre-constructed 
frovedis-like inputs can be used to speed up the training time, especially when 
same data would be used for multiple executions.  

For example,   

    # loading sample csr matrix data representing the input corpus-vocabulary count 
    corpus = [
    'This is the first document.',
    'This document is the second document.',
    'And this is the third one.',
    'Is this the first document?',
    ]
    
    # feature transformers CountVectorizer can be used for converting text to word count vectors.
    from sklearn.feature_extraction.text import CountVectorizer
    vectorizer = CountVectorizer()
    csr_mat = vectorizer.fit_transform(corpus)
    
    # Since "csr_mat" is scipy sparse data, we have created FrovedisCRSMatrix 
    # Also it only supports int64 as itype for input sparse data during training
    from frovedis.matrix.crs import FrovedisCRSMatrix
    cmat = FrovedisCRSMatrix(csr_mat, dtype = np.int32, itype = np.int64)
    
    # fitting input matrix on lda object  
    from frovedis.mllib.decomposition import LatentDirichletAllocation
    lda = LatentDirichletAllocation(n_components = 3)
    lda.fit(cmat)  

__Return Value__  
It simply returns "self" reference.  

### 3. transform(X)  
__Parameters__  
**_X_**: A scipy sparse matrix or an instance of FrovedisCRSMatrix for sparse data of int, float, 
double(float64) type. It has shape **(n_samples, n_features)**.  

__Purpose__  
Transform input matrix X according to the trained model.  

For example,   

    # loading sample csr matrix data representing the input corpus-vocabulary count 
    test_corpus = [
    'This is the first second third document.',
    'This This one third document.'
    ]
    
    # feature transformers CountVectorizer can be used for converting text to word count vectors.    
    from sklearn.feature_extraction.text import CountVectorizer
    vectorizer = CountVectorizer()
    csr_mat = vectorizer.fit_transform(test_corpus)
    
    # transform input according to trained model 
    from frovedis.mllib.decomposition import LatentDirichletAllocation
    lda = LatentDirichletAllocation(n_components = 3)
    lda.fit(csr_mat)
    print(lda.transform(csr_mat))

Output

    [[0.47619048 0.19047619 0.47619048]
     [0.26666667 0.26666667 0.66666667]]

Like in fit(), we can also provide frovedis-like input in transform() for faster computation.  

For example,   

    # loading sample csr matrix data representing the input corpus-vocabulary count 
    test_corpus = [
    'This is the first second third document.',
    'This This one third document.'
    ]
    
    # feature transformers CountVectorizer can be used for converting text to word count vectors.
    from sklearn.feature_extraction.text import CountVectorizer
    vectorizer = CountVectorizer()
    csr_mat = vectorizer.fit_transform(test_corpus)
    
    # Since "csr_mat" is scipy sparse data, we have created FrovedisCRSMatrix 
    # Also it only supports int64 as itype for input sparse data during training
    from frovedis.matrix.crs import FrovedisCRSMatrix
    cmat = FrovedisCRSMatrix(csr_mat, dtype = np.int32, itype = np.int64)
    
    # transform input according to trained model 
    from frovedis.mllib.decomposition import LatentDirichletAllocation
    lda = LatentDirichletAllocation(n_components = 3)
    lda.fit(cmat)
    lda.transform(cmat).debug_print()

Output

    matrix:
    num_row = 2, num_col = 3
    node 0
    node = 0, local_num_row = 2, local_num_col = 3, val = 0.47619 0.333333 0.333333 0.266667 0.0666667 0.866667

__Return Value__  
**When X is python native input:**  
It returns a numpy ndarray of shape **(n_samples, n_components)** and double (float64) 
type values. It contains the document-wise topic distribution for input data X.  
**When X is frovedis-like input:**  
It returns a FrovedisRowmajorMatrix of shape **(n_samples, n_components)** and 
double (float64) type values, containing document-wise topic distribution for input data X.  

### 4. perplexity(X, sub_sampling = False)  
__Parameters__  
**_X_**: A scipy sparse matrix or an instance of FrovedisCRSMatrix for sparse data of int, float, 
double(float64) type. It has shape **(n_samples, n_features)**.  
**_sub\_sampling_**: A boolean parameter that specifies whether to do sub-sampling or not. It is 
simply ignored in frovedis implementation. (Default: False)  

__Purpose__  
Calculate approximate perplexity for data X.  

Perplexity is defined as exp(-1. * log-likelihood per word).  

Ideally, as the number of components increase, the perplexity of model should decrease.  

For example,  

    perp = lda.perplexity(csr_mat)
    print("perplexity: %.2f" % (perp))

Ouput
    
    perplexity: 8.81
    
__Return Value__  
It returns float type perplexity score.  

### 5. score(X, y = None)  
__Parameters__  
**_X_**: A scipy sparse matrix or an instance of FrovedisCRSMatrix for sparse data of int, float, 
double(float64) type. It has shape **(n_samples, n_features)**.  
**_y_**:  None or any python array-like object (any shape). It is simply ignored in frovedis 
implementation, like in Scikit-learn.  

__Purpose__  
Calculate approximate log-likelihood as score for data X.  

Likelihood is a measure of how plausible model parameters are, given the data. Taking a logarithm 
makes calculations easier. All values are negative: when x < 1, log(x) < 0.  

The idea is to search for the largest log-likelihood (good score will be close to 0).  

For example,  

    print("score: %.2f" % (lda.score(csr_mat)))

Output

    score: -2.18

__Return Value__  
It returns a float value as likelihood score  

### 6. get_params(deep = True)  
__Parameters__   
_**deep**_: A boolean parameter, used to get parameters and their values for an estimator. If 
True, it will return the parameters for an estimator and contained subobjects that are 
estimators. (Default: True)  

__Purpose__    
This method belongs to the BaseEstimator class inherited by LatentDirichletAllocation. It 
is used to get parameters and their values of LatentDirichletAllocation class.  

For example, 
 
    print(lda.get_params())  

Output  

    {'algorithm': 'original', 'batch_size': 128, 'doc_topic_prior': 0.3333333333333333, 
    'evaluate_every': -1, 'explore_iter': 0, 'learning_decay': 0.7, 'learning_method': 'batch', 
    'learning_offset': 10.0, 'max_doc_update_iter': 100, 'max_iter': 10, 'mean_change_tol': 0.001, 
    'n_components': 3, 'n_jobs': None, 'perp_tol': 0.1, 'random_state': None, 
    'topic_word_prior': 0.3333333333333333, 'total_samples': 1000000.0, 'verbose': 0}

__Return Value__  
A dictionary of parameter names mapped to their values.  

### 7. set_params(\*\*params)  
__Parameters__  
_**\*\*params**_: All the keyword arguments are passed this function as dictionary. This 
dictionary contains parameters of an estimator with its given values to set.  

__Purpose__  
This method belongs to the BaseEstimator class inherited by LatentDirichletAllocation, used 
to set parameter values.  

For example,   

    print("get parameters before setting:") 
    print(lda.get_params())
    # User just needs to provide the arguments and internally it will create a 
    dictionary over the arguments given by user
    lda.set_params(n_components = 4, algorithm = 'dp') 
    print("get parameters after setting:") 
    print(lda.get_params())

Output  
     
    get parameters before setting:
    {'algorithm': 'original', 'batch_size': 128, 'doc_topic_prior': 0.3333333333333333, 
    'evaluate_every': -1, 'explore_iter': 0, 'learning_decay': 0.7, 'learning_method': 'batch', 
    'learning_offset': 10.0, 'max_doc_update_iter': 100, 'max_iter': 10, 'mean_change_tol': 0.001, 
    'n_components': 3, 'n_jobs': None, 'perp_tol': 0.1, 'random_state': None, 
    'topic_word_prior': 0.3333333333333333, 'total_samples': 1000000.0, 'verbose': 0}
    get parameters after setting:
    {'algorithm': 'dp', 'batch_size': 128, 'doc_topic_prior': 0.3333333333333333, 
    'evaluate_every': -1, 'explore_iter': 0, 'learning_decay': 0.7, 'learning_method': 'batch', 
    'learning_offset': 10.0, 'max_doc_update_iter': 100, 'max_iter': 10, 'mean_change_tol': 0.001, 
    'n_components': 4, 'n_jobs': None, 'perp_tol': 0.1, 'random_state': None, 
    'topic_word_prior': 0.3333333333333333, 'total_samples': 1000000.0, 'verbose': 0}
    
__Return Value__  
It simply returns "self" reference.  

### 8. fit_transform(X, y = None)  
__Parameters__  
**_X_**: A scipy sparse matrix or an instance of FrovedisCRSMatrix for sparse data of int, float, 
double(float64) type. It has shape **(n_samples, n_features)**.  
**_y_**:  None or any python array-like object (any shape). It is simply ignored in frovedis 
implementation, like in Scikit-learn.  

__Purpose__  
It performs fit() and transform() on X and y (unused), and returns a transformed version of X.  

For example,   

    # loading sample csr matrix data representing the input corpus-vocabulary count 
    test_corpus = [
    'This is the first second third document.',
    'This This one third document.'
    ]
    
    # feature transformers CountVectorizer can be used for converting text to word count vectors.    
    from sklearn.feature_extraction.text import CountVectorizer
    vectorizer = CountVectorizer()
    csr_mat = vectorizer.fit_transform(test_corpus)
    
    # transform input after fitting   
    from frovedis.mllib.decomposition import LatentDirichletAllocation
    lda = LatentDirichletAllocation(n_components = 3)
    print(lda.fit_transform(csr_mat))

Output

    [[0.04761905 0.61904762 0.47619048]
     [0.06666667 0.66666667 0.46666667]]

Like in fit() and transform(), we can also provide frovedis-like input in fit_transform() 
for faster computation.  

For example,   

    # loading sample csr matrix data representing the input corpus-vocabulary count 
    test_corpus = [
    'This is the first second third document.',
    'This This one third document.'
    ]
    
    # feature transformers CountVectorizer can be used for converting text to word count vectors.
    from sklearn.feature_extraction.text import CountVectorizer
    vectorizer = CountVectorizer()
    csr_mat = vectorizer.fit_transform(test_corpus)
    
    # Since "csr_mat" is scipy sparse data, we have created FrovedisCRSMatrix
    # Also it only supports int64 as itype for input sparse data during training
    from frovedis.matrix.crs import FrovedisCRSMatrix
    cmat = FrovedisCRSMatrix(csr_mat, dtype = np.int32, itype = np.int64)
    
    # transform input according to trained model 
    from frovedis.mllib.decomposition import LatentDirichletAllocation
    lda = LatentDirichletAllocation(n_components = 3)
    lda.fit_transform(cmat).debug_print()

Output

    matrix:
    num_row = 2, num_col = 3
    node 0
    node = 0, local_num_row = 2, local_num_col = 3, val = 0.333333 0.619048 0.190476 
    0.266667 0.666667 0.266667 
    
__Return Value__  
**When X is python native input:**  
It returns a numpy ndarray of shape **(n_samples, n_components)** and double (float64) 
type values. This is a transformed array X.  
**When X is frovedis-like input:**  
It returns a FrovedisRowmajorMatrix of shape **(n_samples, n_components)** and 
double (float64) type values. This is a transformed array X.  

### 9. load(fname, dtype = None)  
__Parameters__  
**_fname_**:  A string object containing the name of the file having model information
to be loaded.  
**_dtype_**: A data-type is inferred from the input data. Currently, expected input 
data-type is either float or double(float64). (Default: None)  

__Purpose__  
It loads the model from the specified file (having little-endian binary data).  

For example,  

    lda.load("./out/trained_lda_model")

__Return Value__  
It simply returns "self" reference.  

### 10. save(fname)  
__Parameters__  
**_fname_**: A string object containing the name of the file on which the target model 
is to be saved.  

__Purpose__  
On success, it writes the model information (after-fit populated attributes) in the 
specified file as little-endian binary data. Otherwise, it throws an exception.  

For example,   

    # To save the lda model
    lda.save("./out/trained_lda_model")  

This will save the lda model on the path "/out/trained_lda_model".  

The 'trained_lda_model' directory has  

**trained_lda_model**  
|-----**corpus_topic**  
|-----metadata  
|-----**word_topic**  

Here, the **corpus_topic** directory contains the corpus topic count as sparse matrix in binary 
format.  
The metadata file contains the information of model kind, input datatype used for trained model.  
Here, the **word_topic** directory contains the word topic count as sparse matrix in binary format.  

__Return Value__  
It returns nothing.  

### 11. release()  

__Purpose__  
It can be used to release the in-memory model at frovedis server.  

For example,
 
    lda.release()

This will reset the after-fit populated attributes (like components_) to None, along 
with releasing server side memory.  

__Return Value__  
It returns nothing.  

### 12. is_fitted()  

__Purpose__  
It can be used to confirm if the model is already fitted or not.  

__Return Value__  
It returns ‘True’, if the model is already fitted otherwise, it returns ‘False’.  

# SEE ALSO  
crs_matrix