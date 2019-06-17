import os
import numpy as np
from scipy.sparse import csr_matrix
from frovedis.exrpc.server import FrovedisServer
from frovedis.matrix.crs import FrovedisCRSMatrix 

# sample 3x3 scipy csr matrix creation
data = np.array([1, 2, 3, 4, 5, 6])
indices = np.array([0, 2, 2, 0, 1, 2])
indptr = np.array([0, 2, 3, 6])
mat = csr_matrix((data, indices, indptr),
                 dtype=np.float64,
                 shape=(3, 3))

FrovedisServer.initialize("mpirun -np 2 {}".format(os.environ['FROVEDIS_SERVER']))
print ("-- matrix created from scipy csr matrix --")
fmat = FrovedisCRSMatrix(mat)
fmat.debug_print()
fmat.save("./result")
fmat.release()
print ("\n-- matrix loaded from text file --")
fmat2 = FrovedisCRSMatrix().load_text("./result")
fmat2.debug_print()
fmat2.release()
FrovedisServer.shut_down()
