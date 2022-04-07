% FrovedisGroupedDataframe  
  
# NAME  

FrovedisGroupedDataframe - A python class for handling grouped dataframes. These are returned by groupby calls.  

# SYNOPSIS  

    frovedis.dataframe.grouped_df.FrovedisGroupedDataframe(df = None)  

# DESCRIPTION  

FrovedisGroupedDataframe instance contains information about the grouped dataframe. After columns are 
grouped, various aggregations can be performed like groupwise average, groupwise variance, etc.  
**In FrovedisGroupedDataframe, currently aggregation operations is performed along the rows only.**  

This module provides a client-server implementation, where the client application is a normal python 
program. The FrovedisGroupedDataframe interface is almost same as pandas DataFrameGroupBy interface, but 
it doesn’t have any dependency on pandas. It can be used simply even if the system doesn’t have pandas 
installed. The FrovedisGroupedDataframe instance is created when groupby interface is called for frovedis 
Dataframe instance. Thus, in this implementation, a python client can interact with a frovedis server sending 
the required python dataframe for performing query at frovedis side. Python dataframe is converted 
into frovedis compatible dataframe internally and the python client side call is linked with the 
respective frovedis side call to get the job done at frovedis server.  

## Detailed description  

### 1. FrovedisGroupedDataframe(df = None)  

__Parameters__  
**_df_**: It accepts only frovedis dataframe as parameter. (Default: None)  
When it is None (not specified explicitly), an empty FrovedisGroupedDataframe instance is created.  

__Purpose__  
It is used for a specific purpose. It's instance is created in order to hold result of groupby method calls.  
This instance can then further be used with aggregate functions such as mean(), sem(), etc.  

**Creating FrovedisGroupedDataframe instance using groupby operation:**  

For example,  
    
    # create frovedis dataframe
    import pandas as pd
    import frovedis.dataframe as fdf
    peopleDF = {
            'Ename' : ['Michael', 'Andy', 'Tanaka', 'Raul', 'Yuta'], 
            'Age' : [29, 30, 27, 19, 31],
            'Country' : ['USA', 'England', 'Japan', 'France', 'Japan'],
            'isMale': [False, False, False, False, True]
           }
    pdf1 = pd.DataFrame(peopleDF)
    fdf1 = fdf.DataFrame(pdf1)
    
    # create FrovedisGroupedDataframe object
    g_df = fdf1.groupby('Country')

The groupby call returns a FrovedisGroupedDataframe instance.  

**Multiple columns can be used in groupby operation to create this instance:**  

For example,  

    # using multiple columns to create FrovedisGroupedDataframe object
    g_df = fdf1.groupby(['Age','Country'])

**In order to select one of the grouped column:**   

For example,  

    print(g_df['Country'])

Output  

    Country
    England
    France
    Japan
    USA

**In order to select multiple columns from the grouped dataframe:**   

For example,  

    print(g_df[['Age','Country'])

Output  

    index   Age     Country
    0       19      France
    1       27      Japan
    2       29      USA
    3       30      England
    4       31      Japan


__Return Value__  
It simply returns "self" reference.  

### 2. release()
  
__Purpose__  
This method acts like a destructor. It is used to release dataframe pointer from server heap 
and it resets all its attributes to None.  
  
For example,  

    g_df.release()

__Return Value__  
It returns nothing.  
  
## Public Member Functions  
  
FrovedisGroupedDataFrame provides a lot of utilities to perform various operations.  

### List of Aggregate Functions  

1. **agg()** - it agggregates using one or more operations over the specified axis. It is an alias for aggregate().  
2. **aggregate()** - it agggregates using one or more operations over the specified axis. The alias agg() can be used instead.  
3. **count()** - it computes count of group, excluding missing values.  
4. **max()** - it computes maximum of group values.  
5. **mean()** - it computes mean of groups, excluding missing values.  
6. **min()** - it computes minimum of group values.  
7. **sem()** - it computes standard error of the mean of groups, excluding missing values.  
8. **size()** - it computes group sizes.  
9. **sum()** - it computes sum of group values.  
10. **var()** - it computes variance of groups, excluding missing values.  

# SEE ALSO  
- **[Using aggregate functions on FrovedisGroupedDataFrame](./grouped_df_agg_func.md)**  
- **[Introduction to frovedis DataFrame](./df_intro.md)**  