% DataFrame Conversion and Sorting Functions
  
# NAME
  
DataFrame Conversion and Sorting Functions - functions related to conversion of datatypes and sorting are illustrated here.  
  
## SYNOPSIS  
    
    frovedis.dataframe.df.DataFrame(df = None, is_series = False)  
  
## Detailed Description  
  
### 1. asDF(df)  

__Parameters__  
**_df_**: It takes either a Frovedis DataFrame or a Pandas DataFrame or a Series instance.    

__Purpose__  
It creates a new Frovedis DataFrame after suitable conversion.   

For example,  

	# example to convert a pandas dataframe to frovedis dataframe object
    
    import pandas as pd
	import frovedis.dataframe as fdf
	
	# a dictionary
	peopleDF = {
				'Ename' : ['Michael', 'Andy', 'Tanaka', 'Raul', 'Yuta'],
				'Age' : [29, 30, 27, 19, 31],
				'Country' : ['USA', 'England', 'Japan', 'France', 'Japan'],
				'isMale': [False, False, False, False, True]
			   }
	# convert to pandas dataframe
	pd_df = pd.DataFrame(peopleDF)

	print("display the pandas dataframe type")
    print(type(pd_df))
    
    print("display type after conversion to frovedis dataframe")
	print(type(fdf.DataFrame.asDF(pd_df)))

Output  

    display the pandas dataframe type
    <class 'pandas.core.frame.DataFrame'>
    
    display type after conversion to frovedis dataframe
	<class 'frovedis.dataframe.df.DataFrame'>

For example,  
    
    # example to convert a Series object to Frovedis DataFrame object
    
    import pandas as pd
	import frovedis.dataframe as fdf
    
    # a Series object
	sdf1 = pd.Series([1, 2], dtype='int32')
	print("display series type")
    print(type(sdf1))
    
    print("display type after conversion to frovedis dataframe")5
	print(type(fdf.DataFrame.asDF(sdf1)))
    
Output  
    
    display series type
    <class 'pandas.core.series.Series'>
    
	display type after conversion to frovedis dataframe
    <class 'frovedis.dataframe.df.DataFrame'>

__Return Value__  
It returns a Frovedis DataFrame object after suitable conversion 


### 2. astype(dtype, copy = True, errors = 'raise', check_bool_like_string = False)  

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

    # example of using astype on a particular column
    
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
    
    # display the datatype of object after conversion of 'Age' column
	print(fd_df.astype({'Age':'int32'}).dtypes)
    
Output  

	Ename      object
	Age         int64
	Country    object
	isMale       bool
	dtype: object
	
	Ename      object
	Age         int32
	Country    object
	isMale      int32
	dtype: object

For example,  

	# example of using astype on object as a whole
    
    import pandas as pd
	import frovedis.dataframe as fdf
    
	# creating a dataFrame
	pd_df =  pd.DataFrame({'col1': [1, 2], 'col2': [3, 4]})
	fd_df1 = fdf.DataFrame(pf1)
	
    # displaying dataframe
	print(fd_df1.dtypes)

	# cast all columns to int32:
	fd_df2 = fd_df1.astype('int32')
    
    # display dataframe after conversion
	print(fd_df2.dtypes)
    
Output  

	col1    int64
	col2    int64
	dtype: object
	
	col1    int32
	col2    int32
	dtype: object

For example,  

    # example with parameter 'check_bool_like_string'
    
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
    
    #display the dataframes
    fd_df.show()
    
    # display the datatype
	print(fd_df.dtypes)
    
    #display the dataframes
    fd_df.astype({'isMale':'bool'}, check_bool_like_string = True).show()
    
    # display the datatype
    print(fd_df.astype({'isMale':'bool'}, check_bool_like_string = True).dtypes)
    
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
It returns a new DataFrame object with dtype converted as specified.  
  
  
### 3. copy(deep = True)  

__Parameters__  
**_deep_**: A boolean parameter to decide the type of copy operation. When this parameter is True, it creates a deep copy i.e. the copy includes copy of the original DataFrame's data and the indices. Currently this parameter doesn't support shallow copy (deep = False). (Default: True)  
  
__Purpose__  
It creates a deep copy of the Frovedis DataFrame object’s indices and data.  

For example,  

	# example to create a deep copy and changes on copy doesn't affect original object
    
    import pandas as pd
	import frovedis.dataframe as fdf
    
	peopleDF = {
				'Ename' : ['Michael', 'Andy', 'Tanaka', 'Raul', 'Yuta'],
				'Age' : [29, 30, 27, 19, 31],
				'Country' : ['USA', 'England', 'Japan', 'France', 'Japan'],
				'isMale': [False, False, False, False, True]
			   }
	pd_df = pd.DataFrame(peopleDF)
	fd_df = fdf.DataFrame(pd_df)
    
	# creating a deep copy
	fd_df_copy = fd_df.copy()
	
    print('displaying original dataframe object')
	fd_df.show()
	
    print('displaying copied dataframe object')
	fd_df_copy.show()
    
    # changing column Age to Age2 for the copied object
    fd_df_copy.rename({'Age':'Age2', inplace = True}
    
    print('displaying original dataframe object')
	fd_df.show()
	
    print('displaying copied dataframe object')
	fd_df_copy.show()
    
Output  

    displaying original dataframe object
	index   Ename   Age     Country isMale
	0       Michael 29      USA     0
	1       Andy    30      England 0
	2       Tanaka  27      Japan   0
	3       Raul    19      France  0
	4       Yuta    31      Japan   1

	displaying copied dataframe object
    index   Ename   Age     Country isMale
	0       Michael 29      USA     0
	1       Andy    30      England 0
	2       Tanaka  27      Japan   0
	3       Raul    19      France  0
	4       Yuta    31      Japan   1
    
    displaying original dataframe object
	index   Ename   Age     Country isMale
	0       Michael 29      USA     0
	1       Andy    30      England 0
	2       Tanaka  27      Japan   0
	3       Raul    19      France  0
	4       Yuta    31      Japan   1

	displaying copied dataframe object
    index   Ename   Age2     Country isMale
	0       Michael 29      USA     0
	1       Andy    30      England 0
	2       Tanaka  27      Japan   0
	3       Raul    19      France  0
	4       Yuta    31      Japan   1

**NOTE: changes are reflected only in copied DataFrame instance but not in original DataFrame instance.**  

__Return Value__  
It returns a deep copy of the DataFrame object of the same type.  


### 4. dropna(axis = 0, how = 'any', thresh = None, subset = None, inplace = False)  

__Parameters__  
**_axis_**: It accepts an integer value that can be 0 or 1. This parameter is used to determine whether rows or columns containing missing values are to be removed. (Default: 0)  
    - 0 : Drop rows which contain missing values.  
    - 1 : Drop columns which contain missing values.  
**_how_**: It accepts a string object to determine if row or column is removed from DataFrame, when we have at least one 'NaN' or all 'NaN'. (Default: 'any')  
    - 'any' : If any NaN values are present, drop that row or column.  
    - 'all' : If all values are NaN, drop that row or column.  
**_thresh_**: It accepts an integer as parameter which is the number of NaN values required for rows/columns to be dropped. (Default: None)  
**_subset_**: It accepts a python ndarray. It is the name of the labels along the other axis which is being considered.  
For example, if you are dropping rows, then these would be a list of columns. (Default: None)  
**_inplace_**: This parameter accepts a boolean value. When it is set to True, then it performs operation on the original Frovedis DataFrame object itself otherwise operation is performed on a new Frovedis DataFrame object. (Default: False)  

__Purpose__  
It is used to remove missing values from the Frovedis DataFrame.  

For example,  

	# example of using dropna with default values
    
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
	
	# drop the rows where at least one element is missing.
	print("After dropping nan values")
	fd_df.dropna().show()
    
Output  

	Before dropping nan values
	index   name            toy             born
	0       Alfred          NULL            NULL
	1       Batman          Batmobile       1940-04-25
	2       Catwoman        Bullwhip        NULL

	After dropping nan values
	index   name    toy             born
	1       Batman  Batmobile       1940-04-25

For example,  

    # example of using dropna along axis = 1
    
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
    
	# drop the columns where at least one nan value is present
	fd_df.dropna(axis=1)
    
Output  
	
	Before dropping nan values
	index   name            toy             born
	0       Alfred          NULL            NULL
	1       Batman          Batmobile       1940-04-25
	2       Catwoman        Bullwhip        NULL
    
    index   name
	0       Alfred
	1       Batman
	2       Catwoman

For example,  

	# example of using dropna with parameter 'how' = 'all'
    
    import pandas as pd
	import frovedis.dataframe as fdf
    
	# creating a pandas dataframe
	pd_df = pd.DataFrame({"name": [np.nan, 'Batman', 'Catwoman'],
					   "toy": [np.nan, 'Batmobile', 'Bullwhip'],
					   "born": [np.nan, "1940-04-25", np.nan]})
	
    # creating a frovedis dataframe from pandas dataframe
	fd_df = fdf.DataFrame(pd_df)
	
    # display frovedis dataframe
	print("Before dropping nan values")
	fd_df.show()
    
    # drop the rows where all elements are missing.
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

    # example of using drop with parameter 'thresh'
    
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

	# to keep only rows with at least 2 non-NA values.
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

    # example of using dropna with parameter 'subset'
    
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

    # example of using dropna with parameter 'inplace' = True
    
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

	# to modify the original dataframe having only valid entries
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


### 5. fillna(value = None, method = None, axis = None, inplace = False, limit = None, downcast = None)  
 
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

	# example of using fillna to replace NaN with 0 and other default parameters
    
    import pandas as pd
	import frovedis.dataframe as fdf
	
    pd_df = pd.DataFrame([[np.nan, 2, np.nan, 0],
					   [3, 4, np.nan, 1],
					   [np.nan, np.nan, np.nan, 5],
					   [np.nan, 3, np.nan, 4]],
					  columns=list("ABCD"))
	fd_df = fdf.DataFrame(pd_df)
	
    # display frovedis dataframe
	fd_df.show()
    
    # replace all NaN elements with 0s.
	fd_df.fillna(0).show()
    
Output  

	index   A       B       C       D
	0       NULL    2       NULL    0
	1       3       4       NULL    1
	2       NULL    NULL    NULL    5
	3       NULL    3       NULL    4

	index   A       B       C       D
	0       0       2       0       0
	1       3       4       0       1
	2       0       0       0       5
	3       0       3       0       4

For example,  

	# example to use fillna with negative value and inplace = True
    
    import pandas as pd
	import frovedis.dataframe as fdf
	
    pd_df = pd.DataFrame([[np.nan, 2, np.nan, 0],
					   [3, 4, np.nan, 1],
					   [np.nan, np.nan, np.nan, 5],
					   [np.nan, 3, np.nan, 4]],
					  columns=list("ABCD"))
	fd_df = fdf.DataFrame(pd_df)
	# display frovedis dataframe
	fd_df.show()
    
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


### 6. isna()  

__Purpose__  
This method is used to detect missing values in the Frovedis DataFrame.

It returns a boolean same-sized object indicating if the values are NA. NA values, such as None or numpy.NaN, gets mapped to True values. Everything else gets mapped to False values. Characters such as empty strings '' or numpy.inf are not considered NA values.  

For example,  

	# example of using isna() on a pandas DataFrame converted to Frovedis DataFrame
    
    import pandas as pd
	import frovedis.dataframe as fdf
    
    # show which entries in a DataFrame are NA.
	peopleDF = {
            'Ename' : ['Michael', None, 'Tanaka', 'Raul', ''],
            'Age' : [29, 30, 27, 19, 0],
            'Country' : ['USA', np.inf, 'Japan', np.nan, 'Japan'],
            'isMale': [False, False, False, False, True]
           }

	pd_df = pd.DataFrame(peopleDF)
    
	# creating a frovedis dataframe
	fd_df = fdf.DataFrame(pd_df)
    
	# to display frovedis dataframe
	fd_df.show()
    
	# to display fields which are only NA
	fd_df.isna().show()
    
Output  

	index   Ename   Age     Country isMale
	0       Michael 29      USA     0
	1       NULL    30      inf     0
	2       Tanaka  27      Japan   0
	3       Raul    19      NULL    0
	4               0       Japan   1

	index   Ename   Age     Country isMale
	0       0       0       0       0
	1       1       0       0       0
	2       0       0       0       0
	3       0       0       1       0
	4       0       0       0       0

For example,  

    # example of using isna() on a Series object converted to Frovedis DataFrame object
    
	import pandas as pd
	import frovedis.dataframe as fdf
    
    # frovedis dataframe from a Series object.
	ser = pd.Series([5, 6, np.NaN])
	fd_df = fdf.DataFrame(ser)
    
	# display frovedis dataframe
	fd_df.show()
	
    # display na values mapped to corresponding dataframe
	fd_df.isna().show()
    
Output  

	index   0
	0       5
	1       6
	2       NULL

	index   0
	0       0
	1       0
	2       1

__Return Value__  
It returns a new Frovedis DataFrame having all boolean values (0, 1) corresponding to each of the Frovedis DataFrame values depending on  whether it is a valid NaN (True i.e. 1) value or not (False i.e. 0).  


### 7. isnull()  

__Purpose__  
This method is used to detect missing values in the Frovedis DataFrame. It is an alias of isna().

It returns a boolean same-sized object indicating if the values are NA. NA values, such as None or numpy.NaN, gets mapped to True values. Everything else gets mapped to False values. Characters such as empty strings '' or numpy.inf are not considered NA values.  

For example,  

	# example to use isnull
    
    import pandas as pd
	import frovedis.dataframe as fdf
    
    # show which entries in a DataFrame are NA.
	peopleDF = {
				'Ename' : ['Michael', None, 'Tanaka', 'Raul', ''],
				'Age' : [29, 30, 27, 19, 0],
				'Country' : ['USA', 'England', 'Japan', np.nan, 'Japan'],
				'isMale': [False, False, False, False, True]
			   }
	pd_df = pd.DataFrame(peopleDF)
    
	# convert to frovedis dataframe
	fd_df = fdf.DataFrame(pd_df)
	
    # display frovedis dataframe
	fd_df.show()
	
    # displya na values mapped to corresponding dataframe
	fd_df.isnull().show()
    
Output  

	index   Ename   Age     Country isMale
	0       Michael 29      USA     0
	1       NULL    30      England 0
	2       Tanaka  27      Japan   0
	3       Raul    19      NULL    0
	4               0       Japan   1

	index   Ename   Age     Country isMale
	0       0       0       0       0
	1       1       0       0       0
	2       0       0       0       0
	3       0       0       1       0
	4       0       0       0       0

For example,  
	
	# example to use isnull on a frovedis dataframe converted from Series object
    
    import pandas as pd
	import frovedis.dataframe as fdf
    
    # frovedis dataframe from a Series object.
	ser = pd.Series([5, 6, np.NaN])
	fd_df = fdf.DataFrame(ser)
    
	# display frovedis dataframe
	fd_df.show()
	
    # display na values mapped to corresponding dataframe
	fd_df.isnull().show()
    
Output  

	index   0
	0       5
	1       6
	2       NULL

	index   0
	0       0
	1       0
	2       1

__Return Value__  
It returns a Frovedis DataFrame having boolean values (0, 1) corresponding to each of the Frovedis DataFrame value depending of whether it is a valid NaN (True i.e. 1) value or not (False i.e. 0).  

### 8. groupby(by = None, axis = 0, level = None, as_index = True, sort = True, group_keys = True, squeeze = False, observed = False, dropna = True)  

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

### 9. nsort(n, columns, keep='first', is_desc=False)  

__Parameters__  
**_n_**: It accepts a positive integer parameter which represents the number of rows to return.  
**_columns_**: It accepts a string type or list of strings type parameter which represents the column label(s) to order by.  
**_keep_**: It accepts a string object parameter. This parameter is used when there are duplicate values. (Default: 'first')  
	1. **'first'** : it prioritizes the first occurrence(s)  
	2. **'last'** : it prioritizes the last occurrence(s)  
	3. **'all'**  : it is used to not drop any duplicates, even when it means selecting more than **n** items.

**_is\_desc_**: It accepts a boolean value as parameter. This parameter decides the order to sort the data. When this parameter is explicitly passed as True, it sorts the data into descending order. (Default: False)  

__Purpose__  
It is used to return the top **n** rows ordered by the specified columns in ascending or descending order.
The columns that are not specified are returned as well, but not used for ordering.  

For example,  

	# example to use nsort with default parameters with n = 3 and columns = 'population'
    
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
	fd_df.nsort(3, 'population').show()
    
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

    index     population   GDP     alpha-2
    Italy     59000000     1937894 IT
    France    65000000     2583560 FR
    Malta     434000       12011   MT
    Maldives  434000       4520    MV
    Brunei    434000       12128   BN
    Iceland   337000       17036   IS
    Nauru     11300        182     NR
    Tuvalu    11300        38      TV
    Anguilla  11300        311     AI

    index     population   GDP     alpha-2
    Nauru     11300        182     NR
    Tuvalu    11300        38      TV
    Anguilla  11300        311     AI

For example,  
	
    # example to show the behavior of nsort with keep = 'last'
    
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
    
    fd_df.nsort(3, 'population', keep='last').show()
    
Output  

	index      population    GDP      alpha-2
	Italy      59000000      1937894  IT
	France     65000000      2583560  FR
	Malta      434000        12011    MT
	Maldives   434000        4520     MV
	Brunei     434000        12128    BN
	Iceland    337000        17036    IS
	Nauru      11300         182      NR
	Tuvalu     11300         38       TV
	Anguilla   11300         311      AI
    
    index      population    GDP      alpha-2
    Anguilla   11300         311      AI
    Tuvalu     11300         38       TV
    Nauru      11300         182      NR

For example,  

    # example to show the behavior of nsort with keep = 'all'
    
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

	fd_df.nsort(3, 'population', keep='all').show()
    
Output  

	index      population    GDP      alpha-2
	Italy      59000000      1937894  IT
	France     65000000      2583560  FR
	Malta      434000        12011    MT
	Maldives   434000        4520     MV
	Brunei     434000        12128    BN
	Iceland    337000        17036    IS
	Nauru      11300         182      NR
	Tuvalu     11300         38       TV
	Anguilla   11300         311      AI
    
    index      population    GDP     alpha-2
    Nauru      11300         182     NR
    Tuvalu     11300         38      TV
    Anguilla   11300         311     AI

For example,  

    # example to show use of list of columns with nsort
    
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

	fd_df.nsort(3, ['population', 'GDP']).show()
    
Output  

	index      population    GDP      alpha-2
	Italy      59000000      1937894  IT
	France     65000000      2583560  FR
	Malta      434000        12011    MT
	Maldives   434000        4520     MV
	Brunei     434000        12128    BN
	Iceland    337000        17036    IS
	Nauru      11300         182      NR
	Tuvalu     11300         38       TV
	Anguilla   11300         311      AI
    
    index      population    GDP      alpha-2
    Tuvalu     11300         38       TV
    Nauru      11300         182      NR
    Anguilla   11300         311      AI

__Return Value__  
It returns a Frovedis DataFrame object with **n** rows ordered by the specified columns in ascending or descending order.  


### 10. nlargest(n, columns, keep = 'first')  

__Parameters__  
**_n_**: It accepts an integer parameter which represents the number of rows to return.  
**_columns_**: It accepts a string type or list of strings type parameter which represents the column label(s) to order by.  
**_keep_**: It accepts a string object parameter. This parameter is used when there are duplicate values. (Default: 'first')  
	1. **'first'** : it prioritizes the first occurrence(s)  
	2. **'last'** : it prioritizes the last occurrence(s)  
	3. **'all'**  : it is used to not drop any duplicates, even when it means selecting more than **n** items.
  
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


### 11. nsmallest(n, columns, keep = 'first')  

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
    
    # To order by the smallest values in column “population” and then “GDP”, we can specify multiple columns like in the next example.
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


### 12. sort(columns = None, axis = 0, ascending = True, inplace = False, kind = 'radixsort', na\_position = 'last', \*\*kwargs)  

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


### 13. sort_index(axis = 0, ascending = True, inplace = False, kind = 'quicksort', na\_position = 'last')  

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


### 14. sort_values(by, axis = 0, ascending = True, inplace = False, kind = 'radixsort', na\_position = 'last')  

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


### 15. to_dict(orient = "dict", into = dict)  

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


### 16. to_frovedis_colmajor_matrix(t_cols, dtype = np.float32)  

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


### 17. to_frovedis_crs_matrix(t_cols, cat_cols, dtype = np.float32, need_info = False)  

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


### 18. to_frovedis_crs_matrix_using_info(info, dtype = np.float32)  

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


### 19. to_frovedis_rowmajor_matrix(t_cols, dtype = np.float32)  

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


### 20. to_numpy(dtype = None, copy = False, na_value = None)  

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


### 21. to_pandas_dataframe()  

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
    
    # below will display a Pandas dataframe type after conversion
	print(type(fd_df.to_pandas_dataframe()))
    
    # below will display a Pandas dataframe after conversion
	print(fd_df.to_pandas_dataframe())
    
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

# SEE ALSO  

DataFrame_Introduction, DataFrame_ConversionAndSorting, DataFrame_SelectionAndCombinations, DataFrame_BinaryOperators  