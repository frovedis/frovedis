% DataFrame Function Application, Groupby
  
# NAME

DataFrame FunctionApplication, Groupby - list of all functions related to function application, groupby operations on frovedis dataframe are illustrated here.  
  
## SYNOPSIS  
    
    frovedis.dataframe.df.DataFrame(df = None, is_series = False)  
    
## Public Member Functions  

    1. agg(func)  
    2. apply(func, axis = 0, raw = False, result_type = None, args = (), \*\*kwds)  
    3. groupby((min_periods = None, ddof = 1.0, low_memory = True, other = None)

## Detailed Description  

### 1. agg(func)  

__Parameters__  
**_func_**: Names of functions to use for aggregating the data. The input to be used with the function must 
be a frovedis DataFrame instance having atleast one numeric column.  
Accepted combinations for this parameter are:
- A string function name such as 'max', 'min', etc.  
- list of functions and/or function names, For example, ['max', 'mean'].  
- dictionary with keys as column labels and values as function name or list of such functions.  
For Example, {'Age': ['max','min','mean'], 'Ename': ['count']}  

__Purpose__  
It computes an aggregate operation based on the condition specified in 'func'.  

**Currently, this method will perform aggregation operation along the rows.**  

For example,  
    
    import pandas as pd
    import numpy as np
    import frovedis.dataframe as fdf
    
    # a dictionary
    peopleDF = {
                'Name':['Jai', 'Anuj', 'Jai', 'Princi', 'Gaurav', 'Anuj', 'Princi', 'Abhi'],
                'Age':[27, 24, 22, 32, 33, 36, 27, 32],
                'City':['Nagpur', 'Kanpur', 'Allahabad', 'Kannuaj', 'Allahabad', 
                        'Kanpur', 'Kanpur', 'Kanpur'],
                'Qualification':['B.Tech', 'Phd', 'B.Tech', 'Phd', 'Phd', 'B.Tech', 'Phd', 'B.Tech'],
                'Score': [23, 34, 35, 45, np.nan, 50, 52, np.nan]
                }
    
    # create pandas dataframe
    pdf1 = pd.DataFrame(peopleDF)
    
    # create frovedis dataframe
    fdf1 = fdf.DataFrame(pdf1)
    
    # display the frovedis dataframe
    fdf1.show()

Output  
    
    index   Name    Age     City       Qualification  Score
    0       Jai     27      Nagpur     B.Tech         23
    1       Anuj    24      Kanpur     Phd            34
    2       Jai     22      Allahabad  B.Tech         35
    3       Princi  32      Kannuaj    Phd            45
    4       Gaurav  33      Allahabad  Phd            NULL
    5       Anuj    36      Kanpur     B.Tech         50
    6       Princi  27      Kanpur     Phd            52
    7       Abhi    32      Kanpur     B.Tech         NULL

For example,  

    # agg() demo with func as a function string name 
    print(fdf1.agg('max'))
    
Output  

    Name             nan
    Age               36
    City             nan
    Qualification    nan
    Score             52
    Name: max, dtype: object

It displays a pandas dataframe containing numeric column(s) with newly computed aggregates of each groups.  

For example,

    # agg() demo with func as a dictionary 
    print(fdf1.agg({"Age": ["std", "mean"]}))

Output

                Age
    mean  29.125000
    std    4.853202

For example,  
    
    # agg() demo where func is a list of functions 
    print(fdf1['Age'].agg(['max','min','mean'])

Output  

             Age
    max   36.000
    min   22.000
    mean  29.125

__Return Value__  
1. **If one 'func' provided and 'func' is a string:**  
     - It returns a pandas Series instance with numeric column(s) only, after aggregation function is completed.  
2. **If one or more 'func' provided and 'func' is list/dict of string:**  
     - It returns a pandas DataFrame instance with numeric column(s) only, after aggregation function is completed.  

### 2. apply(func, axis = 0, raw = False, result_type = None, args = (), \*\*kwds)  

__Parameters__  
**_func_**: Names of functions to be applied on the data. The input to be used with the function must 
be a frovedis DataFrame instance having atleast one numeric column.  
Accepted combinations for this parameter are:  
- A string function name such as 'max', 'min', etc.  
- list of functions and/or function names, For example, ['max', 'mean'].  
- dictionary with keys as column labels and values as function name or list of such functions.  
For Example, {'Age': ['max','min','mean'], 'Ename': ['count']}  

**_axis_**: It accepts an integer as parameter. It is used to decide whether to perform aggregate operation along the 
columns or rows. (Default: 0)  
_**raw**_: It accepts boolean as parameter. When set to True, the row/column will be passed as an ndarray. (Default: False)  
_**result\_type**_: It accepts string object as parameter. It specifies how the result will be returned. (Default: None)  
These only act when **axis = 1 (columns)**:  
- **expand** : list-like results will be turned into columns.  
- **reduce** : returns a Series if possible rather than expanding list-like results. This is the opposite of 'expand'.  
- **broadcast** : results will be broadcast to the original shape of the DataFrame, the original index and columns will be retained.  

The default behaviour (None) depends on the return value of the applied function. List-like results will be returned 
as a Series of those. However if the apply function returns a Series these are expanded to columns.  
_**args**_: Positional arguments to pass to 'func'. (Default: ())  
_**\*\*kwds**_: This is an unused parameter.  

__Purpose__  
Apply a function along an axis of the DataFrame.  

The parameter: "\*\*kwds" is simply kept in to make the interface uniform to the pandas DataFrame.apply().  
This is not used anywhere within the frovedis implementation.  

For example,  

    import pandas as pd
    import numpy as np
    import frovedis.dataframe as fdf
    
    # a dictionary
    peopleDF = {
                'Name':['Jai', 'Anuj', 'Jai', 'Princi', 'Gaurav', 'Anuj', 'Princi', 'Abhi'],
                'Age':[27, 24, 22, 32, 33, 36, 27, 32],
                'City':['Nagpur', 'Kanpur', 'Allahabad', 'Kannuaj', 'Allahabad', 
                        'Kanpur', 'Kanpur', 'Kanpur'],
                'Qualification':['B.Tech', 'Phd', 'B.Tech', 'Phd', 'Phd', 'B.Tech', 'Phd', 'B.Tech'],
                'Score': [23, 34, 35, 45, np.nan, 50, 52, np.nan]
                }
    
    # create pandas dataframe
    pdf1 = pd.DataFrame(peopleDF)
    
    # create frovedis dataframe
    fdf1 = fdf.DataFrame(pdf1)
    
    # display the frovedis dataframe
    fdf1.show()

Output

    index   Name    Age     City       Qualification  Score
    0       Jai     27      Nagpur     B.Tech         23
    1       Anuj    24      Kanpur     Phd            34
    2       Jai     22      Allahabad  B.Tech         35
    3       Princi  32      Kannuaj    Phd            45
    4       Gaurav  33      Allahabad  Phd            NULL
    5       Anuj    36      Kanpur     B.Tech         50
    6       Princi  27      Kanpur     Phd            52
    7       Abhi    32      Kanpur     B.Tech         NULL

For example,  

    # apply() demo using string function name
    print(fdf1.apply('max'))
    
Output  

    Name             8
    Age              8
    City             8
    Qualification    8
    Score            6
    dtype: int64

For example,  

    # apply() demo using axis = 1
    print(fdf1.apply('max', axis = 1))
    
Output  

    index
    0    27.0
    1    34.0
    2    35.0
    3    45.0
    4    33.0
    5    50.0
    6    52.0
    7    32.0
    dtype: float64

For example,  

    # apply() demo using raw = True
    print(fdf1.apply('max', raw = True))
    
Output  

    Name             Princi
    Age                  36
    City             Nagpur
    Qualification       Phd
    Score                52
    dtype: object

For example,  

    # apply() demo using result_type = 'expand'
    print(fdf1.apply('max', result_type = 'expand'))
    
Output  

    Name             Princi
    Age                  36
    City             Nagpur
    Qualification       Phd
    Score                52
    dtype: object

__Return Value__  
1. **If only one 'func' provided:**  
     - It returns a pandas Series instance with numeric column(s) only, after aggregation function is completed.  
2. **If more than one 'func' provided:**  
     - It returns a pandas DataFrame instance with numeric column(s) only, after aggregation function is completed.  

### 3. groupby(by = None, axis = 0, level = None, as_index = True, sort = True, group_keys = True, squeeze = False, observed = False, dropna = True)  

__Parameters__  
**_by_**: It accepts a string object or an iterable to determine the groups on which groupby operation will 
be applied. Currently, gropuby operation will be applied along the index levels. It must be provided, otherwise 
it will raise an exception. (Default: None)  
**_axis_**: It accepts an integer as parameter. It is used to decide whether to perform groupby operation along 
the indices or by column labels. (Default: 0)  
**Currently, axis = 0 is supported by this method.**  
**_level_**: This is an unused parameter. (Default: None)  
_**as\_index**_: This is an unused parameter. (Default: True)  
_**sort**_: This is an unused parameter. (Default: True)  
_**group\_keys**_: This is an unused parameter. (Default: True)  
_**squeeze**_: This is an unused parameter. (Default: False)  
_**observed**_: This is an unused parameter. (Default: False)  
_**dropna**_: It accepts a boolean parameter. It is used to remove missing values (NaNs) from the frovedis 
DataFrame during groupby operation. Currently, it removes missing values along the index levels. (Default: True)  

__Purpose__  
This method can be used to group large amounts of data and compute operations on these groups.  

The parameters: "level", "as_index", "sort", "group_keys", "squeeze" and "observed" is simply kept in to make the interface uniform to the pandas DataFrame.groupby().  
This is not used anywhere within the frovedis implementation.  

For example,  

    import pandas as pd
    import numpy as np
    import frovedis.dataframe as fdf
    
    # a dictionary
    peopleDF = {
                'Name': ['Jai', 'Anuj', 'Jai', 'Princi', 'Gaurav', 'Anuj', 'Princi', 'Abhi'],
                'Age': [27, 24, 22, 32, 33, 36, 27, 32],
                'City': ['Nagpur', 'Kanpur', 'Allahabad', 'Kannuaj', 'Allahabad', 
                         'Kanpur', 'Kanpur', 'Kanpur'],
                'Qualification': ['B.Tech', 'Phd', 'B.Tech', np.nan, 'Phd', 'B.Tech', 'Phd', np.nan],
                'Score': [23, 34, 35, 45, np.nan, 50, 52, np.nan]
               }

    # create pandas dataframe
    pdf1 = pd.DataFrame(peopleDF)
    
    # create frovedis dataframe
    fdf1 = fdf.DataFrame(pdf1)
    
    # display the frovedis dataframe
    fdf1.show()
        
Output  

    index   Name    Age   City       Qualification  Score
    0       Jai     27    Nagpur     B.Tech         23
    1       Anuj    24    Kanpur     Phd            34
    2       Jai     22    Allahabad  B.Tech         35
    3       Princi  32    Kannuaj    NULL           45
    4       Gaurav  33    Allahabad  Phd            NULL
    5       Anuj    36    Kanpur     B.Tech         50
    6       Princi  27    Kanpur     Phd            52
    7       Abhi    32    Kanpur     NULL           NULL

For example,  

    # groupby() demo
    fdf1.groupby('Qualification')
    
This will perform groupby operation on the dataframe over **'Qualification'** column data.  

For example,  

    # groupby() demo to perform aggregation on resultant grouped dataframe
    # Also, dropna = True by default
    fdf1.groupby('Qualification', dropna = True).agg({'Score': 'count'})

Output  

    Qualification   count_Score
    B.Tech  3
    Phd     2
    
Here, it excludes **NULL** group since missing values were dropped during groupby().  

For example,  

    # groupby() demo to perform aggregation on resultant grouped dataframe
    # Also, dropna = False
    fdf1.groupby('Qualification', dropna = False).agg({'Score': 'count'})

Output  

    Qualification   count_Score
    B.Tech  3
    Phd     2
    NULL    1
    
Here, it includes **NULL** as new group since missing values were not dropped during groupby().  

__Return Value__  
It returns a FrovedisGroupedDataFrame instance.  

# SEE ALSO  

- **[DataFrame - Introduction](./DataFrame_Introduction.md)**  
- **[DataFrame - Selection and Combinations](./DataFrame_SelectionAndCombinations.md)**  
- **[DataFrame - Conversion, Missing data handling, Sorting Functions](./DataFrame_ConversionAndSorting.md)**  
- **[DataFrame - Aggregate Functions](./DataFrame_AggregateFunctions.md)**  
- **[DataFrame - Binary Operators](./DataFrame_BinaryOperators.md)**  
