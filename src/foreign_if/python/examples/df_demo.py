#!/usr/bin/env python

import sys
import numpy as np
import pandas as pd
import frovedis.dataframe as fdf
from frovedis.exrpc.server import FrovedisServer

# initializing the Frovedis server
argvs = sys.argv
argc = len(argvs)
if (argc < 2):
    print ('Please give frovedis_server calling command as the first argument \n(e.g. "mpirun -np 2 -x /opt/nec/nosupport/frovedis/ve/bin/frovedis_server")')
    quit()
FrovedisServer.initialize(argvs[1])

peopleDF = {
            'Ename' : ['Michael', 'Andy', 'Tanaka', 'Raul', 'Yuta'], 
            'Age' : [29, 30, 27, 19, 31],
            'Country' : ['USA', 'England', 'Japan', 'France', 'Japan'],
            'isMale': [False, False, False, False, True]
           }

countryDF = {
             'Ccode' : [1, 2, 3, 4],
             'Country' : ['USA', 'England', 'Japan', 'France']
            }

pdf1 = pd.DataFrame(peopleDF)
pdf2 = pd.DataFrame(countryDF)
fdf1 = fdf.DataFrame(pdf1)
fdf2 = fdf.DataFrame(pdf2)

# displaying created frovedis dataframes
fdf1.show()
fdf2.show()

# select demo
fdf1["Ename"].show()         # single column
fdf1[["Ename","Age"]].show() # multiple column

# filter demo
fdf1[fdf1.Age > 19].show()  
fdf1[(fdf1.Age > 19) & (fdf1.Country == 'Japan')].show()    # AND Demo
fdf1[~((fdf1.Age > 19) & (fdf1.Country == 'Japan'))].show() # NOT Demo

# sort demo 
fdf1.sort_values("Age",ascending=1).show() # single column
fdf1.sort_values(["Country", "Age"], ascending=[0,1]).show() # multiple column

# groupby demo
fdf1.groupby('Country')['Country'].show()
 
fdf1.groupby('Country').agg({'Age': ['max','min','mean'],
                             'Ename': ['count']}).show()

# merge demo
fdf1.merge(fdf2, left_on="Country", right_on="Country").show()

# multi-key join
df_tmp = fdf1.rename({'Age': 'Age2' })
fdf1.join(df_tmp, on = ["Ename", "Country"]).show()

# filter()
print("Filter")
fdf1.filter(items=['Ename']).show()
fdf1.filter(like='C', axis=1).show()

# contains, startswith, endswith
fdf1[fdf1.Country.str.contains("a")].show()
fdf1[fdf1.Country.str.startswith("J")].show()
fdf1[fdf1.Country.str.endswith("e")].show()

# renaming demo
fdf3 = fdf2.rename({'Country' : 'Cname'})
fdf2.show()
fdf3.show()

# join after column renaming
fdf1.merge(fdf3, left_on="Country", right_on="Cname").show() # with defaults
fdf1.merge(fdf3, left_on="Country", right_on="Cname", how='left', join_type='hash').show()
# operation chaining: join -> sort -> select -> show
fdf1.merge(fdf3, left_on="Country", 
           right_on="Cname", how='left', join_type='hash').sort("Age")[["Age", "Ename", "Country"]].show()
fdf3.release()

# merging with panda dataframe
pdf2.rename(columns={'Country' : 'Cname'},inplace=True)
joined = fdf1.merge(pdf2, left_on="Country", right_on="Cname")
joined.show()

# conversion demo
print(fdf1.to_panda_dataframe()); print("\n")
print(joined.to_panda_dataframe()); print("\n")
joined.release()

# miscellaneous
# all column count, sum, max, mkin for all columns
print ("all count: ", fdf1.agg(["count", "sum", "max", "min"])) 
# column specific agg operations 
print ("std(Age): ", fdf1.agg({"Age": ["std"]})); print("\n")
print ("count(Country): ", fdf1.agg({"Country": ["count"]})) ; print("\n")
print ("sum(isMale): ", fdf1.agg({"isMale": ["sum"]})) ;  print("\n")

# read_csv demo
df = fdf.read_csv("./input/numbers.csv", names=['one', 'two', 'three', 'four'])
df.show()

# describing all statictics for numeric columns
print(df.describe()) # prints count, mean, std, sum, min, max

# matrix conversion demo
row_mat = df.to_frovedis_rowmajor_matrix(['one', 'two'], dtype=np.float64)
row_mat.debug_print()

col_mat = df.to_frovedis_colmajor_matrix(['one', 'two']) # default dtype = float32
col_mat.debug_print()

crs_mat,info = df.to_frovedis_crs_matrix(['one', 'two', 'four'], 
                                         ['four'], need_info=True) # default dtype = float32
crs_mat.debug_print()

crs_mat2 = df.to_frovedis_crs_matrix_using_info(info)
crs_mat2.debug_print()

df.release()
row_mat.release()
col_mat.release()
crs_mat.release()
crs_mat2.release()
info.save("./out/info")
info.release()

fdf1.release()
fdf2.release()

FrovedisServer.shut_down()
