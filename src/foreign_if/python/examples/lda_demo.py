import sys
import numpy as np
from scipy.sparse import csr_matrix
from frovedis.exrpc.server import *
from frovedis.matrix.crs import FrovedisCRSMatrix
from frovedis.mllib.decomposition import LatentDirichletAllocation

try:
    from sklearn.feature_extraction.text import CountVectorizer
except:
    print ('Please prepare environment with scikit-learn (see tutorial)')
    quit()

# initializing the Frovedis server
argvs = sys.argv
argc = len(argvs)
if (argc < 2):
    print ('Please give frovedis_server calling command as the first argument \n(e.g. "mpirun -np 2 -x /opt/nec/nosupport/frovedis/ve/bin/frovedis_server")')
    quit()
FrovedisServer.initialize(argvs[1])

corpus = [
    'This is the first document.',
    'This document is the second document.',
    'And this is the third one.',
    'Is this the first document?',
]
vectorizer = CountVectorizer()
fmat = vectorizer.fit_transform(corpus)

lda = LatentDirichletAllocation(n_components=4, max_iter=5)
lda.fit(fmat)
print("components: ")
print(lda.components_)
lda.save('./out/trained_lda_model')

# loading model for test on test corpus
lda.load('./out/trained_lda_model')

test_corpus = [
    'This is the first second third document.',
    'This This one third document.'
]
fmat = vectorizer.fit_transform(test_corpus)

print("transformed: ")
print(lda.fit_transform(fmat))
print("preplexity: %.2f" % (lda.perplexity(fmat)))
print("score: %.2f" % (lda.score(fmat)))

# Shutting down the Frovedis server
FrovedisServer.shut_down()
