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

**DataFrame(df = None)**  

__Parameters__  
**_df_**: A pandas DataFrame instance or pandas Series instance. (Default: None)  
When this parameter is not None (specified explicitly), it will load the pandas dataframe (or series) to perform conversion into frovedis dataframe.  

__Purpose__  
It is used to create a Frovedis dataframe from the given pandas dataframe or series.  

**Constructing frovedis DataFrame from pandas DataFrame:**   

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
_**index\_col**_: It accepts boolean and integer type values as parameter. It specifies the column(s) to use as the row labels of the frovedis dataframe. **Currently, multi-index is not supported in frovedis.** (Default: None)  

- If boolean type value is passed, then currently it can only be set as False. Also, when **index_col = False**, it can be used to force to not use the first column as the index. **Also, 'names' parameter must be provided**.  
- It integer type value is passed, then it must be in the range **(0, nCols - 1)**, where **nCols** is the number of columns present in the input file.  

_**usecols**_: It accepts currently a list-like input of integer values as parameter. It specifies the subset of 
the columns to be used while loading data into frovedis dataframe. (Default: None)  
When it is None (not specified explicitly), it will use all columns while creating frovedis dataframe.  
Also, if the column names are provided and do not match the number of columns in the input file, then the list of names must be same in length as the list of 'usecols' parameter values. Otherwise, it raises an exception.  
_**squeeze**_: An unused parameter. (Default: False)  
_**prefix**_: It accepts a string value as parameter. It specifies the prefix which needs to be added to the 
column numbers. (Default: None)  

    For example, 'X' for X0, X1, X2, ...  

**This is an optional paramter which will be used when column names are not provided and header is None (no header)**.  
When it is None (not specified explicitly), it will create list of numbers as column labels of length N - 1, where **'N'** is the number of columns in the input file.  
_**mangle\_dupe_cols**_: It accepts boolean value as parameter that specifies the action taken for duplicate columns present in the input file. (Default: True)  

- **When set to True (not specified explicitly) and if duplicate columns are present**, they will be specified as ‘X’, ‘X.1’, …’X.N’, rather than ‘X’…’X’.  
- **When set to False and duplicate columns are present**, it will raise an exeption **(not supported in frovedis yet)**.  

_**dtype**_: It accepts datatype or dictionary of columns which is used for explicitly setting type for data or columns in frovedis dataframe. (Default: None)  
For boolwan type, frovedis supports: 

    True/False, On/OFF, 0/1, yes/No, y/n 

User must provide this, then frovedis can convert to boolean type. **Make sure user provides correct dtype , otherwise it will cause issues while creating frovedis dataframe**.  
_**na\_values**_: It accepts scalar value or array-like input as parameter that specifies the additional strings to recognize as  missing values (NA/NaN). Currently, it does not accept dictionary as value for this parameter. (Default: None)  
When it is None (not specified explicitly), it will interpret the list of following values as missing values:  

    ['null', 'NULL', 'nan', '-nan', 'NaN', '-NaN', 'NA', 'N/A', 'n/a']  

In case we want some other value to be interpreted as missing value, it is explicitly provided.  
_**verbose**_: It accepts a bollean values as parameter that specifies the log level to use. Its value is **False by 
default (for INFO mode)** and it can be set to **True (for DEBUG mode)**. This is used for getting the loading time logs from frovedis server. It is useful for debugging purposes. (Default: False)  
_**comment**_: This is an unused parameter. (Default: None)  
_**low\_memory**_: It accepts boolean value as parameter that specifies if the dataframe is to be loaded chunk wise. The chunk size then would be specified by the 'separate_mb' parameter. (Default: True)  
_**rows\_to\_see**_: It accepts integer value as parameter. It is useful when creating frovedis dataframe by loading from large files.   While using big files, only some rows would be used to infer the data types of columns. This paramter specifies the number of such rows which will used to infer data types. (Default: 1024)  
_**separate\_mb**_: It accepts double (float64) value as parameter. It is a memory optimization parameter. (Default: 1024)  

- **If low_memory = True**, then internally frovedis dataframes of size **'separate_mb'** will be loaded separately and would be combined later into a single frovedis dataframe.  
- **If low_memory = False**, then complete frovedis dataframe will be loaded at once.  

__Purpose__  

This method reads a comma-separated values (csv) file into frovedis dataframe.  

**Note:- While loading file into frovedis dataframe, the file should not contain blank line at beginning.**  

The parameters: "squeeze" and "comment" are simply kept in to to make the interface uniform to the pandas read_csv() method. These are not used internally within the frovedis implementation.  

**Using read_csv() to load data from given csv file path into frovedis dataframe: **  

**File: numbers.csv**  

10,10.23,F,0  
12,12.20,nan,0  
13,34.90,D,1  
15,100.12,A,2  

In order to load csv file will into frovedis dataframe,  

For example,  

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

**Using read_csv() with header = None and no column name is provided: **  

For example,  

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

**Using read_csv() with header = None. Also, column names are provided: **  

For example,  

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

Also, the input file contained no duplicate column names in its header.  

**Using read_csv() with prefix = True: **  

For example,  

    import frovedis.dataframe as fdf
    df = fdf.read_csv("./input/numbers.csv", header = None, prefix = "X")
    df.show()

Output  

    index   X0      X1      X2      X3
    0       10      10.23   F       0
    1       12      12.2    NULL    0
    2       13      34.8999 D       1
    3       15      100.12  A       2

Here, 'prefix' is an optional paremeter which will be in use when header = None and names = None.  
It will use column numbers by default when prefix = None.  

**To load specific columns in frovedis dataframe, usecols parameter will be used: **  

For example,  

    import frovedis.dataframe as fdf
    df = fdf.read_csv("./input/numbers.csv",names=['one', 'two', 'three', 'four'], usecols = [1,2])
    df.show()

Output  

    index   two     three
    0       10.23   F
    1       12.2    NULL
    2       34.8999 D
    3       100.12  A

**Using read_csv() with dtype parameter: **  

**File: numbers.csv**  

10,10.23,E,TRUE  
12,12.20,nan,TRUE  
13,34.90,D,FALSE  
15,100.12,A,FALSE  

In above data, **TRUE/FALSE** in python can be converted to boolean **(0/1)** type values using dtype parameter.  

For example,  

    import frovedis.dataframe as fdf
    df = fdf.read_csv("./input/numbers.csv", names = ['one', 'two', 'three', 'four'], 
                       dtype = {'four': bool})
    df.show()

Output  

    index   one     two     three   four
    0       10      10.23   E       1
    1       12      12.2    NULL    1
    2       13      34.8999 D       0
    3       15      100.12  A       0

**In order to use second column as index, index_col parameter will be used: **  

For example,  

    import frovedis.dataframe as fdf
    df = fdf.read_csv("./input/numbers.csv", names = ['one', 'two', 'three', 'four'], index_col = 1)
    df.show()

Output  

    two     one     three   four
    10.23   10      F       0
    12.2    12      NULL    0
    34.8999 13      D       1
    100.12  15      A       2

**When index_col = False, it will force to not use the first column as the index: **  

For example,  

    import frovedis.dataframe as fdf
    df = fdf.read_csv("./input/numbers.csv", names = ['one', 'two', 'three', 'four'], index_col = False)
    df.show()

Output  

    index   one     two     three   four
    0       10      10.23   E       0
    1       12      12.2    NULL    0
    2       13      34.8999 D       1
    3       15      100.12  A       2

**Using mangle_dupe_cols = True, when input file contains duplicate columns in header:**  

**File: numbers_dupl.csv**  

score,score,grade,pass  
11.2,10.23,F,0  
21.2,12.20,nan,0  
43.6,34.90,D,1  
75.1,100.12,A,1  

The input file contains duplicate column labels. To make sure while loading data into frovedis dataframe, mangle_dupe_cols will be set as True.  

For example,  

    import frovedis.dataframe as fdf
    df = fdf.read_csv("./input/numbers_dupl.csv",mangle_dupe_cols = True)
    df.show()

Output  

    index   score   score.1 grade   pass
    0       10      10.23   F       0
    1       12      12.2    NULL    0
    2       13      34.8999 D       1
    3       15      100.12  A       1

**Using read_csv() with na_values parameter:**  

**File: numbers.csv**  

11,10.23,E,0,1  
12,12.20,F,0,1  
10,34.90,D,1,0  
15,10,A,10,0  
10,10.23,B,0,1  
10,10.23,B,0,1  

When using na_values = None (by default), it will interpret the values nan, Nan, NULL, etc. as missing values (explained in parameter description)  

    # demo for read_csv() with na_values = None by default
    import frovedis.dataframe as fdf
    df = fdf.read_csv("./input/numbers.csv", na_values = None)
    df.show()

Output  

    index   11      10.23   E       0       1
    0       12      12.2    F       0       1
    1       10      34.8999 D       1       0
    2       15      10      A       10      0
    3       10      10.23   B       0       1
    4       10      10.23   B       0       1

**When list of strings are provided to be set as missing values: **  

For example,  

    import frovedis.dataframe as fdf
    df = fdf.read_csv("./input/numbers.csv", na_values = ['10','1'])
    df.show()

Output  

    index   11      10.23   E       0       1
    0       12      12.2    F       0       NULL
    1       NULL    34.8999 D       NULL    0
    2       15      NULL    A       NULL    0
    3       NULL    10.23   B       0       NULL
    4       NULL    10.23   B       0       NULL

It will interpret the list of values **['10','1']** as missing values (explicitly provided) while loading input file into frovedis dataframe.  

**When a string is to be set as missing value:**  

For example,  

    import frovedis.dataframe as fdf
    df = fdf.read_csv("./input/numbers.csv", na_values = '10')
    df.show()

Output  

    index   11      10.23   E       0       1
    0       12      12.2    F       0       1
    1       NULL    34.8999 D       1       0
    2       15      NULL    A       NULL    0
    3       NULL    10.23   B       0       1
    4       NULL    10.23   B       0       1

It will interpret the string **'10'** as missing value (explicitly provided) while loading input file into frovedis dataframe.  

__Return Value__  
It returns a frovedis DataFrame instance.  

### 2. Destructing the frovedis DataFrame instance   

**Using release() method**  
  
__Purpose__  
This method acts like a destructor.  
It is used to release dataframe pointer from server heap and it resets all its attributes to None.  
  
**Releasing dataframe pointers from frovedis:**  

For example,  

    # fdf1 is same from above example 
    fdf1.release()

__Return Value__  
It returns nothing.  


### 3. Public Member Functions of DataFrame  
  
DataFrame provides a lot of utilities to perform various operations. For simplicity we have categorized them into three lists: 
**_Basic functions_**, **_Application, groupby functions_**, **_Aggregate functions_** and **_Binary operator functions_**.  
  
#### a) List of Basic Functions  

**Basic functions** are further categorized into two sub parts - **_conversion and sorting functions_** and **_selection and combination functions_**.  

In the basic functions, we will discuss the common and essential functionalities of dataframe like conversion of dataframes, sorting of data, selection of specified data and combining two or more data.  

##### Conversion Functions:  
Conversion functions are essential part of DataFrame which are basically used to perform conversions to other types and to narrow down the data as per specification.  

1. **asDF()** - Returns a Frovedis DataFrame after suitable conversion from other DataFrame types.  
2. **to_dict()** - Convert the dataframe to a dictionary.  
3. **to_numpy()** - Converts a frovedis dataframe to numpy array.  
4. **to_pandas()** - Returns a pandas dataframe object from frovedis dataframe.  
5. **to_frovedis_rowmajor_matrix()** - Converts a frovedis dataframe to FrovedisRowmajorMatrix.   
6. **to_frovedis_colmajor_matrix()** - Converts a frovedis dataframe to FrovedisColmajorMatrix.  
7. **to_frovedis_crs_matrix()** - Converts a frovedis dataframe to FrovedisCRSMatrix.  
8. **to_frovedis_crs_matrix_using_info()** - Converts a frovedis dataframe to FrovedisCRSMatrix provided an info object of 
df_to_sparse_info class.  

##### Sorting Functions:  
Sorting functions are essential part of DataFrame.  

1. **nlargest()** - Return the first n rows ordered by columns in descending order.  
2. **nsmallest()** - Return the first n rows ordered by columns in ascending order.  
3. **sort()** - Sort by the values on a column.  
4. **sort_index()** - Sort dataframes according to index.  
5. **sort_values()** - Sort by the values along either axis.  

##### Generic Functions:  
DataFrame provides various facilities to easily select and combine together specified values and support join/merge operations.  

1. **add_index()** - Adds index column to the dataframe in-place.  
2. **append()** - Union of dataframes according to rows.  
3. **apply()** - Apply a function along an axis of the DataFrame.  
4. **astype()** - Cast a selected column to a specified dtype.  
5. **between()** - Filters rows according to the specified bound over a single column at a time.  
6. **copy()** - Make a copy of the indices and data of this object.  
7. **countna()** - Count NA values for each column/row.  
8. **describe()** - Generate descriptive statistics.  
9. **drop()** - Drop specified labels from rows or columns.  
10. **drop_duplicates()** - Return DataFrame with duplicate rows removed.  
11. **dropna()** - Remove missing values.  
12. **fillna()** - Fill NA/NaN values using specified values.  
13. **filter()** - Subset the dataframe rows or columns according to the specified index labels.  
14. **get_index_loc()** - Returns integer location, slice or boolean mask for specified value in index column.  
15. **head()** - Return the first n rows.  
16. **insert()** - Insert column into DataFrame at specified location.  
17. **isna()** - Detect missing values.  
18. **isnull()** - Is an alias of isna().  
19. **join()** - Join columns of another DataFrame.  
20. **merge()** - Merge dataframes according to specified parameters.  
21. **rename()** - Used to rename column.  
22. **rename_index()** - Renames index field (inplace).  
23. **reset_index()** - Reset the index.  
24. **set_index()** - Set the DataFrame index using existing columns.  
25. **show()** - Displays the selected dataframe values on console.  
26. **tail()** - Return the last n rows.  
27. **update_index()** - Updates/sets index values.  

#### b) List of Aggregate Functions  
Aggregate functions of dataframe help to perform computations on the specified values and helps with efficient summarization of data. 
The calculated values gives insight into the nature of potential data.  

1. **agg()** - Aggregate using the specified functions and columns.  
2. **cov()** - Returns the pairwise covariance of columns, excluding missing values.  
3. **mad()** - Returns the mean absolute deviation of the values over the requested axis.  
4. **max()** - Returns the maximum of the values over the requested axis.  
5. **mean()** - Returns the mean of the values over the requested axis.  
6. **median()** - Returns the median of the values over the requested axis.  
7. **min()** - Returns the minimum of the values over the requested axis.  
8. **mode()** - Returns the mode(s) of each element along the selected axis.  
9. **sem()** - Returns the unbiased standard error of the mean over requested axis.  
10. **std()** - Returns the sample standard deviation over requested axis.  
11. **sum()** - Returns the sum of the values over the requested axis.  
12. **var()** - Returns unbiased variance over requested axis.  

#### c) List of Math Functions  
DataFrame has methods for carrying out binary operations like add(), sub(), etc and related functions like radd(), rsub(), etc. for carrying out reverse binary operations.  

1. **abs()** - Return a DataFrame with absolute numeric value of each element.  
2. **add()** - Get addition of dataframe and other specified value. It is equivalent to dataframe + other.  
3. **div()** - Get floating division of dataframe and other specified value. It is equivalent to dataframe / other.  
4. **floordiv()** - Get integer division of dataframe and other specified value. It is equivalent to dataframe // other.  
5. **mod()** - Get modulo of dataframe and other specified value. It is equivalent to dataframe % other.  
6. **mul()** - Get multiplication of dataframe and other specified value. It is equivalent to dataframe \* other.  
7. **pow()** - Get exponential power of dataframe and other specified value. It is equivalent to dataframe \*\* other.  
8. **sub()** - Get subtraction of dataframe and other specified value. It is equivalent to dataframe - other.  
9. **truediv()** - Get floating division of dataframe and other specified value. It is equivalent to dataframe / other.  
10. **radd()** - Get addition of other specified value and dataframe. It is equivalent to other + dataframe.  
11. **rdiv()** - Get floating division of other specified value and dataframe. It is equivalent to other / dataframe.  
12. **rfloordiv()** - Get integer division of other specified value and dataframe. It is equivalent to other // dataframe.  
13. **rmod()** - Get modulo of other specified value and dataframe. It is equivalent to other % dataframe.  
14. **rmul()** - Get multiplication of other specified value and dataframe. It is equivalent to other \* dataframe.  
15. **rpow()** - Get exponential power of other specified value and dataframe. It is equivalent to other \*\* dataframe.  
16. **rsub()** - Get subtraction of other specified value and dataframe. It is equivalent to other - dataframe.  
17. **rtruediv()** - Get floating division of other specified value and dataframe. It is equivalent to other / dataframe.  

# SEE ALSO  

- **[DataFrame - Data Extraction Methods](./df_data_extraction.md)**  
- **[DataFrame - Indexing Operations](./df_indexing_operations.md)**  
- **[DataFrame - Conversion Functions](./df_conversion.md)**  
- **[DataFrame - Sorting Functions](./df_sort.md)**  
- **[DataFrame - Math Functions](./df_math_func.md)**  
- **[DataFrame - Aggregate Functions](./df_agg_func.md)**  
