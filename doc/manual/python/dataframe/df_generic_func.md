% DataFrame Generic Functions
  
# NAME
  
DataFrame Generic Functions - it contains list of all generally used functions with frovedis dataframe.  
  
## DESCRIPTION  
  
Frovedis dataframe provides various functions which are generally/frequently used with it to perform operations:  
- Selecting / Filtering / Modifying data in frovedis dataframe.  
- Combining two frovedis dataframes.  
- Dropping data from rows or columns ina frovedis dataframe.  
- Display frovedis dataframe (full/partially)  

## Public Member Functions  
    1. add_index(name)
    2. append(other, ignore_index = False, verify_integrity = False, sort = False)
    3. apply()
    4. astype(dtype, copy = True, errors = 'raise', check_bool_like_string = False)
    5. between(left, right, inclusive="both")
    6. copy(deep = True)
    7. countna()
    8. describe()        
    9. drop(labels = None, axis = 0, index = None, columns = None, level = None, 
            inplace = False, errors = 'raise')
    10. drop_duplicates(subset = None, keep = 'first', inplace = False, ignore_index = False)
    11. dropna(axis = 0, how = 'any', thresh = None, subset = None, inplace = False)
    12. fillna(value = None, method = None, axis = None, inplace = False, limit = None, downcast = None)
    13. filter(items = None, like = None, regex = None, axis = None)
    14. get_index_loc(value)
    15. head(n = 5)
    16. insert(loc, column, value, allow_duplicates = False)
    17. isna()
    18. isnull()
    19. join(right, on, how = 'inner', lsuffix = '_left', rsuffix = '_right', 
             sort = False, join_type = 'bcast')
    20. merge(right, on = None, how = 'inner', left_on = None, right_on = None, 
              left_index = False, right_index = False, sort = False, suffixes = ('_x', '_y'), 
              copy = True, indicator = False, join_type = 'bcast')
    21. rename(columns, inplace = False)
    22. rename_index(new_name, inplace = False)
    23. reset_index(drop = False, inplace = False)
    24. set_index(keys, drop = True, append = False, inplace = False, verify_integrity = False)
    25. show()
    26. tail(n = 5)
    27. update_index(value, key = None, verify_integrity = False, inplace = False) 

## Detailed Description   

### 1. DataFrame.add_index(name)  

__Parameters__  
**_name_**: It accepts a string object as parameter which represents the name of the index label.  

__Purpose__  
It adds index column to the Frovedis DataFrame object. The index added is zero based i.e. [0, 1, ... n-1] where n is the number of rows.  

For example,  

    import pandas as pd
    import frovedis.dataframe as fdf
    
    peopleDF = {
                'Ename' : ['Michael', 'Andy', 'Tanaka', 'Raul', 'Yuta'],
                'Age' : [29, 30, 27, 19, 31],
                'Country' : ['USA', 'England', 'Japan', 'France', 'Japan'],
                'isMale': [False, False, False, False, True]
               }

    # create a pandas dataframe
    pd_df = pd.DataFrame(peopleDF)
    
    # create a frovedis dataframe
    fd_df = fdf.DataFrame(pd_df)

    # display the frovedis dataframe
    fd_df.show()

Output  
    
    index   Ename   Age     Country isMale
    0       Michael 29      USA     0
    1       Andy    30      England 0
    2       Tanaka  27      Japan   0
    3       Raul    19      France  0
    4       Yuta    31      Japan   1

For example,  

    # add_index() demo
    fd_df.add_index("SNo").show()

Output  

    SNo   index   Ename   Age     Country isMale
    0     0       Michael 29      USA     0
    1     1       Andy    30      England 0
    2     2       Tanaka  27      Japan   0
    3     3       Raul    19      France  0
    4     4       Yuta    31      Japan   1  

__Return Value__  
It returns a self reference.  

### 2. DataFrame.append(other, ignore_index = False, verify_integrity = False, sort = False)   

__Parameters__  
**_other_**: It accepts a Frovedis DataFrame instance or a Pandas DataFrame instance or a list of Frovedis DataFrame instances which are to be appended.  
**_ignore\_index_**: It accepts a boolean type parameter. If True, old index axis is ignored and a new index axis is added with values 0 to n - 1, where n is the number of rows in the DataFrame. (Default: False)  
**_verify\_integrity_**: It accepts a boolean type as parameter. If it is set to True, it checks 'index' label for duplicate entries before appending and when there are duplicate entries in the DataFrame, it doesn't append. Otherwise duplicate entries in the 'index' label will be appended. It will also append duplicate entries when 'ignore_index' = True. (Default: False)  
**_sort_**: It accepts a boolean type as parameter. It sorts the columns, if the columns of the given DataFrame and other DataFrame are not aligned. (Default: False)  
  
__Purpose__  
It is used to append entries of dataframe at the end of another dataframe. The columns of other DataFrame instance that are not in the calling DataFrame instance are added as new columns.  

For example,  

    import pandas as pd
    import frovedis.dataframe as fdf
    
    # creating a pandas dataframe
    pd_df1 = pd.DataFrame([[1, 2], [3, 4]], columns = list('AB'), index = ['x', 'y'])

    # create a frovedis dataframe
    fd_df1 = fdf.DataFrame(pd_df1)
    
    # display frovedis dataframe
    fd_df1.show()
    
    # create another pandas dataframe
    pd_df2 = pd.DataFrame([[5, 6], [7, 8]], columns = list('AB'), index = ['x', 'y'])
    
    # create another frovedis dataframe
    fd_df2 = fdf.DataFrame(pd_df2)
    
    # display other frovedis dataframe
    fd_df2.show()

Output  
    
    index   A       B
    x       1       2
    y       3       4

    index   A       B
    x       5       6
    y       7       8

For example, 

    # example of using append method with default values
    # appending fd_df2 to fd_df1
    fd_df1.append(fd_df2).show() 
    
Output  

    index   A       B
    x       1       2
    y       3       4
    x       5       6
    y       7       8 

For example,  

    # append() demo With ignore_index = True
    fd_df1.append(fd_df2, ignore_index = True).show()
    
Output,    
        
    index   A       B
    0       1       2
    1       3       4
    2       5       6
    3       7       8

For example,  

    import pandas as pd
    import frovedis.dataframe as fdf
    
    # creating a pandas dataframe
    pd_df1 = pd.DataFrame([[1, 2], [3, 4]], columns = list('AB'), index = ['x', 'y'])

    # create a frovedis dataframe
    fd_df1 = fdf.DataFrame(pd_df1)
    
    # display frovedis dataframe
    fd_df1.show()
    
    # create another pandas dataframe
    pd_df2 = pd.DataFrame([[5, 6], [7, 8]], columns = list('CD'), index = ['x', 'y'])
    
    # create another frovedis dataframe
    fd_df2 = fdf.DataFrame(pd_df2)
    
    # display other frovedis dataframe
    fd_df2.show()

Output  

    index   A       B
    x       1       2
    y       3       4

    index   C       D
    x       5       6
    y       7       8

For example,  

    # append() demo when there are no common columns
    fd_df1.append(fd_df2).show() 
    
Output  

    index   A       B       D       C
    x       1       2       NULL    NULL
    y       3       4       NULL    NULL
    x       NULL    NULL    6       5
    y       NULL    NULL    8       7

For example,  
    
    import pandas as pd
    import frovedis.dataframe as fdf

    # creating a pandas dataframe
    pd_df1 = pd.DataFrame([[1, 2], [3, 4]], columns=list('AB'), index=['x', 'y'])
    
    # creating a frovedis dataframe
    fd_df1 = fdf.DataFrame(pd_df1)
    
    # display a frovedis dataframe
    fd_df1.show()
    
    # creating another pandas dataframe
    pd_df2 = pd.DataFrame([[1, 2], [7, 8]], columns=list('AB'), index=['u', 'v'])    
    
    # creating another frovedis dataframe
    fd_df2 = fdf.DataFrame(pd_df2)
    
    # display other frovedis dataframe
    fd_df2.show()
    
Output  

    index   A       B
    x       1       2
    y       3       4

    index   A       B
    u       1       2
    v       7       8

For example,  

    # appending fd_df2 to fd_df1 with verify_integrity = True
    fd_df1.append(fd_df2, verify_integrity = True).show()
    
Output  
    
    index   A       B
    x       1       2
    y       3       4
    u       1       2
    v       7       8

For example,  
    
    import pandas as pd
    import frovedis.dataframe as fdf

    # creating a pandas dataframe
    pd_df1 = pd.DataFrame([[1, 2], [3, 4]], columns=list('YZ'), index=['x', 'y'])

    # creating a frovedis dataframe
    fd_df1 = fdf.DataFrame(pd_df1)

    # display a frovedis dataframe
    fd_df1.show()
    
    # creating another pandas dataframe
    pd_df2 = pd.DataFrame([[1, 2], [7, 8]], columns=list('AB'), index=['x', 'y'])    

    # creating another frovedis dataframe
    fd_df2 = fdf.DataFrame(pd_df2)

    # display other frovedis dataframe
    fd_df2.show()

Output  

    index   Y       Z
    x       1       2
    y       3       4

    index   A       B
    x       1       2
    y       7       8

For example,  

    # appending fd_df2 with fd_df1 with sort = True
    fd_df1.append(fd_df2, sort = True).show()
    
Output  

    index   A       B       Y       Z
    x       NULL    NULL    1       2
    y       NULL    NULL    3       4
    x       1       2       NULL    NULL
    y       7       8       NULL    NULL

__Return Value__  
It returns a new Frovedis DataFrame consisting of the rows of original DataFrame object and the rows of other DataFrame object.  

### 3. DataFrame.apply(func, axis = 0, raw = False, result_type = None, args = (), \*\*kwds)  

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

**It actually uses pandas.apply() internally**.  

Basically, it converts the pandas dataframe into frovedis dataframe and after performing apply(), it returns the result back as pandas dataframe.  

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

### 4. DataFrame.astype(dtype, copy = True, errors = 'raise', check_bool_like_string = False)  

__Parameters__  
**_dtype_**: It accepts a string, numpy.dtype or a dict of column labels to cast entire DataFrame object to same type or one or more columns to column-specific types.  
**_copy_**: It accepts a boolean parameter and returns a new DataFrame object when it is True. Currently this parameter will always return a copy. The original DataFrame is not modified. (Default: True)  
**_errors_**: This is an unused parameter. (Default: 'raise')  
**_check\_bool\_like\_string_**: A boolean parameter which when set to True will cast string columns having boolean like case-insensitive strings (True, False, yes, No, On, Off, Y, N, T, F) to boolean columns. (Default: False)  

__Purpose__  
It cast an entire Frovedis DataFrame or selected columns of Frovedis DataFrame to the specified dtype.  

The parameter: "errors" is simply kept in to make the interface uniform to the pandas DataFrame.astype().  
This is not used anywhere within the frovedis implementation.  

For example,  
    
    import pandas as pd
    import frovedis.dataframe as fdf

    # a dictionary
    peopleDF = {
				'Ename' : ['Michael', 'Andy', 'Tanaka', 'Raul', 'Yuta'],
				'Age' : [29, 30, 27, 19, 31],
				'Country' : ['USA', 'England', 'Japan', 'France', 'Japan'],
				'isMale': [False, False, False, False, True]
			   }
    # converting to pandas dataframe  
    pd_df = pd.DataFrame(peopleDF)

    # converting to frovedis dataframe
    fd_df = fdf.DataFrame(pd_df)

    # display the datatype of object
    print(fd_df.dtypes)

Output  

    Ename      object
    Age         int64
    Country    object
    isMale       bool
    dtype: object

For example,  

    # display the datatype of object after conversion of 'Age' column
    print(fd_df.astype({'Age':'int32'}).dtypes)
    
Output  

    Ename      object
    Age         int32
    Country    object
    isMale      int32
    dtype: object

For example,  
    
    import pandas as pd
    import frovedis.dataframe as fdf
    
    # creating a pandas dataframe
    pd_df =  pd.DataFrame({'col1': [1, 2], 'col2': [3, 4]})
    
    # creating a frovedis dataframe
    fd_df1 = fdf.DataFrame(pf1)

    # displaying a frovedis dataframe dtype 
    print(fd_df1.dtypes)

Output  

    col1    int64
    col2    int64
    dtype: object

For example,  

    # cast all columns to int32:
    fd_df2 = fd_df1.astype('int32')
    
    # display dataframe after conversion
    print(fd_df2.dtypes)
    
Output  

    col1    int32
    col2    int32
    dtype: object

For example,  
    
    import pandas as pd
    import frovedis.dataframe as fdf

    # a dictionary
    peopleDF = {
				'Ename' : ['Michael', 'Andy', 'Tanaka', 'Raul', 'Yuta'],
				'Age' : [29, 30, 27, 19, 31],
				'Country' : ['USA', 'England', 'Japan', 'France', 'Japan'],
				'isMale': ['F', 'No', 'Off', False, 'Y']
			   }
    # converting to pandas dataframe  
    pd_df = pd.DataFrame(peopleDF)
    
    # converting to frovedis dataframe
    fd_df = fdf.DataFrame(pd_df)
    
    # display the dataframes
    fd_df.show()
    
    # display the datatype
    print(fd_df.dtypes)
    
Output  

    index   Ename   Age     Country isMale
    0       Michael 29      USA     F
    1       Andy    30      England No
    2       Tanaka  27      Japan   Off
    3       Raul    19      France  False
    4       Yuta    31      Japan   Y
    
    Ename      object
    Age         int64
    Country    object
    isMale     object
    dtype: object

For example,  

    # astype() demo with check_bool_like_string = True
    fd_df.astype({'isMale':'bool'}, check_bool_like_string = True).show()
    
    # display the datatype
    print(fd_df.astype({'isMale':'bool'}, check_bool_like_string = True).dtypes)
    
Output  
    
    index   Ename   Age     Country isMale
    0       Michael 29      USA     0
    1       Andy    30      England 0
    2       Tanaka  27      Japan   0
    3       Raul    19      France  0
    4       Yuta    31      Japan   1
    
    Ename      object
    Age         int64
    Country    object
    isMale       bool
    dtype: object

__Return Value__  
It returns a new DataFrame instance with dtype converted as specified.  

### 5. DataFrame.between(left, right, inclusive = "both")  

__Parameters__  
**_left_**: It accepts scalar values as parameter.  
**_right_**: It accepts scalar values as parameter.  
**_inclusive_**: It accepts string object as parameter tha specifies which boundaries to include, whether to set 
bounds as open or closed. (Default: 'both')  
- **'left'**: left boundary value is included.  
- **'right'**: right boundary value is included.  
- **'both'**: boundary values are included.  
- **'neither'**: boundary values are excluded.  

__Purpose__  
This method performs filtering of rows according to the specified bound over a single column at a time.  

**Currently, this method filters data for numeric column data only.**  

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

    index   Name    Age     City       Qualification   Score
    0       Jai     27      Nagpur     B.Tech          23
    1       Anuj    24      Kanpur     Phd             34
    2       Jai     22      Allahabad  B.Tech          35
    3       Princi  32      Kannuaj    Phd             45
    4       Gaurav  33      Allahabad  Phd             NULL
    5       Anuj    36      Kanpur     B.Tech          50
    6       Princi  27      Kanpur     Phd             52
    7       Abhi    32      Kanpur     B.Tech          NULL

For example,  

    # between() demo used with given column of dataframe
    # Also, both boundaries are included
    fdf1['Score'].between(40,50)
    
Unlike pandas, it returns an instance of dfoperator **(present in frovedis only)** which contains masked boolean data as result of between operation. In order to unmask the data, **to_mask_array()** must be used.

    fdf1['Score'].between(40,50).to_mask_array
    
Output  

    [False False False  True False  True False False]

This returns a boolean array containing True wherever the corresponding element is between the boundary values 'left' and 'right'. The missing values are treated as False.  

Also, this method can be used in the form of given expression below:  

For example,  
    
    # between() demo used with given column of dataframe
    # Also, both boundaries are included
    res = fdf1[fdf1['Score'].between(40,50)]
    print(res)

Output  

    index   Name    Age     City    Qualification   Score
    3       Princi  32      Kannuaj Phd     45
    5       Anuj    36      Kanpur  B.Tech  50
    
It can also be expressed as follows:  

For example,  

    print(fdf1[fdf1.Score.between(40,50)])

Output  

    index   Name    Age     City    Qualification   Score
    3       Princi  32      Kannuaj Phd     45
    5       Anuj    36      Kanpur  B.Tech  50

Now, when 'inclusive' parameter is provided, then,  

For example,  

    # between() demo and only left boundary included
    print(fdf1['Score'].between(40,50,inclusive = 'left').to_mask_array())
    
Output  

    [False False False  True False False False False]

For example,  

    # between() demo and only right boundary included
    print(fdf1['Score'].between(40,50,inclusive = 'right').to_mask_array())
    
Output  

    [False False False  True False  True False False]

For example,  

    # between() demo and both boundaries excluded
    print(fdf1['Score'].between(40,50,inclusive = 'neither').to_mask_array())
    
Output  

    [False False False  True False False False False]
    
__Return Value__  
It returns a dfoperator instance.  

### 6. DataFrame.copy(deep = True)  

__Parameters__  
**_deep_**: A boolean parameter to decide the type of copy operation. (Default: True)  
When it is True (not specified explicitly), it creates a deep copy i.e. the copy includes copy of the original DataFrame's data and the indices. **Currently this parameter doesn't support shallow copy (deep = False).**  
  
__Purpose__  
It creates a deep copy of the Frovedis DataFrame object’s indices and data.  

For example,  
    
    import pandas as pd
    import frovedis.dataframe as fdf
    
    peopleDF = {
				'Ename' : ['Michael', 'Andy', 'Tanaka', 'Raul', 'Yuta'],
				'Age' : [29, 30, 27, 19, 31],
				'Country' : ['USA', 'England', 'Japan', 'France', 'Japan'],
				'isMale': [False, False, False, False, True]
			   }
    # creating a pandas dataframe
    pd_df = pd.DataFrame(peopleDF)
    
    #creating a frovedis dataframe
    fd_df = fdf.DataFrame(pd_df)
    
    print('displaying original dataframe object')
    fd_df.show()

Output  

    displaying original dataframe object
    index   Ename   Age     Country isMale
    0       Michael 29      USA     0
    1       Andy    30      England 0
    2       Tanaka  27      Japan   0
    3       Raul    19      France  0
    4       Yuta    31      Japan   1

For example,  

    # creating a deep copy
    fd_df_copy = fd_df.copy()

    print('displaying copied dataframe object')
    fd_df_copy.show()

Output  

    displaying copied dataframe object
    index   Ename   Age     Country isMale
    0       Michael 29      USA     0
    1       Andy    30      England 0
    2       Tanaka  27      Japan   0
    3       Raul    19      France  0
    4       Yuta    31      Japan   1

For example,  

    # changing column Age to Age2 for the copied object
    fd_df_copy.rename({'Age':'Age2', inplace = True}
    
    print('displaying copied dataframe object')
    fd_df_copy.show()
    
Output  

    displaying copied dataframe object
    index   Ename   Age2     Country isMale
    0       Michael 29      USA     0
    1       Andy    30      England 0
    2       Tanaka  27      Japan   0
    3       Raul    19      France  0
    4       Yuta    31      Japan   1

**NOTE: changes are reflected only in copied DataFrame instance but not in original DataFrame instance.**  

__Return Value__  
It returns a deep copy of the DataFrame instance of the same type.  

### 7. DataFrame.countna(axis = 0)  

__Parameters__  
**_axis_**: It accepts an integer or string object as parameter. It is used to decide whether to 
count missing values along the indices or by column labels. (Default: 0)  
- **0 or 'index'**: count missing values along the indices.  
- **1 or 'columns'**: count missing values along the columns.  

__Purpose__  
It counts number of missing values in the given axis.  

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

    # countna() demo
    fdf1.countna().show()

Output

    index   count
    Name    0
    Age     0
    City    0
    Qualification   0
    Score   2
    
For example,

    # countna() demo using axis = 1
    fdf1.countna(axis = 1).show()
    
Output

    index   count
    0       0
    1       0
    2       0
    3       0
    4       1
    5       0
    6       0
    7       1

__Return Value__  
It returns a frovedis DataFrame instance.  

### 8. DataFrame.describe()  

__Purpose__  
It generates descriptive statistics. Descriptive statistics include count, mean, median, etc, excluding missing values.  

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

    print(fdf1.describe())

Output  

                   Age       Score
    count     8.000000    6.000000
    mean     29.125000   39.833333
    median   29.500000   40.000000
    var      23.553571  123.766667
    mad       4.125000    9.166667
    std       4.853202   11.125047
    sem       1.715866    4.541781
    sum     233.000000  239.000000
    min      22.000000   23.000000
    max      36.000000   52.000000

__Return Value__  
It returns a pandas DataFrame instance with the result of the specified aggregate operation.  

### 9. DataFrame.drop(labels = None, axis = 0, index = None, columns = None, level = None, inplace = False, errors = 'raise')  

__Parameters__  
**_labels_**: It takes an integer or string type as argument. It represents column labels and integer represent index values of rows to be dropped. If any of the label is not found in the selected axis, it will raise an exception. (Default: None)  
When it is None (not specified explicitly), 'index' or 'columns' parameter must be provided. Otherwise it will drop specific rows of the DataFrame.  
**_axis_**: It accepts an integer type parameter where the value indicates the direction of drop operation according to below conditions:  
- 0 : The corresponding index labels will be dropped.  
- 1 : The corresponding columns labels will be dropped.  

**_index_**: It accepts an integer or string object as parameter. It is equivalent to dropping indices along the axis = 0. (Default: None)  
When it is None (not specified explicitly), 'columns' or 'labels' must be provided.  
**_columns_**: It accepts a string object or list of strings. It is equivalent to dropping columns along the axis = 1. (Default: None)  
When it is None (not specified explicitly), 'index' or 'labels' must be provided.  
**_level_**: This is an unsed parameter. (Default: None)   
**_inplace_**: It accepts a boolean as a parameter. It return a copy of DataFrame instance by default but when explicitly set to True, it performs operation on original DataFrame. (Default: False)  
**_errors_**: This is an unsed parameter. (Default: ‘raise’)  

__Purpose__  
It is used to drop specified labels from rows or columns.  
Rows or columns can be removed by specifying label names and corresponding axis, or by specifying index or column names.  

For example,  
    
    import pandas as pd
    import numpy as np
    import frovedis.dataframe as fdf
    
    pd_df = pd.DataFrame(np.arange(12).reshape(3, 4), columns = ['A', 'B', 'C', 'D'])
    
    # creating a frovedis dataframe
    fd_df = fdf.DataFrame(pd_df).
    
    # display frovedis dataframe
    fd_df.show()

Output  

    index   A       B       C       D
    0       0       1       2       3
    1       4       5       6       7
    2       8       9       10      11

For example,  

    # drop() demo with label and axis parameter
    fd_df.drop(labels = ['B', 'C'], axis = 1).show()
    
Output  
    
    index   A       D
    0       0       3
    1       4       7
    2       8       11
    
For example,  

    # drop() demo with columns parameter     
    fd_df.drop(columns = ['B', 'C']).show()
    
Output  

    index   A       D
    0       0       3
    1       4       7
    2       8       11

For example,  
    
    # Drop a row by index
    fd_df.drop(labels = [0, 1]).show()
    
Output  

    index   A       B       C       D
    2       8       9       10      11

__Return Value__  
1. It returns a new of Frovedis DataFrame having remaining entries.  
2. It returns None when parameter 'inplace' = True.  

### 10. DataFrame.drop_duplicates(subset = None, keep = 'first', inplace = False, ignore_index = False)

__Parameters__  
**_subset_**:  It accepts a string object or a list of strings which only consider certain columns for identifying duplicates. (Default: None)  
When it is None (not specified explicitly), it will consider all of the columns.  
**_keep_**: It accepts a string object which is used to determine which duplicates values to keep. (Default: 'first')  
1. **'first'** : Drop duplicates except for the first occurrence.  
2. **'last'** : Drop duplicates except for the last occurrence.  

**_inplace_**: It accepts a boolean as a parameter. It return a copy of DataFrame instance by default but when explicitly set to True, it performs operation on original DataFrame. (Default: False)  
**_ignore\_index_**: It accepts a boolean type parameter. If True, old index axis is ignored and a new index axis is added with values 0 to n - 1, where n is the number of rows in the DataFrame. (Default: False)  

__Purpose__  
It is used to remove duplicate rows.  

For example,  
    
    import pandas as pd
    import frovedis.dataframe as fdf
    
    # Consider dataset containing ramen rating.
    pd_df = pd.DataFrame({
            'brand': ['Yum Yum', 'Yum Yum', 'Indomie', 'Indomie', 'Indomie'],
            'style': ['cup', 'cup', 'cup', 'pack', 'pack'],
            'rating': [4, 4, 3.5, 15, 5]})
    
    # creating a frovedis dataframe
    fd_df = fdf.DataFrame(pd_df)
    
    # display frovedis dataframe
    fd_df.show()

Output  

    index   brand   style   rating
    0       Yum Yum cup     4
    1       Yum Yum cup     4
    2       Indomie cup     3.5
    3       Indomie pack    15
    4       Indomie pack    5

For example,  

    # drop_duplicates() demo
    fd_df.drop_duplicates()
    
Output  

    index   brand   style   rating
    0       Yum Yum cup     4
    2       Indomie cup     3.5
    3       Indomie pack    15
    4       Indomie pack    5

By default, it removes duplicate rows based on all columns

For example,  
    
    # drop_duplicates() demo with subset parameter
    fd_df.drop_duplicates(subset = ['brand']).show()

Output  
    
    index   brand   style   rating
    0       Yum Yum cup     4
    2       Indomie cup     3.5

For example,  
    
    # drop_duplicates() demo with parameters: subset and keep = 'last'    
    fd_df.drop_duplicates(subset = ['brand', 'style'], keep = 'last').show()
    
Output  

    index   brand   style   rating
    1       Yum Yum cup     4
    2       Indomie cup     3.5
    4       Indomie pack    5

__Return Value__  
1. It returns a new of Frovedis DataFrame having remaining entries.  
2. It returns None when parameter 'inplace' = True.  

### 11. DataFrame.dropna(axis = 0, how = 'any', thresh = None, subset = None, inplace = False)  

__Parameters__  
**_axis_**: It accepts an integer value that can be 0 or 1. This parameter is used to determine whether rows or columns containing missing values are to be removed. (Default: 0)  
- 0 : Drop rows which contain missing values.  
- 1 : Drop columns which contain missing values.  

**_how_**: It accepts a string object to determine if row or column is removed from DataFrame, when we have at least one 'NaN' or all 'NaN'. (Default: 'any')  
- 'any' : If any NaN values are present, drop that row or column.  
- 'all' : If all values are NaN, drop that row or column.  

**_thresh_**: It accepts an integer as parameter which is the number of NaN values required for rows/columns to be 
dropped. (Default: None)  
**_subset_**: It accepts a python ndarray. It is the name of the labels along the other axis which is being considered.  
For example, if you are dropping rows, then these would be a list of columns. (Default: None)  
**_inplace_**: This parameter accepts a boolean value. When it is set to True, then it performs operation on the original Frovedis DataFrame object itself otherwise operation is performed on a new Frovedis DataFrame object. (Default: False)  

__Purpose__  
It is used to remove missing values from the Frovedis DataFrame.  

For example,  
    
    import pandas as pd
    import frovedis.dataframe as fdf
    
    # creating a pandas dataframe
    pd_df = pd.DataFrame({"name": ['Alfred', 'Batman', 'Catwoman'],
					   "toy": [np.nan, 'Batmobile', 'Bullwhip'],
					   "born": [np.nan, "1940-04-25", np.nan]})

    # creating a frovedis dataframe from pandas dataframe
    fd_df = fdf.DataFrame(pd_df)

    # display frovedis dataframe
    print("Before dropping nan values")
    fd_df.show()

Output  

    Before dropping nan values
    index   name            toy             born
    0       Alfred          NULL            NULL
    1       Batman          Batmobile       1940-04-25
    2       Catwoman        Bullwhip        NULL

For example,  

    # drop the rows where at least one element is missing.
    print("After dropping nan values")
    fd_df.dropna().show()
    
Output  

    After dropping nan values
    index   name    toy             born
    1       Batman  Batmobile       1940-04-25

For example,  

    # display frovedis dataframe
    print("Before dropping nan values")
    fd_df.show()
    
    # drop the columns where at least one nan value is present
    print("After dropping nan values")
    fd_df.dropna(axis=1)
    
Output  

    Before dropping nan values
    index   name            toy             born
    0       Alfred          NULL            NULL
    1       Batman          Batmobile       1940-04-25
    2       Catwoman        Bullwhip        NULL
    
    After dropping nan values
    index   name
    0       Alfred
    1       Batman
    2       Catwoman

For example,  

    # display frovedis dataframe
    print("Before dropping nan values")
    fd_df.show()
    
    # drop the rows where all elements are missing (how='all')
    fd_df.dropna(how='all').show()
    
Output  

    Before dropping nan values
    index   name            toy             born
    0       NULL            NULL            NULL
    1       Batman          Batmobile       1940-04-25
    2       Catwoman        Bullwhip        NULL

    index   name            toy             born
    1       Batman          Batmobile       1940-04-25
    2       Catwoman        Bullwhip        NULL

For example,  
        
    # display frovedis dataframe
    print("Before dropping nan values")
    fd_df.show()

    # example of using drop with parameter 'thresh'
    # dropna() demo with only rows with at least 2 non-NA values (thresh = 2) to keep
    fd_df.dropna(thresh=2).show()

Output  

    Before dropping nan values
    index   name            toy             born
    0       Alfred          NULL            NULL
    1       Batman          Batmobile       1940-04-25
    2       Catwoman        Bullwhip        NULL

    index   name            toy             born
    1       Batman          Batmobile       1940-04-25
    2       Catwoman        Bullwhip        NULL

**NOTE: Since row 0 had 2 NaN values, so it was dropped, but other rows are not dropped.**
    
For example,  
        
    # display frovedis dataframe
    print("Before dropping nan values")
    fd_df.show()

    # to drop missing values only from specified columns
    fd_df.dropna(subset=[1, 2], axis = 1).show()
    
Output  
    
    Before dropping nan values
    index   name            toy             born
    0       Alfred          NULL            NULL
    1       Batman          Batmobile       1940-04-25
    2       Catwoman        Bullwhip        NULL

    index   name            toy
    0       Alfred          NULL
    1       Batman          Batmobile
    2       Catwoman        Bullwhip

For example,  

    # display frovedis dataframe
    print("Before dropping nan values")
    fd_df.show()

    # dropna() demo with inplace = True
    fd_df.dropna(inplace=True)
    fd_df.show()
    
Output  

    Before dropping nan values
    index   name            toy     born
    0       Alfred          NULL    NULL
    1       Batman          Batmobile       1940-04-25
    2       Catwoman        Bullwhip        NULL

    index   name    toy             born
    1       Batman  Batmobile       1940-04-25

__Return Value__  
1. If inplace = False, it returns a new Frovedis DataFrame with NA entries dropped.  
2. If inplace = True, it returns None.  

### 12. DataFrame.fillna(value = None, method = None, axis = None, inplace = False, limit = None, downcast = None)  
 
__Parameters__  
**_value_**: It accepts a numeric type as parameter which is used to replace all NA values (e.g. 0, NULL). (Default: None)  
When it is None (not specified explicitly), it must be NaN value or numeric, non-numeric value otherwise it will raise an exception.  
**_method_**: This is an unused parameter. (Default: None)  
**_axis_**: It accepts an integer or string object as parameter. It decides the axis along which missing values will be filled. (Default: None)  
- 0 or 'index': Operation will be carried out on rows. Currently only axis = 0 is supported in Frovedis DataFrame.   
**_inplace_**: This parameter accepts a boolean value. When it is set to True, then it performs operation on the original Frovedis DataFrame object itself otherwise operation is performed on a new Frovedis DataFrame object. (Default: False)  
**_limit_**: This is an unused parameter. (Default: None)  
**_downcast_**: This is an unused parameter. (Default: None)  

__Purpose__  
It replaces NA/NaN values with the specified value provided in 'value' parameter in the Frovedis DataFrame.  

The parameters: "method", "limit" and "downcast" are simply kept in to make the interface uniform to the pandas DataFrame.fillna().  
This is not used anywhere within the frovedis implementation.  

For example,  
    
    import pandas as pd
    import frovedis.dataframe as fdf

    # creating a pandas dataframe
    pd_df = pd.DataFrame([[np.nan, 2, np.nan, 0],
					   [3, 4, np.nan, 1],
					   [np.nan, np.nan, np.nan, 5],
					   [np.nan, 3, np.nan, 4]],
					  columns=list("ABCD"))
    
    # creating a frovedis dataframe
    fd_df = fdf.DataFrame(pd_df)

    # display frovedis dataframe
    fd_df.show()

Output  

    index   A       B       C       D
    0       NULL    2       NULL    0
    1       3       4       NULL    1
    2       NULL    NULL    NULL    5
    3       NULL    3       NULL    4

For example,  

    # fillna() demo where NaN will be repaced with 0
    fd_df.fillna(0).show()
    
Output  

    index   A       B       C       D
    0       0       2       0       0
    1       3       4       0       1
    2       0       0       0       5
    3       0       3       0       4

For example,  

    # replace all NaN elements with -1s and inplace = True
    fd_df.fillna(-1, inplace = True)
    
    # display after modifying the original object
    fd_df.show()
    
Output  

    index   A       B       C       D
    0       NULL    2       NULL    0
    1       3       4       NULL    1
    2       NULL    NULL    NULL    5
    3       NULL    3       NULL    4

    index   A       B       C       D
    0       -1      2       -1      0
    1       3       4       -1      1
    2       -1      -1      -1      5
    3       -1      3       -1      4

__Return Value__  
1. It returns a Frovedis DataFrame object with missing values replaced when 'inplace' parameter is False.  
2. It returns None when 'inplace' parameter is set to True.  

### 13. DataFrame.filter(items = None, like = None, regex = None, axis = None)

__Parameters__  
**_items_**: It accepts a list of string as parameter. It filters only those labels which are mentioned. (Default: None)  
When it is None (not specified explicitly), 'like' or 'regex' must be provided.  
**_like_**: It accepts a string object parameter. It keeps the column labels if 'like in label == True'. (Default: None)   
When it is None (not specified explicitly), 'items' or 'regex' must be provided.  
**_regex_**: It accepts a regular expression as a string parameter. It keeps the column labels if 're.search(regex, label) == True'. (Default: None)  
When it is None (not specified explicitly), 'items' or 'like' must be provided.  
**_axis_**: It accepts an integer or string type parameter. It specifies the axis on which filter operation will be performed.  (Default: None)  
- 0 or 'index' : The corresponding index labels will be filtered. Currently this is not supported in Frovedis.  
- 1 or 'columns': The corresponding columns labels will be filtered.  

When it is None (not specified explicitly), it will filter along axis = 1 or 'columns'.  

__Purpose__  
It is used to filter the DataFrame according to the specified column labels.  

For example,  
    
    import pandas as pd
    import frovedis.dataframe as fdf
    
    pd_df = pd.DataFrame(np.array(([1, 2, 3], [4, 5, 6])),
                      index=['mouse', 'rabbit'],
                      columns=['one', 'two', 'three'])
    fd_df = fdf.DataFrame(pd_df)
    
    # display the frovedis dataframe
    fd_df.show()

Output  

    index   one     two     three
    mouse   1       2       3
    rabbit  4       5       6

For example,  

    # filter(0 demo with items parameter
    # select columns by name
    fd_df.filter(items = ['one', 'three']).show()
    
Output  

    index   one     three
    mouse   1       3
    rabbit  4       6

For example,  
    
    # select columns by regular expression
    # will display only those columns whose label ends with 'e'
    fd_df.filter(regex='e$', axis=1).show()
    
Output  
    
    index   one     three
    mouse   1       3
    rabbit  4       6

For example,  

    # select column containing 'hre'
    fd_df.filter(like='hre', axis=1)
    
Output  
    
    index   three
    mouse   3
    rabbit  6

__Return Value__  
It returns a new Frovedis DataFrame instance with the column labels that matches the given conditions.  

### 14. DataFrame.get_index_loc(value)  

__Parameters__  
**_value_**: It accepts an integer or string parameter. It is the index value whose location is to be determined.  

__Purpose__  
It provides integer location, slice or boolean mask for requested label.  

**It is present only in frovedis**.  

For example,  
    
    import pandas as pd
    import frovedis.dataframe as fdf
    
    peopleDF = {
                'Ename' : ['Michael', 'Andy', 'Tanaka', 'Raul', 'Yuta'],
                'Age' : [29, 30, 27, 19, 31],
                'Country' : ['USA', 'England', 'Japan', 'France', 'Japan'],
                'isMale': [False, False, False, False, True]
               }
    pd_df = pd.DataFrame(peopleDF)
    pd_df.index = ['a', 'b', 'c', 'd', 'e']
    fd_df = fdf.DataFrame(pd_df)
    
    # display frovedis dataframe
    fd_df.show()

Output  

    index   Ename   Age     Country isMale
    a       Michael 29      USA     0
    b       Andy    30      England 0
    c       Tanaka  27      Japan   0
    d       Raul    19      France  0
    e       Yuta    31      Japan   1

For example,  

    # get_index_loc() demo when there are unique index
    # getting index location of 'd' index value
    fd_df.get_index_loc('d').show()
    
Output  

    3

For example,  
    
    import pandas as pd
    import frovedis.dataframe as fdf
    
    peopleDF = {
                'Ename' : ['Michael', 'Andy', 'Tanaka', 'Raul', 'Yuta'],
                'Age' : [29, 30, 27, 19, 31],
                'Country' : ['USA', 'England', 'Japan', 'France', 'Japan'],
                'isMale': [False, False, False, False, True]
               }
    # creating a pandas dataframe
    pd_df = pd.DataFrame(peopleDF)
    pd_df.index = ['a', 'a', 'd', 'd', 'e']
    
    # creating a frovedis dataframe
    fd_df = fdf.DataFrame(pd_df)
    
    # display a frovedis dataframe
    fd_df.show()

Output  

    index   Ename   Age     Country isMale
    a       Michael 29      USA     0
    a       Andy    30      England 0
    d       Tanaka  27      Japan   0
    d       Raul    19      France  0
    e       Yuta    31      Japan   1

For example,  

    # get_index_loc(0 demo when there are duplicate index but consecutively
    # getting index location of 'd' index value
    fd_df.get_index_loc('d').show()
    
Output  

    slice(2, 4, None)
    
For example,  
    
    import pandas as pd
    import frovedis.dataframe as fdf
    
    peopleDF = {
                'Ename' : ['Michael', 'Andy', 'Tanaka', 'Raul', 'Yuta'],
                'Age' : [29, 30, 27, 19, 31],
                'Country' : ['USA', 'England', 'Japan', 'France', 'Japan'],
                'isMale': [False, False, False, False, True]
               }
    # creating a pandas dataframe
    pd_df = pd.DataFrame(peopleDF)   
    pd_df.index = ['a', 'b', 'c', 'd', 'a']
    
    # creating a frovedis dataframe
    fd_df = fdf.DataFrame(pd_df)

    # display a frovedis dataframe
    fd_df.show()

Output

    index   Ename   Age     Country isMale
    a       Michael 29      USA     0
    b       Andy    30      England 0
    c       Tanaka  27      Japan   0
    d       Raul    19      France  0
    a       Yuta    31      Japan   1

For example,  

    # get_index_loc() demo when there are duplicate index but randomly
    # getting index location of 'a' index value
    fd_df.get_index_loc('a').show()
    
Output  

    [ True False False False  True]

__Return Value__  
It returns the following values:  
1. **integer:** when there is a unique index.  
2. **slice:** when there is a monotonic index i.e. repetitive values in index.  
3. **mask:** it returns a list of boolean values.  

### 15. DataFrame.head(n = 5)

__Parameters__  
**_n_**: It accepts an integer parameter which represents the number of rows to select. (Default: 5)  

__Purpose__   
This function returns the first n rows for the object based on position. It is useful for quickly testing if your object has the right type of data in it.  

**For negative values of n, this function returns all rows except the last n rows, equivalent to df[:-n]**.  

For example,  
    
    import pandas as pd
    import frovedis.dataframe as fdf
    
    # creating the dataframe    
    pd_df = pd.DataFrame({'animal': ['alligator', 'bee', 'falcon', 'lion',
                          'monkey', 'parrot', 'shark', 'whale', 'zebra']
                         })
    
    #creating frovedis dataframe
    fd_df = fdf.DataFrame(pd_df)
    
    # display frovedis dataframe
    fd_df.show()

Output  

    index   animal
    0       alligator
    1       bee
    2       falcon
    3       lion
    4       monkey
    5       parrot
    6       shark
    7       whale
    8       zebra

For example,  

    # head() demo with default n value
    fd_df.head().show()
    
Output  

    index   animal
    0       alligator
    1       bee
    2       falcon
    3       lion
    4       monkey 

For example,  
     
    # head() demo with n = 2
    fd_df.head(2).show()
    
Output  

    index   animal
    0       alligator
    1       bee

For example,   
     
    # head() demo with n = -3
    fd_df.head(-3).show()
    
Output  

    index   animal
    0       alligator
    1       bee
    2       falcon
    3       lion
    4       monkey
    5       parrot

__Return Value__  
1. It n is positive integer, it returns a new DataFrame with the first n rows.  
2. If n is negative integer, it returns a new DataFrame with all rows except last n rows.  

### 16. DataFrame.insert(loc, column, value, allow_duplicates = False)

__Parameters__  
**_loc_**: It accepts an integer as parameter which represents the Insertion index. It must be in range **(0, n - 1)** where **n** is number of columns in dataframe.  
**_column_**: It accepts a string object as parameter. It is the label of the inserted column.  
**_value_**: It accepts an integer or a pandas Series instance or python ndarray as parameter. These are the values to be inserted in the specified 'column'.  
**_allow\_duplicates_**: It accepts a boolean value as parameter. Currently, frovedis does not support duplicate column names. (Default: False)  

__Purpose__  
It is used to insert column into DataFrame at specified location.  

For example,  
    
    import pandas as pd
    import frovedis.dataframe as fdf
    
    pd_df = pd.DataFrame({'col1': [1, 2], 'col2': [3, 4]})
    fd_df = fdf.DataFrame(pd_df)
    fd_df.show()
    
Output  

    index   col1    col2
    0       1       3
    1       2       4

For example,  

    # inserting "newcol" as position 1 with values [99, 99]
    fd_df.insert(1, "newcol", [99, 99])
    
    # display frovedis dataframe after insertion
    fd_df.show()

Output  

    index   col1    newcol  col2
    0       1       99      3
    1       2       99      4

For example,  

    # insert() demo with Series object having values [5,6] in "col0" at 0th position 
    fd_df.insert(0, "col0", pd.Series([5, 6])).show()
    
Output  

    index   col0    col1    col2
    0       5       1       3
    1       6       2       4

__Return Value__  
It returns a self reference.  

### 17. DataFrame.isna()  

__Purpose__  
This method is used to detect missing values in the frovedis dataframe.  

It returns a boolean same-sized object indicating if the values are NA. NA values, such as None or numpy.NaN, gets mapped to True values. Everything else gets mapped to False values. Characters such as empty strings '' or numpy.inf are not considered NA values.  

For example,  
    
    import pandas as pd
    import frovedis.dataframe as fdf
    
    # show which entries in a DataFrame are NA.
    peopleDF = {
            'Ename' : ['Michael', None, 'Tanaka', 'Raul', ''],
            'Age' : [29, 30, 27, 19, 0],
            'Country' : ['USA', np.inf, 'Japan', np.nan, 'Japan'],
            'isMale': [False, False, False, False, True]
           }

    # creating a pandas dataframe
    pd_df = pd.DataFrame(peopleDF)
    
    # creating a frovedis dataframe
    fd_df = fdf.DataFrame(pd_df)
    
    # to display frovedis dataframe
    fd_df.show()

Output  

    index   Ename   Age     Country isMale
    0       Michael 29      USA     0
    1       NULL    30      inf     0
    2       Tanaka  27      Japan   0
    3       Raul    19      NULL    0
    4               0       Japan   1

For example,  

    # isna() demo to display fields which are only NA    
    fd_df.isna().show()
    
Output  

    index   Ename   Age     Country isMale
    0       0       0       0       0
    1       1       0       0       0
    2       0       0       0       0
    3       0       0       1       0
    4       0       0       0       0

For example,  
    
    import pandas as pd
    import frovedis.dataframe as fdf
    
    # frovedis dataframe from a Series object.
    ser = pd.Series([5, 6, np.NaN])
    
    # creating a frovedis dataframe
    fd_df = fdf.DataFrame(ser)
    
    # display frovedis dataframe
    fd_df.show()

Output  

    index   0
    0       5
    1       6
    2       NULL

For example,  

    # isna() demo to display na values mapped to corresponding dataframe
    fd_df.isna().show()
    
Output  

    index   0
    0       0
    1       0
    2       1

__Return Value__  
It returns a new Frovedis DataFrame having all boolean values (0, 1) corresponding to each of the Frovedis DataFrame values depending on  whether it is a valid NaN (True i.e. 1) value or not (False i.e. 0).  

### 18. DataFrame.isnull()  

__Purpose__  
This method is used to detect missing values in the frovedis dataframe. It is an alias of isna().  

It returns a boolean same-sized object indicating if the values are NA. NA values, such as None or numpy.NaN, gets mapped to True values. Everything else gets mapped to False values. Characters such as empty strings '' or numpy.inf are not considered NA values.  

For example,  
    
    import pandas as pd
    import frovedis.dataframe as fdf
    
    # show which entries in a DataFrame are NA.
    peopleDF = {
				'Ename' : ['Michael', None, 'Tanaka', 'Raul', ''],
				'Age' : [29, 30, 27, 19, 0],
				'Country' : ['USA', 'England', 'Japan', np.nan, 'Japan'],
				'isMale': [False, False, False, False, True]
			   }
               
    # creating a pandas dataframe
    pd_df = pd.DataFrame(peopleDF)
    
    # convert to frovedis dataframe
    fd_df = fdf.DataFrame(pd_df)

    # display frovedis dataframe
    fd_df.show()

Output  

    index   Ename   Age     Country isMale
    0       Michael 29      USA     0
    1       NULL    30      England 0
    2       Tanaka  27      Japan   0
    3       Raul    19      NULL    0
    4               0       Japan   1

For example,  

    # isnull() demo
    fd_df.isnull().show()
    
Output  

    index   Ename   Age     Country isMale
    0       0       0       0       0
    1       1       0       0       0
    2       0       0       0       0
    3       0       0       1       0
    4       0       0       0       0

For example,  
    
    import pandas as pd
    import frovedis.dataframe as fdf
    
    # frovedis dataframe from a Series object.
    ser = pd.Series([5, 6, np.NaN])

    # creating a frovedis dataframe
    fd_df = fdf.DataFrame(ser)
    
    # display frovedis dataframe
    fd_df.show()

Output  

    index   0
    0       5
    1       6
    2       NULL

For example,  

    # isnull() demo on a frovedis dataframe converted from Series object
    fd_df.isnull().show()
    
Output  

    index   0
    0       0
    1       0
    2       1

__Return Value__  
It returns a Frovedis DataFrame having boolean values (0, 1) corresponding to each of the Frovedis DataFrame value depending of whether it is a valid NaN (True i.e. 1) value or not (False i.e. 0).  

### 19. DataFrame.join(right, on, how = 'inner', lsuffix = '\_left', rsuffix = '\_right', sort = False, join_type = 'bcast')  

__Parameters__  
**_right_**: It accepts a Frovedis DataFrame instance or a pandas DataFrame instance or a list of DataFrame instances as parameter. Index should be similar to one of the columns in this one. If a pandas Series instance is passed, its name attribute must be set, and that will be used as the column name in the resulting joined dataframe.  
**_on_**: It accepts a string object or a list of strings as parameter. It is the column or index name(s) in the caller to join on the index in other, otherwise joins index-on-index. **This parameter must be provided. It can not be None.**  
**_how_**: It accepts a string object as parameter that specifies how to handle the operation of the two dataframes. (Default: 'inner')  
1. **'left'**: form union of calling dataframe’s index (or column if 'on' is specified) with other dataframe’s index, and sort it lexicographically.  
2. **'inner'**: form intersection of calling dataframe’s index (or column if 'on' is specified) with other dataframe’s index, preserving the order of the calling’s one.  

**_lsuffix_**: It accepts a string object as parameter. It adds the suffix to left DataFrame’s overlapping columns. (Default: '\_left')  
**_rsuffix_**: It accepts a string object as parameter. It adds the suffix to right DataFrame’s overlapping columns. (Default: '\_right')  
**_sort_**: It accepts a boolean type value. It orders resultant dataframe lexicographically by the join key. If False, the order of the join key depends on the join type ('how' keyword). (Default: False)  
**_join\_type_**: It accepts a string type object as parameter. It represents the type of join to be used internally. It can be specified as 'bcast' for broadcast join or 'hash' for hash join. (Default: 'bcast')  

__Purpose__  
It joins columns of another dataframe.  

It joins columns with other dataframe either on index or on a key column. Efficiently join multiple DataFrame instances by index at once by passing a list.  

**Note:- Parameters 'on', 'lsuffix', and 'rsuffix' are not supported when passing a list of DataFrame instances.**  

For example,  
    
    import pandas as pd
    import frovedis.dataframe as fdf
    
    # creating a pandas dataframe
    pd_df1 = pd.DataFrame({'key': ['K0', 'K1', 'K2', 'K3', 'K4', 'K5'],
                           'A': ['A0', 'A1', 'A2', 'A3', 'A4', 'A5']})

    # creating a frovedis dataframe
    fd_df1 = fdf.DataFrame(pd_df1)

    # display frovedis dataframe
    fd_df1.show()
    
    # creating another pandas dataframe
    pd_df2 = pd.DataFrame({'key': ['K0', 'K1', 'K2'],
                          'B': ['B0', 'B1', 'B2']})

    # creating another frovedis dataframe
    fd_df2 = fdf.DataFrame(pd_df2)

    # display other frovedis dataframe
    fd_df2.show()

Output  

    index   key     A
    0       K0      A0
    1       K1      A1
    2       K2      A2
    3       K3      A3
    4       K4      A4
    5       K5      A5

    index   key     B
    0       K0      B0
    1       K1      B1
    2       K2      B2

For example,  

    # join() demo with lsuffix, rsuffix and index parameters
    fd_df1.join(fd_df2, 'index', lsuffix = '_caller', rsuffix = '_other').show()
    
Output  

    index   key_caller      A       key_other       B
    0       K0              A0      K0              B0
    1       K1              A1      K1              B1
    2       K2              A2      K2              B2

For example,  

    # join() demo using the 'key' columns,
    # 'key' to be available in both fd_df1 and fd_df2
    fd_df1.join(fd_df2, on = 'key').show()
    
Output  

    index   key     A       B
    0       K0      A0      B0
    1       K1      A1      B1
    2       K2      A2      B2
    
For example,   

    # join(0 demo with parameter how = 'left'
    fd_df1.join(fd_df2, 'key', how = 'left').show()
    
Output,

    index   key     A       B
    0       K0      A0      B0
    1       K1      A1      B1
    2       K2      A2      B2
    3       K3      A3      NULL
    4       K4      A4      NULL
    5       K5      A5      NULL

__Return Value__  
It returns a new Frovedis DataFrame containing columns from both the DataFrame instances.  

### 20. DataFrame.merge(right, on = None, how = 'inner', left_on = None, right_on = None, left_index = False, right_index = False, sort = False, suffixes = ('\_x', '\_y'), copy = True, indicator = False, join_type = 'bcast')  

__Parameters__  
**_right_**: It accepts a Frovedis DataFrame instance or a pandas DataFrame instance or a list of Frovedis DataFrame instances as parameter. Index should be similar to one of the columns in this one. If a panads Series instance is passed, its name attribute must be set, and that will be used as the column name in the resulting joined dataframe.  
**_on_**: It accepts a string object or a list of strings as parameter. It is the column or index level names to join on. These must be present in both dataframes. (Default: None)  
When it is None and not merging on indexes then this defaults to the intersection of the columns in both dataframes.  
**_how_**: It accepts a string object as parameter. It informs the type of merge operation on the two objects. (Default: 'inner')  
1. **'left'**: form union of calling dataframe’s index (or column if 'on' is specified) with other’s index and sort it lexicographically.  
2. **'inner'**: form intersection of calling dataframe’s index (or column if 'on' is specified) with other’s index, preserving the order of the  calling’s one.  

**_left\_on_**: It accepts a string object or a list of strings as parameter. It represents column names to join on in the left dataframe. It can also be an array or list of arrays of the length of the left dataframe. These arrays are treated as if they are columns. (Default: None)  
**_right\_on_**: It accepts a string object or a list of strings as parameter. It represents column names to join on in the right dataframe. It can also be an array or list of arrays of the length of the right dataframe. These arrays are treated as if they are columns. (Default: None)  
**_left\_index_**: It accepts a boolean value as parameter. It is used to specify whether to use the index from the left dataframe as the join key. **Either parameter 'left_on' or 'left_index' can be used, but not combination of both.** (Default: False)  
**_right\_index_**: It accepts a boolean value as parameter. It is used to specify whether to use the index from the right dataframe as the join key. **Either parameter 'right_on' or 'right_index' can be used, but not combination of both.** (Default: False)  
**_sort_**: It accepts a boolean value. When this is explicitly set to True, it sorts the join keys lexicographically in the resultant dataframe. When it is False, the order of the join keys depends on the join type ('how' parameter). (Default: False)  
**_suffixes_**: It accepts a list like (list or tuple) object of strings of length two as parameter. It indicates the suffix to be added to the overlapping column names in left and right respectively. Need to explicitly pass a value of None instead of a string to indicate that the column name from left or right should be left as-it is, with no suffix. At least one of the values must not be None. (Default: (“\_x”, “\_y”))  

**Note:- During merging two DataFrames, the overlapping column names should be different. For example: suffixes = (False, False), then the overlapping columns would have the same name so merging operation will fail. Also when there is no overlapping column, then this parameter is ignored automatically.**  

**_copy_**: It is an unused parameter. (Default: True)  
**_indicator_**: It is an unused parameter. (Default: False)  
**_join\_type_**: It accepts a string type object as parameter. It represents the type of join to be used internally. It can be specified as 'bcast' for broadcast join or 'hash' for hash join. (Default: 'bcast')  

__Purpose__  
It is a utility to merge dataframe objects with a database-style join.  

The join is done on columns or indexes. If joining columns on columns, the DataFrame indexes will be ignored. Otherwise if joining indexes on indexes or indexes on a column or columns, the index will be passed on.  

For example,  
    
    import pandas as pd
    import frovedis.dataframe as fdf
    
    # creating a pandas dataframe
    pd_df1 = pd.DataFrame({'lkey': ['foo', 'bar', 'baz', 'foo'],
                        'value': [1, 2, 3, 5]})
    
    # creating another pandas dataframe
    pd_df2 = pd.DataFrame({'rkey': ['foo', 'bar', 'baz', 'foo'],
                        'value': [5, 6, 7, 8]})
    
    # creating a frovedis dataframe
    fd_df1 = fdf.DataFrame(pd_df1)

    # display frovedis dataframe
    fd_df1.show()
    
    # creating another frovedis dataframe
    fd_df2 = fdf.DataFrame(pd_df2)

    # display other frovedis dataframe
    fd_df2.show()

Output  

    index   lkey    value
    0       foo     1
    1       bar     2
    2       baz     3
    3       foo     5

    index   rkey    value
    0       foo     5
    1       bar     6
    2       baz     7
    3       foo     8

For example,  

    # merge() demo with parameters left_on and right_on
    fd_df1.merge(fd_df2, left_on = 'lkey', right_on = 'rkey').show()

Output  

    index   lkey    value_x rkey    value_y
    0       foo     1       foo     8
    1       foo     1       foo     5
    2       bar     2       bar     6
    3       baz     3       baz     7
    4       foo     5       foo     8
    5       foo     5       foo     5

For example,  

    # merge() demo with parameters left_on , right_on and suffixes
    fd_df1.merge(fd_df2, left_on = 'lkey', right_on = 'rkey', suffixes = ('_left', '_right')).show()
    
Output  

    index   lkey    value_left      rkey    value_right
    0       foo     1               foo     8
    1       foo     1               foo     5
    2       bar     2               bar     6
    3       baz     3               baz     7
    4       foo     5               foo     8
    5       foo     5               foo     5
    
For example,  
    
    import pandas as pd
    import frovedis.dataframe as fdf
    
    # creating a pandas dataframe
    pd_df1 = pd.DataFrame({'lkey': ['foo', 'bar', 'baz', 'foo'],
                        'left_value': [1, 2, 3, 5]})
    
    # creating another pandas dataframe
    pd_df2 = pd.DataFrame({'rkey': ['foo', 'bar', 'baz', 'foo'],
                        'right_value': [5, 6, 7, 8]})
    
    # creating a frovedis dataframe
    fd_df1 = fdf.DataFrame(pd_df1)
    
    # display a frovedis dataframe
    fd_df1.show()
    
    # creating another frovedis dataframe
    fd_df2 = fdf.DataFrame(pd_df2)

    # display other frovedis dataframe
    fd_df2.show()     

Output  

    index   lkey    left_value
    0       foo     1
    1       bar     2
    2       baz     3
    3       foo     5

    index   rkey    right_value
    0       foo     5
    1       bar     6
    2       baz     7
    3       foo     8

For example,  

    # merge(0 demo with same suffixes on 'lkey' and 'rkey'
    fd_df1.merge(fd_df2, left_on = 'lkey', right_on = 'rkey', suffixes = ('_test', '_test')).show()
    
Output  

    index   lkey    left_value      rkey    right_value
    0       foo     1               foo     8
    1       foo     1               foo     5
    2       bar     2               bar     6
    3       baz     3               baz     7
    4       foo     5               foo     8
    5       foo     5               foo     5

**Note:-** In above example, suffix is ignored as merging column labels are different. To have suffix, the column name must be same in both dataframes but then the suffixes must be different.  

For example,  
    
    import pandas as pd
    import frovedis.dataframe as fdf
    
    # creating two pandas datfarames
    pd_df1 = pd.DataFrame({'lkey': ['foo', 'bar', 'baz', 'foo'],
                        'value': [1, 2, 3, 5]})
    pd_df2 = pd.DataFrame({'rkey': ['foo', 'bar', 'baz', 'foo'],
                                            'value': [5, 6, 7, 8]})
    
    # creating a frovedis dataframe
    fd_df1 = fdf.DataFrame(pd_df1)

    # update index values of fd_df1
    fd_df1.update_index(['a','b', 'c', 'd'], key = 'index', inplace = True)
    
    # display frovedis dataframe
    fd_df1.show()

    # creating another frovedis dataframe
    fd_df2 = fdf.DataFrame(pd_df2)

    # update index values of fd_df2
    fd_df2.update_index(['a','b', 'c', 'd'], key = 'index', inplace = True)
    
    # display frovedis dataframe
    fd_df2.show()

Output  

    index   lkey    value
    a       foo     1
    b       bar     2
    c       baz     3
    d       foo     5

    index   rkey    value
    a       foo     5
    b       bar     6
    c       baz     7
    d       foo     8

For example,  

    # merge(0 demo with left_index = True and right_index = True
    fd_df1.merge(fd_df2, left_index = True, right_index = True).show()
    
Output  

    index   lkey    value_x rkey    value_y
    a       foo     1       foo     5
    b       bar     2       bar     6
    c       baz     3       baz     7
    d       foo     5       foo     8

For example,  
    
    import pandas as pd
    import frovedis.dataframe as fdf
    
    # creating two pandas datframes
    pd_df1 = pd.DataFrame({'a': ['foo', 'bar'], 'b': [1, 2]})
    pd_df2 = pd.DataFrame({'a': ['foo', 'baz'], 'c': [3, 4]})

    # creating two frovedis datframes
    fd_df1 = fdf.DataFrame(pd_df1)
    fd_df2 = fdf.DataFrame(pd_df2)
    
    # display frovedis dataframes
    fd_df1.show()
    fd_df2.show()

Output  

    index   a       b
    0       foo     1
    1       bar     2

    index   a       c
    0       foo     3
    1       baz     4

For example,  

    # merge() demo with how = 'inner' and 'on' parameter
    fd_df1.merge(fd_df2, how = 'inner', on = 'a').show()
    
Output  

    index   a       b       c
    0       foo     1       3

For example,  

    # merge using 'how' = left and 'on' parameter
    fd_df1.merge(fd_df2, how = 'left', on = 'a').show()
    
Output,
    
    index   a       b       c
    0       foo     1       3
    1       bar     2       NULL

__Return Value__  
It returns a new Frovedis DataFrame instance with the merged entries of the two DataFrame instances.  

### 21. DataFrame.rename(columns, inplace = False)  

__Parameters__  
**_columns_**: It accepts a dictionary object as parameter. It contains the key as the name of the labels to be renamed and values as the final names.  
**_inplace._**: It accepts a boolean object as parameter which specify whether to modify the original DataFrame instance or to return a copy. When it is set to True then the original DataFrame instance is modified.  (Default: False)  

__Purpose__  
It is used to set the name of the columns.  

For example,  
    
    import pandas as pd
    import frovedis.dataframe as fdf
    
    # creating a pandas dataframe
    pd_df = pd.DataFrame({"A": [1, 2, 3], "B": [4, 5, 6]})

    # creating a frovedis dataframe
    fd_df = fdf.DataFrame(pd_df)
    
    # display frovedis dataframe
    fd_df.show()

Output  

    index   A       B
    0       1       4
    1       2       5
    2       3       6

For example,  

    # rename columns 'A' and 'B' to 'a' and 'c' respectively
    fd_df.rename(columns={"A": "a", "B": "c"}).show()
    
Output  

    index   a       c
    0       1       4
    1       2       5
    2       3       6

For example,  
    
    # rename() demo with implace = True
    fd_df.rename(columns={"A": "X", "B": "Y"}, inplace = True)
    
Output,

    index   X       Y
    0       1       4
    1       2       5
    2       3       6

__Return Value__  
1. It returns a new Frovedis DataFrame with the updated label name for the specified columns.  
2. It returns None when 'inplace' parameter is set to True.  


### 22. DataFrame.rename_index(new_name, inplace = False)  

__Parameters__  
**_new\_name_**: It accepts a string object as parameter. It renames the index for which the value is provided.  
**_inplace_**: It accepts a boolean values as parameter. When it is explicitly set to True, it modifies the original object directly instead of creating a copy of DataFrame instance. (Default: False)  

__Purpose__   
It is used to rename the index label.  

For example,  
    
    import pandas as pd
    import frovedis.dataframe as fdf
    
    peopleDF = {
                'Ename' : ['Michael', 'Andy', 'Tanaka', 'Raul', 'Yuta'],
                'Age' : [29, 30, 27, 19, 31],
                'Country' : ['USA', 'England', 'Japan', 'France', 'Japan'],
                'isMale': [False, False, False, False, True]
               }
    
    # creating a pandas dataframe
    pd_df = pd.DataFrame(peopleDF)
    
    # creating a frovedis dataframe
    fd_df = fdf.DataFrame(pd_df)
    
    # display a frovedis dataframe
    fd_df.show()

Output  

    index   Ename   Age     Country isMale
    0       Michael 29      USA     0
    1       Andy    30      England 0
    2       Tanaka  27      Japan   0
    3       Raul    19      France  0
    4       Yuta    31      Japan   1

For example,  

    # rename_index() demo where renaming index to "SNo"
    fd_df.rename_index("SNo").show()
    
Output  

    SNo     Ename   Age     Country isMale
    0       Michael 29      USA     0
    1       Andy    30      England 0
    2       Tanaka  27      Japan   0
    3       Raul    19      France  0
    4       Yuta    31      Japan   1

__Return Value__  
1. It returns a new Frovedis DataFrame with the updated label for the Index label.  
2. It returns None when inplace parameter is set to True.  

### 23. DataFrame.reset_index(drop = False, inplace = False)  

__Parameters__  
**_drop_**: It accepts a boolean value as parameter. Do not try to insert index into dataframe columns. This resets the index to the default integer index. (Default: False)  
**_inplace_**: It accepts a boolean values as parameter. When it is explicitly set to True, it modifies the original object directly instead of creating a copy of DataFrame object. (Default: False)  

__Purpose__   
It is used to reset the Index label of the DataFrame. A new Index label is inserted with default integer values from 0 to n-1 where n is the number of rows.  

**Note:- MultiIndex is not supported by Frovedis DataFrame.**  

For example,  
    
    import pandas as pd
    import frovedis.dataframe as fdf
    
    peopleDF = {
                'Ename' : ['Michael', 'Andy', 'Tanaka', 'Raul', 'Yuta'],
                'Age' : [29, 30, 27, 19, 31],
                'Country' : ['USA', 'England', 'Japan', 'France', 'Japan'],
                'isMale': [False, False, False, False, True]
               }

    # creating a pandas dataframe
    pd_df = pd.DataFrame(peopleDF)
   
    # updating the index values of pandas dataframe
    pd_df.index = ['a', 'b', 'c', 'd', 'a']
    
    # creating frovedis dataframe
    fd_df = fdf.DataFrame(pd_df)
    
    # display the frovedid dataframe
    fd_df.show()

Output  

    index   Ename   Age     Country isMale
    a       Michael 29      USA     0
    b       Andy    30      England 0
    c       Tanaka  27      Japan   0
    d       Raul    19      France  0
    a       Yuta    31      Japan   1

For example,  

    # reset_index() demo
    fd_df.reset_index().show
    
Output  

    label_0 index_col       Ename   Age     Country isMale
    0       a               Michael 29      USA     0
    1       b               Andy    30      England 0
    2       c               Tanaka  27      Japan   0
    3       d               Raul    19      France  0
    4       a               Yuta    31      Japan   1

When we reset the index, the old index is added as a column, and a new sequential index is used.  

For example,  
    
    # reset_index() demo with drop parameter 
    fd_df.reset_index(drop = True).show()
    
Output  

    index   Ename   Age     Country isMale
    0       Michael 29      USA     0
    1       Andy    30      England 0
    2       Tanaka  27      Japan   0
    3       Raul    19      France  0
    4       Yuta    31      Japan   1

We can use the drop parameter to avoid the old index being added as a column.  

__Return Value__   
1. It returns a new Frovedis DataFrame with the default sequence in Index label.  
2. It returns None if 'inplace' parameter is set to True.  
  
### 24. DataFrame.set_index(keys, drop = True, append = False, inplace = False, verify_integrity = False)  

__Parameters__  
**_keys_**: It accepts a string object as parameter. This parameter can be a single column key.  
**_drop_**: It accepts a boolean value as parameter. When it is set to True, it will remove the column which is selected as new 
index. **Currently, Frovedis doesn't support drop = False.** (Default: True)  
**_append_**: It accepts a boolean value as parameter. It will decide whether to append columns to existing index. **Currently, Frovedis doesn't support append = True.** (Default: False)  
**_inplace_**: It accepts a boolean values as parameter which is when explicitly set to True, it modifies the original object directly instead of creating a copy of DataFrame instance. (Default: False)  
**_verify\_integrity_**: It accepts a boolean value as parameter. When it is set to True, it checks the new index for duplicates. Performance of this method will be better when it is set to False. (Default: False)  

__Purpose__  
It is used to set the frovedis dataframe index using existing columns. The index will replace the existing index.  

**Note:- Frovedis DataFrame does not support Multi Index.**  

For example,  
    
    import pandas as pd
    import frovedis.dataframe as fdf
    
    # creating a pandas dataframe
    pd_df = pd.DataFrame({'month': [1, 4, 1, 10],
                       'year': [2012, 2014, 2013, 2014],
                       'sale': [55, 40, 84, 31]})
    
    # creating a frovedis dataframe
    fd_df = fdf.DataFrame(pd_df)
    
    # display frovedis dataframe
    fd_df.show()

Output  

    index   month   year    sale
    0       1       2012    55
    1       4       2014    40
    2       1       2013    84
    3       10      2014    31

For example,  

    # set_index() demo where ‘month’ column is set as index
    fd_df.set_index('month').show()
    
Output  

    month   year    sale
    1       2012    55
    4       2014    40
    1       2013    84
    10      2014    31
    
For example,  
    
    # set_index() demo with verify_integrity = True
    fd_df.set_index('month', verify_integrity = True).show()
    
Output  

    month   year    sale
    1       2012    55
    4       2014    40
    7       2013    84
    10      2014    31

**Note:-** In above example, the column which is being selected as index must have unique values.  

__Return Value__  
1. It returns a new Frovedis DataFrame where the Index column label is replace with specified column label.  
2. It returns None when 'inplace' parameter is set to True.  

### 25. DataFrame.show()  

__Purpose__  
This method is used to display the Frovedis DataFrame on the console. It can display full dataframe or some selected columns of the DataFrame (single or multi-column).  

It can be used either with any method which returns a Frovedis DataFrame instance type compatible with string type.  
  
For example,  
     
    import pandas as pd
    import frovedis.dataframe as fdf
    
    # creating the dataframe    
    peopleDF = {
            'Ename' : ['Michael', 'Andy', 'Tanaka', 'Raul', 'Yuta'], 
            'Age' : [29, 30, 27, 19, 31],
            'Country' : ['USA', 'England', 'Japan', 'France', 'Japan'],
            'isMale': [False, False, False, False, True]
           }
    
    pdf = pd.DataFrame(peopleDF)
    
    # creating a frovedis dataframe
    fd_df = fdf.DataFrame(pdf)

    # display frovedis dataframe
    print("Displaying complete frovedis dataframe")
    fd_df.show() 

Output  

    Displaying complete frovedis dataframe
    index   Ename   Age     Country isMale
    0       Michael 29      USA     0
    1       Andy    30      England 0
    2       Tanaka  27      Japan   0
    3       Raul    19      France  0
    4       Yuta    31      Japan   1

For example,  
 
    # Display dataframe by selecting single column
    print("Displaying frovedis dataframe with just Ename column")
    fd_df["Ename"].show()         # single column

Output  
    
    Displaying frovedis dataframe with just Ename column
    index   Ename
    0       Michael
    1       Andy
    2       Tanaka
    3       Raul
    4       Yuta 

For example,  

    # Selecting multiple columns
    print("Displaying frovedis dataframe with Ename  and Age columns")
    fd_df[["Ename","Age"]].show() # multiple column

Output  

    Displaying frovedis dataframe with Ename  and Age columns
    index   Ename   Age
    0       Michael 29
    1       Andy    30
    2       Tanaka  27
    3       Raul    19
    4       Yuta    31

For example,  

    # With slicing operator
    print("Displaying frovedis dataframe using condition based slicing operator")
    fd_df[fd_df.Age > 19].show()  

Output  

    Displaying frovedis dataframe using condition based slicing operator
    index   Ename   Age     Country isMale
    0       Michael 29      USA     0
    1       Andy    30      England 0
    2       Tanaka  27      Japan   0
    4       Yuta    31      Japan   1

For example,  

    # With slicing operator
    print("Displaying frovedis dataframe using chaining of methods")
    fd_df[fd_df.Country.str.contains("a")].show()
    
Output  

    Displaying frovedis dataframe using chaining of methods
    index   Ename   Age     Country isMale
    1       Andy    30      England 0
    2       Tanaka  27      Japan   0
    3       Raul    19      France  0
    4       Yuta    31      Japan   1

__Return Value__  
It return nothing.  

### 26. DataFrame.tail(n = 5)  

__Parameters__  
**_n_**: It accepts an integer as parameter. It represents the number of rows which is to be selected. (Default: 5)  

__Purpose__  
This utility is used to return the last **n** rows from the DataFrame.  

It is useful for quickly verifying data, for example, after sorting or appending rows.  

For negative values of n, this function returns all rows except the first n rows, equivalent to **fdf[n:]**.  

For example,  
    
    import pandas as pd
    import frovedis.dataframe as fdf
    
    # creating a pandas dataframe
    pd_df = pd.DataFrame({'animal': ['alligator', 'bee', 'falcon', 'lion',
                   'monkey', 'parrot', 'shark', 'whale', 'zebra']})

    # creating a frovedis dataframe
    fd_df = fdf.DataFrame(pd_df)
    
    print("Viewing all entries of dataframe")
    fd_df.show()

Output  

    Viewing all entries of dataframe
    index   animal
    0       alligator
    1       bee
    2       falcon
    3       lion
    4       monkey
    5       parrot
    6       shark
    7       whale
    8       zebra

For example,  

    # tail() demo
    print("Viewing the last 5 lines")
    fd_df.tail().show()
    
Output  

    Viewing the last 5 lines
    index   animal
    4       monkey
    5       parrot
    6       shark
    7       whale
    8       zebra    
    
For example,  
   
    # tail(0 demo with n = 3
    print("Viewing the last 3 lines")
    fd_df.tail(3).show()
    
Output  

    Viewing the last 3 lines
    index   animal
    6       shark
    7       whale
    8       zebra
    
For example,  
    
    # tail() demo with n = -2
    print("For negative values of n")
    fd_df.tail(-2).show()
    
Output  
    
    For negative values of n
    index   animal
    2       falcon
    3       lion
    4       monkey
    5       parrot
    6       shark
    7       whale
    8       zebra

__Return Value__   
It returns a new Frovedis DataFrame instance with last **n** rows.  

### 27. DataFrame.update_index(value, key = None, verify_integrity = False, inplace = False)  

__Parameters__  
**_value_**: It accepts a list-like object (list or tuple) as parameter. It contains the entries of the new index label.  
**_key_**: It accepts a string object as parameter. It is the name of the index label. (Default: None)  
When it is None (not specified explicitly), it will not update the index entries and it will raise an exception.  
**_verify\_integrity_**: It accepts a boolean value as parameter. When it is explicitly set to True, it checks the new index for duplicates values before update and it will not update the index when there are duplicate values. Performance of this method will be improved when it is set to False. (Default: False)  
**_inplace_**: It accepts a boolean as a parameter. It return a copy of DataFrame instance by default but when explicitly set to True, it performs operation on original DataFrame. (Default: False)  

__Purpose__  
It sets the DataFrame index using existing columns. The index can replace the existing index or can expand it.  

For example,  
        
    import pandas as pd
    import frovedis.dataframe as fdf
    
    # creating a pandas dataframe
    pd_df = pd.DataFrame({'month': [1, 4, 7, 10],
                         'year': [2012, 2014, 2013, 2014],
                         'sale': [55, 40, 84, 31]})

    # creating a frovedis dataframe
    fd_df = fdf.DataFrame(pd_df)

    # display frovedis dataframe
    fd_df.show()

Output  

    index   month   year    sale
    0       1       2012    55
    1       4       2014    40
    2       7       2013    84
    3       10      2014    31

For example,  

    # update_index() demo
    fd_df.update_index(['a','b','a','d'], key = 'index').show()
    
Output  

    index   month   year    sale
    a       1       2012    55
    b       4       2014    40
    a       7       2013    84
    d       10      2014    31
    
For example,  
    
    # update_index() demo with verify_integrity = True
    fd_df.update_index(['a','b','c','d'], key = 'index', verify_integrity = True).show()
    
Output  

    index   month   year    sale
    a       1       2012    55
    b       4       2014    40
    c       7       2013    84
    d       10      2014    31

**Note:-** Values used to update 'index' must be different when parameter 'verify_integrity' = True.  

__Return Value__  
1. It returns a new Frovedis DataFrame with updated row values for index field along with specified label.  
2. It returns None when 'inplace' parameter is set to True.  

# SEE ALSO  

- **[DataFrame - Introduction](./df_intro.md)**  
- **[DataFrame - Conversion Functions](./df_conversion.md)**  
- **[DataFrame - Sorting Functions](./df_sort.md)**  
- **[DataFrame - Math Functions](./df_math_func.md)**  
- **[DataFrame - Aggregate Functions](./df_agg_func.md)**  