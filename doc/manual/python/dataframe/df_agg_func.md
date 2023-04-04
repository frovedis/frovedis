% DataFrame Aggregate Functions
  
# NAME

DataFrame Aggregate Functions - list of all functions related to aggregate operations on frovedis dataframe are illustrated here.  
  
## DESCRIPTION  
An essential piece of analysis of large data is efficient summarization: computing aggregations like sum(), mean(), median(), min(), and max(), which gives insight into the nature of a potentially large dataset. In this section, we will explore list of all such aggregation operations done on frovedis dataframe.  

Aggregation can be performed in **two ways** on frovedis dataframe:  

- Either using agg().  
- Or using aggregation functions such as min(), max() median(), mode(), etc. on frovedis dataframe.  

    
## Public Member Functions  
    1. agg(func=None, axis=0, *args, **kwargs)  
    2. cov((min_periods = None, ddof = 1.0, low_memory = True, other = None)
    3. mad(axis = None, skipna = None, level = None, numeric_only = None, **kwargs)  
    4. max(axis = None, skipna = None, level = None, numeric_only = None, **kwargs)  
    5. mean(axis = None, skipna = None, level = None, numeric_only = None, **kwargs)  
    6. median(axis = None, skipna = None, level = None, numeric_only = None, **kwargs)  
    7. min(axis = None, skipna = None, level = None, numeric_only = None, **kwargs)  
    8. mode(axis = 0, numeric_only = False, dropna = True)
    9. sem(axis = None, skipna = None, level = None, ddof = 1, numeric_only = None, **kwargs)  
    10. std(axis = None, skipna = None, level = None, ddof = 1, numeric_only = None, **kwargs)  
    11. sum(axis = None, skipna = None, level = None, numeric_only = None, 
            min_count = 0, **kwargs)
    12. var(axis = None, skipna = None, level = None, ddof = 1, numeric_only = None, **kwargs)  

## Detailed Description  

### 1. DataFrame.agg(func=None, axis=0, *args, **kwargs)  

__Parameters__  
**_func_**: Names of functions to use for aggregating the data. The input to be used with the function must 
be a frovedis DataFrame instance having atleast one numeric column.  
Accepted combinations for this parameter are:

- A string function name such as 'max', 'min', etc.  
- list of functions and/or function names, For example, ['max', 'mean'].  
- dictionary with keys as column labels and values as function name or list of such functions. 
- In case func is None, then it will confirm if  any keyword arguments are provided. If **kwargs are not 
provided then it will raise an Exception.  

For Example, {'Age': ['max','min','mean'], 'Ename': ['count']}  

**_axis_**: It accepts an integer or string object as parameter. It is used to decide whether to 
perform aggregation along the columns or rows. (Default: 0)  

- **0 or 'index'**: perform aggregation along the indices.  

_**\*args**_: This is an unused parameter.  
_**\*\*kwargs**_: Additional keyword arguments to be passed to the function.  

__Purpose__  
It computes an aggregate operation based on the condition specified in 'func'.  

**Currently, this method will perform aggregation operation to each column.**  

**Creating frovedis DataFrame from pandas DataFrame:**  

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

**Aggregate these functions over the rows, where func is a function string name:**  

For example,  

    print(fdf1.agg('max'))
    
Output  

    Name             nan
    Age               36
    City             nan
    Qualification    nan
    Score             52
    Name: max, dtype: object

It displays a pandas dataframe containing numeric column(s) with newly computed aggregates of each groups.  

**Performing aggregation along the rows where func is a dictionary:**   

For example,  

    print(fdf1.agg({"Age": ["std", "mean"]}))

Output

                Age
    mean  29.125000
    std    4.853202

However, when aggregation is performed where func = 'cov', then it will perfom covariance only on numeric columns.  

For example,  

    print(fdf1.agg('cov'))

Output  

    index   Age     Score
    Age     23.5535 31.8
    Score   31.8    123.766

**Aggregation along the rows where func is a list of functions:**  

For example,  
    
    print(fdf1.agg(['max','min','mean']))

Output  

          Name     Age  City  Qualification      Score
    max    NaN  36.000   NaN            NaN  52.000000
    min    NaN  22.000   NaN            NaN  23.000000
    mean   NaN  29.125   NaN            NaN  39.833333

However, when list of functions contain combination of cov + other func, currently it will not compute covariance rather compute only aggregation on other function.  

For example,  

    print(fdf1.agg(['cov','min']))

Output  

        Name  Age City Qualification  Score
    min  nan   22  nan           nan   23.0

**Using keyword arguments in order to perform aggregation operation:**  

For example,  

    fdf1.agg(sum_age = ("Age", "sum"), min_score = ("Score", "min"))

Output  

                 Age  Score
    sum_age    233.0    NaN
    min_score    NaN   23.0

__Return Value__  

1. **If one 'func' provided and 'func' is a string:**  
     - It returns a pandas Series instance with numeric column(s) only, after aggregation function is completed.  
2. **If one or more 'func' provided and 'func' is list/dict of string:**  
     - It returns a pandas DataFrame instance with numeric column(s) only, after aggregation function is completed.  

### 2. DataFrame.cov(min_periods = None, ddof = 1.0, low_memory = True, other = None)  

__Parameters__  
**_min\_periods_**: It accepts an integer as parameter. It specifies the minimum number of observations required per 
pair of columns to have a valid result.  (Default: None)  
When it is None (not specified explicitly), then **min_periods = 1**.  
**_ddof_**: It accepts a float parameter that specifies the delta degrees of freedom. (Default: 1.0)  
**_low\_memory_**: It accepts boolean parameter that specifies whethet to enable memory optimised computation or time optimsed computation. (Default: True)  
**_other_**: It accepts frovedis dataframe as parameter, where it must be expressed in **"df[col_name]"** form. Also, it can be expressed in **"df.col_name"** form as well. (Default: None)  

- **When it is not None (specified explicitly)**, it performs covariance operation between both the given frovedis dataframes. Although, the input dataframe must be used as expressions mentioned above.  
- **When it is None (not specified explicitly)**, it will perform covarince on input dataframe to give covraince matrix represented as a dataframe.  

__Purpose__  
It computes pairwise covariance of columns, excluding missing values.  

**Creating frovedis DataFrame from pandas DataFrame:**  

For example,  

    import pandas as pd
    import numpy as np
    import frovedis.dataframe as fdf
    
    # a dictionary
    peopleDF = {
                'Age':[27, 24, 22, 32, 33, 36, 27, 32],
                'Score': [23, 34, 35, 45, 23, 50, 52, 34]
                }
    
    # create pandas dataframe
    pdf1 = pd.DataFrame(peopleDF)
    
    # create frovedis dataframe
    fdf1 = fdf.DataFrame(pdf1)
    
    # display the frovedis dataframe
    fdf1.show()

Output

    index   Age     Score
    0       27      23
    1       24      34
    2       22      35
    3       32      45
    4       33      23
    5       36      50
    6       27      52
    7       32      34

**Compute covariance on frovedis dataframe:**  

For example,

    # cov() demo
    fdf1.cov().show()

Output

    index   Age     Score
    Age     23.5535 11
    Score   11      124.571

It displays a covariance matrix as the frovedis DataFrame instance.  

**Using min_periods parameter to calculate covariance:**  

For example,  

    # cov() demo using min_periods = 8
    fdf1.cov(min_periods = 8).show()

Output  

    index   Age     Score
    Age     NULL    NULL
    Score   NULL    NULL

**Using ddof parameter to calculate covariance:**  

For example,  

    # cov() demo using ddof = 2 
    fdf1.cov(ddof = 2).show()
    
Output  

    index   Age     Score
    Age     27.4791 12.8333
    Score   12.8333 145.333

**In the below example, while using 'other' parameter, both inputs must be frovedis series. Also, the output returned by this method will be a float value.**  

For example,  

    # create another dataframe
    pdf2 = pd.DataFrame({'Score': [51, 34, 33, 45, 12, 82, 67, 91]})
    fdf2 = fdf.DataFrame(pdf2)

    # cov() demo using 'other' parameter 
    print(fdf1['Score'].cov(other = fdf2['Score']))
    
Output  

    158.28571428571428

Here, it could also be expressed as **"fdf1['Score'].cov(other = fdf2['Score'])"**.  

**Note:-** While using input dataframe in the form **'fdf1['Score']' or 'fdf1.Score'**, **'other'** parameter must be provided.  

__Return Value__  

- **If other = None:**  
It returns a covariance matrix represented as frovedis DataFrame instance.  
- **If other != None:**  
It returns covariance as scalar value.  

### 3. DataFrame.mad(axis = None, skipna = None, level = None, numeric_only = None, \*\*kwargs)  

__Parameters__  
**_axis_**: It accepts an integer or string object as parameter. It is used to decide whether to 
perform mean absolute deviation along the columns or rows. (Default: None)  

- **0 or 'index'**: perform mean absolute deviation along the indices.  
- **1 or 'columns'**: perform mean absolute deviation along the columns.  

When it is None (not specified explicitly), it performs mean absolute deviation along the rows.  

_**skipna**_: It accepts boolean as parameter. When set to True, it will exclude missing values while computing the 
result. (Default: None)  
When it is None (not specified explicitly), it excludes missing values during mean absolute deviation computation.  
_**level**_: This is an unused parameter. (Default: None)  
_**numeric\_only**_: This is an unsed parameter. (Default: None)  
_**\*\*kwargs**_: Additional keyword arguments to be passed to the function.  

__Purpose__  
It computes the mean absolute deviation of the values over the requested axis.  

**Currently, mean absolute deviation will be calculated for dataframe having atleast one numeric columns**.  

The parameters: "level", "numeric_only", "\*\*kwargs" are simply kept in to make the interface uniform to the 
pandas DataFrame.mad(). These are not used internally in frovedis.  

**Creating frovedis DataFrame from pandas DataFrame:**  

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

**Mean absolute deviation along the rows (by default):**  

For example,  

    # mad() demo, axis = 0 by default
    fdf1.mad().show()

Output  

    index   mad
    Age     4.125
    Score   9.16666

It displays a frovedis dataframe with numeric column(s) containing the newly computed mean absolute 
deviation for each column.  

Also, it excludes the missing value in **'Score'** column while computing the mean absolute deviation.  

**Mean absolute deviation along the rows and using skipna parameter:**  

For example,  

    # mad() demo using skipna = False
    fdf1.mad(skipna = False).show()
    
Output  

    index   mad
    Age     4.125
    Score   NULL

Here, it includes the missing value in **'Score'** column while computing the mean absolute deviation.  

**Mean absolute deviation along the columns:**  

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

**Mean absolute deviation along the columns and using skipna parameter:**  

For example,  

    # mad() demo using axis = 1 and skipna = False
    fdf1.mad(axis = 1, skipna = False).show()
    
Output  

    index   mad
    0       2
    1       5
    2       6.5
    3       6.5
    4       NULL
    5       7
    6       12.5
    7       NULL

__Return Value__  
It returns a frovedis DataFrame instance with the result of the specified aggregate operation.  

### 4. DataFrame.max(axis = None, skipna = None, level = None, numeric_only = None, \*\*kwargs)  

__Parameters__  
**_axis_**: It accepts an integer or string object as parameter. It is used to decide whether to 
perform maximum operation along the columns or rows. (Default: None)  

- **0 or 'index'**: perform maximum operation along the indices to get the maximum value.  
- **1 or 'columns'**: perform maximum operation along the columns to get the maximum value.  

When it is None (not specified explicitly), it performs maximum operation along the rows.  

_**skipna**_: It accepts boolean as parameter. When set to True, it will exclude missing values while computing the 
result. (Default: None)  
When it is None (not specified explicitly), it excludes missing values during maximum value computation.  
_**level**_: This is an unused parameter. (Default: None)  
_**numeric\_only**_: It accepts a boolean parameter. It determines whether only numeric columns 
are used or non-numeric also. (Default: None)  

- **True**: Use only float, int, boolean columns.  
- **False/None**: Attempt to use all columns(see note below).  

_**\*\*kwargs**_: Additional keyword arguments to be passed to the function.  

__Purpose__  
It computes the maximum of the values over the requested axis.  

The parameters: "level", "\*\*kwargs" are simply kept in to make the interface uniform to the 
pandas DataFrame.max(). These are not used internally in frovedis.  

**Note:-**  

- Parameter "numeric_only" is None(interpreted as False) by default, in this state 
all numeric columns and non-numeric columns including Datetime and Timedelta type 
are valid input. In case value is set as True then only numeric(float, int, boolean) 
columns are valid input.  
- As of now Frovedis does not support mixing of numeric and non-numeric columns. Also 
mixing of non-numeric columns of different types is not supported.  


**Following input dataframes would result in exception as explained above:**  

For example,  

    # numeric mixed with datetime
    index   data0   data1
    0       91      2022-11-30
    1       21      2022-11-30
    2       21      2022-11-29
    3       43      2022-11-28
    
    # Input dataframe with datetime mixed with timedelta
    index   data1           data2
    0       2022-11-30      1669766400000000091
    1       2022-11-27      1669766400000000021
    2       2022-11-29      1669766400000000041
    3       2022-11-28      1669766400000000045
    
    # Input dataframes like above would result in following exception:
    TypeError: Frovedis does not support mixing of numeric and non-numeric columns. Also mixing of 
    non-numeric columns of different types is not supported.  

**Creating frovedis DataFrame from pandas DataFrame:**  

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

**Max operation along the rows (by default):**  

For example,  

    # max() demo
    fdf1.max().show()

Output  

    index   max
    Age     36
    Score   52

It displays a frovedis dataframe with numeric column(s) containing the newly computed maximum value for each column.  

Also, it excludes the missing value in **'Score'** column while computing the maximum value.  

**Max operation along the rows and using skipna parameter:**  

For example,  

    # max() demo using skipna = False
    fdf1.max(skipna = False).show()
    
Output  

    index   max
    Age     36
    Score   NULL

Here, it includes the missing value in **'Score'** column while computing the maximum value.  

**Max operation along the columns:**  

For example,  

    # max() demo using axis = 1
    fdf1.max(axis = 1).show()
    
Output  

    index   max
    0       27
    1       34
    2       35
    3       45
    4       33
    5       50
    6       52
    7       32

**Max operation along the columns and using skipna parameter:**  

For example,  

    # max() demo using axis = 1 and skipna = False
    fdf1.max(axis = 1, skipna = False).show()
    
Output  

    index   max
    0       27
    1       34
    2       35
    3       45
    4       NULL
    5       50
    6       52
    7       NULL

__Return Value__  
It returns a frovedis DataFrame instance with the result of the specified aggregate operation.  

### 5. DataFrame.mean(axis = None, skipna = None, level = None, numeric_only = None, \*\*kwargs)  

__Parameters__  
**_axis_**: It accepts an integer or string object as parameter. It is used to decide whether to 
perform mean along the columns or rows. (Default: None)  
- **0 or 'index'**: perform mean along the indices.  
- **1 or 'columns'**: perform mean along the columns.  

When it is None (not specified explicitly), it performs mean operation along the rows.  

_**skipna**_: It is a boolean parameter. When set to True, it will exclude missing values while computing the result. (Default: None)  
When it is None (not specified explicitly), it excludes missing values during mean computation.  
_**level**_: This is an unused parameter. (Default: None)  
_**numeric\_only**_: It accepts a boolean parameter. It determines whether only numeric columns 
are used or non-numeric also. (Default: None)  

- **True**: Use only float, int, boolean columns.  
- **False/None**: Attempt to use all columns(see note below).  

_**\*\*kwargs**_: Additional keyword arguments to be passed to the function.  

__Purpose__  
It computes mean of the values over the requested axis.  

The parameters: "level", "\*\*kwargs" are simply kept in to make the interface uniform to the 
pandas DataFrame.mean(). These are not used internally in frovedis.  

**Note:-**  

- Parameter "numeric_only" is None(interpreted as False) by default, in this state 
all numeric columns and non-numeric columns including Datetime and Timedelta type 
are valid input. In case value is set as True then only numeric(float, int, boolean) 
columns are valid input.  
- As of now Frovedis does not support mixing of numeric and non-numeric columns. Also 
mixing of non-numeric columns of different types is not supported.  


**Following input dataframes would result in exception as explained above:**  

For example,  

    # numeric mixed with datetime
    index   data0   data1
    0       91      2022-11-30
    1       21      2022-11-30
    2       21      2022-11-29
    3       43      2022-11-28
    
    # Input dataframe with datetime mixed with timedelta
    index   data1           data2
    0       2022-11-30      1669766400000000091
    1       2022-11-27      1669766400000000021
    2       2022-11-29      1669766400000000041
    3       2022-11-28      1669766400000000045
    
    # Input dataframes like above would result in following exception:
    TypeError: Frovedis does not support mixing of numeric and non-numeric columns. Also mixing of 
    non-numeric columns of different types is not supported.  

**Creating frovedis DataFrame from pandas DataFrame:**  

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

**Mean computation along the rows (by default):**  

For example,  

    # mean() demo
    fdf1.mean(),show()

Output  

    index   mean
    Age     29.125
    Score   39.8333

It displays a frovedis dataframe with numeric column(s) containing the newly computed mean for each column.  

Also, it excludes the missing value in **'Score'** column while computing the mean.  

**Mean computation along the rows and using skipna parameter:**  

For example,  

    # mean() demo using skipna = False
    fdf1.mean(skipna = False).show()

Output

    index   mean
    Age     29.125
    Score   NULL

Here, it includes the missing value in **'Score'** column while computing the mean.  

**Max operation along the columns:**  

For example,  

    # mean() demo using axis = 1
    fdf1.mean(axis = 1).show()

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

**Max operation along the columns and using skipna parameter:**  

For example,  

    # mean() demo using axis = 1 and skipna = False
    fdf1.mean(axis = 1, skipna = False).show()

Output  

    index   mean
    0       25
    1       29
    2       28.5
    3       38.5
    4       NULL
    5       43
    6       39.5
    7       NULL

__Return Value__  
It returns a frovedis DataFrame instance.  

### 6. DataFrame.median(axis = None, skipna = None, level = None, numeric_only = None, \*\*kwargs)  

__Parameters__  
**_axis_**: It accepts an integer or string object as parameter. It is used to decide whether to 
perform median operation along the columns or rows. (Default: None)  

- **0 or 'index'**: perform median operation along the indices.  
- **1 or 'columns'**: perform median operation along the columns.  

When it is None (not specified explicitly), it performs median operation along the rows.  

_**skipna**_: It accepts boolean as parameter. When set to True, it will exclude missing values while computing 
the result. (Default: None)  
When it is None (not specified explicitly), it excludes missing values during median computation.  
_**level**_: This is an unused parameter. (Default: None)  
_**numeric\_only**_: It accepts a boolean parameter. It determines whether only numeric columns 
are used or non-numeric also. (Default: None)  

- **True**: Use only float, int, boolean columns.  
- **False/None**: Attempt to use all columns(see note below).  

_**\*\*kwargs**_: Additional keyword arguments to be passed to the function.  

__Purpose__  
It computes median of the values over the requested axis.  

The parameters: "level", "\*\*kwargs" are simply kept in to make the interface uniform to the 
pandas DataFrame.mean(). These are not used internally in frovedis.  

**Note:-**  

- Parameter "numeric_only" is None(interpreted as False) by default, in this state 
all numeric columns and non-numeric columns including Datetime and Timedelta type 
are valid input. In case value is set as True then only numeric(float, int, boolean) 
columns are valid input.  
- As of now Frovedis does not support mixing of numeric and non-numeric columns. Also 
mixing of non-numeric columns of different types is not supported.  


**Following input dataframes would result in exception as explained above:**  

For example,  

    # numeric mixed with datetime
    index   data0   data1
    0       91      2022-11-30
    1       21      2022-11-30
    2       21      2022-11-29
    3       43      2022-11-28
    
    # Input dataframe with datetime mixed with timedelta
    index   data1           data2
    0       2022-11-30      1669766400000000091
    1       2022-11-27      1669766400000000021
    2       2022-11-29      1669766400000000041
    3       2022-11-28      1669766400000000045
    
    # Input dataframes like above would result in following exception:
    TypeError: Frovedis does not support mixing of numeric and non-numeric columns. Also mixing of 
    non-numeric columns of different types is not supported.  

**Creating frovedis DataFrame from pandas DataFrame:**  

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

**Median computation along the rows (by default):**  

For example,  

    # median() demo
    fdf1.median().show()

Output  

    index   median
    Age     29.5
    Score   40

It displays a frovedis dataframe with numeric column(s) containing the newly computed median for each column.  

Also, it excludes the missing value in **'Score'** column while computing the median.  

**Median computation along the rows and using skipna parameter:**  

For example,  

    # median() demo using skipna = False
    fdf1.median(skipna = False).show()

Output

    index   median
    Age     29.5
    Score   NULL

Here, it includes the missing value in **'Score'** column while computing the median.  

**Median computation along the columns:**  

For example,  

    # median() demo using axis = 1
    fdf1.median(axis = 1).show()

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

**Median computation along the columns and using skipna parameter:**  

For example,  

    # median() demo using axis = 1 and skipna = False
    fdf1.median(axis = 1, skipna = False).show()

Output  

    index   median
    0       25
    1       29
    2       28.5
    3       38.5
    4       NULL
    5       43
    6       39.5
    7       NULL

__Return Value__  
It returns a frovedis DataFrame instance.  

### 7. DataFrame.min(axis = None, skipna = None, level = None, numeric_only = None, \*\*kwargs)  

__Parameters__  
**_axis_**: It accepts an integer or string object as parameter. It is used to decide whether to 
perform minimum operation along the columns or rows. (Default: None)  

- **0 or 'index'**: perform minimum operation along the indices to get the minimum value.  
- **1 or 'columns'**: perform minimum operation along the columns to get the minimum value.  

When it is None (not specified explicitly), it performs minimum operation along the rows.  

_**skipna**_: It accepts boolean as parameter. When set to True, it will exclude missing values while computing 
the result. (Default: None)  
When it is None (not specified explicitly), it excludes missing values during minimum value computation.  
_**level**_: This is an unused parameter. (Default: None)  
_**numeric\_only**_: It accepts a boolean parameter. It determines whether only numeric columns 
are used or non-numeric also. (Default: None)  

- **True**: Use only float, int, boolean columns.  
- **False/None**: Attempt to use all columns(see note below).  

_**\*\*kwargs**_: Additional keyword arguments to be passed to the function.  

__Purpose__  
It computes the minimum of the values over the requested axis.  

The parameters: "level", "\*\*kwargs" are simply kept in to make the interface uniform to the 
pandas DataFrame.min(). These are not used internally in frovedis.  

**Note:-**  

- Parameter "numeric_only" is None(interpreted as False) by default, in this state 
all numeric columns and non-numeric columns including Datetime and Timedelta type 
are valid input. In case value is set as True then only numeric(float, int, boolean) 
columns are valid input.  
- As of now Frovedis does not support mixing of numeric and non-numeric columns. Also 
mixing of non-numeric columns of different types is not supported.  


**Following input dataframes would result in exception as explained above:**  

For example,  

    # numeric mixed with datetime
    index   data0   data1
    0       91      2022-11-30
    1       21      2022-11-30
    2       21      2022-11-29
    3       43      2022-11-28
    
    # Input dataframe with datetime mixed with timedelta
    index   data1           data2
    0       2022-11-30      1669766400000000091
    1       2022-11-27      1669766400000000021
    2       2022-11-29      1669766400000000041
    3       2022-11-28      1669766400000000045
    
    # Input dataframes like above would result in following exception:
    TypeError: Frovedis does not support mixing of numeric and non-numeric columns. Also mixing of 
    non-numeric columns of different types is not supported.  

**Creating frovedis DataFrame from pandas DataFrame:**  

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

**Min operation along the rows (by default):**  

For example,  

    # min() demo
    fdf1.min().show()

Output  

    index   min
    Age     22
    Score   23

It displays a frovedis dataframe with numeric column(s) containing the newly computed minimum value for each column.  

Also, it excludes the missing value in **'Score'** column while computing the minimum value.  

**Min operation along the rows and using skipna parameter:**  

For example,  

    # min() demo using skipna = False
    fdf1.min(skipna = False).show()
    
Output  

    index   min
    Age     22
    Score   NULL

Here, it includes the missing value in **'Score'** column while computing the median.  

**Min operation along the columns:**  

For example,  

    # min() demo using axis = 1
    fdf1.min(axis = 1).show()
    
Output  

    index   min
    0       23
    1       24
    2       22
    3       32
    4       33
    5       36
    6       27
    7       32

**Min operation along the columns and using skipna parameter:**  

For example,  

    # min() demo using axis = 1 and skipna = False
    fdf1.min(axis = 1, skipna = False).show()
    
Output  

    index   min
    0       23
    1       24
    2       22
    3       32
    4       NULL
    5       36
    6       27
    7       NULL
    
__Return Value__  
It returns a frovedis DataFrame instance with the result of the specified aggregate operation.  

### 8. DataFrame.mode(axis = 0, numeric_only = False, dropna = True)  
**_axis_**: It accepts an integer or string object as parameter. It is used to decide whether to 
perform standard error of the mean along the columns or rows. (Default: 0)  

- **0 or 'index'**: perform mode along the indices.  
- **1 or 'columns'**: perform mode along the columns.  

When it is None (not specified explicitly), it performs standard error of the mean along the rows.  

_**numeric\_only**_: It accepts string object as parameter. If True, mode operation will result in a dataframe having only numeric columns. Otherwise, it will result in a dataframe having both numeric and non-numeric columns. (Default: False)  
_**dropna**_: It accepts boolean as parameter. When set to True, it will exclude missing values while 
computing the result of mode operation. (Default: True)  
When it is None (not specified explicitly), it excludes missing values during mode computation.  

__Purpose__  
This method gets the mode(s) of each element along the selected axis.  

The mode of a set of values is the value that appears most often. It can be multiple values.  

**When input with non-numeric columns is used with mode(), then it ignores the non-numeric columns for mode computation.**  

**Creating frovedis DataFrame from pandas DataFrame:**  

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

**Mode computation along the rows (by default):**  

For example,  

    # mode() demo
    fdf1.mode().show()

Output  

    index   Score   Name    Age     City    Qualification
    0       23      Anuj    27      Kanpur  B.Tech
    1       34      Jai     32      NULL    Phd
    2       35      Princi  NULL    NULL    NULL
    3       45      NULL    NULL    NULL    NULL
    4       50      NULL    NULL    NULL    NULL
    5       52      NULL    NULL    NULL    NULL

**Mode will be calculated for dataframe having string and numeric columns when axis = 0 or 'index'.**  
**Also, resultant dataframe has both numeric and non-numeric columns.**  

**Mode computation along the rows and using numeric_only parameter:**  

For example,  

    # mode() demo using numeric_ony = True
    fdf1.mode(numeric_ony = True).show()

Output  

    index   Score   Age
    0       23      27
    1       34      32
    2       35      NULL
    3       45      NULL
    4       50      NULL
    5       52      NULL

Here, resultant dataframe has only numeric columns.  

**Mode computation along the rows and using dropna parameter:**  

For example,  

    # mode() demo using dropna = False
    fdf1.mode(dropna = False).show()

Output  

    index   Name    Age     City    Qualification  Score
    0       Anuj    27      Kanpur  B.Tech         NULL
    1       Jai     32      NULL    Phd            NULL
    2       Princi  NULL    NULL    NULL           NULL

**For axis = 1 or 'columns', mode will be calculated for dataframe having only numeric columns.**  

For example,  

    import pandas as pd
    import numpy as np
    import frovedis.dataframe as fdf
    
    # a dictionary
    bmiDF = {
            'height':[157, 124, 162, np.nan, 133, 176, np.nan, 152],
            'weight': [53, 64, np.nan, 65, 63, 80, np.nan, 84]
            }

    # create pandas dataframe with only numeric columns
    pdf1 = pd.DataFrame(bmiDF)
    
    # create frovedis dataframe
    fdf1 = fdf.DataFrame(pdf1)
    
    # display the frovedis dataframe
    fdf1.show()

Output  

    index   height  weight
    0       157     53
    1       124     64
    2       162     NULL
    3       NULL    65
    4       133     63
    5       176     80
    6       NULL    NULL
    7       152     84

**Mode computation along the columns:**  

For example,  

    # mode() demo using axis = 1
    fdf1.mode(axis = 1).show()

Output  

    index   0
    0       157
    1       124
    2       162
    3       65
    4       133
    5       176
    6       0
    7       152

**Mode computation along the columns and using dropna parameter:**  

For example,  

    # mode() demo using axis = 1 and dropna = False
    fdf1.mode(axis = 1, dropna = False).show()

Output  

    index   0
    0       157
    1       124
    2       162
    3       NULL
    4       133
    5       176
    6       NULL
    7       152

__Return Value__  

- **If numeric_only = False**:  
It returns a frovedis DataFrame instance having both numeric and non-numeric columns (if any).  
- **If numeric_only = True**:  
It returns a frovedis DataFrame instance having only numeric columns.  

### 9. DataFrame.sem(axis = None, skipna = None, level = None, ddof = 1, numeric_only = None, \*\*kwargs)  

__Parameters__  
**_axis_**: It accepts an integer or string object as parameter. It is used to decide whether to 
perform standard error of the mean along the columns or rows. (Default: None)  

- **0 or 'index'**: perform standard error of the mean along the indices.  
- **1 or 'columns'**: perform standard error of the mean along the columns.  

When it is None (not specified explicitly), it performs standard error of the mean along the rows.  

_**skipna**_: It accepts boolean as parameter. When set to True, it will exclude missing values while 
computing the result. (Default: None)  
When it is None (not specified explicitly), it excludes missing values during standard error of the 
mean computation.  
_**level**_: This is an unused parameter. (Default: None)  
**_ddof_**: It accepts an integer parameter that specifies the delta degrees of freedom. (Default: 1)  
_**numeric\_only**_: This is an unsed parameter. (Default: None)  
_**\*\*kwargs**_: Additional keyword arguments to be passed to the function.  

__Purpose__  
It computes standard error of the mean over requested axis.  

**Currently, standard error of the mean will be calculated for dataframe having atleast one numeric columns**.  

The parameters: "level", "numeric_only", "\*\*kwargs" are simply kept in to make the interface uniform to the 
pandas DataFrame.sem(). These are not used internally in frovedis.  

**Creating frovedis DataFrame from pandas DataFrame:**  

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

**Standard error of the mean operation along the rows (by default):**  

For example,  

    # sem() demo
    fdf1.sem().show()

Output  

    index   sem
    Age     1.71586
    Score   4.54178

It displays a frovedis dataframe with numeric column(s) containing the newly computed standard error 
of the mean for each column.  

**Standard error of the mean operation along the rows and using ddof parameter:**  

For example,  

    # sem() demo using ddof = 2 
    fdf1.sem(ddof = 2).show()
    
Output  

    index   sem
    Age     1.85334
    Score   5.07786

**Standard error of the mean operation along the rows and using skipna parameter:**  

For example,  

    # sem() demo using skipna = False
    fdf1.sem(skipna = False).show()
    
Output  

    index   sem
    Age     1.71586
    Score   NULL

**Standard error of the mean operation along the columns:**  

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

**Standard error of the mean operation along the columns and using skipna parameter:**  

For example,  

    # sem() demo using axis = 1 and skipna = False
    fdf1.sem(axis = 1, skipna = False).show()
    
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

__Return Value__  
It returns a frovedis DataFrame instance with the result of the specified aggregate operation.  

### 10. DataFrame.std(axis = None, skipna = None, level = None, ddof = 1, numeric_only = None, \*\*kwargs)  

__Parameters__  
**_axis_**: It accepts an integer or string object as parameter. It is used to decide whether to 
perform standard deviation along the columns or rows. (Default: None)  

- **0 or 'index'**: perform standard deviation along the indices.  
- **1 or 'columns'**: perform standard deviation along the columns.  

When it is None (not specified explicitly), it performs standard deviation along the rows.  
_**skipna**_: It accepts boolean as parameter. When set to True, it will exclude missing values while computing the 
result. (Default: None)  
When it is None (not specified explicitly), it excludes missing values during standard deviation computation.  
_**level**_: This is an unused parameter. (Default: None)  
**_ddof_**: It accepts an integer parameter that specifies the delta degrees of freedom. (Default: 1)  
_**numeric\_only**_: It accepts a boolean parameter. It determines whether only numeric columns 
are used or non-numeric also. (Default: None)  

- **True**: Use only float, int, boolean columns.  
- **False/None**: Attempt to use all columns(see note below).  

_**\*\*kwargs**_: Additional keyword arguments to be passed to the function.  

__Purpose__  
It computes standard deviation over requested axis.  

The parameters: "level", "\*\*kwargs" are simply kept in to make the interface uniform to the 
pandas DataFrame.min(). These are not used internally in frovedis.  

**Note:-**  

- Parameter "numeric_only" is None(interpreted as False) by default, in this state 
all numeric columns and non-numeric columns including Datetime and Timedelta type 
are valid input. In case value is set as True then only numeric(float, int, boolean) 
columns are valid input.  
- As of now Frovedis does not support mixing of numeric and non-numeric columns. Also 
mixing of non-numeric columns of different types is not supported.  


**Following input dataframes would result in exception as explained above:**  

For example,  

    # numeric mixed with datetime
    index   data0   data1
    0       91      2022-11-30
    1       21      2022-11-30
    2       21      2022-11-29
    3       43      2022-11-28
    
    # Input dataframe with datetime mixed with timedelta
    index   data1           data2
    0       2022-11-30      1669766400000000091
    1       2022-11-27      1669766400000000021
    2       2022-11-29      1669766400000000041
    3       2022-11-28      1669766400000000045
    
    # Input dataframes like above would result in following exception:
    TypeError: Frovedis does not support mixing of numeric and non-numeric columns. Also mixing of 
    non-numeric columns of different types is not supported.  

**Creating frovedis DataFrame from pandas DataFrame:**  

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

**Standard deviation computation along the rows (by default):**  

For example,  

    # std() demo
    fdf1.std().show()

Output  

    index   std
    Age     4.8532
    Score   11.125

It displays a frovedis dataframe with numeric column(s) containing the newly computed standard deviation 
for each column.  

**Standard deviation computation along the rows and using skipna parameter:**  

For example,  

    # std() demo using skipna = False
    fdf1.std(skipna = False).show()
    
Output  

    index   std
    Age     4.8532
    Score   NULL

**Standard deviation computation along the rows and using ddof parameter:**  

For example,  

    # std() demo using ddof = 2 
    fdf1.std(ddof = 2).show()
    
Output  

    index   std
    Age     5.24205
    Score   12.4381

**Standard deviation computation along the columns:**  

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

**Standard deviation computation along the columns and using skipna parameter:**  

For example,  

    # std() demo using axis = 1 and skipna = False
    fdf1.std(axis = 1, skipna = False).show()
    
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

__Return Value__  
It returns a frovedis DataFrame instance with the result of the specified aggregate functions.  

### 11. DataFrame.sum(axis = None, skipna = None, level = None, numeric_only = None, min_count = 0, \*\*kwargs)  

__Parameters__  
**_axis_**: It accepts an integer or string object as parameter. It is used to decide whether to 
perform summation operation along the columns or rows. (Default: None)  

- **0 or 'index'**: perform summation operation along the indices.  
- **1 or 'columns'**: perform summation operation along the columns.  

When it is None (not specified explicitly), it performs summation operation along the rows.  
_**skipna**_: It accepts boolean as parameter. When set to True, it will exclude missing values while computing the 
result. (Default: None)  
When it is None (not specified explicitly), it excludes missing values during summation computation.  
_**level**_: This is an unused parameter. (Default: None)  
_**numeric\_only**_: This is an unsed parameter. (Default: None)  
_**min\_count**_: It is an integer, float or double (float64) parameter that specifies the minimum number of values that needs to be present to perform the action. (Default: 0)  
_**\*\*kwargs**_: Additional keyword arguments to be passed to the function.  

__Purpose__  
It computes the sum of the values over the requested axis.  

**Currently, summation will be calculated for dataframe having atleast one numeric columns**.  

The parameters: "level", "numeric_only", "\*\*kwargs" are simply kept in to make the interface uniform to the 
pandas DataFrame.sum(). These are not used internally in frovedis.  

**Creating frovedis DataFrame from pandas DataFrame:**  

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

**Sum operation along the rows (by default):**  

For example,  

    # sum() demo
    fdf1.sum().show()

Output  

    index   sum
    Age     233
    Score   239

It displays a frovedis dataframe with numeric column(s) containing the newly computed summation 
for each column.  

Also, it excludes the missing value in **'Score'** column while computing summation.  

**Sum operation along the rows and using skipna parameter:**  

For example,  

    # sum() demo using skipna = False
    fdf1.sum(skipna = False).show()
    
Output  

    index   sum
    Age     233
    Score   NULL

Here, it includes the missing value in **'Score'** column while computing the sum.  

**Sum operation along the columns:**  

For example,  

    # sum() demo using axis = 1
    fdf1.sum(axis = 1).show()
    
Output  

    index   sum
    0       50
    1       58
    2       57
    3       77
    4       33
    5       86
    6       79
    7       32

**Sum operation along the columns and using skipna parameter:**  

For example,  

    # sum() demo using axis = 1 and skipna = False
    fdf1.sum(axis = 1, skipna = False).show()
    
Output  

    index   sum
    0       50
    1       58
    2       57
    3       77
    4       NULL
    5       86
    6       79
    7       NULL

__Return Value__  
It returns a frovedis DataFrame instance with the result of the specified aggregate functions.  

### 12. DataFrame.var(axis = None, skipna = None, level = None, ddof = 1, numeric_only = None, \*\*kwargs)  

__Parameters__  
**_axis_**: It accepts an integer or string object as parameter. It is used to decide whether to 
perform variance along the columns or rows. (Default: None)  

- **0 or 'index'**: perform variance along the indices.  
- **1 or 'columns'**: perform variance along the columns.  

When it is None (not specified explicitly), it performs variance along the rows.  
_**skipna**_: It accepts boolean as parameter. When set to True, it will exclude missing values while computing the 
result. (Default: None)  
When it is None (not specified explicitly), it excludes missing values during variance computation.  
_**level**_: This is an unused parameter. (Default: None)  
**_ddof_**: It accepts an integer parameter that specifies the delta degrees of freedom. (Default: 1)  
_**numeric\_only**_: This is an unsed parameter. (Default: None)  
_**\*\*kwargs**_: Additional keyword arguments to be passed to the function.  

__Purpose__  
It computes variance over requested axis.  

**Currently, variance will be calculated for dataframe having atleast one numeric columns**.  

The parameters: "level", "numeric_only", "\*\*kwargs" are simply kept in to make the interface uniform to the 
pandas DataFrame.var(). These are not used internally in frovedis.  

**Creating frovedis DataFrame from pandas DataFrame:**  

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

**Variance computation along the rows (by default):**  

For example,  

    # var() demo
    fdf1.var().show()

Output  

    index   var
    Age     23.5535
    Score   123.766

It displays a frovedis dataframe with numeric column(s) containing the newly computed variance 
for each column.  

Also, it excludes the missing value in **'Score'** column while computing variance.  

**Variance computation along the rows and using skipna parameter:**  

For example,  

    # var() demo using skipna = False
    fdf1.var(skipna = False).show()
    
Output  

    index   var
    Age     23.5535
    Score   NULL

**Variance computation along the rows and using ddof parameter:**  

For example,  

    # var() demo using ddof = 2 
    fdf1.var(ddof = 2).show()
    
Output  

    index   var
    Age     27.4791
    Score   154.708

**Variance computation along the columns:**  

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

**Variance computation along the columns and using skipna parameter:**  

For example,  

    # var() demo using axis = 1 and skipna = False
    fdf1.var(axis = 1, skipna = False).show()
    
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

__Return Value__  
It returns a frovedis DataFrame instance with the result of the specified aggregate operation.  

# SEE ALSO  

- **[DataFrame - Introduction](./df_intro.md)**  
- **[DataFrame - Indexing Operations](./df_indexing_operations.md)**  
- **[DataFrame - Generic Fucntions](./df_generic_func.md)**  
- **[DataFrame - Conversion Functions](./df_conversion.md)**  
- **[DataFrame - Sorting Functions](./df_sort.md)**  
- **[DataFrame - Math Functions](./df_math_func.md)**  
