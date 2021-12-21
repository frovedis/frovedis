% Word2Vec  

# NAME  

Word2Vec - Word embedding is one of the most popular representation of document 
vocabulary. It is capable of capturing context of a word in a document, 
semantic and syntactic similarity, relation with other words, etc.  

# SYNOPSIS  

    class frovedis.mllib.feature.Word2Vec(sentences=None, corpusFile=None, 
                                          outDirPath=None, hiddenSize=100, 
                                          learningRate=0.025, n_iter=1,  
                                          minCount=5, window=5, threshold=1e-3,  
                                          negative=5, modelSyncPeriod=0.1, minSyncWords=1024,  
                                          fullSyncTimes=0, messageSize=1024, numThreads=None)  

## Public Member Functions  

build_vocab(corpusIterable = None, corpusFile = None, outDirPath = None, update = False)  
build_vocab_and_dump(corpusIterable = None, corpusFile = None, outDirPath = None, update = False)   
to_gensim_model()  
train(corpusIterable = None, corpusFile = None)  
fit(corpusIterable = None, corpusFile = None)  
save(modelPath, binary = False)  
transform(corpusIterable = None, corpusFile = None, func = None)  
fit_transform(corpusIterable = None, corpusFile = None, func = None)  

# DESCRIPTION
Word2vec is a two-layer neural net that processes text by “vectorizing” words. 
Its input is a text corpus and its output is a set of vectors (feature vectors 
that represent words in that corpus). While Word2vec is not a deep neural network, 
it turns text into a numerical form that deep neural networks can understand.  

Word2vec’s applications extend beyond parsing sentences in the wild. 
It can be applied just as well to genes, code, likes, playlists, social media 
graphs and other verbal or symbolic series in which patterns may be discerned.  

This module provides a client-server implementation, where the client application 
is a normal python program. The frovedis interface is almost same as Gensim 
Word2Vec interface. It needs to be used when a system has Gensim installed. Thus 
in this implementation, a python client can interact with a frovedis server sending 
the required python data for training at frovedis side. Python data is converted 
into frovedis compatible data internally and the python ML call is linked with the 
respective frovedis ML call to get the job done at frovedis server.  

Python side calls for Word2Vec on the frovedis server. Once the training is completed 
with the input data at the frovedis server, it returns an abstract model with a unique 
model ID to the client python program.  

When transform-like request would be made on the trained model, python program 
will send the same request to the frovedis server. After the request is served at 
the frovedis server, the output would be sent back to the python client.  

## Detailed Description  

### 1. Word2Vec()   

__Parameters__   
**_sentences_**: This parameter is an iterable of list of strings. If not provided, the 
'corpusFile' parameter must be provided for creating the model. (Default: None)  
**_corpusFile_**: A string object parameter which specifies the path to a corpus file. If 
not provided, the 'sentences' parameter must be provided for creating the model. (Default: None)  
**_outDirPath_**: A string object parameter specifying the path of output directory. The 
newly built vocabulary generated from the input data file is dumped into the provided 
output file path. (Default: None)  
**_hiddenSize_**: An integer parameter specifying the dimensionality of the word vectors. For 
fast computation, this value should be an even value and less than 512 during training. (Default: 100)  
**_learningRate_**: An unused parameter specifying the initial learning rate. (Default: 0.025)  
**_n\_iter_**: An unused parameter specifying the number of iterations over the corpus. (Default: 1)  
**_minCount_**: An integer parameter that ignores all words with total frequency lower than 
this value during vocabulary building. This value should be within 1 to largest total 
frequency of a word in vocabulary. (Default: 5)  
**_window_**: An integer parameter specifying the maximum distance between the current and 
predicted word within a sentence. For fast computation, this value should be less than or 
equal to 8 during training. (Default: 5)  
**_threshold_**: An unused parameter specifying the threshold for configuring which 
higher-frequency words are randomly downsampled. (Default: 1e-3)  
**_negative_**: An integer parameter. This value specifies how many "noise words" should be 
drawn (usually between 5-20). The default value should be used for fast computation during 
training. (Default: 5)  
**_modelSyncPeriod_**: An unused parameter specifying the model synchronous period. (Default: 0.1)  
**_minSyncWords_**: An unused parameter specifying the minimum number of words to be synced 
at each model sync. (Default: 1024)  
**_fullSyncTimes_**: An unused parameter specifying the full-model sync-up time during 
training. (Default: 0)  
**_messageSize_**: An unused parameter specifying the message size in megabytes. (Default: 1024)  
**_numThreads_**: An integer parameter specifying the number of worker threads to be used to 
train the model. Ideally, the number of threads should range between 1 and 8. (Default: None)  
In case the environment variable 'VE_OMP_NUM_THREADS' is defined, then the value for 'numThreads' 
would be derived from VE_OMP_NUM_THREADS, otherwise, 'numThreads' would default to value 1.  
__Attributes__  
**_wv_**: The trained words are stored in KeyedVectors (mapping between keys such as words and 
embeddings as arrays) instance. It contains the mapping between words and embeddings.  

For example,

    # Using an iterable data  
    data = [["cat", "say", "meow"], ["dog", "say", "woof"]]    
    
    # Training the Word2Vec model using build_vocab() and train()
    from frovedis.mllib import Word2Vec
    wv_model = Word2Vec(minCount = 2)
    wv_model.build_vocab(data) 
    wv_model.train(data) 
    print(wv_model.wv)

Output

    {'</s>': array([ 0.00400269,  0.0044194 , -0.00383026, ...,  0.0015239 ,  0.00305939,
        0.00019363]), 'say': array([ 0.00227966, -0.00495255,  0.00431488, ...,  0.00019882,
        -0.0016861 , -0.00112656])}

It displays words as keys and embeddings as arrays.  

__Purpose__    
It initializes a Word2Vec object with the given parameters.  

The parameters: "learningRate", "n_iter", "threshold", "modelSyncPeriod", "minSyncWords", 
"fullSyncTimes" and "messageSize" are simply kept in to to make the interface uniform to the 
gensim Word2Vec module. They are not used anywhere within the frovedis implementation.  

__Return Value__    
It simply returns "self" reference.  

### 2. build_vocab(corpusIterable = None, corpusFile = None, outDirPath = None, update = False)
__Parameters__   
**_corpusIterable_**: Use the 'sentences' iterable which itself must be an iterable of list 
of list of tokens. If None, the 'corpusFile' must be provided for building the 
vocabulary. (Default: None)  
**_corpusFile_**: A string object parameter specifying the path of a corpus file. If None, 
the 'corpusIterable' must be provided for building the vocabulary. (Default: None)  
**_outDirPath_**: A string object parameter specifying the path of output directory './out'. 
The newly built vocabulary generated from the input data file is dumped into provided output 
file path. (Default: None)  
**_update_**: A boolean parameter if set to True, will add the new words present in sentences 
to the model’s vocabulary. (Default: False)  

__Purpose__    
It builds the vocabulary from input data file and dumped into provided output files. 
It also initializes the 'wv' attribute.  

For example, building vocabulary from an iterable  
    
    # Using an iterable data  
    data = [["cat", "say", "meow"], ["dog", "say", "woof"]] 
    
    # Building vocabulary from input iterable data 
    from frovedis.mllib import Word2Vec
    wv_model = Word2vec(minCount = 2)
    wv_model.build_vocab(corpusIterable = data)   

For example, building vocabulary from a text file

    # Using a text file  
    textfile = "./input/text8-10k"
    modelpath = "./out/text_model.txt"
    
    # Building vocabulary from input text file    
    from frovedis.mllib import Word2Vec
    wv_model = Word2Vec(minCount = 2)
    wv_model.build_vocab(corpusFile = textfile, outDirPath = modelpath)  

__Return Value__  
It simply returns "self" reference.  

### 3. build_vocab_and_dump(corpusIterable = None, corpusFile = None, outDirPath = None, update = False)
__Parameters__  
**_corpusIterable_**: Use the 'sentences' iterable which itself must be an iterable of list 
of list of tokens. If None, the 'corpusFile' must be provided for building the 
vocabulary. (Default: None)  
**_corpusFile_**: A string object parameter specifying the path to a corpus file. If None, 
the 'corpusIterable' must be provided for building the vocabulary. (Default: None)  
**_outDirPath_**: A string object parameter specifying the path to output directory './out'. 
The newly build vocabulary generated from the input data file is dumped into provided output 
file path. (Default: None)  
**_update_**: A boolean parameter if set to True, will add the new words present in sentences 
to the model’s vocabulary. (Default: False)  

__Purpose__    
It builds the vocabulary from input data file and dump into provided output files. 
It also initializes the 'wv' attribute. This method is an alias to build_vocab().  

For example, building vocabulary from an iterable  
    
    # Using an iterable data      
    data = [["cat", "say", "meow"], ["dog", "say", "woof"]]  
    
    # Building vocabulary from input iterable data    
    from frovedis.mllib import Word2Vec
    wv_model = Word2vec(minCount = 2)
    wv_model.build_vocab_and_dump(corpusIterable = data)   

For example, building vocabulary from a text file

    # Using a text file    
    textfile = "./input/text8-10k"
    modelpath = "./out/text_model.txt"
    
    # Building vocabulary from input text file    
    from frovedis.mllib import Word2Vec
    wv_model = Word2Vec(minCount = 2)
    wv_model.build_vocab_and_dump(corpusFile = textfile, outDirPath = modelpath)  

__Return Value__  
It simply returns "self" reference.  

### 4. to_gensim_model()  
__Purpose__  
It generates a gensim like 'wv' (KeyedVectors instance) attribute for Word2Vec model.  

For example,

    wv_model.to_gensim_model()

**Note:-** In order to use this method, gensim version installed by users needs to be 4.0.1 or above.  

__Return Value__  
It returns gensim like KeyedVectors instance.  

### 5. train(corpusIterable = None, corpusFile = None)  
__Parameters__  
**_corpusIterable_**: Use the 'sentences' iterable which itself must be an iterable of list 
of list of tokens. If None, the 'corpusFile' must be provided for building the 
vocabulary. (Default: None)  
**_corpusFile_**: A string object parameter specifying the path to a corpus file. If None, 
the 'corpusIterable' must be provided for building the vocabulary. (Default: None)  

__Purpose__  
It trains the Word2Vec model on input vocabulary.  

For example, training with an iterable data  
    
    # Using an iterable data  
    data = [["cat", "say", "meow"], ["dog", "say", "woof"]] 
    
    # Training the model  
    from frovedis.mllib import Word2Vec
    wv_model = Word2vec(minCount = 2)
    wv_model.build_vocab(corpusIterable = data)
    wv_model.train(corpusIterable = data)  
    
For example, training with a text file

    # Using a text file  
    textfile = "./input/text8-10k"
    modelpath = "./out/text_model.txt"
    
    # Training the model  
    from frovedis.mllib import Word2Vec
    wv_model = Word2Vec(minCount = 2)
    wv_model.build_vocab(corpusFile = textfile, outDirPath = modelpath)  
    wv_model.train(corpusFile = textfile)  

__Return Value__  
It simply returns "self" reference.  

### 6. fit(corpusIterable = None, corpusFile = None)  
__Parameters__   
**_corpusIterable_**: Use the 'sentences' iterable which itself must be an iterable of list 
of list of tokens. If None, the 'corpusFile' must be provided for building the 
vocabulary. (Default: None)  
**_corpusFile_**: A string object parameter specifying the path to a corpus file. If None, 
the 'corpusIterable' must be provided for building the vocabulary. (Default: None)  

__Purpose__  
It trains the Word2Vec model on input vocabulary. This method is an alias to train().  

For example, training with an iterable data  
    
    # Using an iterable data  
    data = [["cat", "say", "meow"], ["dog", "say", "woof"]] 

    # Training the model
    from frovedis.mllib import Word2Vec
    wv_model = Word2vec(minCount = 2)
    wv_model.build_vocab(corpusIterable = data)
    wv_model.fit(corpusIterable = data)  

For example, training with a text file

    # Using a text file  
    textfile = "./input/text8-10k"
    modelpath = "./out/text_model.txt"

    # Training the model      
    from frovedis.mllib import Word2Vec
    wv_model = Word2Vec(minCount = 2)
    wv_model.build_vocab(corpusFile = textfile, outDirPath = modelpath)  
    wv_model.fit(corpusFile = textfile)  

__Return Value__  
It simply returns "self" reference.  

### 7. save(modelPath, binary = False)  
__Parameters__  
**_modelPath_**: A string object parameter specifying the path of the output file in order 
to save the embeddings.  
**_binary_**: A boolean parameter if set to True, will save the data in binary format, otherwise, 
it will be saved in plain text. (Default: False)  

__Purpose__  
It saves the word2vec model information to a file.  

On success, it writes the model information (after-fit populated attribute like 'wv') in 
the specified file as little-endian binary data. Otherwise, it throws an exception.  

For example,   

    # To save the word embeddings in a file
    model = "./out/text_model.txt"  
    wv_model.save(model, binary = False)  

This will save the word2vec model information on the path "/out/text_model.txt".  
It would raise exception if the 'text_model.txt' file already existed with same name.  

The 'text_model.txt' file contains the count of all the words with total frequency greater 
than 'minCount', 'hiddenSize' and dictionary having words (as keys) and embeddings (as values).  

__Return Value__  
It returns nothing.  

### 8. transform(corpusIterable = None, corpusFile = None, func = None)  
__Parameters__  
**_corpusIterable_**: Use the 'sentences' iterable which itself must be an iterable of list 
of list of tokens. If None, the 'corpusFile' must be provided for building the 
vocabulary. (Default: None)  
**_corpusFile_**: A string object parameter specifying the path to a corpus file. If None, 
the 'corpusIterable' must be provided for building the vocabulary. (Default: None)  
**_func_**: A function to apply on document vector. (Default: None) 
If None, then np.mean() is used as 'func'.  

__Purpose__  
It transforms the document text to word2vec embeddings.  

For example, training with an iterable  

    # Using an iterable data  
    data = [["cat", "say", "meow"], ["dog", "say", "woof"]] 
    
    # Training the model 
    from frovedis.mllib import Word2Vec
    wv_model = Word2vec(minCount = 2)
    wv_model.build_vocab(corpusIterable = data)
    wv_model.fit(corpusIterable = data)
    embeddings = wv_model.transform(corpusIterable = data)
    print(embeddings)  
    
Output  

    [[ 0.00227966 -0.00495255  0.00431488 ...  0.00019882 -0.0016861
      -0.00112656]
     [ 0.00227966 -0.00495255  0.00431488 ...  0.00019882 -0.0016861
      -0.00112656]]

For example, training with a text file  

    # Using a text file  
    textfile = "./input/text8-10k"
    
    # Training the model 
    wv_model = Word2Vec(minCount = 2)
    wv_model.build_vocab(corpusFile = textfile)
    wv_model.fit(corpusFile = textfile)
    embeddings = wv_model.transform(corpusFile = textfile)
    print(embeddings)  
    
Output  

    [[-0.00013626 -0.00059639  0.00063703 ...  0.0004084  -0.00033636
      -0.00013291]]  

__Return Value__  
It returns document embeddings (numpy array) of shape **(n_samples, hiddenSize)**.  

### 9. fit_transform(corpusIterable = None, corpusFile = None, func = None)  
__Parameters__  
**_corpusIterable_**: Use the 'sentences' iterable which itself must be an iterable of list 
of list of tokens. If None, the 'corpusFile' must be provided for building the 
vocabulary. (Default: None)  
**_corpusFile_**: A string object parameter specifying the path to a corpus file. If None, 
the 'corpusIterable' must be provided for building the vocabulary. (Default: None)  
**_func_**: A function to apply on document vector. (Default: None) 
If None, then np.mean() is used as 'func'.  

__Purpose__  
It trains the word2vec model on input document and transforms the document text to 
word2vec embeddings.  

For example, training with an iterable  

    # Using an iterable data  
    data = [["cat", "say", "meow"], ["dog", "say", "woof"]] 
    
    # Training the model 
    from frovedis.mllib import Word2Vec
    wv_model = Word2vec(minCount = 2)
    wv_model.build_vocab(corpusIterable = data)
    embeddings = wv_model.fit_transform(corpusIterable = data)
    print(embeddings)  
    
Output  
    
    [[ 0.00227966 -0.00495255  0.00431488 ...  0.00019882 -0.0016861
      -0.00112656]
     [ 0.00227966 -0.00495255  0.00431488 ...  0.00019882 -0.0016861
      -0.00112656]]

For example, with a text file  

    # Using a text file  
    textfile = "./input/text8-10k"
    
    # Training the model 
    wv_model = Word2Vec(minCount = 2)
    wv_model.build_vocab(corpusFile = textfile)
    embeddings = wv_model.fit_transform(corpusFile = textfile)
    print(embeddings)  
    
Output  

    [[-0.00013626 -0.00059639  0.00063703 ...  0.0004084  -0.00033636
      -0.00013291]]  

__Return Value__  
It returns document embeddings (numpy array) of shape **(n_samples, hiddenSize)**.  