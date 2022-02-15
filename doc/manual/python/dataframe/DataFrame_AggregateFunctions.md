% DataFrame Aggregate Functions
  
# NAME

DataFrame Aggregate Functions - list of all functions related to aggregate operations on frovedis dataframe are illustrated here.  
  
## SYNOPSIS  
    
    frovedis.dataframe.df.DataFrame(df = None, is_series = False)  
    
## Public Member Functions  
describe()  
agg(func)  
apply(func, axis = 0, raw = False, result_type = None, args = (), \*\*kwds)  
abs()  
mean(axis = None, skipna = None, level = None, numeric_only = None, \*\*kwargs)  
var(axis = None, skipna = None, level = None, ddof = 1, numeric_only = None, \*\*kwargs)  
mad(axis = None, skipna = None, level = None, numeric_only = None, \*\*kwargs)  
std(axis = None, skipna = None, level = None, ddof = 1, numeric_only = None, \*\*kwargs)  
sem(axis = None, skipna = None, level = None, ddof = 1, numeric_only = None, \*\*kwargs)  
median(axis = None, skipna = None, level = None, numeric_only = None, \*\*kwargs)  
countna(axis = 0)  

## Detailed Description  

### 1. describe()  

__Purpose__  
It generates descriptive statistics. Descriptive statistics include count, mean, median, etc, excluding missing values.  

For example,  

    import pandas as pd
    import numpy as np
    import frovedis.dataframe as fdf
    
    # a dictionary
    peopleDF = {
                'Name':['Jai', 'Anuj', 'Jai', 'Princi', 'Gaurav', 'Anuj', 'Princi', 'Abhi'],
                'Age':[27, 24, 22, 32, 33, 36, 27, 32],
                'City':['Nagpur', 'Kanpur', 'Allahabad', 'Kannuaj', 'Allahabad', 
                        'Kanpur', 'Kanpur', 'Kanpur'],
                'Qualification':['B.Tech', 'Phd', 'B.Tech', 'Phd', 'Phd', 'B.Tech', 'Phd', 'B.Tech'],
                'Score': [23, 34, 35, 45, np.nan, 50, 52, np.nan]
                }
    
    # create pandas dataframe
    pdf1 = pd.DataFrame(peopleDF)
    
    # create frovedis dataframe
    fdf1 = fdf.DataFrame(pdf1)
    
    # display the frovedis dataframe
    fdf1.show()

Output  

    index   Name    Age     City       Qualification  Score
    0       Jai     27      Nagpur     B.Tech         23
    1       Anuj    24      Kanpur     Phd            34
    2       Jai     22      Allahabad  B.Tech         35
    3       Princi  32      Kannuaj    Phd            45
    4       Gaurav  33      Allahabad  Phd            NULL
    5       Anuj    36      Kanpur     B.Tech         50
    6       Princi  27      Kanpur     Phd            52
    7       Abhi    32      Kanpur     B.Tech         NULL

For example,  

    print(fdf1.describe())

Output  

                   Age       Score
    count     8.000000    6.000000
    mean     29.125000   39.833333
    median   29.500000   40.000000
    var      23.553571  123.766667
    mad       4.125000    9.166667
    std       4.853202   11.125047
    sem       1.715866    4.541781
    sum     233.000000  239.000000
    min      22.000000   23.000000
    max      36.000000   52.000000

__Return Value__  
It returns a pandas DataFrame instance with the result of the specified aggregate operation.  

### 2. agg(func)  

__Parameters__  
**_func_**: Names of functions to use for aggregating the data. The input to be used with the function must 
be a frovedis DataFrame instance having atleast one numeric column.  
Accepted combinations for this parameter are:
- A string function name such as 'max', 'min', etc.  
- list of functions and/or function names, For example, ['max', 'mean'].  
- dictionary with keys as column labels and values as function name or list of such functions. 
For Example, {'Age': ['max','min','mean'], 'Ename': ['count']}  

__Purpose__  
It computes an aggregate operation based on the condition specified in 'func'.  

**Currently, this method will perform aggregation operation along the rows.**  

For example,  
    
    import pandas as pd
    import numpy as np
    import frovedis.dataframe as fdf
    
    # a dictionary
    peopleDF = {
                'Name':['Jai', 'Anuj', 'Jai', 'Princi', 'Gaurav', 'Anuj', 'Princi', 'Abhi'],
                'Age':[27, 24, 22, 32, 33, 36, 27, 32],
                'City':['Nagpur', 'Kanpur', 'Allahabad', 'Kannuaj', 'Allahabad', 
                        'Kanpur', 'Kanpur', 'Kanpur'],
                'Qualification':['B.Tech', 'Phd', 'B.Tech', 'Phd', 'Phd', 'B.Tech', 'Phd', 'B.Tech'],
                'Score': [23, 34, 35, 45, np.nan, 50, 52, np.nan]
                }
    
    # create pandas dataframe
    pdf1 = pd.DataFrame(peopleDF)
    
    # create frovedis dataframe
    fdf1 = fdf.DataFrame(pdf1)
    
    # display the frovedis dataframe
    fdf1.show()

Output  
    
    index   Name    Age     City       Qualification  Score
    0       Jai     27      Nagpur     B.Tech         23
    1       Anuj    24      Kanpur     Phd            34
    2       Jai     22      Allahabad  B.Tech         35
    3       Princi  32      Kannuaj    Phd            45
    4       Gaurav  33      Allahabad  Phd            NULL
    5       Anuj    36      Kanpur     B.Tech         50
    6       Princi  27      Kanpur     Phd            52
    7       Abhi    32      Kanpur     B.Tech         NULL

For example,  

    # agg() demo with func as a function string name 
    print(fdf1.agg('max'))
    
Output  

    Name             nan
    Age               36
    City             nan
    Qualification    nan
    Score             52
    Name: max, dtype: object

It displays a pandas dataframe containing numeric column(s) with newly computed aggregates of each groups.  

For example,

    # agg() demo with func as a dictionary 
    print(fdf1.agg({"Age": ["std", "mean"]}))

Output

                Age
    mean  29.125000
    std    4.853202

For example,  
    
    # agg() demo where func is a list of functions 
    print(fdf1['Age'].agg(['max','min','mean'])

Output  

             Age
    max   36.000
    min   22.000
    mean  29.125

__Return Value__  
1. **If one 'func' provided and 'func' is a string:**  
     - It returns a pandas Series instance with numeric column(s) only, after aggregation function is completed.  
2. **If one or more 'func' provided and 'func' is list/dict of string:**  
     - It returns a pandas DataFrame instance with numeric column(s) only, after aggregation function is completed.  

### 3. apply(func, axis = 0, raw = False, result_type = None, args = (), \*\*kwds)  

__Parameters__  
**_func_**: Names of functions to be applied on the data. The input to be used with the function must 
be a frovedis DataFrame instance having atleast one numeric column.  
Accepted combinations for this parameter are:  
- A string function name such as 'max', 'min', etc.  
- list of functions and/or function names, For example, ['max', 'mean'].  
- dictionary with keys as column labels and values as function name or list of such functions.  
For Example, {'Age': ['max','min','mean'], 'Ename': ['count']}  

**_axis_**: It accepts an integer as parameter. It is used to decide whether to perform aggregate operation along the 
columns or rows. (Default: 0)  
_**raw**_: It accepts boolean as parameter. When set to True, the row/column will be passed as an ndarray. (Default: False)  
_**result\_type**_: It accepts string object as parameter. It specifies how the result will be returned. (Default: None)  
These only act when **axis = 1 (columns)**:  
- **expand** : list-like results will be turned into columns.  
- **reduce** : returns a Series if possible rather than expanding list-like results. This is the opposite of 'expand'.  
- **broadcast** : results will be broadcast to the original shape of the DataFrame, the original index and columns will be retained.  

The default behaviour (None) depends on the return value of the applied function. List-like results will be returned 
as a Series of those. However if the apply function returns a Series these are expanded to columns.  
_**args**_: Positional arguments to pass to 'func'. (Default: ())  
_**\*\*kwds**_: This is an unused parameter.  

__Purpose__  
Apply a function along an axis of the DataFrame.  

The parameter: "\*\*kwds" is simply kept in to make the interface uniform to the pandas DataFrame.apply().  
This is not used anywhere within the frovedis implementation.  

For example,  

    import pandas as pd
    import numpy as np
    import frovedis.dataframe as fdf
    
    # a dictionary
    peopleDF = {
                'Name':['Jai', 'Anuj', 'Jai', 'Princi', 'Gaurav', 'Anuj', 'Princi', 'Abhi'],
                'Age':[27, 24, 22, 32, 33, 36, 27, 32],
                'City':['Nagpur', 'Kanpur', 'Allahabad', 'Kannuaj', 'Allahabad', 
                        'Kanpur', 'Kanpur', 'Kanpur'],
                'Qualification':['B.Tech', 'Phd', 'B.Tech', 'Phd', 'Phd', 'B.Tech', 'Phd', 'B.Tech'],
                'Score': [23, 34, 35, 45, np.nan, 50, 52, np.nan]
                }
    
    # create pandas dataframe
    pdf1 = pd.DataFrame(peopleDF)
    
    # create frovedis dataframe
    fdf1 = fdf.DataFrame(pdf1)
    
    # display the frovedis dataframe
    fdf1.show()

Output

    index   Name    Age     City       Qualification  Score
    0       Jai     27      Nagpur     B.Tech         23
    1       Anuj    24      Kanpur     Phd            34
    2       Jai     22      Allahabad  B.Tech         35
    3       Princi  32      Kannuaj    Phd            45
    4       Gaurav  33      Allahabad  Phd            NULL
    5       Anuj    36      Kanpur     B.Tech         50
    6       Princi  27      Kanpur     Phd            52
    7       Abhi    32      Kanpur     B.Tech         NULL

For example,  

    # apply() demo using string function name
    print(fdf1.apply('max'))
    
Output  

    Name             8
    Age              8
    City             8
    Qualification    8
    Score            6
    dtype: int64

For example,  

    # apply() demo using axis = 1
    print(fdf1.apply('max', axis = 1))
    
Output  

    index
    0    27.0
    1    34.0
    2    35.0
    3    45.0
    4    33.0
    5    50.0
    6    52.0
    7    32.0
    dtype: float64

For example,  

    # apply() demo using raw = True
    print(fdf1.apply('max', raw = True))
    
Output  

    Name             Princi
    Age                  36
    City             Nagpur
    Qualification       Phd
    Score                52
    dtype: object

For example,  

    # apply() demo using result_type = 'expand'
    print(fdf1.apply('max', result_type = 'expand'))
    
Output  

    Name             Princi
    Age                  36
    City             Nagpur
    Qualification       Phd
    Score                52
    dtype: object

__Return Value__  
1. **If only one 'func' provided:**  
     - It returns a pandas Series instance with numeric column(s) only, after aggregation function is completed.  
2. **If more than one 'func' provided:**  
     - It returns a pandas DataFrame instance with numeric column(s) only, after aggregation function is completed.  

### 4. abs()  

__Purpose__  
It computes absolute numeric value of each element.  

This function only applies to elements that are all numeric.  

For example,  

    import pandas as pd
    import numpy as np
    import frovedis.dataframe as fdf
    
    # a dictionary
    tempDF = {
                'City': ['Nagpur', 'Kanpur', 'Allahabad', 'Kannuaj', 'Allahabad', 'Kanpur', 'Kanpur', 'Kanpur'],
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

### 5. mean(axis = None, skipna = None, level = None, numeric_only = None, \*\*kwargs)  

__Parameters__  
**_axis_**: It accepts an integer as parameter. It is used to decide whether to perform mean along the 
columns or rows. (Default: None)  
When it is None (not specified explicitly), it performs mean operation along the rows.  
_**skipna**_: It is a boolean parameter. When set to True, it will exclude missing values while computing the result. (Default: None)  
When it is None (not specified explicitly), it excludes missing values during mean computation.  
_**level**_: This is an unused parameter. (Default: None)  
_**numeric\_only**_: This is an unsued parameter. (Default: None)  
_**\*\*kwargs**_: This is an unused parameter.  

__Purpose__  
It computes mean of the values over the requested axis.  

The parameters: "level", "\*\*kwargs" are simply kept in to make the interface uniform to the pandas DataFrame.mean().  
This is not used anywhere within the frovedis implementation.  

For example,  

    import pandas as pd
    import numpy as np
    import frovedis.dataframe as fdf
    
    # a dictionary
    peopleDF = {
                'Name':['Jai', 'Anuj', 'Jai', 'Princi', 'Gaurav', 'Anuj', 'Princi', 'Abhi'],
                'Age':[27, 24, 22, 32, 33, 36, 27, 32],
                'City':['Nagpur', 'Kanpur', 'Allahabad', 'Kannuaj', 'Allahabad', 
                        'Kanpur', 'Kanpur', 'Kanpur'],
                'Qualification':['B.Tech', 'Phd', 'B.Tech', 'Phd', 'Phd', 'B.Tech', 'Phd', 'B.Tech'],
                'Score': [23, 34, 35, 45, np.nan, 50, 52, np.nan]
                }
    
    # create pandas dataframe
    pdf1 = pd.DataFrame(peopleDF)
    
    # create frovedis dataframe
    fdf1 = fdf.DataFrame(pdf1)
    
    # display the frovedis dataframe
    fdf1.show()

Output

    index   Name    Age     City       Qualification  Score
    0       Jai     27      Nagpur     B.Tech         23
    1       Anuj    24      Kanpur     Phd            34
    2       Jai     22      Allahabad  B.Tech         35
    3       Princi  32      Kannuaj    Phd            45
    4       Gaurav  33      Allahabad  Phd            NULL
    5       Anuj    36      Kanpur     B.Tech         50
    6       Princi  27      Kanpur     Phd            52
    7       Abhi    32      Kanpur     B.Tech         NULL

For example,  

    # mean() demo
    print(fdf1.mean())

Output  

    index   mean
    Age     29.125
    Score   39.8333

For example,  

    # mean() demo using axis = 1
    print(fdf1.mean(axis = 1))

Output  

    index   mean
    0       25
    1       29
    2       28.5
    3       38.5
    4       33
    5       43
    6       39.5
    7       32
    
For example,  

    # mean() demo using skipna = False
    print(fdf1.mean(skipna = False))

Output

    index   mean
    Age     29.125
    Score   NULL

__Return Value__  
It returns a frovedis DataFrame instance.  

### 6. var(axis = None, skipna = None, level = None, ddof = 1, numeric_only = None, \*\*kwargs)  

__Parameters__  
**_axis_**: It accepts an integer as parameter. It's used to decide whether to perform variance along the 
columns or rows. (Default: None)  
When it is None (not specified explicitly), it performs variance along the rows.  
_**skipna**_: It accepts boolean as parameter. When set to True, it will exclude missing values while computing the result. (Default: None)  
When it is None (not specified explicitly), it excludes missing values during variance computation.  
_**level**_: This is an unused parameter. (Default: None)  
**_ddof_**: It accepts an integer parameter that specifies the delta degrees of freedom. (Default: 1)  
_**numeric\_only**_: This is an unsed parameter. (Default: None)  
_**\*\*kwargs**_: This is an unused parameter.  

__Purpose__  
It computes variance over requested axis.  

The parameters: "level", "\*\*kwargs" are simply kept in to make the interface uniform to the pandas DataFrame.var().  
This is not used anywhere within the frovedis implementation.  

For example,  

    import pandas as pd
    import numpy as np
    import frovedis.dataframe as fdf
    
    # a dictionary
    peopleDF = {
                'Name':['Jai', 'Anuj', 'Jai', 'Princi', 'Gaurav', 'Anuj', 'Princi', 'Abhi'],
                'Age':[27, 24, 22, 32, 33, 36, 27, 32],
                'City':['Nagpur', 'Kanpur', 'Allahabad', 'Kannuaj', 'Allahabad', 
                        'Kanpur', 'Kanpur', 'Kanpur'],
                'Qualification':['B.Tech', 'Phd', 'B.Tech', 'Phd', 'Phd', 'B.Tech', 'Phd', 'B.Tech'],
                'Score': [23, 34, 35, 45, np.nan, 50, 52, np.nan]
                }
    
    # create pandas dataframe
    pdf1 = pd.DataFrame(peopleDF)
    
    # create frovedis dataframe
    fdf1 = fdf.DataFrame(pdf1)
    
    # display the frovedis dataframe
    fdf1.show()

Output

    index   Name    Age     City       Qualification  Score
    0       Jai     27      Nagpur     B.Tech         23
    1       Anuj    24      Kanpur     Phd            34
    2       Jai     22      Allahabad  B.Tech         35
    3       Princi  32      Kannuaj    Phd            45
    4       Gaurav  33      Allahabad  Phd            NULL
    5       Anuj    36      Kanpur     B.Tech         50
    6       Princi  27      Kanpur     Phd            52
    7       Abhi    32      Kanpur     B.Tech         NULL

For example,  

    # var() demo
    fdf1.var().show()

Output  

    index   var
    Age     23.5535
    Score   123.766

It displays a frovedis dataframe with numeric column(s) containing the newly computed variance 
for each groups.  

Also, it excludes the missing value in **'Score'** column while computing variance of groups **'B.Tech'** 
and **'Phd'**.  

For example,  

    # var() demo using axis = 1
    fdf1.var(axis = 1).show()
    
Output  

    index   var
    0       8
    1       50
    2       84.5
    3       84.5
    4       NULL
    5       98
    6       312.5
    7       NULL

For example,  

    # var() demo using skipna = False
    fdf1.var(skipna = False).show()
    
Output  

    index   var
    Age     23.5535
    Score   NULL

For example,  

    # var() demo using ddof = 2 
    fdf1.var(ddof = 2).show()
    
Output  

    index   var
    Age     27.4791
    Score   154.708

__Return Value__  
It returns a frovedis DataFrame instance with the result of the specified aggregate operation.  

### 7. mad(axis = None, skipna = None, level = None, numeric_only = None, \*\*kwargs)  

__Parameters__  
**_axis_**: It accepts an integer as parameter. It is used to decide whether to perform mean absolute 
deviation along the columns or rows. (Default: None)  
When it is None (not specified explicitly), it performs mean absolute deviation along the rows.  
_**skipna**_: It accepts boolean as parameter. When set to True, it will exclude missing values while computing the result. (Default: None)  
When it is None (not specified explicitly), it excludes missing values during mean absolute deviation computation.  
_**level**_: This is an unused parameter. (Default: None)  
_**numeric\_only**_: This is an unsed parameter. (Default: None)  
_**\*\*kwargs**_: This is an unused parameter.  

__Purpose__  
It computes the mean absolute deviation of the values over the requested axis.  

The parameters: "level", "\*\*kwargs" are simply kept in to make the interface uniform to the pandas DataFrame.mad().  
This is not used anywhere within the frovedis implementation.  

For example,  

    import pandas as pd
    import numpy as np
    import frovedis.dataframe as fdf
    
    # a dictionary
    peopleDF = {
                'Name':['Jai', 'Anuj', 'Jai', 'Princi', 'Gaurav', 'Anuj', 'Princi', 'Abhi'],
                'Age':[27, 24, 22, 32, 33, 36, 27, 32],
                'City':['Nagpur', 'Kanpur', 'Allahabad', 'Kannuaj', 'Allahabad', 
                        'Kanpur', 'Kanpur', 'Kanpur'],
                'Qualification':['B.Tech', 'Phd', 'B.Tech', 'Phd', 'Phd', 'B.Tech', 'Phd', 'B.Tech'],
                'Score': [23, 34, 35, 45, np.nan, 50, 52, np.nan]
                }
    
    # create pandas dataframe
    pdf1 = pd.DataFrame(peopleDF)
    
    # create frovedis dataframe
    fdf1 = fdf.DataFrame(pdf1)
    
    # display the frovedis dataframe
    fdf1.show()

Output  

    index   Name    Age     City       Qualification  Score
    0       Jai     27      Nagpur     B.Tech         23
    1       Anuj    24      Kanpur     Phd            34
    2       Jai     22      Allahabad  B.Tech         35
    3       Princi  32      Kannuaj    Phd            45
    4       Gaurav  33      Allahabad  Phd            NULL
    5       Anuj    36      Kanpur     B.Tech         50
    6       Princi  27      Kanpur     Phd            52
    7       Abhi    32      Kanpur     B.Tech         NULL

For example,  

    # mad() demo
    fdf1.mad().show()

Output  

    index   mad
    Age     4.125
    Score   9.16666

It displays a frovedis dataframe with numeric column(s) containing the newly computed mean absolute 
deviation for each groups.  

Also, it excludes the missing value in **'Score'** column while computing the mean absolute 
deviation of groups **'B.Tech'** and **'Phd'**.  

For example,  

    # mad() demo using axis = 1
    fdf1.mad(axis = 1).show()
    
Output  

    index   mad
    0       2
    1       5
    2       6.5
    3       6.5
    4       0
    5       7
    6       12.5
    7       0

For example,  

    # mad() demo using skipna = False
    fdf1.mad(skipna = False).show()
    
Output  

    index   mad
    Age     4.125
    Score   NULL

__Return Value__  
It returns a frovedis DataFrame instance with the result of the specified aggregate operation.  

### 8. std(axis = None, skipna = None, level = None, ddof = 1, numeric_only = None, \*\*kwargs)  

__Parameters__  
**_axis_**: It accepts an integer as parameter. It is used to decide whether to perform standard deviation along the 
columns or rows. (Default: None)  
When it is None (not specified explicitly), it performs standard deviation along the rows.  
_**skipna**_: It accepts boolean as parameter. When set to True, it will exclude missing values while computing the result. (Default: None)  
When it is None (not specified explicitly), it excludes missing values during standard deviation computation.  
_**level**_: This is an unused parameter. (Default: None)  
**_ddof_**: It accepts an integer parameter that specifies the delta degrees of freedom. (Default: 1)  
_**numeric\_only**_: This is an unsed parameter. (Default: None)  
_**\*\*kwargs**_: This is an unused parameter.  

__Purpose__  
It computes standard deviation over requested axis.  

The parameters: "level", "\*\*kwargs" are simply kept in to make the interface uniform to the pandas DataFrame.std().  
This is not used anywhere within the frovedis implementation.  

For example,  

    import pandas as pd
    import numpy as np
    import frovedis.dataframe as fdf
    
    # a dictionary
    peopleDF = {
                'Name':['Jai', 'Anuj', 'Jai', 'Princi', 'Gaurav', 'Anuj', 'Princi', 'Abhi'],
                'Age':[27, 24, 22, 32, 33, 36, 27, 32],
                'City':['Nagpur', 'Kanpur', 'Allahabad', 'Kannuaj', 'Allahabad', 
                        'Kanpur', 'Kanpur', 'Kanpur'],
                'Qualification':['B.Tech', 'Phd', 'B.Tech', 'Phd', 'Phd', 'B.Tech', 'Phd', 'B.Tech'],
                'Score': [23, 34, 35, 45, np.nan, 50, 52, np.nan]
                }
    
    # create pandas dataframe
    pdf1 = pd.DataFrame(peopleDF)
    
    # create frovedis dataframe
    fdf1 = fdf.DataFrame(pdf1)
    
    # display the frovedis dataframe
    fdf1.show()

Output

    index   Name    Age     City       Qualification  Score
    0       Jai     27      Nagpur     B.Tech         23
    1       Anuj    24      Kanpur     Phd            34
    2       Jai     22      Allahabad  B.Tech         35
    3       Princi  32      Kannuaj    Phd            45
    4       Gaurav  33      Allahabad  Phd            NULL
    5       Anuj    36      Kanpur     B.Tech         50
    6       Princi  27      Kanpur     Phd            52
    7       Abhi    32      Kanpur     B.Tech         NULL

For example,  

    # std() demo
    fdf1.std().show()

Output  

    index   std
    Age     4.8532
    Score   11.125

It displays a frovedis dataframe with numeric column(s) containing the newly computed standard deviation 
for each groups.  

For example,  

    # std() demo using axis = 1
    fdf1.std(axis = 1).show()
    
Output  

    index   std
    0       2.82842
    1       7.07106
    2       9.19238
    3       9.19238
    4       NULL
    5       9.89949
    6       17.6776
    7       NULL

For example,  

    # std() demo using skipna = False
    fdf1.std(skipna = False).show()
    
Output  

    index   std
    Age     4.8532
    Score   NULL

For example,  

    # std() demo using ddof = 2 
    fdf1.std(ddof = 2).show()
    
Output  

    index   std
    Age     5.24205
    Score   12.4381

__Return Value__  
It returns a frovedis DataFrame instance with the result of the specified aggregate functions.  

### 9. sem(axis = None, skipna = None, level = None, ddof = 1, numeric_only = None, \*\*kwargs)  

__Parameters__  
**_axis_**: It accepts an integer as parameter. It is used to decide whether to perform standard 
error of the mean along the columns or rows. (Default: None)  
When it is None (not specified explicitly), it performs standard error of the mean along the rows.  
_**skipna**_: It accepts boolean as parameter. When set to True, it will exclude missing values while 
computing the result. (Default: None)  
When it is None (not specified explicitly), it excludes missing values during standard error of the 
mean computation.  
_**level**_: This is an unused parameter. (Default: None)  
**_ddof_**: It accepts an integer parameter that specifies the delta degrees of freedom. (Default: 1)  
_**numeric\_only**_: This is an unsed parameter. (Default: None)  
_**\*\*kwargs**_: This is an unused parameter.  

__Purpose__  
It computes standard error of the mean over requested axis.  

The parameters: "level", "\*\*kwargs" are simply kept in to make the interface uniform to the pandas DataFrame.sem().  
This is not used anywhere within the frovedis implementation.  

For example,  

    import pandas as pd
    import numpy as np
    import frovedis.dataframe as fdf
    
    # a dictionary
    peopleDF = {
                'Name':['Jai', 'Anuj', 'Jai', 'Princi', 'Gaurav', 'Anuj', 'Princi', 'Abhi'],
                'Age':[27, 24, 22, 32, 33, 36, 27, 32],
                'City':['Nagpur', 'Kanpur', 'Allahabad', 'Kannuaj', 'Allahabad', 
                        'Kanpur', 'Kanpur', 'Kanpur'],
                'Qualification':['B.Tech', 'Phd', 'B.Tech', 'Phd', 'Phd', 'B.Tech', 'Phd', 'B.Tech'],
                'Score': [23, 34, 35, 45, np.nan, 50, 52, np.nan]
                }
    
    # create pandas dataframe
    pdf1 = pd.DataFrame(peopleDF)
    
    # create frovedis dataframe
    fdf1 = fdf.DataFrame(pdf1)
    
    # display the frovedis dataframe
    fdf1.show()

Output

    index   Name    Age     City       Qualification  Score
    0       Jai     27      Nagpur     B.Tech         23
    1       Anuj    24      Kanpur     Phd            34
    2       Jai     22      Allahabad  B.Tech         35
    3       Princi  32      Kannuaj    Phd            45
    4       Gaurav  33      Allahabad  Phd            NULL
    5       Anuj    36      Kanpur     B.Tech         50
    6       Princi  27      Kanpur     Phd            52
    7       Abhi    32      Kanpur     B.Tech         NULL

For example,  

    # sem() demo
    fdf1.sem().show()

Output  

    index   sem
    Age     1.71586
    Score   4.54178

It displays a frovedis dataframe with numeric column(s) containing the newly computed standard error 
of the mean for each groups.  

For example,  

    # sem() demo using ddof = 2 
    fdf1.sem(ddof = 2).show()
    
Output  

    index   sem
    Age     1.85334
    Score   5.07786

For example,  

    # sem() demo using axis = 1
    fdf1.sem(axis = 1).show()
    
Output  

    index   sem
    0       2
    1       5
    2       6.49999
    3       6.49999
    4       NULL
    5       6.99999
    6       12.5
    7       NULL

For example,  

    # sem() demo using skipna = False
    fdf1.sem(skipna = False).show()
    
Output  

    index   sem
    Age     1.71586
    Score   NULL

__Return Value__  
It returns a frovedis DataFrame instance with the result of the specified aggregate operation.  

### 10. median(axis = None, skipna = None, level = None, numeric_only = None, \*\*kwargs)  

__Parameters__  
**_axis_**: It accepts an integer as parameter. It is used to decide whether to perform median operation along 
the columns or rows. (Default: None)  
When it is None (not specified explicitly), it performs median along the rows.  
_**skipna**_: It accepts boolean as parameter. When set to True, it will exclude missing values while computing 
the result. (Default: None)  
When it is None (not specified explicitly), it excludes missing values during median computation.  
_**level**_: This is an unused parameter. (Default: None)  
_**numeric\_only**_: This is an unsued parameter. (Default: None)  
_**\*\*kwargs**_: This is an unused parameter.  

__Purpose__  
It computes median of the values over the requested axis.  

The parameters: "level", "\*\*kwargs" are simply kept in to make the interface uniform to the pandas DataFrame.median().  
This is not used anywhere within the frovedis implementation.  

For example,  

    import pandas as pd
    import numpy as np
    import frovedis.dataframe as fdf
    
    # a dictionary
    peopleDF = {
                'Name':['Jai', 'Anuj', 'Jai', 'Princi', 'Gaurav', 'Anuj', 'Princi', 'Abhi'],
                'Age':[27, 24, 22, 32, 33, 36, 27, 32],
                'City':['Nagpur', 'Kanpur', 'Allahabad', 'Kannuaj', 'Allahabad', 
                        'Kanpur', 'Kanpur', 'Kanpur'],
                'Qualification':['B.Tech', 'Phd', 'B.Tech', 'Phd', 'Phd', 'B.Tech', 'Phd', 'B.Tech'],
                'Score': [23, 34, 35, 45, np.nan, 50, 52, np.nan]
                }
    
    # create pandas dataframe
    pdf1 = pd.DataFrame(peopleDF)
    
    # create frovedis dataframe
    fdf1 = fdf.DataFrame(pdf1)
    
    # display the frovedis dataframe
    fdf1.show()

Output

    index   Name    Age     City       Qualification  Score
    0       Jai     27      Nagpur     B.Tech         23
    1       Anuj    24      Kanpur     Phd            34
    2       Jai     22      Allahabad  B.Tech         35
    3       Princi  32      Kannuaj    Phd            45
    4       Gaurav  33      Allahabad  Phd            NULL
    5       Anuj    36      Kanpur     B.Tech         50
    6       Princi  27      Kanpur     Phd            52
    7       Abhi    32      Kanpur     B.Tech         NULL

For example,  

    # median() demo
    print(fdf1.median())

Output  

    index   median
    Age     29.5
    Score   40

For example,  

    # median() demo using axis = 1
    print(fdf1.median(axis = 1))

Output  

    index   median
    0       25
    1       29
    2       28.5
    3       38.5
    4       33
    5       43
    6       39.5
    7       32
    
For example,  

    # median() demo using skipna = False
    print(fdf1.median(skipna = False))

Output

    index   median
    Age     29.5
    Score   NULL

__Return Value__  
It returns a frovedis DataFrame instance.  

### 11.countna(axis = 0)  

__Parameters__  
**_axis_**: It accepts an integer or string object as parameter. It is used to decide whether to 
count missing values along the indices or by column labels. (Default: 0)  
- **0 or 'rows'**: count missing values along the indices.  
- **1 or 'columns'**: count missing values along the columns.  

__Purpose__  
It counts number of missing values in the given axis.  

For example,  

    import pandas as pd
    import numpy as np
    import frovedis.dataframe as fdf
    
    # a dictionary
    peopleDF = {
                'Name':['Jai', 'Anuj', 'Jai', 'Princi', 'Gaurav', 'Anuj', 'Princi', 'Abhi'],
                'Age':[27, 24, 22, 32, 33, 36, 27, 32],
                'City':['Nagpur', 'Kanpur', 'Allahabad', 'Kannuaj', 'Allahabad', 
                        'Kanpur', 'Kanpur', 'Kanpur'],
                'Qualification':['B.Tech', 'Phd', 'B.Tech', 'Phd', 'Phd', 'B.Tech', 'Phd', 'B.Tech'],
                'Score': [23, 34, 35, 45, np.nan, 50, 52, np.nan]
                }
    
    # create pandas dataframe
    pdf1 = pd.DataFrame(peopleDF)
    
    # create frovedis dataframe
    fdf1 = fdf.DataFrame(pdf1)
    
    # display the frovedis dataframe
    fdf1.show()

Output

    index   Name    Age     City       Qualification  Score
    0       Jai     27      Nagpur     B.Tech         23
    1       Anuj    24      Kanpur     Phd            34
    2       Jai     22      Allahabad  B.Tech         35
    3       Princi  32      Kannuaj    Phd            45
    4       Gaurav  33      Allahabad  Phd            NULL
    5       Anuj    36      Kanpur     B.Tech         50
    6       Princi  27      Kanpur     Phd            52
    7       Abhi    32      Kanpur     B.Tech         NULL

For example,

    # countna() demo
    fdf1.countna().show()

Output

    index   count
    Name    0
    Age     0
    City    0
    Qualification   0
    Score   2
    
For example,

    # countna() demo using axis = 1
    fdf1.countna(axis = 1).show()
    
Output

    index   count
    0       0
    1       0
    2       0
    3       0
    4       1
    5       0
    6       0
    7       1

__Return Value__  
It returns a frovedis DataFrame instance.  

# SEE ALSO  

DataFrame_Introduction, DataFrame_ConversionAndSorting, DataFrame_SelectionAndCombinations, DataFrame_BinaryOperators