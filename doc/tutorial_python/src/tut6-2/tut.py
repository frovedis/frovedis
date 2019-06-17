import os
import numpy as np
import pandas as pd
from frovedis.exrpc.server import FrovedisServer
from frovedis.dataframe.df import FrovedisDataframe

FrovedisServer.initialize("mpirun -np 2 {}".format(os.environ['FROVEDIS_SERVER']))

data = {'A': [10, 12, 13, 15],
        'B': [10.23, 12.20, 34.90, 100.12],
        'C': ['male', 'female', 'female', 'male'],
       }
pdf = pd.DataFrame(data)
df = FrovedisDataframe(pdf)
print (pdf); print

row_mat = df.to_frovedis_rowmajor_matrix(['A', 'B'], dtype=np.float64)
print (row_mat.to_numpy_matrix()); print

#col_mat = df.to_frovedis_colmajor_matrix(['A', 'B'], dtype=np.float64)
#print (col_mat.to_numpy_matrix())

crs_mat,info = df.to_frovedis_crs_matrix(['A', 'B', 'C'], 
                                         ['C'], need_info=True)
crs_mat.debug_print(); print

data2 = {'A': [12,13],
         'B': [34.56, 78.9],
         'C': ['male','male'],
     }
pdf2 = pd.DataFrame(data2)
df2 = FrovedisDataframe(pdf2)
print (pdf2); print

crs_mat2 = df2.to_frovedis_crs_matrix_using_info(info)
crs_mat2.debug_print()

df.release()
row_mat.release()
#col_mat.release()
crs_mat.release()
crs_mat2.release()
info.save("info")
info.release()

FrovedisServer.shut_down()
