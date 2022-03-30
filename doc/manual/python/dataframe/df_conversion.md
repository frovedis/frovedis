% DataFrame Conversion and Sorting Functions
  
# NAME
  
DataFrame Conversion and Sorting Functions - this manual contains all functions related to conversion of datatypes with respect to frovedis dataframe.  
  
## DESCRIPTION  
    
In frovedis during data pre-processing and manipulation, there might be the need to change the data type of the variable to a particular type for better cleaning and understanding of the data.

For this inter-conversion within the variables, frovedis dataframe offer various conversion functions like asDF(), to_dict(), to_numpy(),etc.  
  
## Public Member Functions  
    1. asDF(df)
    2. to_dict(orient = "dict", into = dict)
    3. to_numpy(dtype = None, copy = False, na_value = None)
    4. to_pandas()
    5. to_frovedis_rowmajor_matrix(t_cols, dtype = np.float32)
    6. to_frovedis_colmajor_matrix(t_cols, dtype = np.float32)
    7. to_frovedis_crs_matrix(t_cols, cat_cols, dtype = np.float32, need_info = False)
    8. to_frovedis_crs_matrix_using_info(info, dtype = np.float32)

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

### 2. to_dict(orient = "dict", into = dict)  

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

### 3. to_numpy(dtype = None, copy = False, na_value = None)  

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

### 4. to_pandas()  

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

### 5. to_frovedis_rowmajor_matrix(t_cols, dtype = np.float32)  

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

### 6. to_frovedis_colmajor_matrix(t_cols, dtype = np.float32)  

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

### 7. to_frovedis_crs_matrix(t_cols, cat_cols, dtype = np.float32, need_info = False)  

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

### 8. to_frovedis_crs_matrix_using_info(info, dtype = np.float32)  

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
- **[DataFrame - Sorting Functions](./df_sort.md)**  
- **[DataFrame - Math Functions](./df_math_func.md)**  
- **[DataFrame - Aggregate Functions](./df_agg_func.md)**   