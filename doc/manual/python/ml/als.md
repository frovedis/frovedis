% Matrix Factorization using ALS  

# NAME

Matrix Factorization using Alternating Least Square (ALS) - is a matrix 
factorization algorithm commonly used for recommender systems.   

# SYNOPSIS

class frovedis.mllib.recommendation.ALS(rank=None, max_iter=100, alpha=0.01,  
\  \  \  \  \  \  \  \  \  \  \  \  \  \  \  \  \  \  
\  \  \  \  \  \  \  \  \  \ reg_param=0.01, similarity_factor=0.1,  
\  \  \  \  \  \  \  \  \  \  \  \  \  \  \  \  \  \  
\  \  \  \  \  \  \  \  \  \ seed=0, verbose=0)     

## Public Member Functions

fit(X)  
predict(ids)  
recommend_users(pid, k)  
recommend_products(uid, k)  
load(fname, dtype = None)  
save(fname)  
debug_print()  
release()  
is_fitted()  

 
# DESCRIPTION
Collaborative filtering is commonly used for recommender systems. 
These techniques aim to fill in the missing entries of a user-item 
association matrix. Frovedis currently supports model-based collaborative 
filtering, in which users and products are described by a small set of 
latent factors that can be used to predict missing entries. 

Frovedis uses the alternating least squares(ALS) algorithm to learn these 
latent factors. The algorithm is based on a paper “Collaborative Filtering 
for Implicit Feedback Datasets” by Hu, et al.

This module provides a client-server implementation, where the client 
application is a normal python program. Scikit-learn does not have
any collaborative filtering recommender algorithms like ALS. In this 
implementation, python side recommender interfaces are provided, where 
a python client can interact with a frovedis server sending the required 
python data for training at frovedis side. Python data is converted into 
frovedis compatible data internally and the python ALS call is linked 
with the frovedis ALS call to get the job done at frovedis server. 
 
Python side calls for ALS on the frovedis server. Once the training is completed 
with the input data at the frovedis server, it returns an abstract model with a
unique model ID to the client python program.  

When recommendation-like request would be made on the trained model, python 
program will send the same request to the frovedis server. After the request 
is served at the frovedis server, the output would be sent back to the python 
client. 

## Detailed Description 
 
### 1. ALS()   

_**rank**_: A positive integer parameter containing the user given rank for the 
input matrix. (Default: None)  
When rank is None (not specified explicitly), it will be the minimum(256, min(M,N)), 
where M is number of users and N is number of items in input data. It must be within 
the range of 0 to max(M, N).  
_**max\_iter**_: A positive integer specifying maximum iteration count. (Default: 100)  
_**alpha**_: A positive double(float64) parameter containing the learning rate. (Default: 0.01)  
_**reg\_param**_: A positive double(float64) parameter, also called as the regularization 
parameter. (Default: 0.01)  
_**similarity\_factor**_: A double(float64) parameter, which helps to identify whether 
the algorithm will optimize the computation for similar user/item or not. If similarity 
percentage of user or item features is more than or equal to the given similarity_factor, 
the algorithm will optimize the computation for similar user/item. Otherwise, each user 
and item feature will be treated uniquely. Similarity factor must be in range of >= 0.0 
to <= 1.0. (Default: 0.1)  
_**seed**_: An int64 parameter containing the seed value to initialize the model 
structures with random values. (Default: 0)  
_**verbose**_: An integer parameter specifying the log level to use. Its value 
is 0 by default(for INFO mode and not speicifed explicitly). But it can be set 
to 1(for DEBUG mode) or 2(for TRACE mode) for getting training time logs from 
frovedis server.  

__Purpose__    

It initializes an ALS object with the given parameters.    

__Return Value__    
It simply returns "self" reference. 

### 2. fit(X)
__Parameters__   
_**X**_: A scipy sparse matrix or any python array-like object or an instance 
of FrovedisCRSMatrix. It has shape (n_samples, n_features).  
 
__Purpose__    
It accepts the training sparse matrix (X) and trains a matrix factorization model 
on that at frovedis server. 

It starts with initializing the model structures of the size MxF 
and NxF(where M is the number of users, N is the products in the given 
rating matrix and F is the given rank) with random values and keeps updating 
them until maximum iteration count is reached. 

For example,   

    # creating csr matrix 
    import numpy as np
    from scipy.sparse import csr_matrix 
    row = np.array([0, 0, 1, 2, 2, 3, 3, 3, 4, 4, 5, 6, 6, 7, 7, 7])
    col = np.array([0, 4, 0, 2, 3, 0, 1, 6, 0, 4, 0, 2, 3, 0, 1, 6])
    data = np.array([2.0, 9.0, 1.0, 4.0, 8.0, 2.0, 3.0, 8.9, 2.0, 9.0, 1.0, 4.0, 8.0, 2.0,3.0, 8.9])
    csr_matrix = csr_matrix((data, (row, col)), shape = (8, 7))
        
    # fitting input matrix on ALS object
    from frovedis.mllib.recommendation import ALS
    als = ALS(rank = 4).fit(csr_matrix)

When native python data is provided, it is converted to frovedis-like inputs 
and sent to frovedis server which consumes some data transfer time. 
Pre-constructed frovedis-like inputs can be used to speed up the training
time, especially when same data would be used for multiple executions.  

For example,  

    # Since "mat" is scipy csr sparse matrix, we have created FrovedisCRSMatrix.
    from frovedis.matrix.crs import FrovedisCRSMatrix
    crs_mat = FrovedisCRSMatrix(mat)  
   
    # ALS with pre-constructed frovedis-like inputs
    from frovedis.mllib.recommendation import ALS
    als = ALS(rank = 4).fit(crs_mat)  

__Return Value__  
It simply returns "self" reference.     

### 3. predict(ids)
__Parameters__   
_**ids**_: A python tuple or list object containing the pairs of user id and product 
id to predict.       

__Purpose__    
It accepts a list of pair of user ids and product ids(0-based Id) in order to make 
prediction for their ratings from the trained model at frovedis server. 

For example,

    # prints the predicted ratings for the given list of id pairs
    als.predict([(1,1),(0,1),(2,3),(3,1)])   

Output: 

    [ 0.00224735  0.00152505  0.99515575  0.99588757]
    
__Return Value__  
It returns a numpy array containing the predicted ratings, of float or double(float64) 
type depending upon the input type.

### 4. recommend_users(pid, k)
__Parameters__   
_**pid**_: An integer parameter specifying the product ID(0-based Id) for which
to recommend users.   
_**k**_: An integer parameter specifying the number of users to be recommended.   

__Purpose__    
It recommends the best "k" users with highest rating confidence in sorted 
order for the given product.    

If k > number of rows (number of users in the given matrix when training the 
model), then it resets the k as "number of rows in the given matrix". This is 
done in order to recommend all the users with rating confidence values in 
descending order.   

For example,

    # recommend 2 users for second product
    als.recommend_users(1,2)

Output: 

    ('uids:', array([7, 3], dtype=int32))
    ('scores:', array([ 0.99588757,  0.99588757]))

__Return Value__  
It returns a python list containing the pairs of recommended users and 
their corresponding rating confidence values(double(float64)) in descending order.    

### 5. recommend_products(uid, k)
__Parameters__   
_**uid**_: An integer parameter specifying the user ID(0-based Id) for which
to recommend products.  
_**k**_: An integer parameter specifying the number of products to be recommended.  

__Purpose__    
It recommends the best "k" products with highest rating confidence in sorted 
order for the given user.    

If k > number of columns (number of products in the given matrix when training the 
model), then it resets the k as "number of columns in the given matrix". This is 
done in order to recommend all the products with rating confidence values in 
descending order.  

For example,  

    # recommend 2 products for second user
    print als.recommend_products(1,2)

Output: 

    (' recommend_product  pids:', array([0, 4], dtype=int32))
    ('scores:', array([ 0.99494576,  0.0030741 ]))[(0, 0.9949457612078868), (4, 0.0030740973144160397)]

__Return Value__  
It returns a python list containing the pairs of recommended products and 
their corresponding rating confidence values(double(float64)) in descending order.    

### 6. save(fname)
__Parameters__   
_**fname**_: A string object containing the name of the file on which the target 
model is to be saved.    

__Purpose__    
On success, it writes the model information(user-product features etc.) in the 
specified file as little-endian binary data. Otherwise, it throws an exception. 

For example,   

    # saving the model
    als.save("./out/MyMFModel")

__Return Value__  
It returns nothing.   

### 7. load(fname, dtype = None)
__Parameters__   
_**fname**_: A string object containing the name of the file having model 
information to be loaded.    
_**dtype**_: A data-type is inferred from the input data. Currently, expected 
input data-type is either float or double(float64). (Default: None)

__Purpose__    
It loads the model from the specified file(having little-endian binary data). 

For example,  

    # loading the same model
    als.load("./out/MyMFModel")

__Return Value__  
It simply returns "self" instance.   

### 8. debug_print()

__Purpose__    
It shows the target model information on the server side user terminal. 
It is mainly used for debugging purpose. 

For example,

    als.debug_print()

Output: 

     ========== Matrix::X ==========
     0.829524 -0.84477 -0.152624 0.569863
     0.829528 -0.844775 -0.152625 0.569867
     0.829921 -0.845174 -0.152697 0.570136
    ========== Matrix::Y ==========
     0.473117 -0.481813 -0.087049 0.32502
     0.473117 -0.481813 -0.087049 0.32502
     0.473117 -0.481813 -0.087049 0.32502
     
It will print the matrix and labels of training data. 

__Return Value__  
It returns nothing.   

### 9. release()

__Purpose__    
It can be used to release the in-memory model at frovedis server.   

For example,  

    als.release()
    
This will reset the after-fit populated attributes to None, along with 
releasing server side memory.  

__Return Value__  
It returns nothing.   

### 10. is_fitted()

__Purpose__    
It can be used to confirm if the model is already fitted or not. In case, predict() 
is used before training the model, then it can prompt the user to train the model first.

__Return Value__  
It returns 'True', if the  model is already fitted otherwise, it returns 'False'.

# SEE ALSO  
crs_matrix   