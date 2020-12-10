#!/usr/bin/env python

import sys
from frovedis.exrpc.server import FrovedisServer
from frovedis.mllib.feature import Word2Vector

# initializing the Frovedis server
argvs = sys.argv
argc = len(argvs)
if (argc < 2):
    print ('Please give frovedis_server calling command as the first argument \n(e.g. "mpirun -np 2 -x /opt/nec/nosupport/frovedis/ve/bin/frovedis_server")')
    quit()

textfile = "./input/text8-10k"
encode = "./out/text_encode.bin"
vocab = "./out/text_vocab.txt"
count = "./out/text_count.bin"

weight = "./out/text_weight.bin"
model = "./out/text_model.txt"

# x86 task
wv = Word2Vector(minCount=5)
wv.build_vocab_and_dump(textfile, encode, vocab, count) 

# ve task
FrovedisServer.initialize(argvs[1])
wv.fit(encode, count, weight)
FrovedisServer.shut_down()

#x86 task
wv.save(weight, vocab, model)
