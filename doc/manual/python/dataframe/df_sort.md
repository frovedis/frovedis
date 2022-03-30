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

### 1. nlargest(n, columns, keep = 'first')  

__Parameters__  
**_n_**: It accepts an integer parameter which represents the number of rows to return.  
**_columns_**: It accepts a string type or list of strings type parameter which represents the column label(s) to order by.  
**_keep_**: It accepts a string object parameter. This parameter is used when there are duplicate values. (Default: 'first')  
- **'first'** : it prioritizes the first occurrence(s).  
- **'last'** : it prioritizes the last occurrence(s).  
- **'all'**  : it is used to not drop any duplicates, even when it means selecting more than **n** items.  
  
__Purpose__  
It is used to return the top **n** rows ordered by the specified columns in descending order.
The columns that are not specified are returned as well, but not used for ordering.

This method is equivalent to DataFrame.sort_values(columns, ascending = False).head(n), but it is more efficient.  

For example,  

	# example to use nlargest with default parameters with n = 3 and columns = 'population'
    
    import pandas as pd
	import frovedis.dataframe as fdf
    
	# In the following example, we will use nlargest() to select the three rows having the largest values in column “population”.
	pd_df = pd.DataFrame({'population': [59000000, 65000000, 434000, 434000, 
										434000, 337000, 11300, 11300, 11300],
						'GDP': [1937894, 2583560 , 12011, 4520, 12128,
								17036, 182, 38, 311],
						'alpha-2': ["IT", "FR", "MT", "MV", "BN",
									"IS", "NR", "TV", "AI"]},
						index=["Italy", "France", "Malta",
								"Maldives", "Brunei", "Iceland",
								"Nauru", "Tuvalu", "Anguilla"])
	fd_df = fdf.DataFrame(pd_df)
    
	# to display frovedis dataframe
	fd_df.show()
    
	# to display n largest rows according to given column
	fd_df.nlargest(3, 'population').show()
    
Output  

	index           population      GDP     alpha-2
	Italy           59000000        1937894 IT
	France          65000000        2583560 FR
	Malta           434000          12011   MT
	Maldives        434000          4520    MV
	Brunei          434000          12128   BN
	Iceland         337000          17036   IS
	Nauru           11300           182     NR
	Tuvalu          11300           38      TV
	Anguilla        11300           311     AI

	index           population      GDP     alpha-2
	France          65000000        2583560 FR
	Italy           59000000        1937894 IT
	Malta           434000          12011   MT

For example,  
	
    # example to show the behavior of nlargest with keep = 'last'
    
    import pandas as pd
	import frovedis.dataframe as fdf
    
	# When using keep='last', it will show the row only having last occurrence of the duplicate :
	pd_df = pd.DataFrame({'population': [59000000, 65000000, 434000, 434000, 
										434000, 337000, 11300, 11300, 11300],
						'GDP': [1937894, 2583560 , 12011, 4520, 12128,
								17036, 182, 38, 311],
						'alpha-2': ["IT", "FR", "MT", "MV", "BN",
									"IS", "NR", "TV", "AI"]},
						index=["Italy", "France", "Malta",
								"Maldives", "Brunei", "Iceland",
								"Nauru", "Tuvalu", "Anguilla"])
	fd_df = fdf.DataFrame(pd_df)
    
	# to display frovedis dataframe
	fd_df.show()
    
    fd_df.nlargest(3, 'population', keep='last').show()
    
Output  

	index           population      GDP     alpha-2
	Italy           59000000        1937894 IT
	France          65000000        2583560 FR
	Malta           434000          12011   MT
	Maldives        434000          4520    MV
	Brunei          434000          12128   BN
	Iceland         337000          17036   IS
	Nauru           11300           182     NR
	Tuvalu          11300           38      TV
	Anguilla        11300           311     AI
    
    index           population      GDP     alpha-2
	France          65000000        2583560 FR
	Italy           59000000        1937894 IT
	Brunei  -       434000          12128   BN

For example,  

    # example to show the behavior of nlargest with keep = 'all'
    
    import pandas as pd
	import frovedis.dataframe as fdf
    
	# When using keep='all', all duplicate items are maintained:
	pd_df = pd.DataFrame({'population': [59000000, 65000000, 434000, 434000, 
										434000, 337000, 11300, 11300, 11300],
						'GDP': [1937894, 2583560 , 12011, 4520, 12128,
								17036, 182, 38, 311],
						'alpha-2': ["IT", "FR", "MT", "MV", "BN",
									"IS", "NR", "TV", "AI"]},
						index=["Italy", "France", "Malta",
								"Maldives", "Brunei", "Iceland",
								"Nauru", "Tuvalu", "Anguilla"])
	fd_df = fdf.DataFrame(pd_df)
    
	# to display frovedis dataframe
	fd_df.show()

	fdf.nlargest(3, 'population', keep='all').show()
    
Output  

	index           population      GDP     alpha-2
	Italy           59000000        1937894 IT
	France          65000000        2583560 FR
	Malta           434000          12011   MT
	Maldives        434000          4520    MV
	Brunei          434000          12128   BN
	Iceland         337000          17036   IS
	Nauru           11300           182     NR
	Tuvalu          11300           38      TV
	Anguilla        11300           311     AI
    
    index           population      GDP     alpha-2
	France          65000000        2583560 FR
	Italy           59000000        1937894 IT
	Malta           434000          12011   MT
	Maldives        434000          4520    MV
	Brunei          434000          12128   BN

For example,  

    # example to show use of list of columns with nlargest
    
    import pandas as pd
	import frovedis.dataframe as fdf
    
	# To order by the largest values in column “population” and then “GDP”
	pd_df = pd.DataFrame({'population': [59000000, 65000000, 434000, 434000, 
										434000, 337000, 11300, 11300, 11300],
						'GDP': [1937894, 2583560 , 12011, 4520, 12128,
								17036, 182, 38, 311],
						'alpha-2': ["IT", "FR", "MT", "MV", "BN",
									"IS", "NR", "TV", "AI"]},
						index=["Italy", "France", "Malta",
								"Maldives", "Brunei", "Iceland",
								"Nauru", "Tuvalu", "Anguilla"])
	fd_df = fdf.DataFrame(pd_df)
    
	# to display frovedis dataframe
	fd_df.show()

	fd_df.nlargest(3, ['population', 'GDP']).show()
    
Output  

	index           population      GDP     alpha-2
	Italy           59000000        1937894 IT
	France          65000000        2583560 FR
	Malta           434000          12011   MT
	Maldives        434000          4520    MV
	Brunei          434000          12128   BN
	Iceland         337000          17036   IS
	Nauru           11300           182     NR
	Tuvalu          11300           38      TV
	Anguilla        11300           311     AI
    
    index           population      GDP     alpha-2
	France          65000000        2583560 FR
	Italy           59000000        1937894 IT
	Brunei          434000          12128   BN

__Return Value__  
It returns a Frovedis DataFrame object with **n** rows ordered by the specified columns in descending order.  

### 2. nsmallest(n, columns, keep = 'first')  

__Parameters__  
**_n_**: It accepts an integer type argument that represents the number of rows to return.  
**_columns_**: It accepts a string type or list of strings type parameter which represents the column label(s) to order by.  
**_keep_**: It accepts a string object parameter. This parameter is used when there are duplicate values. (Default: 'first')  
	1. **'first'** : it prioritizes the first occurrence(s).  
	2. **'last'** : it prioritizes the last last occurrence(s).  
	3. **'all'** : it is used to not drop any duplicates, even it means selecting more than **n** items.  
  
__Purpose__  
It is used to return the top **n** rows ordered by the specified columns in ascending order.

It returns the first n rows with the smallest values in columns, in ascending order. The columns that are not specified are returned as well, but not used for ordering.

This method is equivalent to DataFrame.sort_values(columns, ascending=True).head(n), but it is more efficient.

For example  

	# example to use nsmallest with default values on columns = 'population'
    
    import pandas as pd
	import frovedis.dataframe as fdf
    
    # to display n smallest rows according to given column
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
	fd_df = fdf.DataFrame(pd_df)
	# to display frovedis dataframe
	fd_df.show()
    
	fd_df.nsmallest(3, 'population').show()
    
Output  

	index           population      GDP     alpha-2
	Italy           59000000        1937894 IT
	France          65000000        2583560 FR
	Malta           434000          12011   MT
	Maldives        434000          4520    MV
	Brunei          434000          12128   BN
	Iceland         337000          17036   IS
	Nauru           11300           182     NR
	Tuvalu          11300           38      TV
	Anguilla        11300           311     AI

	index           population      GDP     alpha-2
	Nauru           11300           182     NR
	Tuvalu          11300           38      TV
	Anguilla        11300           311     AI

For example,  

	# example to use nsmallest with keep = 'last' parameter
    
    import pandas as pd
	import frovedis.dataframe as fdf
    
    # When using keep='last', duplicates are resolved in reverse order:
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
	fd_df = fdf.DataFrame(pd_df)
	# to display frovedis dataframe
	fd_df.show()

	fd_df.nsmallest(3, 'population', keep='last').show()
	
Output  

	index           population      GDP     alpha-2
	Italy           59000000        1937894 IT
	France          65000000        2583560 FR
	Malta           434000          12011   MT
	Maldives        434000          4520    MV
	Brunei          434000          12128   BN
	Iceland         337000          17036   IS
	Nauru           11300           182     NR
	Tuvalu          11300           38      TV
	Anguilla        11300           311     AI
    
    index           population      GDP     alpha-2
	Anguilla        11300           311     AI
	Tuvalu          11300           38      TV
	Nauru           11300           182     NR

For example  

	# example to use nsmallest with keep = 'all' parameter
    
    import pandas as pd
	import frovedis.dataframe as fdf
    
    # When using keep='all', all duplicate items are maintained:
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
	fd_df = fdf.DataFrame(pd_df)
    
	# to display frovedis dataframe
	fd_df.show()

	fd_df.nsmallest(3, 'population', keep='all').show()
    
Output  

	index           population      GDP     alpha-2
	Italy           59000000        1937894 IT
	France          65000000        2583560 FR
	Malta           434000          12011   MT
	Maldives        434000          4520    MV
	Brunei          434000          12128   BN
	Iceland         337000          17036   IS
	Nauru           11300           182     NR
	Tuvalu          11300           38      TV
	Anguilla        11300           311     AI
    
    index           population      GDP     alpha-2
	Nauru           11300           182     NR
	Tuvalu          11300           38      TV
	Anguilla        11300           311     AI

For example,  

	# example to use nsmallest on a list of columns names
    
    import pandas as pd
	import frovedis.dataframe as fdf
    
    # To order by the smallest values in column “population” and then “GDP”, 
    # we can specify multiple columns like in the next example.
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
	fd_df = fdf.DataFrame(pd_df)
    
	# to display frovedis dataframe
	fd_df.show()
    
	# display smallest 3 row from population and then GDP columns
    fd_df.nsmallest(3, ['population', 'GDP'])
    
Output  

	index           population      GDP     alpha-2
	Italy           59000000        1937894 IT
	France          65000000        2583560 FR
	Malta           434000          12011   MT
	Maldives        434000          4520    MV
	Brunei          434000          12128   BN
	Iceland         337000          17036   IS
	Nauru           11300           182     NR
	Tuvalu          11300           38      TV
	Anguilla        11300           311     AI
    
    index           population      GDP     alpha-2
	Tuvalu          11300           38      TV
	Nauru           11300           182     NR
	Anguilla        11300           311     AI

__Return Value__  
It returns a Frovedis DataFrame object with **n** rows ordered by the specified columns in ascending order.  

### 3. sort(columns = None, axis = 0, ascending = True, inplace = False, kind = 'radixsort', na\_position = 'last', \*\*kwargs)  

__Parameters__  
**_columns_**: It accepts the name or list of names on which sorting will be applied. (Default: None)  
If axis is 0 or 'index' then this parameter should contain index levels and/or column labels. Currently axis = 0 or 'index' is only supported in Frovedis DataFrame.  
When it is None (not specified explicitly), it will not perform sorting and it will raise an exception.  
**_axis_**: It accepts an integer or a string object as parameter.To perform sorting along rows or columns is selected by this parameter. (Default: 0)  
If axis is 0 or 'index', sorting is performed along the row. Currently axis = 0 or 'index' is only supported in Frovedis DataFrame.  
**_ascending_**: It accepts a boolean value or a list of booleans as parameter. The order of sorting is decided by this parameter. Need to specify a list of booleans for multiple sort orders. If this is a list of booleans, then it must match the length of the 'columns' parameter. By default, the order of sorting will be ascending and to change the order to descending, explicitly pass it as False. (Default: True)  
**_inplace_**: It accepts a boolean value as parameter. When it is explicitly set to True, it modifies the original object directly instead of creating a copy of DataFrame object. (Default: False)  

**NOTE: Currently, 'inplace' = True is not supported by this Frovedis DataFrame method.**  

**_kind_**: It accepts a string object as parameter to select the type of sorting algorithm. (Default: ‘radixsort’)  

**NOTE: Currently Frovedis supports only 'radixsort' and other 'kind' parameters will be ignored internally with a warning.**  

**_na\_position_**: It accepts a string object as parameter. It decides the position of NaNs after sorting. When it is set to 'last', it puts NaNs at the end. Currently, Frovedis only supports 'na\_position' = 'last'. (Default: ‘last’)  
**\*\*_kwargs_**: It accepts a dictionary object as parameter. It is used to pass all the other parameters at once in the form of a dictionary object. Currently this is not supported.  

__Purpose__  
It is used to sort the values in the specified column(s) along axis = 0 or axis = 'index' in the Frovedis DataFrame.  

**This method is present only in frovedis. It internally uses sort_values().**  

For example,  

    # example to sort a column with other default parameters
    
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
    
    # Sort by 'col1' and display
    fd_df.sort('col1').show()
    
Output  

    index   col1    col2    col3    col4
    0       A       2       0       a
    1       A       1       1       B
    2       B       9       9       c
    3       NULL    8       4       D
    4       D       7       2       e
    5       C       4       3       F

    index   col1    col2    col3    col4
    0       A       2       0       a
    1       A       1       1       B
    2       B       9       9       c
    5       C       4       3       F
    4       D       7       2       e
    3       NULL    8       4       D

For example,  

    # example to sort a list of columns
    
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
    
    # Sort by multiple columns and display
    fd_df.sort(['col1', 'col2']).show()
    
Output  

    index   col1    col2    col3    col4
    0       A       2       0       a
    1       A       1       1       B
    2       B       9       9       c
    3       NULL    8       4       D
    4       D       7       2       e
    5       C       4       3       F
    
    index   col1    col2    col3    col4
    1       A       1       1       B
    0       A       2       0       a
    2       B       9       9       c
    5       C       4       3       F
    4       D       7       2       e
    3       NULL    8       4       D
NOTE: In the above example, in case of multiple columns sorting, 'col1' will be sorted first and the 'col2' will only be considered for sorting in case of duplicate entries present in 'col1'.  

For example,  

    # example to sort in descending order

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
    
    # Sort in descending order and display
    fd_df.sort('col1', ascending = False).show()
    
Output  

    index   col1    col2    col3    col4
    0       A       2       0       a
    1       A       1       1       B
    2       B       9       9       c
    3       NULL    8       4       D
    4       D       7       2       e
    5       C       4       3       F
    
    index   col1    col2    col3    col4
    3       NULL    8       4       D
    4       D       7       2       e
    5       C       4       3       F
    2       B       9       9       c
    0       A       2       0       a
    1       A       1       1       B
 
__Return Value__   
It returns a new Frovedis DataFrame with sorted values.  

### 4. sort_index(axis = 0, ascending = True, inplace = False, kind = 'quicksort', na\_position = 'last')  

__Parameters__  
**_axis_**: It accepts an interger or a string object as parameter. It is the axis along which the sorting will be performed. (Default: 0)  
When axis = 0 or axis = 'index', operation will be performed on rows. Currently only axis = 0 or axis = 'index' is supported.  
**_ascending_**: It accepts a boolean value as parameter. This parameter decides the order to sort the data. When this parameter is explicitly passed as False, it sorts the data into descending order. (Default: True)  
**_inplace_**: It accepts a boolean value as paramter. To modify the original DataFrame object, argument is explicitly passed as True. Otherwise operation is performed on a copy of Frovedis DataFrame object. (Default: False)  

**NOTE: Currently 'inplace' = True is not supported by this method.**  

**_kind_**: It accepts a string object as parameter. This parameter is used to select the sorting algorithms. (Default: 'quicksort')  
**_na\_position_**: It accepts a string object as parameter. It is used to decide where to puts NaNs i.e at the beginning or at the end. (Default: 'last')  
When na\_position = 'last', it puts NaNs at the end. Currently only na\_position = 'last' is supported.  
  
__Purpose__ 
It is used to sort Frovedis DataFrame according to index values. It creates a new sorted DataFrame by the specified label.  

**NOTE: Currently it only supports 'radixsort' and other kind of sorting techniques are ignored internally along with a warning.**  

**NOTE: Currently Frovedis DataFrame doesn't support MultiIndex**  

For example,  

	# example to sort index using default values
    
    import pandas as pd
	import frovedis.dataframe as fdf
    
    # creating a pandas dataframe
	pd_df = pd.DataFrame([1, 2, 3, 4, 5], index=[100, 29, 234, 1, 150],
					  columns=['A'])
                      
	# creating frovedis dataframe from pandas dataframe
	fd_df = fdf.DataFrame(pd_df)
    
	# to display frovedis dataframe
	fd_df.show()
    
	# to display sorted dataframe by index
	fd_df.sort_index().show()
    
Output  

	index   A
	100     1
	29      2
	234     3
	1       4
	150     5

	index   A
	1       4
	29      2
	100     1
	150     5
	234     3

For example,  
    
    # example to sort_index in decending order
    
    import pandas as pd
	import frovedis.dataframe as fdf
    
    # creating a pandas dataframe
	pd_df = pd.DataFrame([1, 2, 3, 4, 5], index=[100, 29, 234, 1, 150],
					  columns=['A'])
                      
	# creating frovedis dataframe from pandas dataframe
	fd_df = fdf.DataFrame(pd_df)
    
	# to display frovedis dataframe
	fd_df.show()
    
	# To sort in descending order, use ascending=False
	fd_df.sort_index(ascending=False)
    
Output  

	index   A
	100     1
	29      2
	234     3
	1       4
	150     5
    
	index   A
	234     3
	150     5
	100     1
	29      2
	1       4

__Return Value__  
It returns a new Frovedis DataFrame sorted by the labels.  

### 5. sort_values(by, axis = 0, ascending = True, inplace = False, kind = 'radixsort', na\_position = 'last')  

__Parameters__  
**_by_**: It accepts the name or list of names on which sorting will be applied.  
If axis is 0 or 'index', then 'by' should contain index levels and/or column labels.  
    
**NOTE: Currently axis = 1 or axis = 'columns' is not supported.**  

**_axis_**: It accepts an integer or a string object as parameter. To perform sorting along rows or columns is selected by this parameter. (Default: 0)  
If axis is 0 or 'index', sorting is performed along the row.  
    
**NOTE: Currently axis = 1 or axis = 'columns' is not supported.**  

**_ascending_**: It accepts a boolean value or a list of boolean values as parameter. The order of sorting is decided by this parameter. Need to specify a list of booleans for multiple sorting orders. If this is a list of booleans, the length of the list must match the length of the 'by' parameter list. By default, the order of sorting will be ascending and to change the order to descending, explicitly pass it as False. (Default: True)  
**_inplace_**: It accepts a boolean value as parameter. To modify the original DataFrame object, argument is explicitly passed as True. Otherwise operation is performed on a copy of Frovedis DataFrame object. (Default: False)  

**NOTE: Currently 'inplace' = True is not supported by this method.**  

**_kind_**: It accepts a string object as parameter. The type of sorting algorithm is decided from this parameter. (Default: 'radixsort')  

**NOTE: Currently it only supports 'radixsort' and other kind of sorting techniques are ignored internally along with a warning.**  

**_na\_position_**: It accepts a string object as parameter. It is used to decide where to puts NaNs i.e at the beginning or at the end. (Default: 'last')  
When na\_position = 'last', it puts NaNs at the end. Currently it only supports na\_position = 'last'.  
  
__Purpose__  
To sort the DataFrame by the values along axis = 0 or 'index'.  
 
For example,  

	# example of using sort_values on a column label
    
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
    
	# sorting 'col1' by values on frovedis dataframe
	fd_df.sort_values(by=['col1']).show() #Sort by col1

Output  

	index   col1    col2    col3    col4
	0       A       2       0       a
	1       A       1       1       B
	2       B       9       9       c
	3       NULL    8       4       D
	4       D       7       2       e
	5       C       4       3       F

	index   col1    col2    col3    col4
	0       A       2       0       a
	1       A       1       1       B
	2       B       9       9       c
	5       C       4       3       F
	4       D       7       2       e
	3       NULL    8       4       D

For example,  
    
    # example of using sort_values with parameters 'by' and 'ascending' as list
    
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

	# Sorting multiple columns
	fd_df.sort_values(by=['col1', 'col2'], ascending = [True, True]).show()
    
Output  

	index   col1    col2    col3    col4
	0       A       2       0       a
	1       A       1       1       B
	2       B       9       9       c
	3       NULL    8       4       D
	4       D       7       2       e
	5       C       4       3       F
    
    index   col1    col2    col3    col4
	1       A       1       1       B
	0       A       2       0       a
	2       B       9       9       c
	5       C       4       3       F
	4       D       7       2       e
	3       NULL    8       4       D

For example,  
    
    # example with parameter ascending = False
    
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

	# Sort in descending order
	fd_df.sort_values(by='col1', ascending=False).show()
    
Output  

	index   col1    col2    col3    col4
	0       A       2       0       a
	1       A       1       1       B
	2       B       9       9       c
	3       NULL    8       4       D
	4       D       7       2       e
	5       C       4       3       F
    
    index   col1    col2    col3    col4
	3       NULL    8       4       D
	4       D       7       2       e
	5       C       4       3       F
	2       B       9       9       c
	0       A       2       0       a
	1       A       1       1       B

__Return Value__  
It returns a new Frovedis DataFrame with sorted values.  

### 6. to_dict(orient = "dict", into = dict)  

__Parameters__  
**_orient_**: It accepts a string object as parameter. It is used to determine the type of the values of the dictionary. (Default: 'dict')  
    1. *'dict'* : dict like {column -> {index -> value}}  
    2. *'list'* : dict like {column -> [values]}
**_into_**: This parameter is used for mapping in the return value. (Default: dict)  
NOTE: Currently it only supports OrderedDict as return type.  

__Purpose__  
It is used to convert the Frovedis DataFrame to a dictionary.  

For example,  
	
    # example of using to_dict with default values
    
    import pandas as pd
	import frovedis.dataframe as fdf
    
    # a pandas dataframe
	pd_df = pd.DataFrame({'col1': [1, 2],
					   'col2': [0.5, 0.75]},
					  index=['row1', 'row2'])
	# a frovedis dataframe
	fd_df = fdf.DataFrame(pd_df)
    
	# to display frovedis dataframe
	fd_df.show()
    
	# to display converted object using to_dict
	print(fd_df.to_dict())
    
Output  

	index   col1    col2
	row1    1       0.5
	row2    2       0.75

	OrderedDict([('col1', {'row1': 1, 'row2': 2}), ('col2', {'row1': 0.5, 'row2': 0.75})])
    
For example,  
	
    # example of using to_dict with orient = 'list'
    
    import pandas as pd
	import frovedis.dataframe as fdf
    
    # a pandas dataframe
	pd_df = pd.DataFrame({'col1': [1, 2],
					   'col2': [0.5, 0.75]},
					  index=['row1', 'row2'])
	# a frovedis dataframe
	fd_df = fdf.DataFrame(pd_df)
    
	# to display frovedis dataframe
	fd_df.show()
    
	# to display converted object using to_dict
	print(fd_df.to_dict(orient = 'list'))
    
Output  

	index   col1    col2
	row1    1       0.5
	row2    2       0.75

	OrderedDict([('index', array(['row1', 'row2'], dtype='<U4')), ('col1', array([1, 2])), ('col2', array([0.5 , 0.75]))])

__Return Value__  
It returns a dictionary representing the Frovedis DataFrame. The resulting transformation depends on the 'orient' parameter.  

### 7. to_numpy(dtype = None, copy = False, na_value = None)  

__Parameters__  
**_dtype_** : It accepts the dtype parameter which decides the datatype of numpy ndarray. (Default: None)  
When it is None (not specified explicitly), it will be set as double (float64).  
**_copy_** : This is an unused parameter. (Default: False)  
**_na\_value_** : This is an unused parameter. (Default: None)  

__Purpose__  
This method is used to convert a frovedis dataframe into a numpy array.  

The parameters: "copy" and "na_value" are simply kept in to make the interface uniform to the pandas DataFrame.to_numpy().  
This is not used anywhere within the frovedis implementation.  

For example,  

	# example to convert a frovedis dataframe into numpy
    
    import pandas as pd
	import frovedis.dataframe as fdf
	
	# a python dict
	peopleDF = {
				'Ename' : ['Michael', 'Andy', 'Tanaka', 'Raul', 'Yuta'],
				'Age' : [29, 30, 27, 19, 31],
				'Country' : ['USA', 'England', 'Japan', 'France', 'Japan'],
				'isMale': [False, False, False, False, True]
			   }
    
	# a pandas dataframe object from dict
	pd_df = pd.DataFrame(peopleDF)
	# to create a frovedis dataframe object
	fd_df = fdf.DataFrame(pd_df)
    
	# below will display a Frovedis dataframe type
	print(type(fd_df))
    
	# display frovedis dataframe
	fd_df.show()
    
	# below will display a numpy array type
	print(type(fd_df.to_numpy()))
    
    # below will display a numpy array type
	print(fd_df.to_numpy())
    
Output  

	<class 'frovedis.dataframe.df.DataFrame'>

	index   Ename   Age     Country isMale
	0       Michael 29      USA     0
	1       Andy    30      England 0
	2       Tanaka  27      Japan   0
	3       Raul    19      France  0
	4       Yuta    31      Japan   1
 
	<<class 'numpy.ndarray'>

	[['Michael' '29' 'USA' '0']
	 ['Andy' '30' 'England' '0']
	 ['Tanaka' '27' 'Japan' '0']
	 ['Raul' '19' 'France' '0']
	 ['Yuta' '31' 'Japan' '1']]

__Return Value__  
It returns a numpy ndarray that represents a Frovedis DataFrame. It has shape (nRows , nCols).  

### 8. to_pandas()  

__Purpose__  
It converts a Frovedis DataFrame instance into a pandas dataframe instance.  

For example,  

	# example to convert frovedis dataframe to pandas dataframe
    
    import pandas as pd
	import frovedis.dataframe as fdf
	
	# a python dictionary
	peopleDF = {
				'Ename' : ['Michael', 'Andy', 'Tanaka', 'Raul', 'Yuta'],
				'Age' : [29, 30, 27, 19, 31],
				'Country' : ['USA', 'England', 'Japan', 'France', 'Japan'],
				'isMale': [False, False, False, False, True]
			   }
	# creating a pandas dataframe from python dict
	pd_df = pd.DataFrame(peopleDF)
    
	# creating a Frovedis dataframe from pandas dataframe
	fd_df = fdf.DataFrame(pd_df)
    
	# below will display a Frovedis dataframe type
	print(type(fd_df))
    
    # below will display a Frovedis dataframe
	fd_df.show()
    
    # below will display a pandas dataframe type after conversion
	print(type(fd_df.to_pandas()))
    
    # below will display a pandas dataframe after conversion
	print(fd_df.to_pandas())
    
Output  
 
    <class 'frovedis.dataframe.df.DataFrame'>

    index   Ename   Age     Country isMale
    0       Michael 29      USA     0
    1       Andy    30      England 0
    2       Tanaka  27      Japan   0
    3       Raul    19      France  0
    4       Yuta    31      Japan   1   

    <class 'pandas.core.frame.DataFrame'>

    index  Ename  Age  Country  isMale
    0      Michael   29      USA   False
    1         Andy   30  England   False
    2       Tanaka   27    Japan   False
    3         Raul   19   France   False
    4         Yuta   31    Japan    True  

__Return Value__  
It returns a pandas dataframe instance after suitable conversion.  

### 9. to_frovedis_rowmajor_matrix(t_cols, dtype = np.float32)  

__Parameters__  
**_t\_cols_**: It accepts a list of string type argument where each of the member of the list is the name of the column labels.  
**_dtype_**: It accepts a dtype type argument which is the type of the resultant values.  
Currently only float32 and float64 types are supported. (Default: np.float32)  

__Purpose__  
This method is used to convert a Frovedis DataFrame into FrovedisRowmajorMatrix instance.  

For example,  

	# example to convert frovedis dataframe into frovedis row major matrix
    
    import frovedis.dataframe as fdf
    import numpy as np

	# read_csv demo to get values
	df = fdf.read_csv("./input/numbers.csv", names = ['one', 'two', 'three', 'four'])
	
    # display frovedis dataframe
    df.show()

	row_mat = df.to_frovedis_rowmajor_matrix(['one', 'two'], dtype = np.float64)
	
    # display row_mat (row major matrix)
    row_mat.debug_print()

	# display type of row_mat
    print(type(row_mat))
    
Output  

	index   one     two     three   four
	0       10      10.23   F       0
	1       12      12.2    F       0
	2       13      34.8999 D       1
	3       15      100.12  A       2

	<class 'frovedis.matrix.dense.FrovedisRowmajorMatrix'>
    
	matrix:
	num_row = 4, num_col = 2
	node 0
	node = 0, local_num_row = 4, local_num_col = 2, val = 10 10.23 12 12.2 13 34.9 15 100.12

__Return Value__  
It returns a FrovedisRowmajorMatrix instance after suitable conversion.  

### 10. to_frovedis_colmajor_matrix(t_cols, dtype = np.float32)  

__Parameters__  
**_t\_cols_**: It accepts a list of string type argument where each of the member of the list is the name of the column labels.  
**_dtype_**: It accepts a dtype type argument which is the type of the resultant values.  
Currently only float32 and float64 types are supported. (Default: np.float32)  

__Purpose__  
This method converts a frovedis dataframe to FrovedisColmajorMatrix.  
 
For example,  

	# example to use to_frovedis_colmajor_matrix
    
    import frovedis.dataframe as fdf
    
    # read_csv demo
	df = fdf.read_csv("./input/numbers.csv", names=['one', 'two', 'three', 'four'])
	df.show()

	col_mat = df.to_frovedis_colmajor_matrix(['one', 'two']) # default dtype = np.float32
	
    # display type of col_mat
	print(type(col_mat))
    
    # display col_mat
    col_mat.debug_print()
    
Output  

	index   one     two     three   four
	0       10      10.23   F       0
	1       12      12.2    F       0
	2       13      34.8999 D       1
	3       15      100.12  A       2

	<class 'frovedis.matrix.dense.FrovedisColmajorMatrix'>
    
	matrix:
	num_row = 4, num_col = 2
	node 0
	node = 0, local_num_row = 4, local_num_col = 2, val = 10 12 13 15 10.23 12.2 34.9 100.12

__Return Value__  
It returns a FrovedisColmajorMatrix instance after converting the original Frovedis DataFrame.  

### 11. to_frovedis_crs_matrix(t_cols, cat_cols, dtype = np.float32, need_info = False)  

__Parameters__  
**_t\_cols_**: It accepts a list of string type argument where each of the member of the list is the name of the column labels.  
**_cat\_cols_**: It accepts a list of strings as parameter where strings are the column names. It stands for categorical columns. It represents how the repetitive values are distributed in the specified column. It adds number of columns corresponding to number of distinct values in the specified column to the matrix. And values corresponding to the value of the specicified column is set to 1 and remaining values are set to 0.  
**_dtype_**: It accepts a dtype type argument which is the type of the resultant values.  
Currently only float32 and float64 types are supported. (Default: np.float32)  
**_need\_info_**: It accepts a boolean value as parameter. When this is explicitly set to True, it returns an additional value of type 'df_to_sparse_info'. (Default: False)  

__Purpose__  
It converts a Frovedis DataFrame object to FrovedisCRSMatrix instance.  

For example,  

	# example to use to_frovedis_crs_matrix
    
    import frovedis.dataframe as fdf
	import numpy as np
	
	# read_csv demo
	df = fdf.read_csv("./input/numbers.csv", names = ['one', 'two', 'three', 'four'])
	df.show()

	crs_mat,info = df.to_frovedis_crs_matrix(['one', 'two', 'four'],
											 ['three'], need_info = True) # default dtype = float32
	
    # display type of crs_mat
    print(type(crs_mat))
    
    # display crs_mat on console
    crs_mat.debug_print()
    
    # converting crs_mat to display it in better form
    mat_t = crs_mat.to_scipy_matrix().todense()
    
    # display matrix
    print(mat_t)
    
Output  

	index   one     two     three   four
	0       10      10.23   F       0
	1       12      12.2    F       0
	2       13      34.8999 D       1
	3       15      100.12  A       2

	Active Elements:  12
    
	<class 'frovedis.matrix.crs.FrovedisCRSMatrix'>
	
    matrix:
	num_row = 4, num_col = 5
	node 0
	local_num_row = 4, local_num_col = 5
	val : 10 10.23 1 12 12.2 1 13 34.9 1 15 100.12 1
	idx : 0 1 2 0 1 2 0 1 3 0 1 4
	off : 0 3 6 9 12
    
    [[ 10.    10.23   0.     0.     1.  ]
     [ 12.    12.2    0.     0.     1.  ]
     [ 13.    34.9    0.     1.     0.  ]
     [ 15.   100.12   1.     0.     0.  ]]

__Return Value__  
It returns a FrovedisCRSMatrix instance after converting the original Frovedis DataFrame.  

### 12. to_frovedis_crs_matrix_using_info(info, dtype = np.float32)  

__Parameters__  
**_info_**: It accepts an instance of 'df_to_sparse_info' type.  
**_dtype_**: It accepts a dtype type argument which is the type of the resultant values.  
Currently only float32 and float64 types are supported. (Default: np.float32)  
 
__Purpose__  
It converts a frovedis dataframe to FrovedisCRSMatrix provided an info object of df_to_sparse_info class.  
  
For example,  
	
    # example to use to_frovedis_crs_matrix_using_info
    
    import frovedis.dataframe as fdf
	
	# read_csv demo
	df = fdf.read_csv("./input/numbers.csv", names=['one', 'two', 'three', 'four'])
	df.show()

	crs_mat,info = df.to_frovedis_crs_matrix(['one', 'two', 'four'],
											 ['four'], need_info=True) # default dtype = np.float32
	print(type(info))

	crs_mat2 = df.to_frovedis_crs_matrix_using_info(info)
    print(type(crs_mat2))
	crs_mat2.debug_print()
    
Output  

	index   one     two     three   four
    0       10      10.23   F       0
    1       12      12.2    F       0
    2       13      34.8999 D       1
    3       15      100.12  A       2

    <class 'frovedis.dataframe.info.df_to_sparse_info'>
    
    <class 'frovedis.matrix.crs.FrovedisCRSMatrix'>
    
    Active Elements:  12
    matrix:
    num_row = 4, num_col = 5
    node 0
    local_num_row = 4, local_num_col = 5
    val : 10 10.23 1 12 12.2 1 13 34.9 1 15 100.12 1
    idx : 0 1 2 0 1 2 0 1 3 0 1 4
    off : 0 3 6 9 12

__Return Value__  
It returns a FrovedisCRSMatrix after converting Frovedis DataFrame.  

# SEE ALSO  

- **[DataFrame - Introduction](./df_intro.md)**  
- **[DataFrame - Generic Functions](./df_generic_func.md)**  
- **[DataFrame - Conversion Functions](./df_conversion.md)**  
- **[DataFrame - Math Functions](./df_math_func.md)**  
- **[DataFrame - Aggregate Functions](./df_agg_func.md)**   