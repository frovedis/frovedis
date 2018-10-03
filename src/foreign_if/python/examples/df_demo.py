#!/usr/bin/env python

from frovedis.exrpc.server import FrovedisServer
from frovedis.dataframe.df import FrovedisDataframe
import sys
import pandas as pd

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
            'Country' : ['USA', 'England', 'Japan', 'France', 'Japan']
           }

countryDF = {
             'Ccode' : [1, 2, 3, 4],
             'Country' : ['USA', 'England', 'Japan', 'France']
            }

pdf1 = pd.DataFrame(peopleDF)
pdf2 = pd.DataFrame(countryDF)
fdf1 = FrovedisDataframe(pdf1)
fdf2 = FrovedisDataframe(pdf2)

# displaying created frovedis dataframes
fdf1.show()
fdf2.show()

# select demo
fdf1["Ename"].show()         # single column
fdf1[["Ename","Age"]].show() # multiple column

# filter demo
fdf1[fdf1.Age > 19].show()  
fdf1[fdf1.Age > 19 and fdf1.Country == 'Japan'].show()

# sort demo 
fdf1.sort("Age",ascending=False).show() # single column
fdf1.sort(["Age", "Country"]).show()    # multiple column

# groupby demo
fdf1.groupby('Country')['Country'].show()

# merge demo
#currently joining on same key is not supported at frovedis server
#fdf1.merge(fdf2, left_on="Country", right_on="Country").show()

# renaming demo
fdf3 = fdf2.rename({'Country' : 'Cname'})
fdf2.show()
fdf3.show()

# join after column renaming
fdf1.merge(fdf3, left_on="Country", right_on="Cname").show() # with defaults
fdf1.merge(fdf3, left_on="Country", right_on="Cname", how='outer', join_type='hash').show()

# note: frovedis doesn't support multiple key joining at this moment.
# thus below call would cause an exception at frovedis server
#fdf1.merge(fdf3, left_on=["Country","Country"], 
#           right_on=["Cname","Cname"], how='outer', join_type='hash').show()

# operation chaining: join -> sort -> select -> show
fdf1.merge(fdf3, left_on="Country", 
           right_on="Cname", how='outer', join_type='hash').sort("Age")[["Age", "Ename", "Country"]].show()
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
print ("all count: ", fdf1.count()); print # all column counts
print ("min(age): ", fdf1.min("Age")); print("\n")
print ("max(age): ", fdf1.max("Age")); print("\n")
print ("sum(age): ", fdf1.sum("Age")); print("\n")
print ("avg(age): ", fdf1.avg("Age")); print("\n")
print ("std(age): ", fdf1.std("Age")); print("\n")
print ("count(age): ", fdf1.count("Age")); print("\n")
print(fdf1.describe()); print("\n")

# describe demo
data = {'one': [10, 12, 13, 15],
        'two': [10.23, 12.20, 34.90, 100.12],
        'three': ['F', 'F', 'D', 'A'],
        'four': [1, 2, 3, 4]
       }
pdf = pd.DataFrame(data)
print(pdf.describe()); print("\n")
df = FrovedisDataframe(pdf)
print(df.describe()); print("\n") # prints count, mean, std, sum, min, max
df.release()

fdf1.release()
fdf2.release()

FrovedisServer.shut_down()
