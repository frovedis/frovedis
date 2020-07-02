#!/usr/bin/env python

import sys
import numpy as np
from scipy.sparse import csr_matrix
from frovedis.exrpc.server import FrovedisServer
from frovedis.decomposition import TruncatedSVD

# initializing the Frovedis server
argvs = sys.argv
argc = len(argvs)
if (argc < 2):
    print ('Please give frovedis_server calling command as the first argument \n(e.g. "mpirun -np 2 -x /opt/nec/nosupport/frovedis/ve/bin/frovedis_server")')
    quit()
FrovedisServer.initialize(argvs[1])

# sample numpy dense data (3x3)
mat = np.asmatrix([[1,0,2], [0,0,3], [4,5,6]], dtype=np.float64)

# corresponding 3x3 scipy csr matrix 
#data = np.array([1, 2, 3, 4, 5, 6])
#indices = np.array([0, 2, 2, 0, 1, 2])
#indptr = np.array([0, 2, 3, 6])
#mat = csr_matrix((data, indices, indptr), dtype=np.float64, shape=(3, 3))

# Calling Frovedis wrapper TruncatedSVD 
frov_svd = TruncatedSVD(n_components = 2, 
                        algorithm = "arpack").fit(mat)
print("[Frovedis] components: ", frov_svd.components_)
print("[Frovedis] singular values: ", frov_svd.singular_values_)
print("[Frovedis] explained variance: ", frov_svd.explained_variance_)
print("[Frovedis] explained variance ratio: ", frov_svd.explained_variance_ratio_)

# Saving results into files
frov_svd.svd_res_.save("./out/svec", "./out/umat", "./out/vmat")

# releasing results from server
frov_svd.svd_res_.release()

try:
  from sklearn.decomposition.truncated_svd import TruncatedSVD as sklearn_svd
  # Calling native sklearn TruncatedSVD
  sk_svd = sklearn_svd(n_components = 2, 
                       algorithm = "arpack").fit(mat)
  print("[sklearn] components: ", sk_svd.components_)
  print("[sklearn] singular values: ", sk_svd.singular_values_)
  print("[sklearn] explained variance: ", sk_svd.explained_variance_)
  print("[sklearn] explained variance ratio: ", sk_svd.explained_variance_ratio_)
except:
  print("\n# sklearn seems to be not installed in your system.\n# Thus skipping sklearn demo.")

# Shutting down the Frovedis server
FrovedisServer.shut_down()
