import os
import numpy as np
from frovedis.exrpc.server import FrovedisServer
from frovedis.matrix.dvector import FrovedisDvector

from frovedis.matrix.dense import FrovedisRowmajorMatrix

FrovedisServer.initialize("mpirun -np 2 {}".format(os.environ['FROVEDIS_SERVER']))
dv = FrovedisDvector([1,2,3,4,5,6,7,8],dtype=np.float64)
dv.debug_print()
FrovedisServer.shut_down()
