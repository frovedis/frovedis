import os
import numpy as np
import pandas as pd
from frovedis.exrpc.server import FrovedisServer
from frovedis.dataframe.df import FrovedisDataframe

FrovedisServer.initialize("mpirun -np 2 {}".format(os.environ['FROVEDIS_SERVER']))

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

# display created frovedis dataframes
print ("* print Frovedis DataFrame")
fdf1.show()
fdf2.show()

# select demo
print ("* select Ename and Age")
fdf1[["Ename","Age"]].show()

# filter demo
print ("* filter by Age > 19 and Contry == 'Japan'")
fdf1[(fdf1.Age > 19) & (fdf1.Country == 'Japan')].show()

# sort demo 
print ("* sort by Age (descending order)")
fdf1.sort("Age",ascending=False).show() # single column, descending
print ("* sort by Country and Age")
fdf1.sort(["Country", "Age"]).show()    # multiple column

# groupby demo
print ("* groupby Country and max/min/mean of Age and count of Ename")
fdf1.groupby('Country').agg({'Age': ['max','min','mean'], 
                             'Ename': ['count']}).show()

# renaming demo
print ("* rename Contry to Cname")
fdf3 = fdf2.rename({'Country' : 'Cname'})
fdf3.show()

# join after column renaming
print ("* merge (join) two tables")
fdf1.merge(fdf3, left_on="Country", right_on="Cname").show() # with defaults

# note: frovedis doesn't support multiple key joining at this moment.
# thus below call would cause an exception at frovedis server
#fdf1.merge(fdf3, left_on=["Country","Country"], 
#           right_on=["Cname","Cname"], how='outer', join_type='hash').show()

# operation chaining: join -> sort -> select -> show
print ("* chaining: merge two tables, sort by Age, and select Age, Ename and Country")
fdf1.merge(fdf3, left_on="Country", right_on="Cname") \
    .sort("Age")[["Age", "Ename", "Country"]].show()

# column statistics
print ("describe: ")
print (fdf1.describe())
print ("\n")

# merging with panda dataframe
print ("* merge with pandas table")
pdf2.rename(columns={'Country' : 'Cname'},inplace=True)
joined = fdf1.merge(pdf2, left_on="Country", right_on="Cname")
joined.show()

# conversion
print ("* convert Frovedis DataFrame to Pandas DataFrame")
print (fdf1.to_pandas())
print ("\n")

FrovedisServer.shut_down()
