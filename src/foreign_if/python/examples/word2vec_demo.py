#!/usr/bin/env python

import sys
from frovedis.exrpc.server import FrovedisServer
from frovedis.mllib import Word2Vec

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

### Get gensim like wv attribute for frovedis word2vec model
try:
    wv = wv_model.to_gensim_model()
    print(wv)
    print('similarity of {} with {} is {}'.format(\
           "cat", "dog", wv.similarity("cat", "dog")))
except:
    print("### gensim is not available... so skipping " +
          "demo for gensim related functionalities")

### Generate document embeddings using fit_transform()
textfile = "./input/text8-10k"
model = "text_model.txt"
wv_model = Word2Vec(minCount=1)
embeddings = wv_model.build_vocab(corpusFile=textfile).\
                      fit_transform(corpusFile=textfile)
print(embeddings)
### Above embedding can be used for further downstream tasks like classification, clustering etc

### Save word2vec embeddings
wv_model.save(model, binary=False)
#wv_model.save(model, binary=True) # In binary format

FrovedisServer.shut_down()
