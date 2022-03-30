% DataFrame Introduction
  
# NAME
  
DataFrame - here refers to a Frovedis dataframe. It is a two-dimensional, size-mutable, potentially heterogeneous tabular data structure.  

# DESCRIPTION  
  
Dataframe is a two-dimensional data structure, i.e., data is aligned in a tabular fashion in rows and columns. This data structure also contains labeled axes (rows and columns). Arithmetic operations align on both row and column labels. It can be thought of as a dict-like container for Series objects or it can be thought of as an SQL table or a spreadsheet data representation.  
  
Features of DataFrame:  
- Columns can be of different types.  
- DataFrame is mutable i.e. the number of rows and columns can be increased or decreased.  
- DataFrame supports indexing and labeled columns name.  
- Supports arithmetic operations on rows and columns.  

Frovedis contains dataframe implementation over a client server architecture (which uses Vector Engine on server side to perform fast computations) where pandas dataframe will be converted to frovedis dataframe and then will be used to perform dataframe related operations.  

**In frovedis, currently dataframe can only support atomic data i.e. a column in a frovedis dataframe can only have primitve type values such as string, integer, float, double, boolean).**  


## Detailed description  

### 1. Creating frovedis DataFrame instance  

**a) Using DataFrame constructor**  

**DataFrame(df = None, is_series = False)**  

__Parameters__  
**_df_**: A pandas DataFrame instance or pandas Series instance. (Default: None)  
When this parameter is not None (specified explicitly), it will load the pandas dataframe (or series) to perform conversion into frovedis dataframe.  
**_is\_series_**: It is a boolean parameter which when set to true indicates that the dataframe consists of 
single column. (Default: False)  

__Purpose__  
It is used to create a Frovedis dataframe from the given pandas dataframe or series.  

For example,  

    # a pandas dictionary having key and values pairs
    peopleDF = {
                'Ename' : ['Michael', 'Andy', 'Tanaka', 'Raul', 'Yuta'], 
                'Age' : [29, 30, 27, 19, 31],
                'Country' : ['USA', 'England', 'Japan', 'France', 'Japan'],
                'isMale': [False, False, False, False, True]
               }

    # creating a pandas dataframe from key value pair
    pdf1 = pd.DataFrame(peopleDF)
    
    # creating frovedis dataframe
    fdf1 = fd.DataFrame(pdf1) 

    # display created frovedis dataframe
    fdf1.show()

Output  

    index   Age     Country Ename   isMale  
    0       29      USA     Michael 0  
    1       30      England Andy    0  
    2       27      Japan   Tanaka  0  
    3       19      France  Raul    0  
    4       31      Japan   Yuta    1  

__Return Value__  
It returns self reference.  

__Attributes__  

**_columns_**: It returns a list of string having the column labels of the frovedis dataframe.  

For example,  
    
    # fdf1 is same from above example
    print(fdf1.columns)
    
Output  

    ['Age', 'Country', 'Ename', 'isMale']
    
**_count_**: It returns an integer value having the number of rows in the frovedis dataframe  

For example,   
    
    # fdf1 is same from above example
    print(fdf1.count)  
  
Output  

    5  

**_dtypes_**: It returns the dtypes of all columns in the frovedis dataFrame. It returns a Series object with the 
data type of each column.  
  
For example,  

    # fdf1 is same from above example
    print(fdf1.dtypes)  
  
Output  

    Age         int64
    Country    object
    Ename      object
    isMale       bool
    dtype: object


**_ndim_**: It returns an integer representing the number of axes / array dimensions.  

For example,  

    # fdf1 is same from above example
    print(fdf1.ndim)  
  
Output  

    2  

**_shape_**: It returns a tuple representing the dimensions of the frovedis dataframe in form: **(nrows, ncols)**  

For example,  

    # fdf1 is same from above example
    print(fdf1.shape)  
  
Output  

    (5, 4)  
  
**_values_**: It returns a numpy representation of the frovedis dataframe.  

For example,  

    # fdf1 is same from above example 
    print(fdf1.values)  
  
Output  

    [['Michael' '29' 'USA' '0']
    ['Andy' '30' 'England' '0']
    ['Tanaka' '27' 'Japan' '0']
    ['Raul' '19' 'France' '0']
    ['Yuta' '31' 'Japan' '1']]   

**b) By loading data using read_csv() method**    

**read_csv(filepath_or_buffer, sep = ',', delimiter = None, header = "infer", names = None, index_col = None, usecols = None, squeeze = False, prefix = None, mangle_dupe_cols = True, dtype = None, na_values = None, verbose = False, comment = None, low_memory = True, rows_to_see = 1024, separate_mb = 1024)**   

__Parameters__  
_**filepath\_or\_buffer**_: It accepts any valid string value that contains the name of the file to access as parameter. The string path can be a URL as well. In case, a file is to be read, then the pathname can be absolute path or relative path to the current working directory of the file to be opened.  
Also, like pandas, it accepts os.PathLike instance for storing file path name as well.  
_**sep**_: It accepts string value as paramter that specifies delimiter to use. (Default: ',')  
**Currently, it supports only single character delimiter.**  
When it is None (if specified explicitly), it will use ',' as delimiter.  
_**delimiter**_: It accepts string value as paramter. It is an alias for 'sep' parameter. (Default: None)  
When it is None (not specified explicitly), it will set ',' as the delimiter.  

_**header**_: It accepts integer or string value that specifies the row number(s) to use as the column names, and the 
start of the data. (Default: 'infer')  
**Currently, frovedis supports only 'infer', 0 or None as values for this parameter.**  
The default behavior is to **'infer'** the column names:  
- **If no column names are passed**, then the behavior is identical to **header = 0** and column names are inferred from the first 
  line of the file.  
- **If column names are passed explicitly**, then the behavior is identical to **header = None** and column names are inferred from the explicitly specified list.  

_**names**_: It accepts array like input as parameter that specifies the list of column names to use. (Default: None)  
When it is None (not specified explicitly), it will get the column names from the first line of the file. Otherwise, it will override with the given list of column names. Since, **header = 0 or None is only supported in frovedis**, the first line of the file will be used as column names. **Duplicates in this list of column names are not allowed.**  
_**index\_col**_: It accepts boolean and integer type values as parameter. It specifies the column(s) to use as the row labels of the frovedis dataframe. (Default: None)  
- If boolean type value is passed, then currently it can only be set as False. Also, when **index_col = False**, it can be used to force to not use the first column as the index. **Also, 'names' parameter must be provided**.  
- It integer type value is passed, then it must be in the range **(0, nCols - 1)**, where **nCols** is the number of columns present in the input file.  
**Currently, multi-index is not supported in frovedis.**  
_**usecols**_: It accepts currently a list-like input of integer values as parameter. It specifies the subset of 
the columns to be used while loading data into frovedis dataframe. (Default: None)  
When it is None (not specified explicitly), it will use all columns while creating frovedis dataframe.  
Also, if the column names are provided and do not match the number of columns in the input file, then the list of names must be same in length as the list of 'usecols' parameter values. Otherwise, it raises an exception.  
_**squeeze**_: An unused parameter. (Default: False)  
_**prefix**_: It accepts a string value as parameter. It specifies the prefix which needs to be added to the column numbers. (Default: None)  
For example, 'X' for X0, X1, X2, ...  
**This is an optional paramter which will be used when column names are not provided and header is None (no header)**.  
When it is None (not specified explicitly), it will create list of numbers as column labels of length N - 1, where **'N'** is the number of columns in the input file.  
_**mangle\_dupe_cols**_: It accepts boolean value as parameter that specifies the action taken for duplicate columns present in the input file. (Default: True)  
- **When set to True (not specified explicitly) and if duplicate columns are present**, they will be specified as ‘X’, ‘X.1’, …’X.N’, rather than ‘X’…’X’.  
- **When set to False and duplicate columns are present**, it will raise an exeption **(not supported in frovedis yet)**.  
_**dtype**_: It accepts datatype or dictionary of columns which is used for explicitly setting type for data or columns in frovedis dataframe. (Default: None)  
_**na\_values**_: It accepts scalar value or array-like input as parameter that specifies the additional strings to recognize as  missing values (NA/NaN). Currently, it doesn't accept dictionary as value for this parameter. (Default: None)  
When it is None (not specified explicitly), it will interpret the list of following values as missing values:  

    ['null', 'NULL', 'nan', '-nan', 'NaN', '-NaN', 'NA', 'N/A', 'n/a']  

In case we want some other value to be interpreted as missing value, it is explicitly provided.  
_**verbose**_: It accepts a bollean values as parameter that specifies the log level to use. Its value is **False by 
default (for INFO mode)** and it can be set to **True (for DEBUG mode)**. This is used for getting the loading time logs from frovedis server. (Default: False)  
_**comment**_: This is an unused parameter. (Default: None)  
_**low\_memory**_: It accepts boolean value as parameter that specifies if the dataframe is to be loaded chunk wise. The chunk size then would be specified by the 'separate_mb' parameter. (Default: True)  

For example,  

    import time
    train_time = []
    start_time = time.time()
    df = fdf.read_csv("./input/numbers.csv", names=['one', 'two', 'three', 'four'], 
                       low_memory = False)
    train_time.append(round(time.time() - start_time, 4))
    print('loading time: 'train_time)

Output  

    loading time:  [0.0063]

Here, it is False, by default. So, it will load entire data into frovedis dataframe at once.  

For example,  

    import time
    train_time = []
    start_time = time.time()
    df = fdf.read_csv("./input/numbers.csv", names=['one', 'two', 'three', 'four'], 
                       low_memory = False)
    train_time.append(round(time.time() - start_time, 4))
    print('loading time: 'train_time)

Output  

    loading time:  [0.0021]

Here it is True, so it will load data chunk wise.  

_**rows\_to\_see**_: It accepts integer value as parameter. It is useful when creating frovedis dataframe by loading from large files.   While using big files, only some rows would be used to infer the data types of columns. This paramter specifies the number of such rows which will used to infer data types. (Default: 1024)  
_**separate\_mb**_: It accepts double (float64) value as parameter. It is a memory optimization parameter. (Default: 1024)  
- **If low_memory = True**, then internally frovedis dataframes of size **'separate_mb'** will be loaded separately and would be combined later into a single frovedis dataframe.  
- **If low_memory = False**, then complete frovedis dataframe will be loaded at once.  

__Purpose__  

This method reads a comma-separated values (csv) file into frovedis dataframe.  

**Note:- While loading file into frovedis dataframe, the file should not contain blank line at beginning.**  

The parameters: "squeeze" and "comment" are simply kept in to to make the interface uniform to the pandas read_csv() method. These are not used internally within the frovedis implementation.  

**File: numbers.csv**  

10,10.23,F,0  
12,12.20,nan,0  
13,34.90,D,1  
15,100.12,A,2  

For example,  

    # demo for read_csv() with csv file path
    import frovedis.dataframe as fdf
    df = fdf.read_csv("./input/numbers.csv")
    df.show()

Output  

    index   10      10.23   F       0
    0       12      12.2    NULL    0
    1       13      34.8999 D       1
    2       15      100.12  A       2

Here, by default it uses ',' delimiter while loading csv data into frovedis dataframe.  

Also, since no column names were provided and header = 'infer', so the newly created frovedis dataframe contains column names inferred from the first line of input file.  

For example,  

    # demo for read_csv() with header = None and column names not provided
    import frovedis.dataframe as fdf
    df = fdf.read_csv("./input/numbers.csv", header = None)
    df.show()

Output  

    index   0       1       2       3
    0       10      10.23   F       0
    1       12      12.2    NULL    0
    2       13      34.8999 D       1
    3       15      100.12  A       2

Here, it creates list of column numbers which is used as column names in newly created frovedis dataframe.  

For example,  

    # demo for read_csv() with header = None and column names are provided
    import frovedis.dataframe as fdf
    df = fdf.read_csv("./input/numbers.csv", names = ['one', 'two', 'three', 'four'], header = None)
    df.show()

Output  

    index   one     two     three   four
    0       10      10.23   F       0
    1       12      12.2    NULL    0
    2       13      34.8999 D       1
    3       15      100.12  A       2

Here, given list of names will be used as column names in the newly generated frovedis dataframe.   

Also, the input file, no duplicates column names were present in its header.  

For example,  

    # demo for read_csv() with prefix = True
    import frovedis.dataframe as fdf
    df = fdf.read_csv("./input/numbers.csv", header = None, prefix = "X")
    df.show()

Output  

    index   X0      X1      X2      X3
    0       10      10.23   F       0
    1       12      12.2    NULL    0
    2       13      34.8999 D       1
    3       15      100.12  A       2

For example,  

    # demo for read_csv() with usecols parameter
    import frovedis.dataframe as fdf
    df = fdf.read_csv("./input/numbers.csv",names=['one', 'two', 'three', 'four'], usecols = [1,2])
    df.show()

Output  

    index   two     three
    0       10.23   F
    1       12.2    NULL
    2       34.8999 D
    3       100.12  A

For example,  

    # demo for read_csv() with verbose = True
    import frovedis.dataframe as fdf
    df = fdf.read_csv("./input/numbers.csv", names = ['one', 'two', 'three', 'four'], verbose = True)
    df.show()

Output  

    [rank 0] load_text: MPI_File_read_all time: 0.0063569 sec
    [rank 0] load_text: find delimiter and align time: 2.01548e-05 sec
    [rank 0] make_dftable_loadtext::load_csv: 0.00673271 sec
    [rank 0] parse_words, words_to_number: 1.4253e-05 sec
    [rank 0] make_dftable_loadtext::parse_words, one: 0.000132132 sec
    [rank 0] parse_words, words_to_number: 2.76566e-05 sec
    [rank 0] make_dftable_loadtext::parse_words, two: 6.97458e-05 sec
    [rank 0] parse_words, extract_compressed_words: 1.748e-05 sec
    [rank 0] init_compressed, create dict locally: 1.55922e-05 sec
    [rank 0] init_compressed, merge_dict: 5.33368e-06 sec
    [rank 0] init_compressed, broadcast dict: 7.31274e-06 sec
    [rank 0] init_compressed, lookup: 1.22013e-05 sec
    [rank 0] make_dftable_loadtext::parse_words, three: 0.000158226 sec
    [rank 0] parse_words, words_to_number: 5.30668e-06 sec
    [rank 0] make_dftable_loadtext::parse_words, four: 3.63998e-05 sec
    [rank 0] make_dftable_loadtext: load separated df: 0.00718475 sec
    [rank 0] make_dftable_loadtext: union tables: 2.9644e-06 sec
    index   one     two     three   four
    0       10      10.23   F       0
    1       12      12.2    NULL    0
    2       13      34.8999 D       1
    3       15      100.12  A       2

For example,  

    # demo for read_csv() with dtype parameter
    import frovedis.dataframe as fdf
    df = fdf.read_csv("./input/numbers.csv", names = ['one', 'two', 'three', 'four'], 
                       dtype = {'two': int})
    df.show()

Output  

    index   one     two     three   four
    0       10      9823    F       0
    1       12      11820   NULL    0
    2       13      33890   D       1
    3       15      99812   A       2

For example,  

    # demo for read_csv() with index_col parameter with integer values
    import frovedis.dataframe as fdf
    df = fdf.read_csv("./input/numbers.csv", names = ['one', 'two', 'three', 'four'], index_col = 1)
    df.show()

Output  

    two     one     three   four
    10.23   10      F       0
    12.2    12      NULL    0
    34.8999 13      D       1
    100.12  15      A       2

For example,  

    # demo for read_csv() with index_col = False
    import frovedis.dataframe as fdf
    df = fdf.read_csv("./input/numbers.csv", names = ['one', 'two', 'three', 'four'], index_col = False)
    df.show()

Output  

    index   one     two     three   four
    0       10      10.23   E       0
    1       12      12.2    NULL    0
    2       13      34.8999 D       1
    3       15      100.12  A       2

Here, it will force the first column to not be used as index.  

In case the input file contains duplicate columns in header, then,  

**File: numbers_dupl.csv**  

score,score,grade,pass  
11.2,10.23,F,0  
21.2,12.20,nan,0  
43.6,34.90,D,1  
75.1,100.12,A,1  

For example,  

    # demo for read_csv() with mangle_dupe_cols = True
    import frovedis.dataframe as fdf
    df = fdf.read_csv("./input/numbers_dupl.csv",mangle_dupe_cols = True)
    df.show()

Output  

    index   score   score.1 grade   pass
    0       10      10.23   F       0
    1       12      12.2    NULL    0
    2       13      34.8999 D       1
    3       15      100.12  A       1

For example,  

**File: numbers.csv**  

10,10.23,F,0  
12,12.20,None,0  
13,34.90,D,1  
15,100.12,A,2  

The above file contains None as data, so by default it will be loaded in frovedis dataframe as string (inferred dtype).  

    # demo for read_csv() with na_values parameter provided
    import frovedis.dataframe as fdf
    df = fdf.read_csv("./input/numbers.csv", names = ['one', 'two', 'three', 'four'])
    df.show()

Output  

    index   one     two     three   four
    0       10      10.23   F       0
    1       12      12.2    None    0
    2       13      34.8999 D       1
    3       15      100.12  A       2

It can be interpreted by passing **"na_values"** parameter.  

    # demo for read_csv() with na_values parameter provided
    import frovedis.dataframe as fdf
    df = fdf.read_csv("./input/numbers.csv", names = ['one', 'two', 'three', 'four'], 
                       na_values = None)
    df.show()

Output  

    index   one     two     three   four
    0       10      10.23   F       0
    1       12      12.2    NULL    0
    2       13      34.8999 D       1
    3       15      100.12  A       2

__Return Value__  
It returns a frovedis DataFrame instance.  

### 2. Destructing the frovedis DataFrame instance   

**Using release() method**  
  
__Purpose__  
This method acts like a destructor.  
It is used to release dataframe pointer from server heap and it resets all its attributes to None.  
  
For example,  

    # fdf1 is same from above example 
    # releasing dataframe pointers
    fdf1.release()

__Return Value__  
It returns nothing.  


### 3. Public Member Functions of DataFrame  
  
DataFrame provides a lot of utilities to perform various operations. For simplicity we have categorized them into three lists: 
**_Basic functions_**, **_Application, groupby functions_**, **_Aggregate functions_** and **_Binary operator functions_**.  
  
#### a) List of Basic Functions  
  
**Basic functions** are further categorized into two sub parts - **_conversion and sorting functions_** and **_selection and combination functions_**. 
In the basic functions, we will discuss the common and essential functionalities of dataframe like conversion of dataframes, sorting of data, selection of specified data and combining two or more data.  
  
##### Conversion and Sorting Functions:  
Conversion and sorting functions are essential part of DataFrame which are basically used to perform conversions to other types and to narrow down the data as per specification.  

1. **asDF()** - Returns a Frovedis DataFrame after suitable conversion from other DataFrame types.  
2. **astype()** - Cast a selected column to a specified dtype.  
3. **copy()** - Make a copy of this object's indices and data.  
4. **dropna()** - Remove missing values.  
5. **fillna()** - Fill NA/NaN values using specified values.  
6. **isna()** - Detect missing values.  
7. **isnull()** - Is an alias of isna().  
8. **nlargest()** - Return the first n rows ordered by columns in descending order.  
9. **nsmallest()** - Return the first n rows ordered by columns in ascending order.  
10. **nsort()** - Return the top **n** rows ordered by the specified columns in ascending or descending order.  
11. **sort()** - Sort by the values on a column.  
12. **sort_index()** - Sort dataframes according to index.  
13. **sort_values()** - Sort by the values along either axis.  
14. **to_dict()** - Convert the dataframe to a dictionary.  
15. **to_numpy()** - Converts a frovedis dataframe to numpy array.  
16. **to_pandas()** - Returns a pandas dataframe object from frovedis dataframe.  
17. **to_frovedis_rowmajor_matrix()** - Converts a frovedis dataframe to FrovedisRowmajorMatrix.   
18. **to_frovedis_colmajor_matrix()** - Converts a frovedis dataframe to FrovedisColmajorMatrix.  
19. **to_frovedis_crs_matrix()** - Converts a frovedis dataframe to FrovedisCRSMatrix.  
20. **to_frovedis_crs_matrix_using_info()** - Converts a frovedis dataframe to FrovedisCRSMatrix provided an info object of 
df_to_sparse_info class.  


##### Selection and Combination Functions:  
DataFrame provides various facilities to easily select and combine together specified values and support join/merge operations.  
  
1. **add_index()** - Adds index column to the dataframe in-place.  
2. **append()** - Union of dataframes according to rows.  
3. **between()** - Filters rows according to the specified bound over a single column at a time.  
4. **drop()** - Drop specified labels from rows or columns.  
5. **drop_cols()** - Drop specified columns.  
6. **drop_duplicates()** - Return DataFrame with duplicate rows removed.  
7. **drop_rows()** - Drop specified rows.  
8. **filter()** - Subset the dataframe rows or columns according to the specified index labels.  
9. **get_index_loc()** - Returns integer location, slice or boolean mask for specified value in index column.  
10. **head()** - Return the first n rows.  
11. **insert()** - Insert column into DataFrame at specified location.  
12. **join()** - Join columns of another DataFrame.  
13. **merge()** - Merge dataframes according to specified parameters.  
14. **rename()** - Used to rename column.  
15. **rename_index()** - Renames index field (inplace).  
16. **reset_index()** - Reset the index.  
17. **set_index()** - Set the DataFrame index using existing columns.  
18. **show()** - Displays the selected dataframe values on console.  
19. **tail()** - Return the last n rows.  
20. **update_index()** - Updates/sets index values.  

#### b) Application, groupby Functions  
Such functions are used to perform operations like agg(), groupby() operations on frovedis dataframe.  

1. **agg()** - Aggregate using the specified functions and columns.  
2. **apply()** - Apply a function along an axis of the DataFrame.  
3. **groupby()** - Group dataframe using the specified columns.  


#### c) List of Aggregate Functions  
Aggregate functions of dataframe help to perform computations on the specified values and helps with efficient summarization of data. 
The calculated values gives insight into the nature of potential data.  
  
1. **abs()** - Return a DataFrame with absolute numeric value of each element.  
2. **countna()** - Count NA values for each column/row.  
3. **cov()** - Returns the pairwise covariance of columns, excluding missing values.  
4. **describe()** - Generate descriptive statistics.  
5. **mad()** - Returns the mean absolute deviation of the values over the requested axis.  
6. **max()** - Returns the maximum of the values over the requested axis.  
7. **mean()** - Returns the mean of the values over the requested axis.  
8. **median()** - Returns the median of the values over the requested axis.  
9. **min()** - Returns the minimum of the values over the requested axis.  
10. **mode()** - Returns the mode(s) of each element along the selected axis.  
11. **sem()** - Returns the unbiased standard error of the mean over requested axis.  
12. **std()** - Returns the sample standard deviation over requested axis.  
13. **sum()** - Returns the sum of the values over the requested axis.  
14. **var()** - Returns unbiased variance over requested axis.  

  
#### d) List of Binary Operator Functions  
DataFrame has methods for carrying out binary operations like add(), sub(), etc and related functions like radd(), rsub(), etc. for carrying out reverse binary operations.  
  
1. **add()** - Get addition of dataframe and other specified value. It is equivalent to dataframe + other.  
2. **div()** - Get floating division of dataframe and other specified value. It is equivalent to dataframe / other.  
3. **floordiv()** - Get integer division of dataframe and other specified value. It is equivalent to dataframe // other.  
4. **mod()** - Get modulo of dataframe and other specified value. It is equivalent to dataframe % other.  
5. **mul()** - Get multiplication of dataframe and other specified value. It is equivalent to dataframe \* other.  
6. **pow()** - Get exponential power of dataframe and other specified value. It is equivalent to dataframe \*\* other.  
7. **sub()** - Get subtraction of dataframe and other specified value. It is equivalent to dataframe - other.  
8. **truediv()** - Get floating division of dataframe and other specified value. It is equivalent to dataframe / other.  
9. **radd()** - Get addition of other specified value and dataframe. It is equivalent to other + dataframe.  
10. **rdiv()** - Get floating division of other specified value and dataframe. It is equivalent to other / dataframe.  
11. **rfloordiv()** - Get integer division of other specified value and dataframe. It is equivalent to other // dataframe.  
12. **rmod()** - Get modulo of other specified value and dataframe. It is equivalent to other % dataframe.  
13. **rmul()** - Get multiplication of other specified value and dataframe. It is equivalent to other \* dataframe.  
14. **rpow()** - Get exponential power of other specified value and dataframe. It is equivalent to other \*\* dataframe.  
15. **rsub()** - Get subtraction of other specified value and dataframe. It is equivalent to other - dataframe.  
16. **rtruediv()** - Get floating division of other specified value and dataframe. It is equivalent to other / dataframe.  


### Indexing in frovedis Dataframe  

Indexing means simply selecting particular rows and columns of data from a dataframe. Indexing can also be known as Subset Selection.  

There are a lot of ways to pull the rows and columns from a dataframe. There are some indexing method which help in getting an element from a dataframe. These indexing methods appear very similar but behave very differently. **Currently, frovedis supports only one type of indexing**. It is:  

- **Dataframe.[ ]**: This function also known as indexing operator. It helps in filtering rows and columns from a dataframe. Also, it returns a pandas dataframe as output.  

**Indexing using [ ]**     

Indexing operator is used to refer to the square brackets following an object.  

In order to select a single column, we simply put the name of the column in-between the brackets.  

For example,  

    import pandas as pd
    import frovedis.dataframe as fdf
    
    # a pandas dataframe from key value pair
    pdf2 = pd.DataFrame({ "Last Bonus": [5, 2, 2, 4], 
                          "Bonus": [5, 2, 2, 4], 
                          "Last Salary": [58, 59, 63, 58], 
                          "Salary": [60, 60, 64, 59]
                        }, index= ["John", "Marry", "Sam", "Jo"]
                       ) 
    # creating frovedis dataframe
    fdf1 = fdf.DataFrame(pdf2)
    
    # display created frovedis dataframe
    fdf1.show()

Output  

    index   Last Bonus   Bonus   Last Salary  Salary
    John    5            5       58           60
    Marry   2            2       59           60
    Sam     2            2       63           64
    Jo      4            4       58           59

For example,  

    # selcting a single column
    fdf1['Bonus'].show()

Output  

    index   Bonus
    John    5
    Marry   2
    Sam     2
    Jo      4

In order to select multiple columns, we have to pass a list of columns in an indexing operator.  

For example,  

    # selecting multiple columns
    fdf1[['Bonus','Salary,']].show()
    
Output  

    index   Bonus   Salary
    John    5       60
    Marry   2       60
    Sam     2       64
    Jo      4       59

In order to select rows between given range using indexing operator, it can be done as follows:  

For example,  

    # filtering dataframe using slice operation with row numbers
    fdf1[1:2].show()

Output  

    index   Last Bonus   Bonus   Last Salary   Salary
    Marry   2            2       59            60

For example,  

    # filtering dataframe using slice operation with row labels
    fdf1['John':'Sam'].show()

Output  

    index   Last Bonus   Bonus   Last Salary   Salary
    Marry   2            2       59            60

**Note:- Frovedis also supports use of attribute operators to filter/select column(s) in dataframe**

For example, in previous example which selects **Bonus** column from the dataframe can be expressed as:  

    fdf1.Bonus

This returns a FrovedisColumn instance.  

Filtering can be done with help of attribute operators as follows:    

For example,  

    # filtering data using given condition
    fdf1[fdf1.Bonus == 2].show()

Output  

    index   Last Bonus  Bonus  Last Salary  Salary
    Marry   2           2      59           60
    Sam     2           2      63           64

For example,  

    # filtering data using '>' operator
    fdf1[fdf1.Bonus > 2].show()

Output  

    index   Last Bonus   Bonus   Last Salary   Salary
    John    5            5       58            60
    Jo      4            4       58            59

For example,  

    # filtering data using '<' operator
    fdf1[fdf1.Bonus < 5].show()

Output  

    index   Last Bonus   Bonus   Last Salary   Salary
    Marry   2            2       59            60
    Sam     2            2       63            64
    Jo      4            4       58            59

For example,  

    # filtering data using '!=' operator
    fdf1[fdf1.Bonus != 2].show()

Output  

    index   Last Bonus    Bonus   Last Salary    Salary
    John    5             5       58             60
    Jo      4             4       58             59

For example,  

    # using '&' operation to filter data
    fdf1[(fdf1.Bonus == 5) & (fdf1.Salary == 60)].show()

Output  

    index   Last Bonus   Bonus   Last Salary   Salary
    John    5            5       58            60

For example,  

    # using '|' operation to filter data
    fdf1[(fdf1.Bonus == 5) | (fdf1.Salary == 60)].show()

Output  

    index   Last Bonus    Bonus   Last Salary   Salary
    John    5             5       58            60
    Marry   2             2       59            60

For example,  

    # using '~' operation to filter data
    fdf1[~(fdf1.Bonus == 5)].show()

Output  

    index   Last Bonus   Bonus   Last Salary   Salary
    Marry   2            2       59            60
    Sam     2            2       63            64
    Jo      4            4       58            59


# SEE ALSO  

- **[DataFrame - Selection and Combinations](./DataFrame_SelectionAndCombinations.md)**  
- **[DataFrame - Conversion, Missing data handling, Sorting Functions](./DataFrame_ConversionAndSorting.md)**  
- **[DataFrame - Function Application, Groupby](./DataFrame_FunctionApplicationGroupby.md)**  
- **[DataFrame - Binary Operators](./DataFrame_BinaryOperators.md)**  
- **[DataFrame - Aggregate Functions](./DataFrame_AggregateFunctions.md)**  