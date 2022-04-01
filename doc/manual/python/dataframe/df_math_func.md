% DataFrame Math Functions  

# NAME  

DataFrame Math Functions - this manual contains all the methods for carrying out mathematical operations.  

## DESCRIPTION  

Frovedis dataframe has several math functions defined for performing operations like add(), sub(), mul(), etc. between two dataframes or between scalar value and dataframe. These functions return a new frovedis DataFrame instance as a result.  

Also, it contains reverse operations such as radd(), rsub(), rmul(), etc.  

## Public Member Functions  

    1. abs()
    2. add(other, axis = 'columns', level = None, fill_value = None)
    3. div(other, axis = 'columns', level = None, fill_value = None)  
    4. floordiv(other, axis = 'columns', level = None, fill_value = None)  
    5. mod(other, axis = 'columns', level = None, fill_value = None)  
    6. mul(other, axis = 'columns', level = None, fill_value = None)  
    7. pow(other, axis = 'columns', level = None, fill_value = None)  
    8. sub(other, axis = 'columns', level = None, fill_value = None)  
    9. truediv(other, axis = 'columns', level = None, fill_value = None)  
    10. radd(other, axis = 'columns', level = None, fill_value = None)  
    11. rdiv(other, axis = 'columns', level = None, fill_value = None)  
    12. rfloordiv(other, axis = 'columns', level = None, fill_value = None)  
    13. rmod(other, axis = 'columns', level = None, fill_value = None)  
    14. rmul(other, axis = 'columns', level = None, fill_value = None)  
    15. rpow(other, axis = 'columns', level = None, fill_value = None)  
    16. rsub(other, axis = 'columns', level = None, fill_value = None)  
    17. rtruediv(other, axis = 'columns', level = None, fill_value = None)  

## Detailed Description  

### 1. DataFrame.abs()  

__Purpose__  
It computes absolute numeric value of each element.  

This function only applies to elements that are all numeric.  

For example,  

    import pandas as pd
    import numpy as np
    import frovedis.dataframe as fdf
    
    # a dictionary
    tempDF = {
                'City': ['Nagpur', 'Kanpur', 'Allahabad', 'Kannuaj', 'Allahabad',
                         'Kanpur', 'Kanpur', 'Kanpur'],
                'Temperature': [-2, 10, 18, 34, -8, -4, 36, 45]
               }
    
    # create pandas dataframe
    pdf1 = pd.DataFrame(tempDF)
    
    # create frovedis dataframe
    fdf1 = fdf.DataFrame(pdf1)
    
    # display the frovedis dataframe
    fdf1.show()

Output

    index   City       Temperature
    0       Nagpur     -2
    1       Kanpur     10
    2       Allahabad  18
    3       Kannuaj    34
    4       Allahabad  -8
    5       Kanpur     -4
    6       Kanpur     36
    7       Kanpur     45

For example,

    # abs() demo
    print(fdf1['Temperature'].abs())

Output

    index   Temperature
    0       2
    1       10
    2       18
    3       34
    4       8
    5       4
    6       36
    7       45

__Return Value__  
It returns a frovedis DataFrame instance.  

### 2. DataFrame.add(other, axis = 'columns', level = None, fill_value = None)  

__Parameters__  
**_other_**: It can accept single or multiple element data structure like the following:  
- Number  
- List. Currently, this method supports operation on only list of numeric values.  
- A numpy ndarray. Currently, this method supports operation on only an array of numeric values.  
- pandas DataFrame. It must not be an empty dataframe.  
- pandas Series  
- frovedis DataFrame. It must not be an empty dataframe.  

Any of these is considered as the value to be added with the current dataframe.  
**_axis_**: It accepts an integer or string object as parameter. It is used to decide whether to 
perform addition operation along the indices or by column labels. (Default: 'columns')  
- **1 or 'columns'**: perform addition operation on the columns. Currently, axis = 1 is supported in this method.  

**_level_**: This is an unused parameter. (Default: None)  
**_fill\_value_**: It accepts scalar values or None. It fills existing missing (NaN) values, and any 
new element needed for successful dataframe alignment, with this value before computation.  (Default: None)  
Irrespective of the specified value, if data in both corresponding dataframe locations is missing, then 
the result will be missing (contains NaNs).  

__Purpose__  
It performs addition operation between two dataframes or dataframe and scalar value. It is equivalent 
to **'dataframe + other'**.  

For example,  

    import pandas as pd
    import frovedis.dataframe as fdf
    
    # a dictionary
    data1 = {
             "points": [5, 6, 4],
             "total": [10, 11, 12]
            }
    
    # create pandas dataframe
    pdf1 = pd.DataFrame(data1)
    
    # create frovedis dataframe
    fdf1 = fdf.DataFrame(pdf1)
    
    # display the frovedis dataframe
    fdf1.show()  
        
Output  

    index   points  total
    0       5       10
    1       6       11
    2       4       12

For example,  

    # add() demo with scalar value using operator version
    print(fdf1 + 10)

Output  

    index   points  total
    0       15      20
    1       16      21
    2       14      22

For example,  

    # add() demo with scalar value using method version
    fdf1.add(10).show()

Output  

    index   points  total
    0       15      20
    1       16      21
    2       14      22

In both versions, all column elements (axis = 1 by default) are added with a scalar value.  

For example,  

    import pandas as pd
    import frovedis.dataframe as fdf
    
    # a dictionary
    data1 = {
             "points": [5, 6, 4],
             "total": [10, 11, 12]
            }

    # create pandas dataframe
    pdf1 = pd.DataFrame(data1)
    
    # create frovedis dataframe
    fdf1 = fdf.DataFrame(pdf1)
    
    # display the frovedis dataframe
    fdf1.show()

Output  

    index   points  total
    0       5       10
    1       6       11
    2       4       12

For example,  

    # a dictionary
    data2 = {
             "points": [2, 3, 8],
             "total": [7, 9, 11]
            }
    
    # create pandas dataframe
    pdf2 = pd.DataFrame(data2)
    
    # create frovedis dataframe
    fdf2 = fdf.DataFrame(pdf2)

    # display the frovedis dataframe
    fdf2.show()

Output  

    index   points  total
    0       2       7
    1       3       9
    2       8       11

For example,  

    # add() demo on two dataframes using operator version
    print(fdf1 + fdf2)

Output  

    index   points  total
    0       7       17
    1       9       20
    2       12      23
    
For example,  

    # add() demo on two dataframes using method version
    fdf1.add(other = fdf2).show()

Output  

    index   points  total
    0       7       17
    1       9       20
    2       12      23

In both versions, only common columns in both dataframes are added. Other are replaced with 
NaN values in resultant dataframe (fill_value = None by default).  

**When fill_value is not None,**  

For example,  

    import pandas as pd
    import frovedis.dataframe as fdf
    
    # a dictionary
    data1 = {
             "points": [5, 6, 4],
             "total": [10, 11, np.nan]
            }

    # create pandas dataframe
    pdf1 = pd.DataFrame(data1)
    
    # create frovedis dataframe
    fdf1 = fdf.DataFrame(pdf1)
    
    # display the frovedis dataframe
    fdf1.show()

Output  

    index   points  total
    0       5       10
    1       6       11
    2       4       NULL

For example,  
    
    # a dictionary
    data2 = {
             "points": [2, 3, 8],
             "total": [7, np.nan, np.nan]
            }
    
    # create pandas dataframe
    pdf2 = pd.DataFrame(data2)
    
    # create frovedis dataframe
    fdf2 = fdf.DataFrame(pdf2)

    # display the frovedis dataframe
    fdf2.show()

Output  

    index   points  total
    0       2       7
    1       3       NULL
    2       8       NULL

For example,  
    
    # add() demo on two dataframes using method version and fill_value = 10
    fdf1.add(other = fdf2, fill_value = 10).show()

Output  

    index   points  total
    0       7       17
    1       9       21
    2       12      20

Here, only common columns in both dataframes are added, excluding the misiing values. Other column elements are 
added with the fill_value = 10 (exluding missing values) in resultant dataframe.  

__Return Value__  
It returns a frovedis DataFrame which contains the result of arithmetic operation.  

### 3. DataFrame.div(other, axis = 'columns', level = None, fill_value = None)  

__Parameters__  
**_other_**: It can accept single or multiple element data structure like the following:  
- Number  
- List. Currently, this method supports operation on only list of numeric values.  
- A numpy ndarray. Currently, this method supports operation on only an array of numeric values.  
- pandas DataFrame. It must not be an empty dataframe.  
- pandas Series  
- frovedis DataFrame. It must not be an empty dataframe.  

Any of these is considered as the value to be divided over the current dataframe.  
**_axis_**: It accepts an integer or string object as parameter. It is used to decide whether to 
perform division operation along the indices or by column labels. (Default: 'columns')  
- **1 or 'columns'**: perform division operation on the columns. Currently, axis = 1 is supported in this method.  

**_level_**: This is an unused parameter. (Default: None)  
**_fill\_value_**: It accepts scalar values or None. It fills existing missing (NaN) values, and any 
new element needed for successful dataframe alignment, with this value before computation.  (Default: None)  
Irrespective of the specified value, if data in both corresponding dataframe locations is missing, then 
the result will be missing (contains NaNs).  

__Purpose__  
It performs floating division operation between two dataframes or dataframe and scalar value. It is 
equivalent to **'dataframe / other'**.  

**It is an alias of truediv().**  

For example,  

    import pandas as pd
    import frovedis.dataframe as fdf
    
    # a dictionary
    data1 = {
             "points": [5, 6, 4],
             "total": [10, 11, 12]
            }
    
    # create pandas dataframe
    pdf1 = pd.DataFrame(data1)
    
    # create frovedis dataframe
    fdf1 = fdf.DataFrame(pdf1)
    
    # display the frovedis dataframe
    fdf1.show()

Output  

    index   points  total
    0       5       10
    1       6       11
    2       4       12

For example,  

    # div() demo with scalar value using operator version
    print(fdf1 / 10)

Output  

    index   points  total
    0       0.5     1
    1       0.6     1.1
    2       0.4     1.19999

For example,  

    # div() demo with scalar value using method version
    fdf1.div(10).show()

Output  

    index   points  total
    0       0.5     1
    1       0.6     1.1
    2       0.4     1.19999

In both versions, all column elements (axis = 1 by default) are divied by a scalar value.  

For example,  

    import pandas as pd
    import frovedis.dataframe as fdf
    
    # a dictionary
    data1 = {
             "points": [5, 6, 4],
             "total": [10, 11, 12]
            }
    
    # create pandas dataframe
    pdf1 = pd.DataFrame(data1)
    
    # create frovedis dataframe
    fdf1 = fdf.DataFrame(pdf1)
    
    # display the frovedis dataframe
    fdf1.show()

Output  

    index   points  total
    0       5       10
    1       6       11
    2       4       12

For example,  

    # a dictionary
    data2 = {
             "points": [2, 3, 8],
             "total": [7, 9, 11]
            }
    
    # create pandas dataframe
    pdf2 = pd.DataFrame(data2)
    
    # create frovedis dataframe
    fdf2 = fdf.DataFrame(pdf2)
    
    # display the frovedis dataframe
    fdf2.show()

Output  

    index   points  total
    0       2       7
    1       3       9
    2       8       11

For example,  
    
    # div() demo on two dataframes using operator version
    print(fdf1 / fdf2)

Output  

    index   points  total
    0       2.5     1.42857
    1       2       1.22222
    2       0.5     1.0909

For example,  

    # div() demo on two dataframes using method version
    fdf1.div(other = fdf2).show()

Output  

    index   points  total
    0       2.5     1.42857
    1       2       1.22222
    2       0.5     1.0909

In both versions, only common columns in both dataframes are divided. Other are replaced with 
NaN values in resultant dataframe (fill_value = None by default).  

**When fill_value is not None,**  

For example,  

    import pandas as pd
    import frovedis.dataframe as fdf
    
    # a dictionary
    data1 = {
             "points": [5, 6, 4],
             "total": [10, 11, np.nan]
            }

    # create pandas dataframe
    pdf1 = pd.DataFrame(data1)
    
    # create frovedis dataframe
    fdf1 = fdf.DataFrame(pdf1)
    
    # display the frovedis dataframe
    fdf1.show()

Output  

    index   points  total
    0       5       10
    1       6       11
    2       4       NULL

For example,  

    # a dictionary
    data2 = {
             "points": [2, 3, 8],
             "total": [7, np.nan, np.nan]
            }
    
    # create pandas dataframe
    pdf2 = pd.DataFrame(data2)
    
    # create frovedis dataframe
    fdf2 = fdf.DataFrame(pdf2)

    # display the frovedis dataframe
    fdf2.show()

Output  

    index   points  total
    0       2       7
    1       3       NULL
    2       8       NULL

For example,  

    # div() demo on two dataframes using method version and fill_value = 10
    fdf1.div(other = fdf2, fill_value = 10).show()

Output  

    index   points  total
    0       2.5     1.42857
    1       2       1.1
    2       0.5     1

Here, only common columns in both dataframes are divided, excluding the missing values. Other column 
elements are divided with the fill_value = 10 (excluding the missing values) in resultant dataframe.  

__Return Value__  
It returns a frovedis DataFrame which contains the result of arithmetic operation.  

### 4. DataFrame.floordiv(other, axis = 'columns', level = None, fill_value = None)  

__Parameters__  
**_other_**: It can accept single or multiple element data structure like the following:  
- Number  
- List. Currently, this method supports operation on only list of numeric values.  
- A numpy ndarray. Currently, this method supports operation on only an array of numeric values.  
- pandas DataFrame. It must not be an empty dataframe.  
- pandas Series  
- frovedis DataFrame. It must not be an empty dataframe.  

Any of these is considered as the value to be divided over the current dataframe.  
**_axis_**: It accepts an integer or string object as parameter. It is used to decide whether to 
perform division operation along the indices or by column labels. (Default: 'columns')  
- **1 or 'columns'**: perform division operation on the columns. Currently, axis = 1 is supported in this method.  

**_level_**: This is an unused parameter. (Default: None)  
**_fill\_value_**: It accepts scalar values or None. It fills existing missing (NaN) values, and any 
new element needed for successful dataframe alignment, with this value before computation.  (Default: None)  
Irrespective of the specified value, if data in both corresponding dataframe locations is missing, then 
the result will be missing (contains NaNs).  

__Purpose__  
It performs integer division operation between two dataframes or dataframe and scalar value. It is 
equivalent to **'dataframe // other'**.  

For example,  

    import pandas as pd
    import frovedis.dataframe as fdf
    
    # a dictionary
    data1 = {
             "points": [5, 6, 4],
             "total": [10, 11, 12]
            }
    
    # create pandas dataframe
    pdf1 = pd.DataFrame(data1)
    
    # create frovedis dataframe
    fdf1 = fdf.DataFrame(pdf1)
    
    # display the frovedis dataframe
    fdf1.show()

Output  

    index   points  total
    0       5       10
    1       6       11
    2       4       12

For example,  

    # floordiv() demo with scalar value using operator version
    print(fdf1 // 10)

Output  

    index   points  total
    0       0       1
    1       0       1
    2       0       1

For example,  

    # floordiv() demo with scalar value using method version
    fdf1.floordiv(10).show()

Output  

    index   points  total
    0       0       1
    1       0       1
    2       0       1

In both versions, all column elements (axis = 1 by default) are divied by a scalar value.  
Also, resultant dataframe column elements will contain floor integer value.  

For example,  

    import pandas as pd
    import frovedis.dataframe as fdf
    
    # a dictionary
    data1 = {
             "points": [5, 6, 4],
             "total": [10, 11, 12]
            }

    # create pandas dataframe
    pdf1 = pd.DataFrame(data1)
    
    # create frovedis dataframe
    fdf1 = fdf.DataFrame(pdf1)
    
    # display the frovedis dataframe
    fdf1.show()

Output  

    index   points  total
    0       5       10
    1       6       11
    2       4       12

For example,  

    # a dictionary
    data2 = {
             "points": [2, 3, 8],
             "total": [7, 9, 11]
            }

    # create pandas dataframe
    pdf2 = pd.DataFrame(data2)
    
    # create frovedis dataframe
    fdf2 = fdf.DataFrame(pdf2)
    
    # display the frovedis dataframe
    fdf2.show()

Output  

    index   points  total
    0       2       7
    1       3       9
    2       8       11

For example,  

    # floordiv() demo on two dataframes using operator version
    print(fdf1 // fdf2)
    
Output  

    index   points  total
    0       2       1
    1       2       1
    2       0       1

For example,  

    # floordiv() demo on two dataframes using method version
    fdf1.floordiv(other = fdf2).show()

Output  

    index   points  total
    0       2       1
    1       2       1
    2       0       1

In both versions, only common columns in both dataframes are divided. Other are replaced with 
NaN values in resultant dataframe (fill_value = None by default).  

**When fill_value is not None,**  

For example,  

    import pandas as pd
    import frovedis.dataframe as fdf
    
    # a dictionary
    data1 = {
             "points": [5, 6, 4],
             "total": [10, 11, np.nan]
            }

    # create pandas dataframe
    pdf1 = pd.DataFrame(data1)
    
    # create frovedis dataframe
    fdf1 = fdf.DataFrame(pdf1)
    
    # display the frovedis dataframe
    fdf1.show()

Output  

    index   points  total
    0       5       10
    1       6       11
    2       4       NULL

For example,  
    
    # a dictionary
    data2 = {
             "points": [2, 3, 8],
             "total": [7, np.nan, np.nan]
            }
    
    # create pandas dataframe
    pdf2 = pd.DataFrame(data2)
    
    # create frovedis dataframe
    fdf2 = fdf.DataFrame(pdf2)

    # display the frovedis dataframe
    fdf2.show()

Output  

    index   points  total
    0       2       7
    1       3       NULL
    2       8       NULL

For example,  
    
    # floordiv() demo on two dataframes using method version and fill_value = 10
    fdf1.floordiv(other = fdf2, fill_value = 10).show()

Output  

    index   points  total
    0       2       1
    1       2       1
    2       0       1

Here, only common columns in both dataframes are divided, excluding the missing values. Other column elements are 
divided with the fill_value = 10 (excluding the missing values) in resultant dataframe.  

__Return Value__  
It returns a frovedis DataFrame which contains the result of arithmetic operation.  

### 5. DataFrame.mod(other, axis = 'columns', level = None, fill_value = None)

__Parameters__  
**_other_**: It can accept single or multiple element data structure like the following:  
- Number  
- List. Currently, this method supports operation on only list of numeric values.  
- A numpy ndarray. Currently, this method supports operation on only an array of numeric values.  
- pandas DataFrame. It must not be an empty dataframe.  
- pandas Series  
- frovedis DataFrame. It must not be an empty dataframe.  

Any of these is considered as the value to perform modulo operation with the current dataframe.  
**_axis_**: It accepts an integer or string object as parameter. It is used to decide whether to 
perform modulo operation along the indices or by column labels. (Default: 'columns')  
- **1 or 'columns'**: perform modulo operation on the columns. Currently, axis = 1 is supported in this method.  

**_level_**: This is an unused parameter. (Default: None)  
**_fill\_value_**: It accepts scalar values or None. It fills existing missing (NaN) values, and any 
new element needed for successful dataframe alignment, with this value before computation.  (Default: None)  
Irrespective of the specified value, if data in both corresponding dataframe locations is missing, then 
the result will be missing (contains NaNs).  

__Purpose__  
It performs modulo operation between two dataframes or dataframe and scalar value. It is equivalent 
to **'dataframe % other'**.  

For example,  

    import pandas as pd
    import frovedis.dataframe as fdf
    
    # a dictionary
    data1 = {
             "points": [5, 6, 4],
             "total": [10, 11, 12]
            }
    
    # create pandas dataframe
    pdf1 = pd.DataFrame()
    
    # create frovedis dataframe
    fdf1 = fdf.DataFrame(pdf1)
    
    # display the frovedis dataframe
    fdf1.show()

Output  

    index   points  total
    0       5       10
    1       6       11
    2       4       12

For example,  

    # mod() demo with scalar value using operator version
    print(fdf1 % 10)
    
Output  

    index   points  total
    0       5       0
    1       6       1
    2       4       2

For example,  

    # mod() demo with scalar value using method version
    fdf1.mod(10).show()

Output  

    index   points  total
    0       5       0
    1       6       1
    2       4       2

In both versions, modulo operation is performed on all column elements (axis = 1 by default) by a scalar value.  

For example,  

    import pandas as pd
    import frovedis.dataframe as fdf
    
    # a dictionary
    data1 = {
             "points": [5, 6, 4],
             "total": [10, 11, 12]
            }

    # create pandas dataframe
    pdf1 = pd.DataFrame(data1)
    
    # create frovedis dataframe
    fdf1 = fdf.DataFrame(pdf1)
    
    # display the frovedis dataframe
    fdf1.show()

Output  

    index   points  total
    0       5       10
    1       6       11
    2       4       12

For example,  
    
    # a dictionary
    data2 = {
             "points": [2, 3, 8],
             "total": [7, 9, 11]
            }
    
    # create pandas dataframe
    pdf2 = pd.DataFrame(data2)
    
    # create frovedis dataframe
    fdf2 = fdf.DataFrame(pdf2)

    # display the frovedis dataframe
    fdf2.show()

Output  

    index   points  total
    0       2       7
    1       3       9
    2       8       11

For example,  

    # mod() demo on two dataframes using operator version
    print(fdf1 % fdf2)

Output  

    index   points  total
    0       1       3
    1       0       2
    2       4       1

**When fill_value is not None,**  

For example,  

    import pandas as pd
    import frovedis.dataframe as fdf
    
    # a dictionary
    data1 = {
             "points": [5, 6, 4],
             "total": [10, 11, np.nan]
            }

    # create pandas dataframe
    pdf1 = pd.DataFrame(data1)
    
    # create frovedis dataframe
    fdf1 = fdf.DataFrame(pdf1)
    
    # display the frovedis dataframe
    fdf1.show()

Output  

    index   points  total
    0       5       10
    1       6       11
    2       4       NULL

For example,  
 
    # a dictionary
    data2 = {
             "points": [2, 3, 8],
             "total": [7, np.nan, np.nan]
            }
    
    # create pandas dataframe
    pdf2 = pd.DataFrame(data2)
    
    # create frovedis dataframe
    fdf2 = fdf.DataFrame(pdf2)

    # display the frovedis dataframe
    fdf2.show()

Output  

    index   points  total
    0       2       7
    1       3       NULL
    2       8       NULL

For example,  

    # mod() demo on two dataframes using method version
    fdf1.mod(other = fdf2).show()

Output  

    index   points  total
    0       1       3
    1       0       2
    2       4       1

In both versions, modulo is performed on only common columns in both dataframes. Other are replaced with 
NaN values in resultant dataframe (fill_value = None by default).  

For example,  
    
    # mod() demo on two dataframes using method version and fill_value = 10
    fdf1.mod(other = fdf2, fill_value = 10).show()

Output  

    index   points  total
    0       1       3
    1       0       1
    2       4       0

Here, modulo is performed on only common columns in both dataframes, excluding the missing values. Modulo 
is performed on other column elements using the value 10 (excluding the missing values) in resultant dataframe.  

__Return Value__  
It returns a frovedis DataFrame which contains the result of arithmetic operation.  

### 6. DataFrame.mul(other, axis = 'columns', level = None, fill_value = None)

__Parameters__  
**_other_**: It can accept single or multiple element data structure like the following:  
- Number  
- List. Currently, this method supports operation on only list of numeric values.  
- A numpy ndarray. Currently, this method supports operation on only an array of numeric values.  
- pandas DataFrame. It must not be an empty dataframe.  
- pandas Series  
- frovedis DataFrame. It must not be an empty dataframe.  

Any of these is considered as the value to be multiplied with the current dataframe.  
**_axis_**: It accepts an integer or string object as parameter. It is used to decide whether to 
perform multiplication operation along the indices or by column labels. (Default: 'columns')  
- **1 or 'columns'**: perform multiplication operation on the columns. Currently, axis = 1 is supported in this method.  

**_level_**: This is an unused parameter. (Default: None)  
**_fill\_value_**: It accepts scalar values or None. It fills existing missing (NaN) values, and any 
new element needed for successful dataframe alignment, with this value before computation.  (Default: None)  
Irrespective of the specified value, if data in both corresponding dataframe locations is missing, then 
the result will be missing (contains NaNs).  

__Purpose__  
It performs multiplication operation between two dataframes or dtaframe and scalar value. It is 
equivalent to **'dataframe * other'**.  

For example,  

    import pandas as pd
    import frovedis.dataframe as fdf
    
    # a dictionary
    data1 = {
             "points": [5, 6, 4],
             "total": [10, 11, 12]
            }
    
    # create pandas dataframe
    pdf1 = pd.DataFrame(data1)
    
    # create frovedis dataframe
    fdf1 = fdf.DataFrame(pdf1)
    
    # display the frovedis dataframe
    fdf1.show()
    
Output  

    index   points  total
    0       5       10
    1       6       11
    2       4       12

For example,  

    # mul() demo with scalar value using operator version
    print(fdf1 * 10)

Output  
    
    index   points  total
    0       50      100
    1       60      110
    2       40      120

For example,  

    # mul() demo with scalar value using method version
    fdf1.mul(10).show()

Output  

    index   points  total
    0       50      100
    1       60      110
    2       40      120

In both versions, all column elements (axis = 1 by default) are multiplied with a scalar value.  

For example,  

    import pandas as pd
    import frovedis.dataframe as fdf
    
    # a dictionary
    data1 = {
             "points": [5, 6, 4],
             "total": [10, 11, 12]
            }

    # create pandas dataframe
    pdf1 = pd.DataFrame(data1)
    
    # create frovedis dataframe
    fdf1 = fdf.DataFrame(pdf1)
    
    # display the frovedis dataframe
    fdf1.show()

Output  

    index   points  total
    0       5       10
    1       6       11
    2       4       12

For example,  

    # a dictionary
    data2 = {
             "points": [2, 3, 8],
             "total": [7, 9, 11]
            }        
    
    # create pandas dataframe
    pdf2 = pd.DataFrame(data2)
    
    # create frovedis dataframe
    fdf2 = fdf.DataFrame(pdf2)

    # display the frovedis dataframe
    fdf2.show()

Output  

    index   points  total
    0       2       7
    1       3       9
    2       8       11

For example,  

    # mult() demo on two dataframes using operator version
    print(fdf1 * fdf2)

Output  

    index   points  total
    0       10      70
    1       18      99
    2       32      132

For example,  

    # mul() demo on two dataframes using method version
    fdf1.mul(other = fdf2).show()

Output  

    index   points  total
    0       10      70
    1       18      99
    2       32      132

In both versions, only common columns in both dataframes are multiplied. Other are replaced with 
NaN values in resultant dataframe (fill_value = None by default).  

**When fill_value is not None,**  

For example,  

    import pandas as pd
    import frovedis.dataframe as fdf
    
    # a dictionary
    data1 = {
             "points": [5, 6, 4],
             "total": [10, 11, np.nan]
            }

    # create pandas dataframe
    pdf1 = pd.DataFrame(data1)
    
    # create frovedis dataframe
    fdf1 = fdf.DataFrame(pdf1)
    
    # display the frovedis dataframe
    fdf1.show()

Output  

    index   points  total
    0       5       10
    1       6       11
    2       4       NULL

For example,  
    
    # a dictionary
    data2 = {
             "points": [2, 3, 8],
             "total": [7, np.nan, np.nan]
            }
    
    # create pandas dataframe
    pdf2 = pd.DataFrame(data2)
    
    # create frovedis dataframe
    fdf2 = fdf.DataFrame(pdf2)

    # display the frovedis dataframe
    fdf2.show()

Output  

    index   points  total
    0       2       7
    1       3       NULL
    2       8       NULL

For example,  
    
    # mul() demo on two dataframes using method version and fill_value = 10
    fdf1.mul(other = fdf2, fill_value = 10).show()

Output  

    index   points  total
    0       10      70
    1       18      110
    2       32      100

Here, only common columns in both dataframes are multiplied, excluding the missing values. Other 
column elements are multiplied with the value 10 (excluding the missing values) in resultant dataframe.  

__Return Value__  
It returns a frovedis DataFrame which contains the result of arithmetic operation.  

### 7. DataFrame.pow(other, axis = 'columns', level = None, fill_value = None)

__Parameters__  
**_other_**: It can accept single or multiple element data structure like the following:  
- Number  
- List. Currently, this method supports operation on only list of numeric values.  
- A numpy ndarray. Currently, this method supports operation on only an array of numeric values.  
- pandas DataFrame. It must not be an empty dataframe.  
- pandas Series  
- frovedis DataFrame. It must not be an empty dataframe.  

Any of these is considered as the value to perform exponential power operation with the current dataframe.  
**_axis_**: It accepts an integer or string object as parameter. It is used to decide whether to 
perform exponential power operation along the indices or by column labels. (Default: 'columns')  
- **1 or 'columns'**: perform exponential power operation on the columns. Currently, axis = 1 is supported in this method.  

**_level_**: This is an unused parameter. (Default: None)  
**_fill\_value_**: It accepts scalar values or None. It fills existing missing (NaN) values, and any 
new element needed for successful dataframe alignment, with this value before computation.  (Default: None)  
Irrespective of the specified value, if data in both corresponding dataframe locations is missing, then 
the result will be missing (contains NaNs).  

__Purpose__  
It performs exponential power operation between two dataframes or dataframe and scalar value. It is 
equivalent to **'dataframe ** other'**.  

For example,  

    import pandas as pd
    import frovedis.dataframe as fdf
    
    # a dictionary
    data1 = {
             "points": [5, 6, 4],
             "total": [10, 11, 12]
            }    
    
    # create pandas dataframe
    pdf1 = pd.DataFrame()
    
    # create frovedis dataframe
    fdf1 = fdf.DataFrame(pdf1)
    
    # display the frovedis dataframe
    fdf1.show()

Output  

    index   points  total
    0       5       10
    1       6       11
    2       4       12

For example,  

    # pow() demo with scalar value using operator version
    print(fdf1 ** 2)

Output  

    index   points  total
    0       10      20
    1       12      22
    2       8       24

For example,  

    # pow() demo with scalar value using method version
    fdf1.pow(2).show()

Output  

    index   points  total
    0       10      20
    1       12      22
    2       8       24

In both versions, exponential power operation is performed on all column 
elements (axis = 1 by default) by a scalar value.  

For example,  

    import pandas as pd
    import frovedis.dataframe as fdf
    
    # a dictionary
    data1 = {
         "points": [5, 6, 4],
         "total": [10, 11, 12]
        }
    
    # create pandas dataframe
    pdf1 = pd.DataFrame(data1)
    
    # create frovedis dataframe
    fdf1 = fdf.DataFrame(pdf1)
    
    # display the frovedis dataframe
    fdf1.show()
    
Output  

    index   points  total
    0       5       10
    1       6       11
    2       4       12

For example,  
    
    # a dictionary
    data2 = {
             "points": [2, 3, 8],
             "total": [7, 9, 11]
            }    
    
    # create pandas dataframe
    pdf2 = pd.DataFrame(data2)
    
    # create frovedis dataframe
    fdf2 = fdf.DataFrame(pdf2)
    
    # display the frovedis dataframe
    fdf2.show()

Output  

    index   points  total
    0       2       7
    1       3       9
    2       8       11

For example,  

    # pow() demo on two dataframes using operator version
    print(fdf1 ** fdf2)

Output  

    index   points  total
    0       10      70
    1       18      99
    2       32      132

For example,

    # pow() demo on two dataframes using method version
    fdf1.pow(other = fdf2).show()

Output  

    index   points  total
    0       10      70
    1       18      99
    2       32      132

In both versions, exponential power operation on only common columns in both dataframes. Exponential 
power operation on other are replaced with NaN values in resultant dataframe (fill_value = None by default).  

**When fill_value is not None,**  

For example,  

    import pandas as pd
    import frovedis.dataframe as fdf
    
    # a dictionary
    data1 = {
             "points": [5, 6, 4],
             "total": [10, 11, np.nan]
            }

    # create pandas dataframe
    pdf1 = pd.DataFrame(data1)
    
    # create frovedis dataframe
    fdf1 = fdf.DataFrame(pdf1)
    
    # display the frovedis dataframe
    fdf1.show()

Output  

    index   points  total
    0       5       10
    1       6       11
    2       4       NULL

For example,  
    
    # a dictionary
    data2 = {
             "points": [2, 3, 8],
             "total": [7, np.nan, np.nan]
            }
    
    # create pandas dataframe
    pdf2 = pd.DataFrame(data2)
    
    # create frovedis dataframe
    fdf2 = fdf.DataFrame(pdf2)

    # display the frovedis dataframe
    fdf2.show()

Output  

    index   points  total
    0       2       7
    1       3       NULL
    2       8       NULL

For example,  
    
    # pow() on two dataframes using method version and fill_value = 10
    fdf1.pow(other = fdf2, fill_value = 10).show()

Output  

    index   points  total
    0       25      1.00000e+07
    1       216     2.59374e+10
    2       65536   1.00000e+10

Here, exponential power operation is performed on only common columns in both 
dataframes, excluding the missing values. Exponential power operation on other column elements is performed with 
the value 10 (excluding the missing values) in resultant dataframe.  

__Return Value__  
It returns a frovedis DataFrame which contains the result of arithmetic operation.  

### 8. DataFrame.sub(other, axis = 'columns', level = None, fill_value = None)

__Parameters__  
**_other_**: It can accept single or multiple element data structure like the following:  
- Number  
- List. Currently, this method supports operation on only list of numeric values.  
- A numpy ndarray. Currently, this method supports operation on only an array of numeric values.  
- pandas DataFrame. It must not be an empty dataframe.  
- pandas Series  
- frovedis DataFrame. It must not be an empty dataframe.  

Any of these is considered as the value to be subtracted with the current dataframe.  
**_axis_**: It accepts an integer or string object as parameter. It is used to decide whether to 
perform subtraction operation along the indices or by column labels. (Default: 'columns')  
- **1 or 'columns'**: perform subtraction operation on the columns. Currently, axis = 1 is supported in this method.  

**_level_**: This is an unused parameter. (Default: None)  
**_fill\_value_**: It accepts scalar values or None. It fills existing missing (NaN) values, and any 
new element needed for successful dataframe alignment, with this value before computation.  (Default: None)  
Irrespective of the specified value, if data in both corresponding dataframe locations is missing, then 
the result will be missing (contains NaNs).  

__Purpose__  
It performs subtraction operation between two dataframes or dtaframe and scalar value. It is 
equivalent to **'dataframe - other'**.  

For example,  

    import pandas as pd
    import frovedis.dataframe as fdf
    
    # a dictionary
    data1 =  {
              "points": [5, 6, 4],
              "total": [10, 11, 12]
            }
    
    # create pandas dataframe
    pdf1 = pd.DataFrame(data1)
    
    # create frovedis dataframe
    fdf1 = fdf.DataFrame(pdf1)
    
    # display the frovedis dataframe
    fdf1.show()
    
Output  

    index   points  total
    0       5       10
    1       6       11
    2       4       12

For example,  

    # sub() demo with scalar value using operator version
    print(fdf1 - 10)

Output  

    index   points  total
    0       -5      0
    1       -4      1
    2       -6      2

For example,  
    
    # sub() demo with scalar value using method version
    fdf1.sub(10).show()

Output  

    index   points  total
    0       -5      0
    1       -4      1
    2       -6      2

In both versions, all column elements (axis = 1 by default) are subtracted by a scalar value.  

For example,  

    import pandas as pd
    import frovedis.dataframe as fdf
    
    # a dictionary
    data1 =  {
              "points": [5, 6, 4],
              "total": [10, 11, 12]
            }

    # create pandas dataframe
    pdf1 = pd.DataFrame(data1)
    
    # create frovedis dataframe
    fdf1 = fdf.DataFrame(pdf1)
    
    # display the frovedis dataframe
    fdf1.show()
    
Output  

    index   points  total
    0       5       10
    1       6       11
    2       4       12

For example,  

    # a dictionary
    data2 = {
             "points": [2, 3, 8],
             "total": [7, 9, 11]
            }

    # create pandas dataframe
    pdf2 = pd.DataFrame(data2)
    
    # create frovedis dataframe
    fdf2 = fdf.DataFrame(pdf2)
    
    # display the frovedis dataframe
    fdf2.show()

Output  

    index   points  total
    0       2       7
    1       3       9
    2       8       11

For example,  

    # sub() demo on two dataframes using operator version
    print(fdf1 - fdf2)

Output  

    index   points  total
    0       3       3
    1       3       2
    2       -4      1

For example,  

    # sub() demo on two dataframes using method version
    fdf1.sub(other = fdf2).show()

Output  

    index   points  total
    0       3       3
    1       3       2
    2       -4      1

In both versions, only common columns in both dataframes are subtracted. Other are replaced with 
NaN values in resultant dataframe (fill_value = None by default).  

**When fill_value is not None,**  

For example,  

    import pandas as pd
    import frovedis.dataframe as fdf
    
    # a dictionary
    data1 = {
             "points": [5, 6, 4],
             "total": [10, 11, np.nan]
            }

    # create pandas dataframe
    pdf1 = pd.DataFrame(data1)
    
    # create frovedis dataframe
    fdf1 = fdf.DataFrame(pdf1)
    
    # display the frovedis dataframe
    fdf1.show()

Output  

    index   points  total
    0       5       10
    1       6       11
    2       4       NULL

For example,  
    
    # a dictionary
    data2 = {
             "points": [2, 3, 8],
             "total": [7, np.nan, np.nan]
            }
    
    # create pandas dataframe
    pdf2 = pd.DataFrame(data2)
    
    # create frovedis dataframe
    fdf2 = fdf.DataFrame(pdf2)

    # display the frovedis dataframe
    fdf2.show()

Output  

    index   points  total
    0       2       7
    1       3       NULL
    2       8       NULL

For example,  
    
    # sub() demo on two dataframes using method version and fill_value = 10
    fdf1.sub(other = fdf2, fill_value = 10).show()

Output  

    index   points  total
    0       3       3
    1       3       1
    2       -4      0

Here, only common columns in both dataframes are subtracted, excluding the missing values. Other 
column elements are subtracted with the value 10 (excluding the missing values) in resultant dataframe.  

__Return Value__  
It returns a frovedis DataFrame which contains the result of arithmetic operation.  

### 9. DataFrame.truediv(other, axis = 'columns', level = None, fill_value = None)

__Parameters__  
**_other_**: It can accept single or multiple element data structure like the following:  
- Number  
- List. Currently, this method supports operation on only list of numeric values.  
- A numpy ndarray. Currently, this method supports operation on only an array of numeric values.  
- pandas DataFrame. It must not be an empty dataframe.  
- pandas Series  
- frovedis DataFrame. It must not be an empty dataframe.  

Any of these is considered as the value to be divided with the current dataframe.  
**_axis_**: It accepts an integer or string object as parameter. It is used to decide whether to 
perform division operation along the indices or by column labels. (Default: 'columns')  
- **1 or 'columns'**: perform division operation on the columns. Currently, axis = 1 is supported in this method.  

**_level_**: This is an unused parameter. (Default: None)  
**_fill\_value_**: It accepts scalar values or None. It fills existing missing (NaN) values, and any 
new element needed for successful dataframe alignment, with this value before computation.  (Default: None)  
Irrespective of the specified value, if data in both corresponding dataframe locations is missing, then 
the result will be missing (contains NaNs).  

__Purpose__  
It performs floating division operation between two dataframes or dtaframe and scalar value. It is 
equivalent to **'dataframe / other'**.  

For example,  

    import pandas as pd
    import frovedis.dataframe as fdf
    
    # a dictionary
    data1 =  {
              "points": [8, 5, 9],
              "total": [3, 2, 1]
            }
    
    # create pandas dataframe
    pdf1 = pd.DataFrame(data1)
    
    # create frovedis dataframe
    fdf1 = fdf.DataFrame(pdf1)
    
    # display the frovedis dataframe
    fdf1.show()
    
Output  

    index   points  total
    0       8       3
    1       5       2
    2       9       1

For example,  

    # truediv() demo with scalar value using operator version
    print(fdf1 / 10)

Output  

    index   points  total
    0       0.8     0.3
    1       0.5     0.2
    2       0.9     0.1

For example,  
    
    # truediv() demo with scalar value using method version
    fdf1.truediv(10).show()

Output  

    index   points  total
    0       0.8     0.3
    1       0.5     0.2
    2       0.9     0.1

In both versions, all column elements (axis = 1 by default) are divied by a scalar value.  

For example,  

    import pandas as pd
    import frovedis.dataframe as fdf
    
    # a dictionary
    data1 =  {
              "points": [8, 5, 9],
              "total": [3, 2, 1]
            }
    
    # create pandas dataframe
    pdf1 = pd.DataFrame(data1)
    
    # create frovedis dataframe
    fdf1 = fdf.DataFrame(pdf1)
    
    # display the frovedis dataframe
    fdf1.show()

Output  

    index   points  total
    0       8       3
    1       5       2
    2       9       1

For example,  

    # a dictionary
    data2 =  {
              "points": [4, 7, 2],
              "total": [9, 1, 9]
            }
    
    # create pandas dataframe
    pdf2 = pd.DataFrame(data2)
    
    # create frovedis dataframe
    fdf2 = fdf.DataFrame(pdf2)
    
    # display the frovedis dataframe
    fdf2.show()

Output  

    index   points  total
    0       4       9
    1       7       1
    2       2       9

For example,  

    # truediv() demo on two dataframes using operator version
    print(fdf1 / fdf2)

Output  

    index   points    total
    0       2         0.333333
    1       0.714285  2
    2       4.5       0.111111

For example,  

    # truediv() demo on two dataframes using method version
    fdf1.truediv(other = fdf2).show()

Output  

    index   points    total
    0       2         0.333333
    1       0.714285  2
    2       4.5       0.111111

In both versions, only common columns in both dataframes are divided. Other are replaced with 
NaN values in resultant dataframe (fill_value = None by default).  

**When fill_value is not None,**  

For example,  

    import pandas as pd
    import frovedis.dataframe as fdf
    
    # a dictionary
    data1 = {
             "points": [5, 6, 4],
             "total": [10, 11, np.nan]
            }

    # create pandas dataframe
    pdf1 = pd.DataFrame(data1)
    
    # create frovedis dataframe
    fdf1 = fdf.DataFrame(pdf1)
    
    # display the frovedis dataframe
    fdf1.show()

Output  

    index   points  total
    0       5       10
    1       6       11
    2       4       NULL

For example,  
    
    # a dictionary
    data2 = {
             "points": [2, 3, 8],
             "total": [7, np.nan, np.nan]
            }
    
    # create pandas dataframe
    pdf2 = pd.DataFrame(data2)
    
    # create frovedis dataframe
    fdf2 = fdf.DataFrame(pdf2)

    # display the frovedis dataframe
    fdf2.show()

Output  

    index   points  total
    0       2       7
    1       3       NULL
    2       8       NULL

For example,  

    # truediv() demo on two dataframes using method version and fill_value = 10
    fdf1.truediv(other = fdf2, fill_value = 10).show()

Output  

    index   points  total
    0       2.5     1.42857
    1       2       1.1
    2       0.5     1

Here, only common columns in both dataframes are divided, excluding the missing values. Other 
column elements are divided with the value 10 (excluding the missing values) in resultant dataframe.  

__Return Value__  
It returns a frovedis DataFrame which contains the result of arithmetic operation.  

### 10. DataFrame.radd(other, axis = 'columns', level = None, fill_value = None)  

__Parameters__  
**_other_**: It can accept single or multiple element data structure like the following:  
- Number  
- List. Currently, this method supports operation on only list of numeric values.  
- A numpy ndarray. Currently, this method supports operation on only an array of numeric values.  
- pandas DataFrame. It must not be an empty dataframe.  
- pandas Series  
- frovedis DataFrame. It must not be an empty dataframe.  

Any of these is considered as the value to be added with the current dataframe.  
**_axis_**: It accepts an integer or string object as parameter. It is used to decide whether to 
perform reverse addition operation along the indices or by column labels. (Default: 'columns')  
- **1 or 'columns'**: perform reverse addition operation on the columns. Currently, axis = 1 is supported in this method.  

**_level_**: This is an unused parameter. (Default: None)  
**_fill\_value_**: It accepts scalar values or None. It fills existing missing (NaN) values, and any 
new element needed for successful dataframe alignment, with this value before computation.  (Default: None)  
Irrespective of the specified value, if data in both corresponding dataframe locations is missing, then 
the result will be missing (contains NaNs).  

__Purpose__  
It performs reverse addition operation between two dataframes or dataframe and scalar value. It is 
equivalent to **'other + dataframe'**.  

**Currently, it does not perform reverse addition of scalar using operator version. Only method version 
is supported.**  

For example,  

    import pandas as pd
    import frovedis.dataframe as fdf
    
    # a dictionary
    data1 = {
             "points": [5, 6, 4],
             "total": [10, 11, 12]
            }
    
    # create pandas dataframe
    pdf1 = pd.DataFrame(data1)
  
    # create frovedis dataframe
    fdf1 = fdf.DataFrame(pdf1)
    
    # display the frovedis dataframe
    fdf1.show()

Output  

    index   points  total
    0       5       10
    1       6       11
    2       4       12

For example,  

    # radd() demo with scalar value using method version
    fdf1.radd(10).show()

Output  

    index   points  total
    0       15      20
    1       16      21
    2       14      22

Here, it adds the scalar to all columns in dataframe (axis = 1 by default).  

For example,  

    import pandas as pd
    import frovedis.dataframe as fdf
    
    # a dictionary
    data1 = {
             "points": [5, 6, 4],
             "total": [10, 11, 12]
            }
    
    # create pandas dataframe
    pdf1 = pd.DataFrame(data1)
    
    # create frovedis dataframe
    fdf1 = fdf.DataFrame(pdf1)
    
    # display the frovedis dataframe
    fdf1.show()
    
Output  

    index   points  total
    0       5       10
    1       6       11
    2       4       12

For examples,  
    
    # a dictionary
    data2 = {
             "points": [2, 3, 8],
             "total": [7, 9, 11]
            }
    
    # create pandas dataframe
    pdf2 = pd.DataFrame(data2)
    
    # create frovedis dataframe
    fdf2 = fdf.DataFrame(pdf2)
    
    # display the frovedis dataframe
    fdf2.show()

Output  

    index   points  total
    0       2       7
    1       3       9
    2       8       11

For example,  

    # radd() demo on two dataframes using method version
    fdf1.radd(other = fdf2).show()

Output  

    index   points  total
    0       7       17
    1       9       20
    2       12      23

Here, only common columns in both dataframes are added. Column values in other 
datframe are replaced with NaN values in resultant dataframe (fill_value = None by default).  

**When fill_value is not None,**     

For example,  

    import pandas as pd
    import frovedis.dataframe as fdf
    
    # a dictionary
    data1 = {
             "points": [5, 6, 4],
             "total": [10, 11, np.nan]
            }

    # create pandas dataframe
    pdf1 = pd.DataFrame(data1)
    
    # create frovedis dataframe
    fdf1 = fdf.DataFrame(pdf1)
    
    # display the frovedis dataframe
    fdf1.show()

Output  

    index   points  total
    0       5       10
    1       6       11
    2       4       NULL

For example,  
    
    # a dictionary
    data2 = {
             "points": [2, 3, 8],
             "total": [7, np.nan, np.nan]
            }
    
    # create pandas dataframe
    pdf2 = pd.DataFrame(data2)
    
    # create frovedis dataframe
    fdf2 = fdf.DataFrame(pdf2)

    # display the frovedis dataframe
    fdf2.show()

Output  

    index   points  total
    0       2       7
    1       3       NULL
    2       8       NULL

For example,  
    
    # radd() demo on two dataframes using method version and fill_value = 10
    fdf1.radd(other = fdf2, fill_value = 10).show()

Output  

    index   points  total
    0       7       17
    1       9       21
    2       12      20

Here, only common columns in both dataframes are added excluding the missing values. The fill_value = 10 
is added to both column values in the dataframe (excluding the missing values) and stored in new dataframe.  

__Return Value__  
It returns a frovedis DataFrame which contains the result of arithmetic operation.  

### 11. DataFrame.rdiv(other, axis = 'columns', level = None, fill_value = None)  

__Parameters__  
**_other_**: It can accept single or multiple element data structure like the following:  
- Number  
- List. Currently, this method supports operation on only list of numeric values.  
- A numpy ndarray. Currently, this method supports operation on only an array of numeric values.  
- pandas DataFrame. It must not be an empty dataframe.  
- pandas Series  
- frovedis DataFrame. It must not be an empty dataframe.  

Any of these is considered as the value to be divided with the current dataframe.  
**_axis_**: It accepts an integer or string object as parameter. It is used to decide whether to 
perform reverse division operation along the indices or by column labels. (Default: 'columns')  
- **1 or 'columns'**: perform reverse division operation on the columns. Currently, axis = 1 is supported in this method.  

**_level_**: This is an unused parameter. (Default: None)  
**_fill\_value_**: It accepts scalar values or None. It fills existing missing (NaN) values, and any 
new element needed for successful dataframe alignment, with this value before computation.  (Default: None)  
Irrespective of the specified value, if data in both corresponding dataframe locations is missing, then 
the result will be missing (contains NaNs).  

__Purpose__  
It performs reverse floating division operation between two dataframes or dataframe and a scalar value. It is 
equivalent to **'other / dataframe'**.  

It is an alias of rtruediv().  

**Currently, it does not perform reverse division of scalar using operator version. Only method version is supported.**  

For example,  

    import pandas as pd
    import frovedis.dataframe as fdf
    
    # a dictionary
    data1 = {
             "points": [5, 6, 4],
             "total": [10, 11, 12]
            }

    # create pandas dataframe
    pdf1 = pd.DataFrame(data1)
    
    # create frovedis dataframe
    fdf1 = fdf.DataFrame(pdf1)
    
    # display the frovedis dataframe
    fdf1.show()
    
Output  

    index   points  total
    0       5       10
    1       6       11
    2       4       12

For example,  

    # rdiv() demo with scalar value using method version
    fdf1.rdiv(10).show()

Output  

    index   points  total
    0       2       1
    1       1.66666 0.90909
    2       2.5     0.833333

Here, it uses the scalar to perform division on all column elements in dataframe (axis = 1 by default).  

For example,  

    import pandas as pd
    import frovedis.dataframe as fdf
    
    # a dictionary
    data1 = {
             "points": [5, 6, 4],
             "total": [10, 11, 12]
            }

    # create pandas dataframe
    pdf1 = pd.DataFrame(data1)
    
    # create frovedis dataframe
    fdf1 = fdf.DataFrame(pdf1)
    
    # display the frovedis dataframe
    fdf1.show()
    
Output  
    
    index   points  total
    0       5       10
    1       6       11
    2       4       12
    
For example,  

    # a dictionary
    data2 = {
             "points": [2, 3, 8],
             "total": [7, 9, 11]
            }
    
    # create pandas dataframe
    pdf2 = pd.DataFrame(data2)
    
    # create frovedis dataframe
    fdf2 = fdf.DataFrame(pdf2)
    
    # display the frovedis dataframe
    fdf2.show()

Output  

    index   points  total
    0       2       7
    1       3       9
    2       8       11

For example,  

    # rdiv() demo on two dataframes using method version
    fdf1.rdiv(other = fdf2).show()

Output  

    index   points  total
    0       0.4     0.7
    1       0.5     0.818181
    2       2       0.916666

Here, only common columns in both dataframes are divided. Other are replaced with 
NaN values in resultant dataframe (fill_value = None by default).  

**When fill_value is not None,**  

For example,  

    import pandas as pd
    import frovedis.dataframe as fdf
    
    # a dictionary
    data1 = {
             "points": [5, 6, 4],
             "total": [10, 11, np.nan]
            }

    # create pandas dataframe
    pdf1 = pd.DataFrame(data1)
    
    # create frovedis dataframe
    fdf1 = fdf.DataFrame(pdf1)
    
    # display the frovedis dataframe
    fdf1.show()

Output  

    index   points  total
    0       5       10
    1       6       11
    2       4       NULL

For example,  
    
    # a dictionary
    data2 = {
             "points": [2, 3, 8],
             "total": [7, np.nan, np.nan]
            }
    
    # create pandas dataframe
    pdf2 = pd.DataFrame(data2)
    
    # create frovedis dataframe
    fdf2 = fdf.DataFrame(pdf2)

    # display the frovedis dataframe
    fdf2.show()

Output  

    index   points  total
    0       2       7
    1       3       NULL
    2       8       NULL

For example,  

    # rdiv() demo on two dataframes using method version and fill_value = 10
    fdf1.rdiv(other = fdf2, fill_value = 10).show()

Output  

    index   points  total
    0       0.4     0.7
    1       0.5     0.90909
    2       2       1

Here, only common columns in both dataframes are divided, excluding the missing values. The fill_value = 10 
is used to divide over column values in other dataframe (excluding the missing values) and stored in new dataframe.  

__Return Value__  
It returns a frovedis DataFrame which contains the result of arithmetic operation.  

### 12. DataFrame.rfloordiv(other, axis = 'columns', level = None, fill_value = None)

__Parameters__  
**_other_**: It can accept single or multiple element data structure like the following:  
- Number  
- List. Currently, this method supports operation on only list of numeric values.  
- A numpy ndarray. Currently, this method supports operation on only an array of numeric values.  
- pandas DataFrame. It must not be an empty dataframe.  
- pandas Series  
- frovedis DataFrame. It must not be an empty dataframe.  

Any of these is considered as the value to be divided with the current dataframe.  
**_axis_**: It accepts an integer or string object as parameter. It is used to decide whether to 
perform reverse division operation along the indices or by column labels. (Default: 'columns')  
- **1 or 'columns'**: perform reverse division operation on the columns. Currently, axis = 1 is supported in this method.  

**_level_**: This is an unused parameter. (Default: None)  
**_fill\_value_**: It accepts scalar values or None. It fills existing missing (NaN) values, and any 
new element needed for successful dataframe alignment, with this value before computation.  (Default: None)  
Irrespective of the specified value, if data in both corresponding dataframe locations is missing, then 
the result will be missing (contains NaNs).  

__Purpose__  
It performs reverse floating division operation between two dataframes or dataframe and scalar value. It is 
equivalent to **'other // dataframe'**.  

**Currently, it does not perform reverse division of scalar using operator version. Only method version is supported.**  

For example,  

    import pandas as pd
    import frovedis.dataframe as fdf
    
    # a dictionary
    data1 = {
             "points": [5, 6, 4],
             "total": [10, 11, 12]
            }

    # create pandas dataframe
    pdf1 = pd.DataFrame(data1)
    
    # create frovedis dataframe
    fdf1 = fdf.DataFrame(pdf1)
    
    # display the frovedis dataframe
    fdf1.show()

Output  

    index   points  total
    0       5       10
    1       6       11
    2       4       12

For example,  

    # rfloordiv() demo with scalar value using method version
    fdf1.rfloordiv(10).show()

Output  

    index   points  total
    0       2       1
    1       1       0
    2       2       0

Here, it uses the scalar to perform division on all column elements (axis = 1 by default).  
Also, resultant dataframe column elements will contain floor integer value.  

For example,  

    import pandas as pd
    import frovedis.dataframe as fdf
    
    # a dictionary
    data1 = {
             "points": [5, 6, 4],
             "total": [10, 11, 12]
            }
    
    # create pandas dataframe
    pdf1 = pd.DataFrame(data1)
    
    # create frovedis dataframe
    fdf1 = fdf.DataFrame(pdf1)
    
    # display the frovedis dataframe
    fdf1.show()

Output  

    index   points  total
    0       5       10
    1       6       11
    2       4       12

For example,  
    
    # a dictionary
    data2 = {
             "points": [2, 3, 8],
             "total": [7, 9, 11]
            }
    
    # create pandas dataframe
    pdf2 = pd.DataFrame(data2)
    
    # create frovedis dataframe
    fdf2 = fdf.DataFrame(pdf2)

    # display the frovedis dataframe
    fdf2.show()

Output  

    index   points  total
    0       2       7
    1       3       9
    2       8       11

For example,  

    # rfloordiv() demo on two dataframes using method version
    fdf1.rfloordiv(other = fdf2).show()

Output  

    index   points  total
    0       0       0
    1       0       0
    2       2       0

Here, only common columns in both dataframes are divided. Other are replaced with 
NaN values in resultant dataframe (fill_value = None by default).  

**When fill_value is not None,**  

For example,  

    import pandas as pd
    import frovedis.dataframe as fdf
    
    # a dictionary
    data1 = {
             "points": [5, 6, 4],
             "total": [10, 11, np.nan]
            }

    # create pandas dataframe
    pdf1 = pd.DataFrame(data1)
    
    # create frovedis dataframe
    fdf1 = fdf.DataFrame(pdf1)
    
    # display the frovedis dataframe
    fdf1.show()

Output  

    index   points  total
    0       5       10
    1       6       11
    2       4       NULL

For example,  
    
    # a dictionary
    data2 = {
             "points": [2, 3, 8],
             "total": [7, np.nan, np.nan]
            }
    
    # create pandas dataframe
    pdf2 = pd.DataFrame(data2)
    
    # create frovedis dataframe
    fdf2 = fdf.DataFrame(pdf2)

    # display the frovedis dataframe
    fdf2.show()

Output  

    index   points  total
    0       2       7
    1       3       NULL
    2       8       NULL

For example,  
    
    # rfloordiv()  demo on two dataframes using method version and fill_value = 10
    fdf1.rfloordiv(other = fdf2, fill_value = 10).show()

Output  

    index   points  total
    0       0       0
    1       0       0
    2       2       1

Here, only common columns in both dataframes are divided, excluding the missing values. The fill_value = 10 
is used to divide over column values in other dataframe (excluding the missing values) and stored in new dataframe.  

__Return Value__  
It returns a frovedis DataFrame which contains the result of arithmetic operation.  

### 13. DataFrame.rmod(other, axis = 'columns', level = None, fill_value = None)  

__Parameters__  
**_other_**: It can accept single or multiple element data structure like the following:  
- Number  
- List. Currently, this method supports operation on only list of numeric values.  
- A numpy ndarray. Currently, this method supports operation on only an array of numeric values.  
- pandas DataFrame. It must not be an empty dataframe.  
- pandas Series  
- frovedis DataFrame. It must not be an empty dataframe.  

Any of these is considered as the value to perform modulo operation with the current dataframe.  
**_axis_**: It accepts an integer or string object as parameter. It is used to decide whether to 
perform reverse modulo operation along the indices or by column labels. (Default: 'columns')  
- **1 or 'columns'**: perform reverse modulo operation on the columns. Currently, axis = 1 is supported in this method.  

**_level_**: This is an unused parameter. (Default: None)  
**_fill\_value_**: It accepts scalar values or None. It fills existing missing (NaN) values, and any 
new element needed for successful dataframe alignment, with this value before computation.  (Default: None)  
Irrespective of the specified value, if data in both corresponding dataframe locations is missing, then 
the result will be missing (contains NaNs).  

__Purpose__  
It performs reverse modulo operation between two dataframes or dataframe and scalar value. It is equivalent 
to **'other % dataframe'**.  

**Currently, it does not perform reverse modulo of scalar using operator version. Only method version is supported.**  

For example,  

    import pandas as pd
    import frovedis.dataframe as fdf
    
    # a dictionary
    data1 = {
            "points": [5, 6, 4],
            "total": [50, 40, 20]
           }

    # create pandas dataframe
    pdf1 = pd.DataFrame(data1)
    
    # create frovedis dataframe
    fdf1 = fdf.DataFrame(pdf1)
    
    # display the frovedis dataframe
    fdf1.show()

Output  

    index   points  total
    0       5       10
    1       6       11
    2       4       12

For example,  

    # rmod() demo with scalar value using method version
    fdf1.rmod(10).show()

Output  

    index   points  total
    0       0       0
    1       4       10
    2       2       10

Here, it uses the scalar to perform modulo operation on all column elements (axis = 1 by default).   

For example,  

    import pandas as pd
    import frovedis.dataframe as fdf
    
    # a dictionary
    data1 = {
             "points": [5, 6, 4],
             "total": [10, 11, 12]
            }
    
    # create pandas dataframe
    pdf1 = pd.DataFrame(data1)
    
    # create frovedis dataframe
    fdf1 = fdf.DataFrame(pdf1)
    
    # display the frovedis dataframe
    fdf1.show()

Output  

    index   points  total
    0       5       10
    1       6       11
    2       4       12

For example,  

    # a dictionary
    data2 = {
             "points": [2, 3, 8],
             "total": [7, 9, 11]
            }    
    
    # create pandas dataframe
    pdf2 = pd.DataFrame(data2)
    
    # create frovedis dataframe
    fdf2 = fdf.DataFrame(pdf2)

    # display the frovedis dataframe
    fdf2.show()

Output  

    index   points  total
    0       2       7
    1       3       9
    2       8       11

For example,  

    # rmod() demo on two dataframes using method version
    fdf1.rmod(other = fdf2).show()

Output  

    index   points  total
    0       2       7
    1       3       9
    2       0       11

Here, modulo is performed on only common columns in both dataframes. Other are replaced with 
NaN values in resultant dataframe (fill_value = None by default).  

**When fill_value is not None,**  

For example,  

    import pandas as pd
    import frovedis.dataframe as fdf
    
    # a dictionary
    data1 = {
             "points": [5, 6, 4],
             "total": [10, 11, np.nan]
            }

    # create pandas dataframe
    pdf1 = pd.DataFrame(data1)
    
    # create frovedis dataframe
    fdf1 = fdf.DataFrame(pdf1)
    
    # display the frovedis dataframe
    fdf1.show()

Output  

    index   points  total
    0       5       10
    1       6       11
    2       4       NULL

For example,  
    
    # a dictionary
    data2 = {
             "points": [2, 3, 8],
             "total": [7, np.nan, np.nan]
            }
    
    # create pandas dataframe
    pdf2 = pd.DataFrame(data2)
    
    # create frovedis dataframe
    fdf2 = fdf.DataFrame(pdf2)

    # display the frovedis dataframe
    fdf2.show()

Output  

    index   points  total
    0       2       7
    1       3       NULL
    2       8       NULL

For example,  
    
    # rmod() demo on two dataframes using method version and fill_value = 10
    fdf1.rmod(other = fdf2, fill_value = 10).show()

Output  

    index   points  total
    0       2       7
    1       3       10
    2       0       0

Here, modulo is performed on only common columns in both dataframes, excluding the missing values. The 
fill_value = 10 is used to perform modulo over column values in other dataframe (excluding the missing values) 
and stored in new dataframe.  

__Return Value__  
It returns a frovedis DataFrame which contains the result of arithmetic operation.  

### 14. DataFrame.rmul(other, axis = 'columns', level = None, fill_value = None)  

__Parameters__  
**_other_**: It can accept single or multiple element data structure like the following:  
- Number  
- List. Currently, this method supports operation on only list of numeric values.  
- A numpy ndarray. Currently, this method supports operation on only an array of numeric values.  
- pandas DataFrame. It must not be an empty dataframe.  
- pandas Series  
- frovedis DataFrame. It must not be an empty dataframe.  

Any of these is considered as the value to be multiplied with the current dataframe.  
**_axis_**: It accepts an integer or string object as parameter. It is used to decide whether to 
perform reverse multiplication operation along the indices or by column labels. (Default: 'columns')  
- **1 or 'columns'**: perform reverse multiplication operation on the columns. Currently, axis = 1 is supported in this method.  

**_level_**: This is an unused parameter. (Default: None)  
**_fill\_value_**: It accepts scalar values or None. It fills existing missing (NaN) values, and any 
new element needed for successful dataframe alignment, with this value before computation.  (Default: None)  
Irrespective of the specified value, if data in both corresponding dataframe locations is missing, then 
the result will be missing (contains NaNs).  

__Purpose__  
It performs reverse multiplication operation between two dataframes or dataframe and scalar value. It is equivalent 
to **'other * dataframe'**.  

**Currently, it does not perform reverse multiplication of scalar using operator version. Only method 
version is supported.**  

For example,  

    import pandas as pd
    import frovedis.dataframe as fdf
    
    # a dictionary
    data1 = {
             "points": [5, 6, 4],
             "total": [10, 11, 12]
            }   

    # create pandas dataframe
    pdf1 = pd.DataFrame(data1)
    
    # create frovedis dataframe
    fdf1 = fdf.DataFrame(pdf1)
    
    # display the frovedis dataframe
    fdf1.show()

Output  

    index   points  total
    0       5       10
    1       6       11
    2       4       12

For example,  

    # rmul() demo with scalar value using method version
    fdf1.rmul(10).show()

Output  

    index   points  total
    0       50      100
    1       60      110
    2       40      120

Here, it uses the scalar to perform multiplication on all column elements in dataframe (axis = 1 by default).  

For example,  

    import pandas as pd
    import frovedis.dataframe as fdf
    
    # a dictionary
    data1 = {
             "points": [5, 6, 4],
             "total": [10, 11, 12]
            }
    
    # create pandas dataframe
    pdf1 = pd.DataFrame(data1)
    
    # create frovedis dataframe
    fdf1 = fdf.DataFrame(pdf1)
    
    # display the frovedis dataframe
    fdf1.show()

Output  

    index   points  total
    0       5       10
    1       6       11
    2       4       12

For example,  

    # a dictionary
    data2 = {
             "points": [2, 3, 8],
             "total": [7, 9, 11]
            }    
    
    # create pandas dataframe
    pdf2 = pd.DataFrame(data2)
    
    # create frovedis dataframe
    fdf2 = fdf.DataFrame(pdf2)

    # display the frovedis dataframe
    fdf2.show()

Output  

    index   points  total
    0       2       7
    1       3       9
    2       8       11

For example,  
    
    # rmul() demo on two dataframes using method version
    fdf1.rmul(other = fdf2).show()

Output  

    index   points  total
    0       10      70
    1       18      99
    2       32      132

Here, only common columns in both dataframes are multiplied. Other are replaced with 
NaN values in resultant dataframe (fill_value = None by default).  

**When fill_value is not None,**  

For example,  

    import pandas as pd
    import frovedis.dataframe as fdf
    
    # a dictionary
    data1 = {
             "points": [5, 6, 4],
             "total": [10, 11, np.nan]
            }

    # create pandas dataframe
    pdf1 = pd.DataFrame(data1)
    
    # create frovedis dataframe
    fdf1 = fdf.DataFrame(pdf1)
    
    # display the frovedis dataframe
    fdf1.show()

Output  

    index   points  total
    0       5       10
    1       6       11
    2       4       NULL

For example,  
    
    # a dictionary
    data2 = {
             "points": [2, 3, 8],
             "total": [7, np.nan, np.nan]
            }
    
    # create pandas dataframe
    pdf2 = pd.DataFrame(data2)
    
    # create frovedis dataframe
    fdf2 = fdf.DataFrame(pdf2)

    # display the frovedis dataframe
    fdf2.show()

Output  

    index   points  total
    0       2       7
    1       3       NULL
    2       8       NULL

For example,  
    
    # rmul() demo on two dataframes using method version and fill_value = 10
    fdf1.rmul(other = fdf2, fill_value = 10).show()

Output  

    index   points  total
    0       10      70
    1       18      110
    2       32      100

Here, only common columns in both dataframes are multiplied, excluding the missing values. The 
fill_value = 10 is multiplied with column values in other dataframe (excluding the missing values) and 
stored in new dataframe.  

__Return Value__  
It returns a frovedis DataFrame which contains the result of arithmetic operation.  

### 15. DataFrame.rpow(other, axis = 'columns', level = None, fill_value = None)  

__Parameters__  
**_other_**: It can accept single or multiple element data structure like the following:  
- Number  
- List. Currently, this method supports operation on only list of numeric values.  
- A numpy ndarray. Currently, this method supports operation on only an array of numeric values.  
- pandas DataFrame. It must not be an empty dataframe.  
- pandas Series  
- frovedis DataFrame. It must not be an empty dataframe.  

Any of these is considered as the value to perfomr exponential power operation with the current dataframe.  
**_axis_**: It accepts an integer or string object as parameter. It is used to decide whether to 
perform reverse exponential power operation along the indices or by column labels. (Default: 'columns')  
- **1 or 'columns'**: perform reverse exponential power operation on the columns. Currently, axis = 1 is supported in this method.  

**_level_**: This is an unused parameter. (Default: None)  
**_fill\_value_**: It accepts scalar values or None. It fills existing missing (NaN) values, and any 
new element needed for successful dataframe alignment, with this value before computation.  (Default: None)  
Irrespective of the specified value, if data in both corresponding dataframe locations is missing, then 
the result will be missing (contains NaNs).  

__Purpose__  
It performs reverse exponential power operation between two dataframes or dtaframe and scalar value. It is 
equivalent to **'other ** dataframe'**.  

**Currently, it does not perform reverse exponential power operation of scalar using operator version. Only 
method version is supported.**  

For example,  

    import pandas as pd
    import frovedis.dataframe as fdf
    
    # a dictionary
    data1 = {
            "points": [5, 6, 4],
            "total": [10, 11, 12]
           }
    
    # create pandas dataframe
    pdf1 = pd.DataFrame(data1)
    
    # create frovedis dataframe
    fdf1 = fdf.DataFrame(pdf1)
    
    # display the frovedis dataframe
    fdf1.show()

Output  

    index   points  total
    0       5       10
    1       6       11
    2       4       12

For example,  

    # rpow() demo with scalar value using method version
    fdf1.rpow(2).show()

Output  

    index   points  total
    0       32      1024
    1       64      2048
    2       16      4096

Here, it uses the scalar to perform exponential power operation on all column 
elements in dataframe (axis = 1 by default).  

For example,  

    import pandas as pd
    import frovedis.dataframe as fdf
    
    # a dictionary
    data1 =  {
              "points": [5, 6, 4],
              "total": [10, 11, 12]
            }
    
    # create pandas dataframe
    pdf1 = pd.DataFrame(data1)
    
    # create frovedis dataframe
    fdf1 = fdf.DataFrame(pdf1)
    
    # display the frovedis dataframe
    fdf1.show()

Output  

    index   points  total
    0       5       10
    1       6       11
    2       4       12

For example,  

    # a dictionary
    data2 = {
             "points": [2, 3, 8],
             "total": [7, 9, 11]
            }    
    
    # create pandas dataframe
    pdf2 = pd.DataFrame(data2)
    
    # create frovedis dataframe
    fdf2 = fdf.DataFrame(pdf2)
    
    # display the frovedis dataframe
    fdf2.show()

Output  

    index   points  total
    0       2       7
    1       3       9
    2       8       11

For example,  

    # rpow() demo on two dataframes using method version
    fdf1.rpow(other = fdf2).show()

Output  

    index   points  total
    0       10      70
    1       18      99
    2       32      132

Here, exponential power operation is performed on only common columns in both 
dataframes. Other are replaced with NaN values in resultant dataframe (fill_value = None by default).  

**When fill_value is not None,**  

For example,  

    import pandas as pd
    import frovedis.dataframe as fdf
    
    # a dictionary
    data1 = {
             "points": [5, 6, 4],
             "total": [10, 11, np.nan]
            }

    # create pandas dataframe
    pdf1 = pd.DataFrame(data1)
    
    # create frovedis dataframe
    fdf1 = fdf.DataFrame(pdf1)
    
    # display the frovedis dataframe
    fdf1.show()

Output  

    index   points  total
    0       5       10
    1       6       11
    2       4       NULL

For example,  
    
    # a dictionary
    data2 = {
             "points": [2, 3, 8],
             "total": [7, np.nan, np.nan]
            }
    
    # create pandas dataframe
    pdf2 = pd.DataFrame(data2)
    
    # create frovedis dataframe
    fdf2 = fdf.DataFrame(pdf2)

    # display the frovedis dataframe
    fdf2.show()

Output  

    index   points  total
    0       2       7
    1       3       NULL
    2       8       NULL

For example,  
    
    # rpow() demo on two dataframes using method version and fill_value = 10
    fdf1.rpow(other = fdf2, fill_value = 10).show()

Output  

    index   points  total
    0       32      2.82475e+08
    1       729     9.99999e+10
    2       4096    1.00000e+10

Here, exponential power operation is performed on only common columns in both dataframes, excluding 
the missing values. The fill_value = 10 is used to perform exponential power operation on column values 
in other dataframe (excluding the missing values) and stored in new dataframe.  

__Return Value__  
It returns a frovedis DataFrame which contains the result of arithmetic operation.  

### 16. DataFrame.rsub(other, axis = 'columns', level = None, fill_value = None)  

__Parameters__  
**_other_**: It can accept single or multiple element data structure like the following:  
- Number  
- List. Currently, this method supports operation on only list of numeric values.  
- A numpy ndarray. Currently, this method supports operation on only an array of numeric values.  
- pandas DataFrame. It must not be an empty dataframe.  
- pandas Series  
- frovedis DataFrame. It must not be an empty dataframe.  

Any of these is considered as the value to be subtracted with the current dataframe.  
**_axis_**: It accepts an integer or string object as parameter. It is used to decide whether to 
perform reverse subtraction operation along the indices or by column labels. (Default: 'columns')  
- **1 or 'columns'**: perform reverse subtraction operation on the columns. Currently, axis = 1 is supported in this method.  

**_level_**: This is an unused parameter. (Default: None)  
**_fill\_value_**: It accepts scalar values or None. It fills existing missing (NaN) values, and any 
new element needed for successful dataframe alignment, with this value before computation.  (Default: None)  
Irrespective of the specified value, if data in both corresponding dataframe locations is missing, then 
the result will be missing (contains NaNs).  

__Purpose__  
It performs reverse subtraction operation between two dataframes or dataframe and scalar value. It is 
equivalent to **'other - dataframe'**.  

**Currently, it does not perform reverse subtraction of scalar using operator version. Only method version is supported.**  

For example,  

    import pandas as pd
    import frovedis.dataframe as fdf
    
    # a dictionary
    data1 =  {
              "points": [5, 6, 4],
              "total": [10, 11, 12]
            }
    
    # create pandas dataframe
    pdf1 = pd.DataFrame(data1)
  
    # create frovedis dataframe
    fdf1 = fdf.DataFrame(pdf1)
    
    # display the frovedis dataframe
    fdf1.show()
    
Output  

    index   points  total
    0       5       10
    1       6       11
    2       4       12

For example,  

    # rsub() with scalar value using method version
    fdf1.rsub(10).show()

Output  

    index   points  total
    0       5       0
    1       4       -1
    2       6       -2

Here, it subtracts the scalar to all columns in dataframe (axis = 1 by default).  

For example,  

    import pandas as pd
    import frovedis.dataframe as fdf
    
    # a dictionary
    data1 =  {
              "points": [5, 6, 4],
              "total": [10, 11, 12]
            }

    # create pandas dataframe
    pdf1 = pd.DataFrame(data1)
    
    # create frovedis dataframe
    fdf1 = fdf.DataFrame(pdf1)
    
    # display the frovedis dataframe
    fdf1.show()

Output  

    index   points  total
    0       5       10
    1       6       11
    2       4       12

For example,  

    # a dictionary
    data2 = {
             "points": [2, 3, 8],
             "total": [7, 9, 11]
            }

    # create pandas dataframe
    pdf2 = pd.DataFrame(data2)
    
    # create frovedis dataframe
    fdf2 = fdf.DataFrame(pdf2)
    
    # display the frovedis dataframe
    fdf2.show()

Output  

    index   points  total
    0       2       7
    1       3       9
    2       8       11

For example,  

    # rsub() on two dataframes using method version
    fdf1.rsub(other = fdf2).show()

Output  

    index   points  total
    0       -3      -3
    1       -3      -2
    2       4       -1

Here, only common columns in both dataframes are subtracted. Other are replaced with 
NaN values in resultant dataframe (fill_value = None by default).  

**When fill_value is not None,**  

For example,  

    import pandas as pd
    import frovedis.dataframe as fdf
    
    # a dictionary
    data1 = {
             "points": [5, 6, 4],
             "total": [10, 11, np.nan]
            }

    # create pandas dataframe
    pdf1 = pd.DataFrame(data1)
    
    # create frovedis dataframe
    fdf1 = fdf.DataFrame(pdf1)
    
    # display the frovedis dataframe
    fdf1.show()

Output  

    index   points  total
    0       5       10
    1       6       11
    2       4       NULL

For example,  
    
    # a dictionary
    data2 = {
             "points": [2, 3, 8],
             "total": [7, np.nan, np.nan]
            }
    
    # create pandas dataframe
    pdf2 = pd.DataFrame(data2)
    
    # create frovedis dataframe
    fdf2 = fdf.DataFrame(pdf2)

    # display the frovedis dataframe
    fdf2.show()

Output  

    index   points  total
    0       2       7
    1       3       NULL
    2       8       NULL

For example,  
    
    # rsub() on two dataframes using method version and fill_value = 10
    fdf1.rsub(other = fdf2, fill_value = 10).show()

Output  

    index   points  total
    0       -3      -3
    1       -3      -1
    2       4       0

Here, only common columns in both dataframes are subtracted, excluding the missing values. The 
fill_value = 10 is subtracted on column values in other dataframe (excluding the missing values) and 
stored in new dataframe.  

__Return Value__  
It returns a frovedis DataFrame which contains the result of arithmetic operation.  

### 17. DataFrame.rtruediv(other, axis = 'columns', level = None, fill_value = None)  

__Parameters__  
**_other_**: It can accept single or multiple element data structure like the following:  
- Number  
- List. Currently, this method supports operation on only list of numeric values.  
- A numpy ndarray. Currently, this method supports operation on only an array of numeric values.  
- pandas DataFrame. It must not be an empty dataframe.  
- pandas Series  
- frovedis DataFrame. It must not be an empty dataframe.  

Any of these is considered as the value to be divided with the current dataframe.  
**_axis_**: It accepts an integer or string object as parameter. It is used to decide whether to 
perform reverse division operation along the indices or by column labels. (Default: 'columns')  
- **1 or 'columns'**: perform reverse division operation on the columns. Currently, axis = 1 is supported in this method.  

**_level_**: This is an unused parameter. (Default: None)  
**_fill\_value_**: It accepts scalar values or None. It fills existing missing (NaN) values, and any 
new element needed for successful dataframe alignment, with this value before computation.  (Default: None)  
Irrespective of the specified value, if data in both corresponding dataframe locations is missing, then 
the result will be missing (contains NaNs).  

__Purpose__  
It performs reverse floating division operation between two dataframes or dtaframe and scalar value. It is 
equivalent to **'other / dataframe'**.  

**Currently, it does not perform reverse division of scalar using operator version. Only method version is supported.**  

For example,  

    import pandas as pd
    import frovedis.dataframe as fdf
    
    # a dictionary
    data1 =  {
              "points": [8, 5, 9],
              "total": [3, 2, 1]
            }

    # create pandas dataframe
    pdf1 = pd.DataFrame(data1)
    
    # create frovedis dataframe
    fdf1 = fdf.DataFrame(pdf1)
    
    # display the frovedis dataframe
    fdf1.show()

Output  

    index   points  total
    0       8       3
    1       5       2
    2       9       1

For example,  

    # rtruediv() demo with scalar value using method version
    fdf1.rtruediv(10).show()

Output  

    index   points  total
    0       1.25    3.33333
    1       2       5
    2       1.11111 10

Here, it uses the scalar to perform division on all column elements in dataframe (axis = 1 by default).  

For example,  

    import pandas as pd
    import frovedis.dataframe as fdf
    
    # a dictionary
    data1 =  {
              "points": [8, 5, 9],
              "total": [3, 2, 1]
            }

    # create pandas dataframe
    pdf1 = pd.DataFrame(data1)
    
    # create frovedis dataframe
    fdf1 = fdf.DataFrame(pdf1)
    
    # display the frovedis dataframe
    fdf1.show()
    
Output  

    index   points  total
    0       8       3
    1       5       2
    2       9       1

For example,  

    # a dictionary
    data2 =  {
              "points": [4, 7, 2],
              "total": [9, 1, 9]
            }

    # create pandas dataframe
    pdf2 = pd.DataFrame(data2)
    
    # create frovedis dataframe
    fdf2 = fdf.DataFrame(pdf2)
    
    # display the frovedis dataframe
    fdf2.show()

Output  

    index   points  total
    0       4       9
    1       7       1
    2       2       9

For example,  

    # rtruediv() demo on two dataframes using method version
    fdf1.rtruediv(other = fdf2).show()

Output  

    index   points    total
    0       0.5       3
    1       1.39999   0.5
    2       0.222222  9

Here, only common columns in both dataframes are divided. Other are replaced with 
NaN values in resultant dataframe (fill_value = None by default).  

**When fill_value is not None,**  

For example,  

    import pandas as pd
    import frovedis.dataframe as fdf
    
    # a dictionary
    data1 = {
             "points": [5, 6, 4],
             "total": [10, 11, np.nan]
            }

    # create pandas dataframe
    pdf1 = pd.DataFrame(data1)
    
    # create frovedis dataframe
    fdf1 = fdf.DataFrame(pdf1)
    
    # display the frovedis dataframe
    fdf1.show()

Output  

    index   points  total
    0       5       10
    1       6       11
    2       4       NULL

For example,  
    
    # a dictionary
    data2 = {
             "points": [2, 3, 8],
             "total": [7, np.nan, np.nan]
            }
    
    # create pandas dataframe
    pdf2 = pd.DataFrame(data2)
    
    # create frovedis dataframe
    fdf2 = fdf.DataFrame(pdf2)

    # display the frovedis dataframe
    fdf2.show()

Output  

    index   points  total
    0       2       7
    1       3       NULL
    2       8       NULL

For example,  
    
    # rtruediv() demo on two dataframes using method version and fill_value = 10
    fdf1.rtruediv(other = fdf2, fill_value = 10).show()

Output  

    index   points  total
    0       0.4     0.7
    1       0.5     0.90909
    2       2       1

Here, only common columns in both dataframes are divided, excluding the missing values. The fill_value = 10 
is divided with column values in other dataframe (excluding the missing values) and stored in new dataframe.  

__Return Value__  
It returns a frovedis DataFrame which contains the result of arithmetic operation.  

# SEE ALSO

- **[DataFrame - Introduction](./df_intro.md)**  
- **[DataFrame - Generic Functions](./df_generic_func.md)**  
- **[DataFrame - Conversion Functions](./df_conversion.md)**  
- **[DataFrame - Sorting Functions](./df_sorting.md)**  
- **[DataFrame - Aggregate Functions](./df_agg_func.md)**   