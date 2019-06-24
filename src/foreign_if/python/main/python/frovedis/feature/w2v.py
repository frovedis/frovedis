#!/usr/bin/env python

from ..exrpc.server import FrovedisServer 
from ..exrpc.rpclib import w2v_build_vocab_and_dump, w2v_train
from ..exrpc.rpclib import w2v_save_model, check_server_exception

class Word2Vector:
    """ Python wrapper of Frovedis Word2Vector """

    def __init__(self, hiddenSize=100, learningRate=0.025, n_iter=1, minCount=5,
                 window=5, threshold=1e-3, negative=5, modelSyncPeriod=0.1, 
                 minSyncWords=1024, fullSyncTimes=0, messageSize=1024,
                 numThreads=8):
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

    def build_vocab_and_dump(self, text_path, encode_path, 
                             vocab_path, vocab_count_path):
        w2v_build_vocab_and_dump(text_path, encode_path,
                                 vocab_path, vocab_count_path,
                                 self.minCount)
        excpt = check_server_exception()
        if excpt["status"]: raise RuntimeError(excpt["info"])
        return self

    def fit(self, encode_path, vocab_count_path, weight_path):
        (host,port) = FrovedisServer.getServerInstance()
        w2v_train(host, port, encode_path, 
                  weight_path, vocab_count_path, 
                  self.hiddenSize, self.window, 
                  self.threshold, self.negative,
                  self.n_iter, self.learningRate,
                  self.modelSyncPeriod,
                  self.minSyncWords, self.fullSyncTimes,
                  self.messageSize, self.numThreads)
        excpt = check_server_exception()
        if excpt["status"]: raise RuntimeError(excpt["info"])
        return self

    def save(self, weight_path, vocab_path, output_path, binary=False):
        w2v_save_model(weight_path, 
                       vocab_path, output_path,
                       self.minCount, binary)
        excpt = check_server_exception()
        if excpt["status"]: raise RuntimeError(excpt["info"])

