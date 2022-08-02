% DataFrame Sorting Functions
  
# NAME
  
DataFrame Sorting Functions - this manual contains list of all functions related to sorting operations performed on frovedis dataframe.  

## DESCRIPTION  

Frovedis datafrme provides various sorting functions such as sort_index(), sort_values(), etc.  

These can be used to sort data along rows or columns of frovedis dataframe, either in ascending or descending order.  

**Currently, sorting opertaions will be performed on a copy of frovedis dataframe. Inplace sorting is not supported yet.**  

## Public Member Functions  
    1. nlargest(n, columns, keep = 'first')
    2. nsmallest(n, columns, keep = 'first')
    3. sort(columns = None, axis = 0, ascending = True, inplace = False, kind = 'radixsort', 
            na_position = 'last', **kwargs)
    4. sort_index(axis = 0, ascending = True, inplace = False, kind = 'quicksort', na_position = 'last')
    5. sort_values(by, axis = 0, ascending = True, inplace = False, kind = 'radixsort', na_position = 'last')

## Detailed Description  

### 1. DataFrame.nlargest(n, columns, keep = 'first')  

__Parameters__  
**_n_**: It accepts an integer parameter which represents the number of rows to return.  
**_columns_**: It accepts a string type or list of strings type parameter which represents the column label(s) to order by.  
**_keep_**: It accepts a string object parameter. This parameter is used when there are duplicate values. (Default: 'first')  
- **'first'** : it prioritizes the first occurrence(s).  
- **'last'** : it prioritizes the last occurrence(s).  
- **'all'**  : it is used to not drop any duplicates, even when it means selecting more than **n** items.  
  
__Purpose__  
It is used to return the **top n rows** ordered by the specified columns in descending order.
The columns that are not specified are returned as well, but not used for ordering.

This method is equivalent to DataFrame.sort_values(columns, ascending = False).head(n), but it is more efficient.  

**Creating frovedis DataFrame from pandas DataFrame:**  

For example,  
    
    import pandas as pd
    import frovedis.dataframe as fdf
    
    pd_df = pd.DataFrame({'population': [59000000, 65000000, 434000, 434000, 
                                        434000, 337000, 11300, 11300, 11300],
                          'GDP': [1937894, 2583560 , 12011, 4520, 12128,
                                  17036, 182, 38, 311],
                          'alpha-2': ["IT", "FR", "MT", "MV", "BN",
                                      "IS", "NR", "TV", "AI"]},
                          index=["Italy", "France", "Malta",
                                 "Maldives", "Brunei", "Iceland",
                                 "Nauru", "Tuvalu", "Anguilla"])
    # create a frovedis dataframe
    fd_df = fdf.DataFrame(pd_df)
    
    # to display frovedis dataframe
    fd_df.show()
    
Output  

    index       population      GDP       alpha-2
    Italy       59000000        1937894   IT
    France      65000000        2583560   FR
    Malta       434000          12011     MT
    Maldives    434000          4520      MV
    Brunei      434000          12128     BN
    Iceland     337000          17036     IS
    Nauru       11300           182       NR
    Tuvalu      11300           38        TV
    Anguilla    11300           311       AI

**In order to use nlargest() to select the 3 rows having the largest values in 'population' column:**  

For example,  

    fd_df.nlargest(3, 'population').show()

Output  

    index      population      GDP       alpha-2
    France     65000000        2583560   FR
    Italy      59000000        1937894   IT
    Malta      434000          12011     MT

**When using keep='last', last ocurrances are prioritized. Ties are resolved in reverse order:**  

For example,  

    # nlargest() demo with keep = 'last'    
    fd_df.nlargest(3, 'population', keep='last').show()
    
Output  
    
    index       population     GDP       alpha-2
    France      65000000       2583560   FR
    Italy       59000000       1937894   IT
    Brunei      434000         12128     BN

**When using keep='all', all duplicate items are maintained (not dropped):**  

For example,  

    # nlargest() demo with keep = 'all'
    fdf.nlargest(3, 'population', keep='all').show()
    
Output  

    index       population     GDP       alpha-2
    France      65000000       2583560   FR
    Italy       59000000       1937894   IT
    Malta       434000         12011     MT
    Maldives    434000         4520      MV
    Brunei      434000         12128     BN

**To order by the largest values in 'population' and then 'GDP' column, multiple columns may be specified:**  

For example,  

    # nlargest() demo with use of list of columns
    fd_df.nlargest(3, ['population', 'GDP']).show()
    
Output  

    index       population     GDP       alpha-2
    France      65000000       2583560   FR
    Italy       59000000       1937894   IT
    Brunei      434000         12128     BN

__Return Value__  
It returns a Frovedis DataFrame instance with **n** rows ordered by the specified columns in descending order.  

### 2. DataFrame.nsmallest(n, columns, keep = 'first')  

__Parameters__  
**_n_**: It accepts an integer type argument that represents the number of rows to return.  
**_columns_**: It accepts a string type or list of strings type parameter which represents the column label(s) to order by.  
**_keep_**: It accepts a string object parameter. This parameter is used when there are duplicate values. (Default: 'first')  
	1. **'first'** : it prioritizes the first occurrence(s).  
	2. **'last'** : it prioritizes the last last occurrence(s).  
	3. **'all'** : it is used to not drop any duplicates, even it means selecting more than **n** items.  
  
__Purpose__  
It is used to return the **top n rows** ordered by the specified columns in ascending order.

It returns the first n rows with the smallest values in columns, in ascending order. The columns that are not specified are returned as well, but not used for ordering.

This method is equivalent to DataFrame.sort_values(columns, ascending=True).head(n), but it is more efficient.

**Creating frovedis DataFrame from pandas DataFrame:**  

For example  

    import pandas as pd
    import frovedis.dataframe as fdf
    
    pd_df = pd.DataFrame({'population': [59000000, 65000000, 434000,
                                        434000, 434000, 337000, 11300,
                                        11300, 11300],
                          'GDP': [1937894, 2583560 , 12011, 4520, 12128,
                                  17036, 182, 38, 311],
                          'alpha-2': ["IT", "FR", "MT", "MV", "BN",
                                      "IS", "NR", "TV", "AI"]},
                          index=["Italy", "France", "Malta",
                                 "Maldives", "Brunei", "Iceland",
                                 "Nauru", "Tuvalu", "Anguilla"])

    # create a frovedis dataframe
    fd_df = fdf.DataFrame(pd_df)
    
    # to display frovedis dataframe
    fd_df.show()

Output  

    index       population     GDP       alpha-2
    Italy       59000000       1937894   IT
    France      65000000       2583560   FR
    Malta       434000         12011     MT
    Maldives    434000         4520      MV
    Brunei      434000         12128     BN
    Iceland     337000         17036     IS
    Nauru       11300          182       NR
    Tuvalu      11300          38        TV
    Anguilla    11300          311       AI

**In order to use nlargest() to select the 3 rows having the largest values in 'population' column:**  

For example,  

    fd_df.nsmallest(3, 'population').show()

Output  

    index       population     GDP      alpha-2
    Nauru       11300          182      NR
    Tuvalu      11300          38       TV
    Anguilla    11300          311      AI

**When using keep='last', last ocurrances are prioritized. Ties are resolved in reverse order:**  

For example,  

    # nsmallest() demo with keep = 'last' parameter
    fd_df.nsmallest(3, 'population', keep='last').show()

Output  

    index       population     GDP      alpha-2
    Anguilla    11300          311      AI
    Tuvalu      11300          38       TV
    Nauru       11300          182      NR

**When using keep='all', all duplicate items are maintained (not dropped):**  

For example  

    # example to use nsmallest with keep = 'all' parameter
    fd_df.nsmallest(3, 'population', keep='all').show()
    
Output  

    index       population     GDP      alpha-2
    Nauru       11300          182      NR
    Tuvalu      11300          38       TV
    Anguilla    11300          311      AI

**To order by the largest values in 'population' and then 'GDP' column, multiple columns may be specified:**  

For example,  

    # nsmallest() demo with using a list of columns names    
    fd_df.nsmallest(3, ['population', 'GDP'])
    
Output  

    index       population     GDP      alpha-2
    Tuvalu      11300          38       TV
    Nauru       11300          182      NR
    Anguilla    11300          311      AI

__Return Value__  
It returns a Frovedis DataFrame object with **n** rows ordered by the specified columns in ascending order.  

### 3. DataFrame.sort(columns = None, axis = 0, ascending = True, inplace = False, kind = 'radixsort', na\_position = 'last', \*\*kwargs)  

__Parameters__  
**_columns_**: It accepts the name or list of names on which sorting will be applied. (Default: None)  
If axis is 0 or 'index' then this parameter should contain index levels and/or column labels. Currently axis = 0 or 'index' is only supported in Frovedis DataFrame.  
When it is None (not specified explicitly), it will not perform sorting and it will raise an exception.  
**_axis_**: It accepts an integer or a string object as parameter.To perform sorting along rows or columns, it is selected by this parameter. (Default: 0)  
If axis is 0 or 'index', sorting is performed along the row. Currently axis = 0 or 'index' is only supported in Frovedis DataFrame.  
**_ascending_**: It accepts a boolean value or a list of booleans as parameter. The order of sorting is decided by this parameter. Need to specify a list of booleans for multiple sort orders. If this is a list of booleans, then it must match the length of the 'columns' parameter. By default, the order of sorting will be ascending and to change the order to descending, explicitly pass it as False. (Default: True)  
**_inplace_**: It accepts a boolean value as parameter. When it is explicitly set to True, it modifies the original object directly instead of creating a copy of DataFrame object. **Currently, 'inplace' = True is not supported by this Frovedis DataFrame 
method.** (Default: False)  
**_kind_**: It accepts a string object as parameter to select the type of sorting algorithm. **Currently, Frovedis supports only 'radixsort' and other values for 'kind' parameter will be ignored internally with a warning.** (Default: 'radixsort')  
**_na\_position_**: It accepts a string object as parameter. It decides the position of NaNs after sorting. When it is set to 'last', it puts NaNs at the end. **Currently, Frovedis only supports 'na\_position' = 'last'**. (Default: 'last')  
**\*\*_kwargs_**: It accepts a dictionary object as parameter. It is used to pass all the other parameters at once in the form of a dictionary object. Currently this is not supported.  

__Purpose__  
It is used to sort the values in the specified column(s) along axis = 0 or axis = 'index' in the Frovedis DataFrame.  

**This method is present only in frovedis. It internally uses sort_values().**  

**Creating frovedis DataFrame from pandas DataFrame:**  

For example,  

    import pandas as pd
    import frovedis.dataframe as fdf
    
    pd_df = pd.DataFrame({
                          'col1': ['A', 'A', 'B', np.nan, 'D', 'C'],
                          'col2': [2, 1, 9, 8, 7, 4],
                          'col3': [0, 1, 9, 4, 2, 3],
                          'col4': ['a', 'B', 'c', 'D', 'e', 'F'] })
    
    # create a frovedis dataframe from pandas dataframe
    fd_df = fdf.DataFrame(pd_df)
    
    # display frovedis dataframe
    fd_df.show()

Output  

    index   col1    col2    col3    col4
    0       A       2       0       a
    1       A       1       1       B
    2       B       9       9       c
    3       NULL    8       4       D
    4       D       7       2       e
    5       C       4       3       F

**Sort by 'col1' on frovedis dataframe:**  

For example,  

    # Sort dataframe by 'col1'
    fd_df.sort('col1').show()
    
Output  

    index   col1    col2    col3    col4
    0       A       2       0       a
    1       A       1       1       B
    2       B       9       9       c
    5       C       4       3       F
    4       D       7       2       e
    3       NULL    8       4       D

**Sorting frovedis dataframe in ascending order (by default) by using multiple columns:**  

For example,  

    # Sort dataframe by multiple columns
    fd_df.sort(['col1', 'col2']).show()
    
Output  
    
    index   col1    col2    col3    col4
    1       A       1       1       B
    0       A       2       0       a
    2       B       9       9       c
    5       C       4       3       F
    4       D       7       2       e
    3       NULL    8       4       D

**NOTE:** In the above example, in case of multiple columns sorting, **'col1'** will be sorted first and **'col2'** will only be considered for sorting in case of duplicate entries present in **'col1'**.  

**Sorting frovedis dataframe in descending orde by using ascending=False:**  

For example,  

    # Sort datafrme in descending order
    fd_df.sort('col1', ascending = False).show()
    
Output  
    
    index   col1    col2    col3    col4
    3       NULL    8       4       D
    4       D       7       2       e
    5       C       4       3       F
    2       B       9       9       c
    0       A       2       0       a
    1       A       1       1       B
 
__Return Value__   
It returns a new Frovedis DataFrame with sorted values.  

### 4. DataFrame.sort_index(axis = 0, ascending = True, inplace = False, kind = 'quicksort', na\_position = 'last')  

__Parameters__  
**_axis_**: It accepts an interger or a string object as parameter. It is the axis along which the sorting will be performed. (Default: 0)  
When axis = 0 or axis = 'index', operation will be performed on rows. **Currently only axis = 0 or axis = 'index' is supported**.  
**_ascending_**: It accepts a boolean value as parameter. This parameter decides the order to sort the data. (Default: True)  
When this parameter is explicitly passed as False, it sorts the data into descending order.  
**_inplace_**: It accepts a boolean value as paramter. To modify the original DataFrame object, argument is explicitly passed as True. Otherwise operation is performed on a copy of Frovedis DataFrame object. **Currently 'inplace' = True is not supported by 
this method.** (Default: False)  
**_kind_**: It accepts a string object as parameter. This parameter is used to select the sorting algorithms. (Default: 'quicksort')  
**_na\_position_**: It accepts a string object as parameter. It is used to decide where to puts NaNs i.e at the beginning or at the end. (Default: 'last')  
When na\_position = 'last', it puts NaNs at the end. **Currently, na\_position = 'last' is only supported for this method.**  
  
__Purpose__  
It is used to sort Frovedis DataFrame according to index values. It creates a new sorted DataFrame by the specified label.  

**Currently it only supports 'radixsort' and other values for 'kind' parameter are ignored internally along with a warning. Also, this method does not support MultiIndex yet.**  

**Creating frovedis DataFrame from pandas DataFrame:**  

For example,  

    import pandas as pd
    import frovedis.dataframe as fdf
    
    # creating a pandas dataframe
    pd_df = pd.DataFrame([1, 2, 3, 4, 5], 
                         index=[100, 29, 234, 1, 150],
                         columns=['A'])
                      
    # creating frovedis dataframe from pandas dataframe
    fd_df = fdf.DataFrame(pd_df)
    
    # to display frovedis dataframe
    fd_df.show()

Output  

    index   A
    100     1
    29      2
    234     3
    1       4
    150     5

**Sort values in dataframe, by default in ascending order (ascending=True):**  

For example,  

    # to display sorted dataframe by index
    fd_df.sort_index().show()
    
Output  

    index   A
    1       4
    29      2
    100     1
    150     5
    234     3

**In order to sort in descending order, use ascending=False:**  

For example,  
    
    fd_df.sort_index(ascending=False)
    
Output  

    index   A
    234     3
    150     5
    100     1
    29      2
    1       4

__Return Value__  
It returns a new Frovedis DataFrame instance sorted by the labels.  

### 5. DataFrame.sort_values(by, axis = 0, ascending = True, inplace = False, kind = 'radixsort', na\_position = 'last')  

__Parameters__  
**_by_**: It accepts the name or list of names on which sorting will be applied.  
If axis is 0 or 'index', then 'by' should contain index levels and/or column labels. **Currently axis = 1 or axis = 'columns' is not supported.**  
**_axis_**: It accepts an integer or a string object as parameter. To perform sorting along rows or columns, it is selected by this parameter. (Default: 0)  
If axis is 0 or 'index', sorting is performed along the row. **Currently axis = 1 or axis = 'columns' is not supported.**  
**_ascending_**: It accepts a boolean value or a list of boolean values as parameter. The order of sorting is decided by this parameter. Need to specify a list of booleans for multiple sorting orders. If this is a list of booleans, the length of the list must match the length of the 'by' parameter list. By default, the order of sorting will be ascending and to change the order to descending, explicitly pass it as False. (Default: True)  
**_inplace_**: It accepts a boolean value as parameter. To modify the original DataFrame object, argument is explicitly passed as True. Otherwise operation is performed on a copy of Frovedis DataFrame object. **Currently 'inplace' = True is not supported by 
this method.** (Default: False)  
**_kind_**: It accepts a string object as parameter. The type of sorting algorithm is decided from this parameter. **Currently it only supports 'radixsort' and other values for 'kind' parameter are ignored internally along with a warning.** (Default: 'radixsort')  
**_na\_position_**: It accepts a string object as parameter. It is used to decide where to puts NaNs i.e at the beginning or at the end. (Default: 'last')  
When na\_position = 'last', it puts NaNs at the end. **Currently, it only supports na\_position = 'last'.**  
  
__Purpose__  
To sort the DataFrame by the values along axis = 0 or 'index'.  

**Creating frovedis DataFrame from pandas DataFrame:**   
 
For example,  

    import pandas as pd
    import frovedis.dataframe as fdf
    
    # creating a pandas dataframe
    pd_df = pd.DataFrame({
                          'col1': ['A', 'A', 'B', np.nan, 'D', 'C'],
                          'col2': [2, 1, 9, 8, 7, 4],
                          'col3': [0, 1, 9, 4, 2, 3],
                          'col4': ['a', 'B', 'c', 'D', 'e', 'F']
                        })
                        
    # creating a frovedis dataframe
    fd_df = fdf.DataFrame(pd_df)
    
    # displaying the frovedis dataframe
    fd_df.show()

Output  

    index   col1    col2    col3    col4
    0       A       2       0       a
    1       A       1       1       B
    2       B       9       9       c
    3       NULL    8       4       D
    4       D       7       2       e
    5       C       4       3       F

**Sort by 'col1' on frovedis dataframe:**  

For example,  

    fd_df.sort_values(by=['col1']).show() #Sort by col1

Output  

    index   col1    col2    col3    col4
    0       A       2       0       a
    1       A       1       1       B
    2       B       9       9       c
    5       C       4       3       F
    4       D       7       2       e
    3       NULL    8       4       D

**Sorting frovedis dataframe in ascending order (by default) by using multiple columns:**  

For example,  
    
    fd_df.sort_values(by=['col1', 'col2'], ascending = [True, True]).show()
    
Output  

    index   col1    col2    col3    col4
    1       A       1       1       B
    0       A       2       0       a
    2       B       9       9       c
    5       C       4       3       F
    4       D       7       2       e
    3       NULL    8       4       D

**Sorting frovedis dataframe in descending orde by using ascending=False:**  

For example,  

    # Sort in descending order
    fd_df.sort_values(by='col1', ascending=False).show()
    
Output  

    index   col1    col2    col3    col4
    3       NULL    8       4       D
    4       D       7       2       e
    5       C       4       3       F
    2       B       9       9       c
    0       A       2       0       a
    1       A       1       1       B

__Return Value__  
It returns a new Frovedis DataFrame with sorted values.  

# SEE ALSO  

- **[DataFrame - Introduction](./df_intro.md)**  
- **[DataFrame - Indexing Operations](./df_indexing_operations.md)**  
- **[DataFrame - Generic Functions](./df_generic_func.md)**  
- **[DataFrame - Conversion Functions](./df_conversion.md)**  
- **[DataFrame - Math Functions](./df_math_func.md)**  
- **[DataFrame - Aggregate Functions](./df_agg_func.md)**   
