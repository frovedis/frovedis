from frovedis.decomposition import LatentDirichletAllocation
import numpy as np
from scipy.sparse import csr_matrix
from frovedis.exrpc.server import *
from frovedis.matrix.crs import FrovedisCRSMatrix
import sys
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
mat = vectorizer.fit_transform(corpus)
fmat = FrovedisCRSMatrix(mat, dtype=np.int32, itype=np.int64)
#fmat.save('input_corpus_crs_001')

test_corpus = [
    'This is the first second third document.',
    'This This one third document.'
]

vectorizer = CountVectorizer()
mat_01 = vectorizer.fit_transform(test_corpus)
fmat_01 = FrovedisCRSMatrix(mat_01, dtype=np.int32, itype=np.int64)
#fmat_01.save('test_corpus_crs_001')


lda = LatentDirichletAllocation(n_components=4, max_iter=5, doc_topic_prior=0.25,algorithm='original',
                                topic_word_prior=0.25, learning_method='batch', learning_offset=50.,
                                verbose=0,random_state=0,explore_iter=1, evaluate_every=2)
lda.fit(fmat)
print(lda.components_)
lda.save('./out/trained_lda_model')

lda.release()
lda.load('./out/trained_lda_model')
lda.fit_transform(fmat)
print(lda.perplexity(fmat_01))
print(lda.score(fmat_01))

