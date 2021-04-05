#!/usr/bin/env python

import sys
from frovedis.exrpc.server import FrovedisServer
from frovedis.mllib import Word2Vec
import numpy as np
np.set_printoptions(threshold=10)

# initializing the Frovedis server
argvs = sys.argv
argc = len(argvs)
if (argc < 2):
    print ('Please give frovedis_server calling command as the first argument \n(e.g. "mpirun -np 2 -x /opt/nec/nosupport/frovedis/ve/bin/frovedis_server")')
    quit()

FrovedisServer.initialize(argvs[1])

data = [["cat", "say", "meow"], ["dog", "say", "woof"]]

### Generate embeddings using build_vocab and train
wv_model = Word2Vec(minCount=1)
wv_model.build_vocab(data) # Using corpus data
wv_model.train(data) # Using corpus data
print(wv_model.wv)

### You can also perform the same in one step as follows(using constructor)
wv_model = Word2Vec(sentences=data, minCount=1) # Using corpus data
print(wv_model.wv)

### gensim needs to be installed in order to use 
### gensim-like functionalities on word2vec model (similarity etc.)
try:
    print('similarity of {} with {} is {}'.format(\
           "cat", "dog", wv_model.wv.similarity("cat", "dog")))
except AttributeError as err:
    print(err)

### Generate document embeddings using fit_transform()
textfile = "./input/text8-10k"
model = "./out/text_model.txt"
wv_model = Word2Vec(minCount=1)
embeddings = wv_model.build_vocab(corpusFile=textfile).\
                      fit_transform(corpusFile=textfile)
print(embeddings)
### Above embedding can be used for further downstream tasks like classification, clustering etc

### Save word2vec embeddings
wv_model.save(model, binary=False)
#wv_model.save(model, binary=True) # In binary format

FrovedisServer.shut_down()
