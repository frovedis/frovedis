% DataFrame Math Functions  

# NAME  

DataFrame Math Functions - this manual contains all the methods for carrying out mathematical operations.  

## DESCRIPTION  

Frovedis dataframe has several math functions defined for performing operations like add(), sub(), mul(), etc. between two dataframes or between scalar value and dataframe. These functions return a new frovedis DataFrame instance as a result.  

Also, it contains reverse operations such as radd(), rsub(), rmul(), etc.  

However, there are some special cases while using frovedis dataframe with mathematical operations as mentioned below:    

**Binary operation on two frovedis dataframes having same columns but different datatypes:**  

For example,  

    import pandas as pd
    import frovedis.dataframe as fdf
    data1 = {
              "points": [5, 6, 4],
               "total": [10, 11, 12]
            }

    pdf1 = pd.DataFrame(data1)
    fdf1 = fdf.DataFrame(pdf1)

    data2 = {
             "points": [2., 3., 8.],
             "total": [7, 9, 11]
            }

    pdf2 = pd.DataFrame(data2)
    fdf2 = fdf.DataFrame(pdf2)

    print(fdf1 + fdf2)

Output  

    index   points  total
    0       7       17
    1       9       20
    2       12      23

Here, **'points'** column in first dataframe is **int type** and in other dataframe is **float type**.  

Type conversion occurs for 'points' column in resultant dataframe.

    fdf1.points(int) + fdf2.points(float) -> res.points(float) 

**Binary operation on two frovedis dataframes having atleast one common column:**  

For example,  

    import pandas as pd
    import frovedis.dataframe as fdf
    data1 = {
              "points": [5, 6, 4],
               "total": [10, 11, 12]
            }

    pdf1 = pd.DataFrame(data1)
    fdf1 = fdf.DataFrame(pdf1)

    data2 = {
             "score": [2, 3, 8],
             "total": [7, 9, 11]
            }

    pdf2 = pd.DataFrame(data2)
    fdf2 = fdf.DataFrame(pdf2)

    print(fdf1 + fdf2)

Output  

    index   points  score   total
    0       NULL    NULL    17
    1       NULL    NULL    20
    2       NULL    NULL    23

Here, the resultant dataframe will contains all columns from input frovedis dataframes lexicographically.  
Also, binary operation (i.e. addition) is performed on the common column only.  

**Binary operation on two frovedis dataframes having same columns. Also, same indices but in different order:**  

For example,  

    import pandas as pd
    import frovedis.dataframe as fdf
    data1 = {
              "points": [5, 6, 4],
               "total": [10, 11, 12]
            }

    pdf1 = pd.DataFrame(data1, index = [1,2,3])
    fdf1 = fdf.DataFrame(pdf1)

    data2 = {
             "points": [2, 3, 8],
             "total": [7, 9, 11]
            }

    pdf2 = pd.DataFrame(data2, index = [2,3,1])
    fdf2 = fdf.DataFrame(pdf2)

    print(fdf1 + fdf2)

Output  

    index   points  total
    1       13      21
    2       8       18
    3       7       21

Binary operation (i.e. addition) will be performed between same indices for each column irrespective of the index order.  

**Currently, binary operation on two frovedis dataframes having same columns but different indices is not supported**.  

For example,  

    import pandas as pd
    import frovedis.dataframe as fdf
    data1 = {
              "points": [5, 6, 4],
               "total": [10, 11, 12]
            }

    pdf1 = pd.DataFrame(data1, index = [0,1,2])
    fdf1 = fdf.DataFrame(pdf1)

    data2 = {
             "points": [2, 3, 8],
             "total": [7, 9, 11]
            }

    pdf2 = pd.DataFrame(data2, index = [0,1,3])
    fdf2 = fdf.DataFrame(pdf2)

    print(fdf1 + fdf2)

In this case, dataframes have same column but indices are different. This will raise an exception in frovedis.  

**Binary operation between columns of frovedis dataframes. Also, having same indices:**  

For example,  

    import pandas as pd
    import frovedis.dataframe as fdf
    data1 = {
              "points": [5, 6, 4],
               "total": [10, 11, 12]
            }

    pdf1 = pd.DataFrame(data1, index = [1,2,3])
    fdf1 = fdf.DataFrame(pdf1)

    data2 = {
             "points": [2, 3, 8],
             "total": [7, 9, 11]
            }

    pdf2 = pd.DataFrame(data2, index = [1,2,3])
    fdf2 = fdf.DataFrame(pdf2)

    print(fdf1['points'] + fdf2['points'])

Output  

    index   (points+points)
    1       7
    2       9
    3       12

**Binary operation between two frovedis dataframes having single column (common) only. Also, having same indices but different order:**  

For example,  

    import pandas as pd
    import frovedis.dataframe as fdf
    pdf1 = pd.Series([27, 24, 22, 32, 33, 36, 27, 32],index = [0,1,2,3,4,5,6,7])
    pdf2 = pd.Series([23, 34, 35, 45, 23, 50, 52, 34],index = [2,3,1,0,7,4,5,6])

    fdf1 = fdf.DataFrame(pdf1)
    fdf2 = fdf.DataFrame(pdf2)

    print(fdf1 + fdf2)

Output  

    index   (0+0)
    0       72
    1       59
    2       45
    3       66
    4       83
    5       88
    6       61
    7       55

Binary operation (i.e. addition) will be performed between same indices irrespective of the index order in the frovedis dataframe.  

**Currently, binary operation on two frovedis dataframes single column only but different indices is not supported**.  

For example,  

    import pandas as pd
    import frovedis.dataframe as fdf
    pdf1 = pd.Series([27, 24, 22, 32, 33, 36, 27, 32],index = [0,1,2,3,4,5,6,7])
    pdf2 = pd.Series([23, 34, 35, 45, 23, 50, 52, 34],index = [2,3,1,0,8,4,5,9])

    fdf1 = fdf.DataFrame(pdf1)
    fdf2 = fdf.DataFrame(pdf2)

    print(fdf1 + fdf2)

In this case, resultant dataframe has single column but indices are different. This will raise an exception in frovedis.  

**Binary operation between frovedis dataframe and scalar value (float type):**  

For example,  

    import pandas as pd
    import frovedis.dataframe as fdf
    data1 = {
          "points": [5, 6, 4],
           "total": [10, 11, 12]
        }

    pdf1 = pd.DataFrame(data1)
    fdf1 = fdf.DataFrame(pdf1)

    print(fdf1 + 12.)

Output  

    index   points  total
    0       17      22
    1       18      23
    2       16      24
    
Here, binary operation (i.e. addition) will be performed between the scalar value of float type on each column of the resultant frovedis dataframe.  
    
**Binary operation between frovedis dataframe and row vector:**  

For example,  

    import pandas as pd
    import frovedis.dataframe as fdf
    data1 = {
          "points": [5, 6, 4],
           "total": [10, 11, 12]
        }

    pdf1 = pd.DataFrame(data1)
    fdf1 = fdf.DataFrame(pdf1)

    print(fdf1 + [51, 34])

Output  

    index   points  total
    0       56      44
    1       57      45
    2       55      46

Here, binary operation (i.e addition) will be performed between each element of row vector with each column on the given index of the frovedis dataframe.  

**Binary operation between frovedis dataframe and column vector:**  

For example,  

    import pandas as pd
    import frovedis.dataframe as fdf
    data1 = {
          "points": [5, 6, 4],
           "total": [10, 11, 12]
        }

    pdf1 = pd.DataFrame(data1)
    fdf1 = fdf.DataFrame(pdf1)

    print(fdf1['points'] + [21, 34, 45])
    print(fdf1['total'] + [21, 34, 45])

Output  

    index   points
    0       26
    1       40
    2       49

    index   total
    0       31
    1       45
    2       57

Here, binary operation (i.e addition) will be performed between the array and the given column of frovedis dataframe.  

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

**Creating frovedis DataFrame from pandas DataFrame:**  

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

**Absolute numeric values in a frovedis dataframe:**  

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
- List having 1 dimension. Currently, this method supports operation on only list of numeric values.  
- A numpy ndarray having 1 dimension. Currently, this method supports operation on only an array of numeric values.  
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
It performs addition between two operands. It is equivalent to **'dataframe + other'**.  

**Creating frovedis DataFrame from pandas DataFrame:**  

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

**Add a scalar value using operator version:**  

For example,  

    print(fdf1 + 10)

Output  

    index   points  total
    0       15      20
    1       16      21
    2       14      22

**Add a scalar value using method version:**  

For example,  

    fdf1.add(10).show()

Output  

    index   points  total
    0       15      20
    1       16      21
    2       14      22

In both versions, all column elements (axis = 1 by default) are added with a scalar value.  

**Creating two frovedis dataframes to perform addition:**  

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

**Add two dataframes using operator version:**  

For example,  

    print(fdf1 + fdf2)

Output  

    index   points  total
    0       7       17
    1       9       20
    2       12      23
    
**Add two dataframes using method version:**  
    
For example,  

    fdf1.add(other = fdf2).show()

Output  

    index   points  total
    0       7       17
    1       9       20
    2       12      23

In both versions, only common columns in both dataframes are added. Other are replaced with 
NaN values in resultant dataframe (fill_value = None by default).  

**Creating two frovedis dataframes in order to use fill_value parameter during addition:**  

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

**Add two dataframes and using fill_value parameter:**  

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
- List having 1 dimension. Currently, this method supports operation on only list of numeric values.  
- A numpy ndarray having 1 dimension. Currently, this method supports operation on only an array of numeric values.  
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
It performs floating division operation between two operands. It is equivalent to **'dataframe / other'**.  

**It is an alias of truediv().**  

**Creating frovedis DataFrame from pandas DataFrame:**  

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

**Divide a scalar value using operator version:**  

For example,  

    print(fdf1 / 10)

Output  

    index   points  total
    0       0.5     1
    1       0.6     1.1
    2       0.4     1.19999

**Divide a scalar value using method version:**  

For example,  

    fdf1.div(10).show()

Output  

    index   points  total
    0       0.5     1
    1       0.6     1.1
    2       0.4     1.19999

In both versions, all column elements (axis = 1 by default) are divied by a scalar value.  

**Creating two frovedis dataframes to perform division:**  

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

**Divide two dataframes using operator version:**  

For example,  
    
    print(fdf1 / fdf2)

Output  

    index   points  total
    0       2.5     1.42857
    1       2       1.22222
    2       0.5     1.0909

**Divide two dataframes using method version:**  

For example,  

    fdf1.div(other = fdf2).show()

Output  

    index   points  total
    0       2.5     1.42857
    1       2       1.22222
    2       0.5     1.0909

In both versions, only common columns in both dataframes are divided. Other are replaced with 
NaN values in resultant dataframe (fill_value = None by default).  

**Creating two frovedis dataframes in order to use fill_value parameter during division:**  

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

**Divide two dataframes and using fill_value parameter:**  

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
- List having 1 dimension. Currently, this method supports operation on only list of numeric values.  
- A numpy ndarray having 1 dimension. Currently, this method supports operation on only an array of numeric values.  
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
It performs integer division operation between operands. It is equivalent to **'dataframe // other'**.  

**Creating frovedis DataFrame from pandas DataFrame:**  

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

**Floor Division on a scalar value using operator version:**  

For example,  

    print(fdf1 // 10)

Output  

    index   points  total
    0       0       1
    1       0       1
    2       0       1

**Floor Division on a scalar value using method version:**  

For example,  

    fdf1.floordiv(10).show()

Output  

    index   points  total
    0       0       1
    1       0       1
    2       0       1

In both versions, all column elements (axis = 1 by default) are divied by a scalar value.  
Also, resultant dataframe column elements will contain floor integer value.  

**Creating two frovedis dataframes to perform floor division:**  

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

**Floor Division on two dataframes using operator version:**  

For example,  

    print(fdf1 // fdf2)
    
Output  

    index   points  total
    0       2       1
    1       2       1
    2       0       1

**Floor Division on two dataframes using method version:**  

For example,  

    fdf1.floordiv(other = fdf2).show()

Output  

    index   points  total
    0       2       1
    1       2       1
    2       0       1

In both versions, only common columns in both dataframes are divided. Other are replaced with 
NaN values in resultant dataframe (fill_value = None by default).  

**Creating two frovedis dataframes in order to use fill_value parameter during floor division:**  

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

**Floor Division on two dataframes and using fill_value parameter:**  

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
- List having 1 dimension. Currently, this method supports operation on only list of numeric values.  
- A numpy ndarray having 1 dimension. Currently, this method supports operation on only an array of numeric values.  
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
It performs modulo operation between two operands. It is equivalent to **'dataframe % other'**.  

**Creating frovedis DataFrame from pandas DataFrame:**  

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

**Modulo on a scalar value using operator version:**  

For example,  

    print(fdf1 % 10)
    
Output  

    index   points  total
    0       5       0
    1       6       1
    2       4       2

**Modulo on a scalar value using method version:**  

For example,  

    fdf1.mod(10).show()

Output  

    index   points  total
    0       5       0
    1       6       1
    2       4       2

In both versions, modulo operation is performed on all column elements (axis = 1 by default) by a scalar value.  

**Creating two frovedis dataframes to perform modulo:**  

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

**Modulo on two dataframes using operator version:**  

For example,  

    print(fdf1 % fdf2)

Output  

    index   points  total
    0       1       3
    1       0       2
    2       4       1

**Creating two frovedis dataframes to perform modulo, use fill_value parameter too:**  

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

**Modulo on two dataframes using method version:**  

For example,  

    fdf1.mod(other = fdf2).show()

Output  

    index   points  total
    0       1       3
    1       0       2
    2       4       1

In both versions, modulo is performed on only common columns in both dataframes. Other are replaced with 
NaN values in resultant dataframe (fill_value = None by default).  

**Modulo on two dataframes and using fill_value parameter:**  

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
- List having 1 dimension. Currently, this method supports operation on only list of numeric values.  
- A numpy ndarray having 1 dimension. Currently, this method supports operation on only an array of numeric values.  
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
It performs multiplication operation between two operands. It is equivalent to **'dataframe * other'**.  

**Creating frovedis DataFrame from pandas DataFrame:**  

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

**Multiply a scalar value using operator version:**  

For example,  

    print(fdf1 * 10)

Output  
    
    index   points  total
    0       50      100
    1       60      110
    2       40      120

**Multiply a scalar value using method version:**  

For example,  

    fdf1.mul(10).show()

Output  

    index   points  total
    0       50      100
    1       60      110
    2       40      120

In both versions, all column elements (axis = 1 by default) are multiplied with a scalar value.  

**Creating two frovedis dataframes to perform multiplication:**  

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

**Multiply two dataframes using operator version:**  

For example,  

    print(fdf1 * fdf2)

Output  

    index   points  total
    0       10      70
    1       18      99
    2       32      132

**Multiply two dataframes using method version:**  

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

**Creating two frovedis dataframes in order to use fill_value parameter during multiplication:**  

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

**Multiply two dataframes and using fill_value parameter:**  

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
- List having 1 dimension. Currently, this method supports operation on only list of numeric values.  
- A numpy ndarray having 1 dimension. Currently, this method supports operation on only an array of numeric values.  
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
It performs exponential power operation between two operands. It is equivalent to **'dataframe ** other'**.  

**Creating frovedis DataFrame from pandas DataFrame:**  

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

**Exponential power operation on a scalar value using operator version:**  

For example,  

    print(fdf1 ** 2)

Output  

    index   points  total
    0       10      20
    1       12      22
    2       8       24

**Exponential power operation on a scalar value using method version:**  

For example,  

    fdf1.pow(2).show()

Output  

    index   points  total
    0       10      20
    1       12      22
    2       8       24

In both versions, exponential power operation is performed on all column 
elements (axis = 1 by default) by a scalar value.  

**Creating two frovedis dataframes to perform exponential power operation:**  

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

**Exponential power operation on two dataframes using operator version:**  

For example,  

    print(fdf1 ** fdf2)

Output  

    index   points  total
    0       10      70
    1       18      99
    2       32      132

**Exponential power operation on two dataframes using method version:**  

For example,

    fdf1.pow(other = fdf2).show()

Output  

    index   points  total
    0       10      70
    1       18      99
    2       32      132

In both versions, exponential power operation on only common columns in both dataframes. Exponential 
power operation on other are replaced with NaN values in resultant dataframe (fill_value = None by default).  

**Creating two frovedis dataframes and use fill_value parameter during exponential power operation:**  

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

**Exponential power operation on two dataframes and using fill_value parameter:**  

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
- List having 1 dimension. Currently, this method supports operation on only list of numeric values.  
- A numpy ndarray having 1 dimension. Currently, this method supports operation on only an array of numeric values.  
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
It performs subtraction operation between two operands. It is equivalent to **'dataframe - other'**.  

**Creating frovedis DataFrame from pandas DataFrame:**  

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

**Subtract a scalar value using operator version:**  

For example,  

    print(fdf1 - 10)

Output  

    index   points  total
    0       -5      0
    1       -4      1
    2       -6      2

**Subtract a scalar value using method version:**  

For example,  
    
    fdf1.sub(10).show()

Output  

    index   points  total
    0       -5      0
    1       -4      1
    2       -6      2

In both versions, all column elements (axis = 1 by default) are subtracted by a scalar value.  

**Creating two frovedis dataframes to perform subtraction:**  

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

**Subtract two dataframes using operator version:**  

For example,  

    print(fdf1 - fdf2)

Output  

    index   points  total
    0       3       3
    1       3       2
    2       -4      1

**Subtract two dataframes using method version:**  

For example,  

    fdf1.sub(other = fdf2).show()

Output  

    index   points  total
    0       3       3
    1       3       2
    2       -4      1

In both versions, only common columns in both dataframes are subtracted. Other are replaced with 
NaN values in resultant dataframe (fill_value = None by default).  

**Creating two frovedis dataframes and use fill_value parameter during subtraction:**  

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

**Subtract two dataframes and using fill_value parameter:**  

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
- List having 1 dimension. Currently, this method supports operation on only list of numeric values.  
- A numpy ndarray having 1 dimension. Currently, this method supports operation on only an array of numeric values.  
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
It performs floating division operation between two operands. It is equivalent to **'dataframe / other'**.  

**Creating frovedis DataFrame from pandas DataFrame:**  

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

**Floating Division on a scalar value using operator version:**  

For example,  

    print(fdf1 / 10)

Output  

    index   points  total
    0       0.8     0.3
    1       0.5     0.2
    2       0.9     0.1

**Floating Division on a scalar value using method version:**  

For example,  
    
    fdf1.truediv(10).show()

Output  

    index   points  total
    0       0.8     0.3
    1       0.5     0.2
    2       0.9     0.1

In both versions, all column elements (axis = 1 by default) are divied by a scalar value.  

**Creating two frovedis dataframes to perform floating division:**  

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

**Floating Division on two dataframes using operator version:**  

For example,  

    print(fdf1 / fdf2)

Output  

    index   points    total
    0       2         0.333333
    1       0.714285  2
    2       4.5       0.111111

**Floating Division on two dataframes using method version:**  

For example,  

    fdf1.truediv(other = fdf2).show()

Output  

    index   points    total
    0       2         0.333333
    1       0.714285  2
    2       4.5       0.111111

In both versions, only common columns in both dataframes are divided. Other are replaced with 
NaN values in resultant dataframe (fill_value = None by default).  

**Creating two frovedis dataframes and use fill_value parameter during floating division:**  

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

**Floating Division on two dataframes and using fill_value parameter:**  

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
- List having 1 dimension. Currently, this method supports operation on only list of numeric values.  
- A numpy ndarray having 1 dimension. Currently, this method supports operation on only an array of numeric values.  
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
It performs reverse addition operation between two operands. It is equivalent to **'other + dataframe'**.  

**Currently, it does not perform reverse addition of scalar using operator version. Only method version 
is supported.**  

**Creating frovedis DataFrame from pandas DataFrame:**  

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

**Reverse addition on a scalar value using method version:**  

For example,  

    # radd() demo with scalar value using method version
    fdf1.radd(10).show()

Output  

    index   points  total
    0       15      20
    1       16      21
    2       14      22

Here, it adds the scalar to all columns in dataframe (axis = 1 by default).  

**Creating two frovedis dataframes to perform reverse addition:**  

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

**Reverse addition on two dataframes using method version:**  

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

**Creating two frovedis dataframes and use fill_value parameter during reverse addition:**  

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

**Reverse addition on two dataframes and using fill_value parameter:**  

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
- List having 1 dimension. Currently, this method supports operation on only list of numeric values.  
- A numpy ndarray having 1 dimension. Currently, this method supports operation on only an array of numeric values.  
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
It performs reverse floating division operation between two operands. It is equivalent to **'other / dataframe'**.  

It is an alias of rtruediv().  

**Currently, it does not perform reverse division of scalar using operator version. Only method version is supported.**  

**Creating frovedis DataFrame from pandas DataFrame:**  

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

**Reverse division on a scalar value using method version:**  

For example,  

    # rdiv() demo with scalar value using method version
    fdf1.rdiv(10).show()

Output  

    index   points  total
    0       2       1
    1       1.66666 0.90909
    2       2.5     0.833333

Here, it uses the scalar to perform division on all column elements in dataframe (axis = 1 by default).  

**Creating two frovedis dataframes to perform reverse division:**  

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

**Reverse division on two dataframes using method version:**  

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

**Creating two frovedis dataframes and use fill_value parameter during reverse division:**  

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

**Reverse division on two dataframes and using fill_value parameter:**  

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
- List having 1 dimension. Currently, this method supports operation on only list of numeric values.  
- A numpy ndarray having 1 dimension. Currently, this method supports operation on only an array of numeric values.  
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
It performs reverse floating division operation between two operands. It is equivalent to **'other // dataframe'**.  

**Currently, it does not perform reverse division of scalar using operator version. Only method version is supported.**  

**Creating frovedis DataFrame from pandas DataFrame:**  

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

**Reverse floor division on a scalar value using method version:**  

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

**Creating two frovedis dataframes to perform reverse floor division:**  

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

**Reverse floor division on two dataframes using method version:**  

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

**Creating two frovedis dataframes and use fill_value parameter during reverse floor division:**  

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

**Reverse floor division on two dataframes and using fill_value parameter:**  

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
- List having 1 dimension. Currently, this method supports operation on only list of numeric values.  
- A numpy ndarray having 1 dimension. Currently, this method supports operation on only an array of numeric values.  
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
It performs reverse modulo operation between two operands. It is equivalent to **'other % dataframe'**.  

**Currently, it does not perform reverse modulo of scalar using operator version. Only method version is supported.**  

**Creating frovedis DataFrame from pandas DataFrame:**  

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

**Reverse modulo on a scalar value using method version:**  

For example,  

    # rmod() demo with scalar value using method version
    fdf1.rmod(10).show()

Output  

    index   points  total
    0       0       0
    1       4       10
    2       2       10

Here, it uses the scalar to perform modulo operation on all column elements (axis = 1 by default).   

**Creating two frovedis dataframes to perform reverse modulo:**  

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

**Reverse modulo on two dataframes using method version:**  

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

**Creating two frovedis dataframes and use fill_value parameter during reverse modulo:**  

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

**Reverse modulo on two dataframes and using fill_value parameter:**  

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
- List having 1 dimension. Currently, this method supports operation on only list of numeric values.  
- A numpy ndarray having 1 dimension. Currently, this method supports operation on only an array of numeric values.  
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
It performs reverse multiplication operation between two operands. It is equivalent to **'other * dataframe'**.  

**Currently, it does not perform reverse multiplication of scalar using operator version. Only method 
version is supported.**  

**Creating frovedis DataFrame from pandas DataFrame:**  

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

**Reverse multiplication on a scalar value using method version:**  

For example,  

    # rmul() demo with scalar value using method version
    fdf1.rmul(10).show()

Output  

    index   points  total
    0       50      100
    1       60      110
    2       40      120

Here, it uses the scalar to perform multiplication on all column elements in dataframe (axis = 1 by default).  

**Creating two frovedis dataframes to perform reverse multiplication:**  

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

**Reverse multiplication on two dataframes using method version:**  

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

**Creating two frovedis dataframes and use fill_value parameter during reverse multiplication:**  

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

**Reverse multiplication on two dataframes and using fill_value parameter:**  

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
- List having 1 dimension. Currently, this method supports operation on only list of numeric values.  
- A numpy ndarray having 1 dimension. Currently, this method supports operation on only an array of numeric values.  
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
It performs reverse exponential power operation between two operands. It is equivalent to **'other ** dataframe'**.  

**Currently, it does not perform reverse exponential power operation of scalar using operator version. Only 
method version is supported.**  

**Creating frovedis DataFrame from pandas DataFrame:**  

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

**Reverse exponential power operation on a scalar value using method version:**  

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

**Creating two frovedis dataframes to perform reverse exponential power operation:**  

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

**Reverse exponential power operation on two dataframes using method version:**  

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

**Creating two frovedis dataframes and use fill_value parameter during reverse exponential power operation:**  

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

**Reverse exponential power operation on two dataframes and using fill_value parameter:**  

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
- List having 1 dimension. Currently, this method supports operation on only list of numeric values.  
- A numpy ndarray having 1 dimension. Currently, this method supports operation on only an array of numeric values.  
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
It performs reverse subtraction operation between two operands. It is equivalent to **'other - dataframe'**.  

**Currently, it does not perform reverse subtraction of scalar using operator version. Only method version is supported.**  

**Creating frovedis DataFrame from pandas DataFrame:**  

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

**Reverse subtraction on a scalar value using method version:**  

For example,  

    # rsub() with scalar value using method version
    fdf1.rsub(10).show()

Output  

    index   points  total
    0       5       0
    1       4       -1
    2       6       -2

Here, it subtracts the scalar to all columns in dataframe (axis = 1 by default).  

**Creating two frovedis dataframes to perform reverse subtraction:**  

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

**Reverse subtraction on two dataframes using method version:**  

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

**Creating two frovedis dataframes and use fill_value parameter during reverse subtraction:**  

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

**Reverse subtraction on two dataframes and using fill_value parameter:**  

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
- List having 1 dimension. Currently, this method supports operation on only list of numeric values.  
- A numpy ndarray having 1 dimension. Currently, this method supports operation on only an array of numeric values.  
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
It performs reverse floating division operation between two operands. It is equivalent to **'other / dataframe'**.  

**Currently, it does not perform reverse division of scalar using operator version. Only method version is supported.**  

**Creating frovedis DataFrame from pandas DataFrame:**  

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

**Reverse floating division on a scalar value using method version:**  

For example,  

    # rtruediv() demo with scalar value using method version
    fdf1.rtruediv(10).show()

Output  

    index   points  total
    0       1.25    3.33333
    1       2       5
    2       1.11111 10

Here, it uses the scalar to perform division on all column elements in dataframe (axis = 1 by default).  

**Creating two frovedis dataframes to perform reverse floating division:**  

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

**Reverse floating division on two dataframes using method version:**  

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

**Creating two frovedis dataframes and use fill_value parameter during reverse floating division:**  

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

**Reverse floating division on two dataframes and using fill_value parameter:**  

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