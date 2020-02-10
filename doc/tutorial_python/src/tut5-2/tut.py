import os
import numpy as np
from frovedis.exrpc.server import FrovedisServer
from frovedis.matrix.dense import FrovedisRowmajorMatrix
#from frovedis.matrix.dense import FrovedisColmajorMatrix 

# sample 4x4 numpy matrix creation
mat = np.matrix([[1,2,3,4],[5,6,7,8],[8,7,6,5],[4,3,2,1]],
                dtype=np.float64)

FrovedisServer.initialize("mpirun -np 2 {}".format(os.environ['FROVEDIS_SERVER']))
print ("-- matrix created from np.matrix --")
fmat = FrovedisRowmajorMatrix(mat)
#fmat = FrovedisColmajorMatrix(mat)
fmat.debug_print()
fmat.save("./result")
fmat.release()
print ("\n-- matrix loaded from text file --")
fmat2 = FrovedisRowmajorMatrix().load_text("./result")
#fmat2 = FrovedisColmajorMatrix().load_text("./result")
fmat2.debug_print()
fmat2.release()
FrovedisServer.shut_down()
