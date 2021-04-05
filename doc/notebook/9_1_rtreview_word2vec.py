##  Word2Vec

### 1. Dataset Description

# The dataset used in this demo is Rotten Tomatoes movie review dataset.
# It is a corpus of movie reviews used majorly for sentiment analysis.
# It can be downloaded from https://drive.google.com/file/d/1w1TsJB-gmIkZ28d1j7sf1sqcPmHXw352/view


#### Import required libraries

import os
import time
import pandas as pd
import nltk
import numpy as np
import string
import gensim
from sklearn.model_selection import train_test_split
from sklearn.linear_model import LogisticRegression
from nltk.tokenize import word_tokenize
from nltk.corpus import stopwords


### 2. Data Preprocessing

#### Loading RT review dataset

# Download the dataset from below link (wget cannot be used bec it is google drive link)
# https://drive.google.com/file/d/1w1TsJB-gmIkZ28d1j7sf1sqcPmHXw352/view


# Load RT review dataset
data_file = "datasets/rt_reviews.csv"
df = pd.read_csv(data_file, encoding="ISO-8859-1")
print(df.shape)

#### Dataset summary

# Dataset Analysis
print("Dataset contains {} reviews".format(df.shape[0]))
print(df.info())


#### Check top rows
print(df.head())


#### Drop the rows with na values and shuffle dataset

# Data Cleaning
df = df.dropna().sample(frac=1, random_state=42)
print(df.shape)


#### Covert the reviews to lower case

df['Review'] = df['Review'].str.lower()
print(df.head())


#### Remove Punctuation Marks

import re, string
def clean_review(review):
    pattern = re.compile(r'[^a-zA-Z0-9 ]')
    review = pattern.sub(' ', review)
    return review

df['Review'] = df['Review'].apply(clean_review)
print(df.head())


#### Tokenize and Remove stopwords

stop = stopwords.words('english')
df['Review'] = df['Review'].apply(lambda x: [item for item in word_tokenize(x) if item not in stop])
print(df.head())


#### Label distribution summary

rating_categories = df["Freshness"].value_counts()
ax = rating_categories.plot(kind='bar', title='Label Distribution').     set(xlabel="Rating Id's", ylabel="No. of reviewes")


#### Encode the labels

mapping = {'fresh': 1, 'rotten': 0}
df['Freshness'] = df.replace({'Freshness': mapping})


#### Split train-test data

df_train, df_test = train_test_split(df, stratify=df["Freshness"], test_size=0.1, random_state = 42)
df_train = df_train.copy(deep=True)
df_test = df_test.copy(deep=True)


#### Check the shape of all the data

print(df_train.shape)
print(df_test.shape)


#### Set number of threads

os.environ["VE_OMP_NUM_THREADS"] = '8'
print(os.environ["VE_OMP_NUM_THREADS"])


### 3. Logistic Regression using Gensim Word2Vec Embeddings

#### Import Gensim Word2Vec

from gensim.models import Word2Vec as Gensim_Word2Vec


#### Generating word2vec embeddings for training vocabulary

time_start = time.time()

gensim_embeddings = Gensim_Word2Vec(df_train["Review"].to_list(), size=512, min_count=2, sg=1, iter=100)

time_stop = time.time()


#### Check time taken by gensim

gensim_elapsed_time = time_stop - time_start
print(gensim_elapsed_time)


#### Store the vocab for further use

gensim_vocab = gensim_embeddings.wv.vocab


#### Generating Train and Test Data Embeddings

no_embedding = np.zeros(gensim_embeddings.vector_size)

def document_vector_gensim(doc):
    """Create document vectors by averaging word vectors. Remove out-of-vocabulary words."""
    
    vocab_doc = [word for word in doc if word in gensim_vocab]
    
    if len(vocab_doc) != 0:
        return list(np.mean(gensim_embeddings.wv[vocab_doc], axis=0))
    else:
        return list(no_embedding)


df_train["Gensim_Embedding"] = df_train["Review"].apply(document_vector_gensim)
df_test["Gensim_Embedding"] = df_test["Review"].apply(document_vector_gensim)


#### Check the shape of all the data

print(df_train.shape)
print(df_test.shape)


#### Classification using gensim word2vec embeddings

# Training
model = LogisticRegression(max_iter=10000)
model.fit(df_train["Gensim_Embedding"].to_list(), df_train["Freshness"].to_list())

# Train Score
gensim_train_score = model.score(df_train["Gensim_Embedding"].to_list(), df_train["Freshness"].to_list())
print(gensim_train_score)

# Test Score
gensim_test_score = model.score(df_test["Gensim_Embedding"].to_list(), df_test["Freshness"].to_list())
print(gensim_test_score)


### 4. Logistic Regression using Frovedis Word2Vector Embeddings

#### Initializing Frovedis Server

from frovedis.exrpc.server import FrovedisServer

FrovedisServer.initialize("mpirun -np 1 -ve 1 " + os.environ["FROVEDIS_SERVER"])

#### Generating word2vec embeddings for data

from frovedis.mllib import Word2Vec as Frovedis_Word2Vec

time_start = time.time()
frovedis_w2v = Frovedis_Word2Vec(df_train["Review"].to_list(), \
                         hiddenSize=512, minCount=2, n_iter=100)
time_stop = time.time()


#### Check time taken by Frovedis

frovedis_elapsed_time = time_stop - time_start
print(frovedis_elapsed_time)


#### Save w2v embeddings

model = "./out/rt_model.txt"
frovedis_w2v.save(model)

#### Check the embeddings

print(list(frovedis_w2v.wv.keys())[:10])
print(frovedis_w2v.wv['film'])
print("Embeddings are of shape ({0}, {1})".format(len(frovedis_w2v.wv.keys()),\
                                           len(frovedis_w2v.wv['film'])))

#### Generating Train and Test Data Embeddings

df_train["Frovedis_Embedding"] = list(frovedis_w2v.transform(df_train["Review"].\
                                                   to_list(), func=np.mean))
df_test["Frovedis_Embedding"] = list(frovedis_w2v.transform(df_test["Review"].\
                                                   to_list(), func=np.mean))

#### Classification using frovedis word2vec embeddings

# Training
model = LogisticRegression(max_iter=10000)
model.fit(df_train["Frovedis_Embedding"].to_list(), df_train["Freshness"].to_list())


# Train Score
frov_train_score = model.score(df_train["Frovedis_Embedding"].to_list(), df_train["Freshness"].to_list())
print(frov_train_score)

# Test Score
frov_test_score = model.score(df_test["Frovedis_Embedding"].to_list(), df_test["Freshness"].to_list())
print(frov_test_score)


### Shutting Down Frovedis Server
FrovedisServer.shut_down()


### 5. Results Comparison

# Training Time
print("Frovedis train time : {:.3f} sec".format(frovedis_elapsed_time))
print("Gensim train time : {:.3f} sec".format(gensim_elapsed_time))

# Score
print('LogisticRegression score using Frovedis word2vec embeddings : '+ str(frov_test_score))
print('LogisticRegression score using Gensim word2vec embeddings : '+ str(gensim_test_score))
