"""Frovedis word2vec python module"""

import os
import fnmatch
from types import GeneratorType
import numpy as np
from collections import Iterable
from ...exrpc.server import FrovedisServer
from ...exrpc.rpclib import w2v_build_vocab_and_dump, w2v_train
from ...exrpc.rpclib import check_server_exception

class w2v_result(object):
    """ contains python dictionary having word2vec result """
    def __init__(self, res, hiddenSize):
        self.result = res
        self.hiddenSize = hiddenSize
        self.gensim_result = None

    def keys(self):
        return self.result.keys()

    def values(self):
        return self.result.values()

    def items(self):
        return self.result.items()

    def __getitem__(self, ind):
        return self.result[ind]

    def __setitem__(self, ind, val):
        self.result[ind] = val

    @property
    def gensim_wv(self):
        """ function to convert self.result attribute to gensim wv"""
        if self.gensim_result is None:
            vocabList = list(self.keys())
            weights = list(self.values())
            from gensim.models import KeyedVectors
            gensim_w2v = KeyedVectors(self.hiddenSize)
            gensim_w2v.add_vectors(vocabList, weights)
            self.gensim_result = gensim_w2v
        return self.gensim_result

    def __getattr__(self, attr):
        #print(attr)
        if attr not in self.__dict__:
            try:
                wv = self.gensim_wv
            except:
                raise AttributeError("%s: gensim needs to be installed to " \
                                     "use this attribute\n" % attr)
            ret = getattr(wv, attr, None)
            if ret is None:
                raise AttributeError("no such attribute, %s for frovedis " \
                                     "Word2Vec is found!\n" % attr)
            return ret
        else:
            return self.__dict__[attr]

    def __str__(self):
        return str(self.result)

    def __repr__(self):
        return repr(self.result)

class Word2Vec:
    """ Python wrapper of Frovedis Word2Vec """

    def __init__(self, sentences=None, corpusFile=None, outDirPath=None,
                 hiddenSize=100, learningRate=0.025, n_iter=1, minCount=5,
                 window=5, threshold=1e-3, negative=5, modelSyncPeriod=0.1,
                 minSyncWords=1024, fullSyncTimes=0, messageSize=1024,
                 numThreads=None):
        self.hiddenSize = hiddenSize
        self.learningRate = learningRate
        self.n_iter = n_iter
        self.minCount = minCount
        self.window = window
        self.threshold = threshold
        self.negative = negative
        self.modelSyncPeriod = modelSyncPeriod
        self.minSyncWords = minSyncWords
        self.fullSyncTimes = fullSyncTimes
        self.messageSize = messageSize
        self.numThreads = numThreads
        # extra
        self.wv = None
        self.wpath = "frovedis-warehouse/w2v"
        self.outDirPath = None
        self.__encodePath = None
        self.__vocabPath = None
        self.__vocabCountPath = None
        self.__is_in_memory_input = False

        if sentences is not None or corpusFile is not None:
            self.build_vocab_and_dump(corpusIterable=sentences,
                                      corpusFile=corpusFile,
                                      outDirPath=outDirPath)
            self.fit(corpusIterable=sentences,
                     corpusFile=corpusFile)

    def _write_to_file(self, corpusIterable):
        """ function to writes corpusIterable data to corpusFile"""
        corpusFile = self.outDirPath + "/dumped_iterable.txt"
        with open(corpusFile, "w") as opfile:
            for line in corpusIterable:
                opfile.write(" ".join(line) + "\n")
        return corpusFile

    def _check_corpus_sanity(self, corpusIterable=None,
                                   corpusFile=None):
        """ function to check whether the corpus parameters are valid."""
        if corpusFile is None and corpusIterable is None:
            raise TypeError("Either one of corpusFile or corpusIterable "+\
                            "value must be provided")
        if corpusFile is not None and corpusIterable is not None:
            raise TypeError("Both corpusFile and corpusIterable "+\
                            "must not be provided at the same time")
        if corpusIterable is None and not os.path.isfile(corpusFile):
            raise TypeError("Parameter corpusFile must be a valid path"+\
                            " to a file, got %r instead" % corpusFile)
        if corpusIterable is not None and not isinstance(corpusIterable, Iterable):
            raise TypeError("The corpusIterable must be an iterable of lists "+\
                            "of strings, got %r instead" % corpusIterable)
        if isinstance(corpusIterable, GeneratorType):
            raise TypeError("You can't pass a generator as the sentences "+\
                            "argument. Try a sequence.")

    def build_vocab(self, corpusIterable=None, corpusFile=None, \
                    outDirPath=None, update=False):
        """ function to build vocabulary from input data file
            and dump into provided output files """
        return self.build_vocab_and_dump(corpusIterable, corpusFile, \
                                         outDirPath, update)

    def build_vocab_and_dump(self, corpusIterable=None, corpusFile=None, \
                             outDirPath=None, update=False):
        """ function to build vocabulary from input data file
            and dump into provided output files """
        self._check_corpus_sanity(corpusIterable, corpusFile)
        if corpusIterable is not None:
            self.__is_in_memory_input = True
        if self.outDirPath is not None and not update:
            raise AttributeError("build_vocab: cannot sort vocabulary after "\
                                 "model weights already initialized!\n")
        if outDirPath is None:
            if self.__is_in_memory_input:
                if not os.path.exists(self.wpath):
                    os.makedirs(self.wpath)
                    file_cnt = 0
                else:
                    file_cnt = len(fnmatch.filter(os.listdir(self.wpath), \
                                                  'res_t*'))
                self.outDirPath = self.wpath + "/res_t" + str(file_cnt + 1)
            else:
                input_basename = os.path.basename(corpusFile).split('.')[0]
                self.outDirPath = self.wpath + "/res_" + input_basename
        else:
            self.outDirPath = outDirPath
        if not os.path.exists(self.outDirPath):
            os.makedirs(self.outDirPath)
        self.__encodePath = os.path.join(self.outDirPath, "encode.bin")
        self.__vocabPath = os.path.join(self.outDirPath, "vocab.txt")
        self.__vocabCountPath = os.path.join(self.outDirPath, "vocab_count.bin")

        if self.__is_in_memory_input:
            corpusFile = self._write_to_file(corpusIterable)

        w2v_build_vocab_and_dump(corpusFile.encode("ascii"),
                                 self.__encodePath.encode("ascii"),
                                 self.__vocabPath.encode("ascii"),
                                 self.__vocabCountPath.encode("ascii"),
                                 self.minCount)
        excpt = check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])

        vocabList = np.loadtxt(self.__vocabPath, usecols=(0,), dtype=str)
        self.vocabSize = len(vocabList)
        initWeight = np.zeros(shape=self.hiddenSize)
        self.wv = w2v_result(dict.fromkeys(vocabList, initWeight), 
                             self.hiddenSize)
        return self

    def to_gensim_model(self):
        """ function to convert wv attribute to gensim wv"""
        vocabList = list(self.wv.keys())
        weights = list(self.wv.values())
        from gensim.models import KeyedVectors
        gensim_w2v = KeyedVectors(self.hiddenSize)
        gensim_w2v.add_vectors(vocabList, weights)
        return gensim_w2v

    def train(self, corpusIterable=None, corpusFile=None):
        """ function to train the w2v model on input vocab """
        return self.fit(corpusIterable, corpusFile)

    def fit(self, corpusIterable=None, corpusFile=None):
        """ function to train the w2v model on input vocab """
        if self.outDirPath is None:
            raise AttributeError("fit: you must first build vocabulary " 
                                 "before calling fit!\n")
        self._check_corpus_sanity(corpusIterable, corpusFile)
        if self.numThreads is None:
            if 'VE_OMP_NUM_THREADS' in os.environ:
                nthreads = int(os.environ['VE_OMP_NUM_THREADS'])
            else:
                nthreads = 1
        else:
            nthreads = self.numThreads

        (host, port) = FrovedisServer.getServerInstance()
        res = w2v_train(host, port, self.__encodePath.encode("ascii"),
                  self.__vocabCountPath.encode("ascii"),
                  self.hiddenSize, self.window,
                  self.threshold, self.negative,
                  self.n_iter, self.learningRate,
                  self.modelSyncPeriod,
                  self.minSyncWords, self.fullSyncTimes,
                  self.messageSize, nthreads)
        excpt = check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])

        vocabSize = len(res) // self.hiddenSize
        if vocabSize != self.vocabSize:
            raise RuntimeError("fit: error in vocabsize, report bug!\n")

        vocab = np.loadtxt(self.__vocabPath, usecols=(0,), dtype=str)
        weight_vector = np.asarray(res).reshape(self.vocabSize, \
                                                self.hiddenSize)
        for ind in range(len(vocab)):
            self.wv[vocab[ind]] = weight_vector[ind]
        return self

    def __save(self, modelPath, binary):
        """ function to save the output w2v model """
        mode = 'w'
        if binary:
            mode = 'wb'
        with open(modelPath, mode) as ofile:
            ofile.write(str(len(self.wv.keys())) + " " +\
                               str(self.hiddenSize)+"\n")
            for key, val in self.wv.items():
                ofile.write(key + " " + str(val)[1:-1]+"\n")

    def save(self, modelPath, binary=False):
        """ function to save the output w2v model """
        if self.wv is None:
            raise AttributeError("save called before training!\n")
        if modelPath is None:
            if binary:
                modelPath = os.path.join(self.outDirPath, "model.bin")
            else:
                modelPath = os.path.join(self.outDirPath, "model.txt") 
        try:
            self.to_gensim_model().save_word2vec_format(modelPath, \
                                                    binary=binary)
        except: #in case gensim is not available in the user environment
            self.__save(modelPath, binary)

    def _document_vector(self, doc, callable_fun):
        vocab_doc = [word for word in doc.split() if word in self.wv.keys()]
        no_embedding = np.zeros(self.hiddenSize)
        if len(vocab_doc) != 0:
            weights = [self.wv[vocab] for vocab in vocab_doc]
            return callable_fun(weights, axis=0)
        return no_embedding

    def transform(self, corpusIterable=None, corpusFile=None, func=None):
        """ function to tranform document text to word2vec embeddings"""
        if self.wv is None:
            raise AttributeError("tranform called before training!\n")
        self._check_corpus_sanity(corpusIterable, corpusFile)
        if corpusIterable is not None:
            corpusFile = self._write_to_file(corpusIterable)
        if func is None:
            func = np.mean
        with open(corpusFile) as file:
            text = file.readlines()
        if self.__is_in_memory_input:
            os.remove(corpusFile)
        from functools import partial
        mapfunc = partial(self._document_vector, callable_fun=func)
        return np.array(list(map(mapfunc, text)))

    def fit_transform(self, corpusIterable=None, corpusFile=None, func=None):
        """ function to fit and tranform word2vec embeddings"""
        return self.fit(corpusIterable=corpusIterable, corpusFile=corpusFile).\
                    transform(corpusIterable=corpusIterable,
                             corpusFile=corpusFile, func=func)
