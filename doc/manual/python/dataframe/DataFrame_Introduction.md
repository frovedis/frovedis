% DataFrame  
  
# NAME
  
DataFrame - here refers to a Frovedis dataframe. It is a two-dimensional, size-mutable, potentially heterogeneous tabular data structure.  
  
# SYNOPSIS  

    frovedis.dataframe.df.DataFrame(df=None, is_series=False)  
  
# DESCRIPTION  
  
Dataframe is a two-dimensional data structure, i.e., data is aligned in a tabular fashion in rows and columns. This data structure also contains labeled axes (rows and columns). Arithmetic operations align on both row and column labels. It can be thought of as a dict-like container for Series objects or it can be thought of as an SQL table or a spreadsheet data representation.  
  
Features of DataFrame:  
- Columns can be of different types.  
- DataFrame is mutable i.e. the number of rows and columns can be increased or decreased.  
- DataFrame supports indexing and labeled columns name.  
- Supports arithmetic operations on rows and columns.  

Frovedis contains dataframe implementation over a client server architecture (which uses Vector Engine on server side to perform fast computations) where pandas dataframe will be converted to frovedis dataframe and then will used to perform datframe related operations.  

**In frovedis, currently dataframe can only support atomic data i.e. a column in a frovedis dataframe can only have primitve type values such as string, integer, float, double, boolean).**  


## Detailed description
  
### 1. DataFrame(df = None, is_series = False)

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

    # a pandas dataframe from key value pair
    pdf1 = pd.DataFrame(peopleDF)
    
    # creating frovedis dataframe
    fdf1 = fd.DataFrame(pdf1) #change to frovedis dataframe

    # display created frovedis dataframes
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

**_columns_**: Returns a list of string having the column labels of the Frovedis dataframe in form of a list having columns names  

For example,    
    
    # fdf1 is same from above example
	print(fdf1.columns)
    
Output  
	
    ['Age', 'Country', 'Ename', 'isMale']
    

**_count_**:  Returns an integer value having the number of rows in the input DataFrame  

For example,   
    
    # fdf1 is same from above example
	print(fdf1.count)  
  
Output  

	5  

**_dtypes_**: Returns the dtypes in the DataFrame  
The dtype attribute returns a Series object with the data type of each column.  
  
For example,  

	# fdf1 is same from above example
	print(fdf1.dtypes)  
  
Output  

	Age         int64
	Country    object
	Ename      object
	isMale       bool
	dtype: object	
  

**_ndim_**: Returns an integer representing the number of axes / array dimensions  

For example,  

	# fdf1 is same from above example
	print(fdf1.ndim)  
  
Output  

    2  

**_shape_**: Returns a tuple representing the dimensions of the DataFrame in form: **(nrows, ncols)**  

For example,  

	# fdf1 is same from above example
	print(fdf1.shape)  
  
Output  
	
    (5, 4)  
  
  
**_values_**: Returns a numpy representation of the DataFrame  

For example,  

	# fdf1 is same from above example 
	print(fdf1.values)  
  
Output  

    [['Michael' '29' 'USA' '0']
    ['Andy' '30' 'England' '0']
    ['Tanaka' '27' 'Japan' '0']
    ['Raul' '19' 'France' '0']
    ['Yuta' '31' 'Japan' '1']]   
  
  
### 2. release()
  
__Purpose__  
This method acts like a destructor.  
It is used to release dataframe pointer from server heap and it resets all its attributes to None.  
  
For example,  

    # fdf1 is same from above example 
	# releasing dataframe pointers
	fdf1.release()
  
__Return Value__  
It returns nothing.  
  
## Public Member Functions of DataFrame  
  
DataFrame provides a lot of utilities to perform various operations. For simplicity we have categorized them into three lists: 
**_Basic functions_**, **_Application, groupby functions_**, **_Aggregate functions_** and **_Binary operator functions_**.  
  
### a) List of Basic Functions  
  
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

### b) Application, groupby Functions  
Such functions are used to perform operations like agg(), groupby() operations on frovedis dataframe.  

1. **agg()** - Aggregate using the specified functions and columns.  
2. **apply()** - Apply a function along an axis of the DataFrame.  
3. **groupby()** - Group dataframe using the specified columns.  


### c) List of Aggregate Functions  
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

  
### d) List of Binary Operator Functions  
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

# SEE ALSO  

- **[DataFrame - Selection and Combinations](./DataFrame_SelectionAndCombinations.md)**  
- **[DataFrame - Conversion, Missing data handling, Sorting Functions](./DataFrame_ConversionAndSorting.md)**  
- **[DataFrame - Function Application, Groupby](./DataFrame_FunctionApplicationGroupby.md)**  
- **[DataFrame - Binary Operators](./DataFrame_BinaryOperators.md)**  
- **[DataFrame - Aggregate Functions](./DataFrame_AggregateFunctions.md)**  