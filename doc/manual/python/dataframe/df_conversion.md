% DataFrame Conversion Functions
  
# NAME
  
DataFrame Conversion Functions - this manual contains all functions related to conversion of datatypes with respect to frovedis dataframe.  
  
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
  
### 1. DataFrame.asDF(df)  

__Parameters__  
**_df_**: It takes either a Frovedis DataFrame or a Pandas DataFrame or a Series instance.    

__Purpose__  
It creates a new Frovedis DataFrame after suitable conversion.   

**Using asDF() to create frovedis DataFrame from pandas DataFrame:**  

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
    # convert to pandas dataframe
    pd_df = pd.DataFrame(peopleDF)

    print("display the pandas dataframe type")
    print(type(pd_df))
    
    # convert pandas DataFrame to frovedis DataFrame object
    print("display type after conversion to frovedis dataframe")
    print(type(fdf.DataFrame.asDF(pd_df)))

Output  

    display the pandas dataframe type
    <class 'pandas.core.frame.DataFrame'>
    
    display type after conversion to frovedis dataframe
    <class 'frovedis.dataframe.df.DataFrame'>

**Using asDF() to create frovedis DataFrame from pandas Series:**  

For example,  
    
    import pandas as pd
    import frovedis.dataframe as fdf
    
    # create a Series object
    sdf1 = pd.Series([1, 2], dtype='int32')
    print("display series type")
    print(type(sdf1))
    
    # convert a Series object to Frovedis DataFrame object
    print("display type after conversion to frovedis dataframe")
    print(type(fdf.DataFrame.asDF(sdf1)))
    
Output  
    
    display series type
    <class 'pandas.core.series.Series'>
    
    display type after conversion to frovedis dataframe
    <class 'frovedis.dataframe.df.DataFrame'>

__Return Value__  
It returns a Frovedis DataFrame instance after suitable conversion 

### 2. DataFrame.to_dict(orient = "dict", into = dict)  

__Parameters__  
**_orient_**: It accepts a string object as parameter. It is used to determine the type of the values of the dictionary. (Default: 'dict')  
1. *'dict'* : dict like {column -> {index -> value}}  
2. *'list'* : dict like {column -> [values]}  

**_into_**: This parameter is used for mapping in the return value. Currently it only supports OrderedDict as return type. (Default: dict)  

__Purpose__  
It is used to convert the Frovedis DataFrame to a dictionary.  

**Creating frovedis DataFrame from pandas DataFrame:**  

For example,  

    import pandas as pd
    import frovedis.dataframe as fdf
    
    # a pandas dataframe
    pd_df = pd.DataFrame({'col1': [1, 2],
                          'col2': [0.5, 0.75]},
                          index=['row1', 'row2'])

    # create a frovedis dataframe
    fd_df = fdf.DataFrame(pd_df)
    
    # to display frovedis dataframe
    fd_df.show()

Output  

    index   col1    col2
    row1    1       0.5
    row2    2       0.75

**Converting frovedis DataFrame to dictionary:**  

For example,  

    print(fd_df.to_dict())
    
Output  

    OrderedDict([('col1', {'row1': 1, 'row2': 2}), ('col2', {'row1': 0.5, 'row2': 0.75})])
    
**Converting frovedis DataFrame to dictionary and using orient parameter:**  

For example,  
    
    # to_dict() demo with orient = 'list'
    print(fd_df.to_dict(orient = 'list'))
    
Output  

    OrderedDict([('index', array(['row1', 'row2'], dtype='<U4')), 
                ('col1', array([1, 2])), ('col2', array([0.5 , 0.75]))])

__Return Value__  
It returns a dictionary representing the Frovedis DataFrame instance. The resulting transformation depends on the 'orient' parameter.  

### 3. DataFrame.to_numpy(dtype = None, copy = False, na_value = None)  

__Parameters__  
**_dtype_** : It accepts the dtype parameter which decides the datatype of numpy ndarray. (Default: None)  
When it is None (not specified explicitly), it will be set as double (float64).  
**_copy_** : This is an unused parameter. (Default: False)  
**_na\_value_** : This is an unused parameter. (Default: None)  

__Purpose__  
This method is used to convert a frovedis dataframe into a numpy array.  

The parameters: "copy" and "na_value" are simply kept in to make the interface uniform to the pandas DataFrame.to_numpy().  
This is not used in the frovedis implementation.  

**Creating frovedis DataFrame from pandas DataFrame:**   

For example,  

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
        
    # display frovedis dataframe
    fd_df.show()

    # below will display a Frovedis dataframe type
    print(type(fd_df))

Output  

    index   Ename   Age     Country isMale
    0       Michael 29      USA     0
    1       Andy    30      England 0
    2       Tanaka  27      Japan   0
    3       Raul    19      France  0
    4       Yuta    31      Japan   1

    <class 'frovedis.dataframe.df.DataFrame'>

**Convert frovedis DataFrame into numpy array:**  

For example,  
    
    # to_numpy() demo to convert a frovedis dataframe into numpy array
    # below will display a numpy array type
    print(fd_df.to_numpy())
    
    # below will display a numpy array type
    print(type(fd_df.to_numpy()))

Output  

    [['Michael' '29' 'USA' '0']
     ['Andy' '30' 'England' '0']
     ['Tanaka' '27' 'Japan' '0']
     ['Raul' '19' 'France' '0']
     ['Yuta' '31' 'Japan' '1']]

    <<class 'numpy.ndarray'>

__Return Value__  
It returns a numpy ndarray that represents a Frovedis DataFrame instance. It has shape **(nRows , nCols)**.  

### 4. DataFrame.to_pandas()  

__Purpose__  
It converts a Frovedis DataFrame instance into a pandas DataFrame instance.  

**Creating frovedis DataFrame from pandas DataFrame:**  

For example,  

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
    
    # creating a frovedis dataframe
    fd_df = fdf.DataFrame(pd_df)
        
    # below will display a Frovedis dataframe
    fd_df.show()
    
    # below will display a Frovedis dataframe type
    print(type(fd_df))

Output  

    index   Ename   Age     Country isMale
    0       Michael 29      USA     0
    1       Andy    30      England 0
    2       Tanaka  27      Japan   0
    3       Raul    19      France  0
    4       Yuta    31      Japan   1   

    <class 'frovedis.dataframe.df.DataFrame'>

**Convert frovedis DataFrame to pandas DataFrame:**  

For example,  

    # to_pandas() demo to convert frovedis dataframe to pandas dataframe    
    print(fd_df.to_pandas())
    
    # below will display a pandas dataframe type after conversion
    print(type(fd_df.to_pandas()))

Output  

    index  Ename  Age  Country  isMale
    0      Michael   29      USA   False
    1         Andy   30  England   False
    2       Tanaka   27    Japan   False
    3         Raul   19   France   False
    4         Yuta   31    Japan    True  

    <class 'pandas.core.frame.DataFrame'>

__Return Value__  
It returns a pandas DataFrame instance after suitable conversion.  

### 5. DataFrame.to_frovedis_rowmajor_matrix(t_cols, dtype = np.float32)  

__Parameters__  
**_t\_cols_**: It accepts a list of string type argument where each of the member of the list is the name of the column labels.  
**_dtype_**: It accepts a dtype type argument which is the type of the resultant values. Currently, only float (float32) and double (float64) types are supported. (Default: np.float32)  

__Purpose__  
This method is used to convert a Frovedis DataFrame instance into FrovedisRowmajorMatrix instance.  

**Loading input file data into frovedis DataFrame:**  

For example,  
    
    import frovedis.dataframe as fdf

    # read_csv demo to get values
    df = fdf.read_csv("./input/numbers.csv", names = ['one', 'two', 'three', 'four'])

    # display frovedis dataframe
    df.show()

Output  

    index   one     two     three   four
    0       10      10.23   F       0
    1       12      12.2    F       0
    2       13      34.8999 D       1
    3       15      100.12  A       2

**Convert frovedis DataFrame into FrovedisRowmajorMatrix:**  

For example,  

    # to convert frovedis dataframe into frovedis rowmajor matrix
    row_mat = df.to_frovedis_rowmajor_matrix(['one', 'two'], dtype = np.float64)
    
    # display row_mat (row major matrix)
    row_mat.debug_print()
    
Output  
    
    matrix:
    num_row = 4, num_col = 2
    node 0
    node = 0, local_num_row = 4, local_num_col = 2, val = 10 10.23 12 12.2 13 34.9 15 100.12

__Return Value__  
It returns a FrovedisRowmajorMatrix instance after suitable conversion.  

### 6. DataFrame.to_frovedis_colmajor_matrix(t_cols, dtype = np.float32)  

__Parameters__  
**_t\_cols_**: It accepts a list of string type argument where each of the member of the list is the name of the column labels.  
**_dtype_**: It accepts a dtype type argument which is the type of the resultant values. Currently only float (float32) and double (float64)  types are supported. (Default: np.float32)  

__Purpose__  
This method converts a Frovedis DataFrame instance to FrovedisColmajorMatrix instance.  

**Loading input file data into frovedis DataFrame:**  

For example,  
    
    import frovedis.dataframe as fdf
    
    # read_csv demo
    df = fdf.read_csv("./input/numbers.csv", names=['one', 'two', 'three', 'four'])
    
    # display frovedis dataframe
    df.show()

Output  

    index   one     two     three   four
    0       10      10.23   F       0
    1       12      12.2    F       0
    2       13      34.8999 D       1
    3       15      100.12  A       2

**Convert frovedis DataFrame into FrovedisColmajorMatrix:**  

For example,  

    # to convert frovedis dataframe into frovedis colmajor matrix
    col_mat = df.to_frovedis_colmajor_matrix(['one', 'two']) # default dtype = np.float32
    
    # display col_mat
    col_mat.debug_print()
    
Output  
    
    matrix:
    num_row = 4, num_col = 2
    node 0
    node = 0, local_num_row = 4, local_num_col = 2, val = 10 12 13 15 10.23 12.2 34.9 100.12

__Return Value__  
It returns a FrovedisColmajorMatrix instance after converting the original Frovedis DataFrame instance.  

### 7. DataFrame.to_frovedis_crs_matrix(t_cols, cat_cols, dtype = np.float32, need_info = False)  

__Parameters__  
**_t\_cols_**: It accepts a list of string type argument where each of the member of the list is the name of the column labels.  
**_cat\_cols_**: It accepts a list of strings as parameter where strings are the column names. It stands for categorical columns. It represents how the repetitive values are distributed in the specified column. It adds number of columns corresponding to the number of distinct values in the specified column to the matrix. And values corresponding to the value of the specified column is set to 1 and remaining values are set to 0.  
**_dtype_**: It accepts a dataype as paramter which is the type of the resultant values. Currently only float (float32) and double (float64)  types are supported. (Default: np.float32)  
**_need\_info_**: It accepts a boolean value as parameter. When this is explicitly set to True, it returns an additional value of type 'df_to_sparse_info'. (Default: False)  

__Purpose__  
It converts a Frovedis DataFrame instance to FrovedisCRSMatrix instance.  

**Loading input file data into frovedis DataFrame:**  

For example,  
    
    import frovedis.dataframe as fdf
    import numpy as np

    # read_csv demo
    df = fdf.read_csv("./input/numbers.csv", names = ['one', 'two', 'three', 'four'])
    
    # display frovedis dataframe
    df.show()

Output  

    index   one     two     three   four
    0       10      10.23   F       0
    1       12      12.2    F       0
    2       13      34.8999 D       1
    3       15      100.12  A       2

**Convert frovedis DataFrame to FrovedisCRSMatrix:**  

For example,  

    # to convert frovedis datafrme to frovedis crs matrix
    crs_mat,info = df.to_frovedis_crs_matrix(['one', 'two', 'four'],
                                             ['three'],
                                             need_info = True) # default dtype = np.float32
    
    # display crs_matrix
    crs_mat.debug_print()
    
    # converting crs_matrix into display it in better form
    mat_t = crs_mat.to_scipy_matrix().todense()
    print(mat_t)    
    
Output  

    Active Elements:  12
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
It returns a FrovedisCRSMatrix instance after converting the original Frovedis DataFrame instance.  

### 8. DataFrame.to_frovedis_crs_matrix_using_info(info, dtype = np.float32)  

__Parameters__  
**_info_**: It accepts an instance of 'df_to_sparse_info' type.  
**_dtype_**: It accepts a dtype type argument which is the type of the resultant values. Currently only float (float32) and double (float64)  types are supported. (Default: np.float32)  
 
__Purpose__  
It converts a Frovedis DataFrame instance to FrovedisCRSMatrix instance and provided an info object of 'df_to_sparse_info' class.  

**Loading input file data into frovedis DataFrame:**    
  
For example,  
    
    import frovedis.dataframe as fdf

    # read_csv demo
    df = fdf.read_csv("./input/numbers.csv", names=['one', 'two', 'three', 'four'])
    
    # display frovedis dataframe
    df.show()

Output  

    index   one     two     three   four
    0       10      10.23   F       0
    1       12      12.2    F       0
    2       13      34.8999 D       1
    3       15      100.12  A       2

**Convert frovedis DataFrame to FrovedisCRSMatrix:**  

For example,  

    # to use to_frovedis_crs_matrix_using_info
    crs_mat,info = df.to_frovedis_crs_matrix(['one', 'two', 'four'],
                                             ['four'], 
                                             need_info=True) # default dtype = np.float32
    print(type(info))

    crs_mat2 = df.to_frovedis_crs_matrix_using_info(info)
    print(type(crs_mat2))
    
    # display crs_matrix
    crs_mat2.debug_print()
    
Output  

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
It returns a FrovedisCRSMatrix instance after converting Frovedis DataFrame instance.  

# SEE ALSO  

- **[DataFrame - Introduction](./df_intro.md)**  
- **[DataFrame - Generic Functions](./df_generic_func.md)**  
- **[DataFrame - Sorting Functions](./df_sort.md)**  
- **[DataFrame - Math Functions](./df_math_func.md)**  
- **[DataFrame - Aggregate Functions](./df_agg_func.md)**   