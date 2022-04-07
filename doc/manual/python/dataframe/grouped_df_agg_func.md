% FrovedisGroupedDataFrame Aggregate Functions
  
# NAME  

FrovedisGroupedDataFrame Aggregate Functions - aggregate operations performed on grouped dataframe are being illustrated here.  
  
## DESCRIPTION  

Once FrovedisGroupedDataframe instance is created, several aggregation operations can be performed on it such as max(), min(), sem(), var(), etc.  

The aggregation operation will basically compute summary for each group. Some examples:  
  
- Compute group sums, means, var or sem.  
- Compute group sizes / counts.  
- Compute max or min in the group.

Also, aggregation functions can be chained along with groupby() calls in frovedis.  

## Public Member Functions  

    1. agg(func, \*args, \*\*kwargs)
    2. aggregate(func, \*args, \*\*kwargs)
    3. count(numeric_only = True)
    4. groupby()
    5. max(numeric_only = True, min_count = -1)
    6. mean(numeric_only = True)
    7. min(numeric_only = True, min_count = -1)
    8. sem(ddof = 1.0)
    9. size(numeric_only = True)
    10. sum(numeric_only = True, min_count = 0)
    11. var(ddof = 1.0)

## Detailed Description  
  
### 1. FrovedisGroupedDataFrame.agg(func, \*args, \*\*kwargs)  

__Parameters__  
**_func_**: Names of functions to use for aggregating the data. The input to be used with the function must 
be a frovedis DataFrame instance having atleast one numeric column.  
Accepted combinations for this parameter are:
- A string function name such as 'max', 'min', etc.  
- list of functions and/or function names, For example, ['max', 'mean'].  
- dictionary with keys as column labels and values as function name or list of such functions.  
For Example, {'Age': ['max','min','mean'], 'Ename': ['count']}  

_**\*args**_: This is an unused parameter.  
_**\*\*kwargs**_: This is an unused parameter.  

__Purpose__  
It computes an aggregate operation based on the condition specified in 'func'. It is an alias for aggregate().  

**Constructing frovedis DataFrame from a pandas DataFrame:**  

For example,  
    
    import pandas as pd
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
                
    # convert to pandas dataframe
    pdf1 = pd.DataFrame(peopleDF)

    # convert to frovedis dataframe
    fdf1 = fdf.DataFrame(pdf1)
    
    # displaying created frovedis dataframe
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

**Aggregate the function on the grouped dataframe where func is a function string name:**  

For example,  

    # agg() demo with func as a function string name 
    fdf1.groupby('Qualification').agg('min').show()

Output  

    Qualification   min_Age min_Score
    B.Tech          22      23
    Phd             24      34

It displays a frovedis dataframe containing numeric column(s) with newly computed minimum of each groups.  

**Aggregate the function on the grouped dataframe where func is a dictionary:**  

For example,  
    
    # agg() demo with func as a dictionary 
    fdf1.groupby('Qualification').agg({'Age': ['max','min','mean'], 'Score': ['sum']}).show()

Output  

    Qualification   max_Age min_Age mean_Age  sum_Score
    B.Tech          36      22      29.25     108
    Phd             33      24      29        13

**Aggregate the function on the grouped dataframe where func is a list of functions:**  

For example,  
    
    # agg() demo where func is a list of functions 
    fdf1.groupby('Qualification').agg(['min', 'max']).show()

Output  

    Qualification   min_Age max_Age min_Score  max_Score
    B.Tech          22      36      23         50
    Phd             24      33      34         52

__Return Value__  
It returns a new frovedis DataFrame instance with the result of the specified aggregate functions.  

### 2. FrovedisGroupedDataFrame.aggregate(func, \*args, \*\*kwargs)  

__Parameters__  
**_func_**: Names of functions to use for aggregating the data. The input to be used with the function must 
be a frovedis DataFrame instance having atleast one numeric column.  
Accepted combinations for this parameter are:
- A string function name such as 'max', 'min', etc.  
- list of functions and/or function names, For example, ['max', 'mean'].  
- dictionary with keys as column labels and values as function name or list of such functions.  
For Example, {'Age': ['max','min','mean'], 'Ename': ['count']}  

_**\*args**_: This is an unused parameter.  
_**\*\*kwargs**_: This is an unused parameter.  

__Purpose__  
It computes an aggregate operation based on the condition specified in 'func'.  

**Constructing frovedis DataFrame from a pandas DataFrame:**  

For example,  
    
    import pandas as pd
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

                
    # convert to pandas dataframe
    pdf1 = pd.DataFrame(peopleDF)

    # convert to frovedis dataframe
    fdf1 = fdf.DataFrame(pdf1)
    
    # displaying created frovedis dataframe
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

**Aggregate the function on the grouped dataframe where func is a function string name:**  

For example,  

    # aggregate() demo using FrovedisGroupedDataframe instance and 'func' as a function string name 
    fdf1.groupby('Qualification').agg('max').show()
    
Output  

    Qualification   max_Age max_Score
    B.Tech          36      50
    Phd             33      52

It displays a frovedis dataframe containing numeric column(s) with newly computed minimum of each groups.   

**Aggregate the function on the grouped dataframe where func is a dictionary:**  

For example,  
    
    # aggregate() demo using FrovedisGroupedDataframe instance and 'func' as a dictionary 
    fdf1.groupby('Qualification').agg({'Age': ['count'], 'Score': ['max','min']}).show()

Output  

    Qualification   count_Age  max_Score  min_Score
    B.Tech          4          50         23
    Phd             4          52         34

**Aggregate the function on the grouped dataframe where func is a list of functions:**  

For example,  
    
    # aggregate() demo using FrovedisGroupedDataframe instance and 'func' is a list of functions 
    fdf1.groupby('Qualification').agg(['mean','sum']).show()  

Output  

    Qualification   mean_Age  sum_Age mean_Score  sum_Score
    B.Tech          29.25     117     36          108
    Phd             29        116     43.6666     131

__Return Value__  
It returns a new frovedis DataFrame instance with the result of the specified aggregate functions.  

### 3. FrovedisGroupedDataFrame.count(numeric_only = True)  

__Parameters__  
**_numeric\_only_**: It accepts a boolean parameter that specifies whether or not to use only numeric 
column data as input. (Default: True)  

__Purpose__  
It computes count of groups, excluding the missing values.  

**Constructing frovedis DataFrame from a pandas DataFrame:**  

For example,  
    
    import pandas as pd
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
                
    # convert to pandas dataframe
    pdf1 = pd.DataFrame(peopleDF)

    # convert to frovedis dataframe
    fdf1 = fdf.DataFrame(pdf1)
    
    # displaying created frovedis dataframe
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

**Count groups in each column of the grouped dataframe:**  

For example,  

    # count() demo using FrovedisGroupedDataframe instance 
    fdf1.groupby('Qualification').count().show()
    
Output  

    Qualification   count_Age   count_Score  count_Name  count_City
    B.Tech          4           3            4           4
    Phd             4           3            4           4

It displays a frovedis dataframe containing the newly computed count of each groups.  

Also, it excludes the missing value(s) in **'Score'** column while computing count 
of groups **'B.Tech'** and **'Phd'**.  

__Return Value__  
It returns a new frovedis DataFrame instance with the result of the specified aggregate functions.  

### 4. DataFrame.groupby(by = None, axis = 0, level = None, as_index = True, sort = True, group_keys = True, squeeze = False, observed = False, dropna = True)  

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

The parameters: "level", "as_index", "sort", "group_keys", "squeeze" and "observed" is simply kept in to make the interface uniform to the pandas DataFrame.groupby(). This is not used anywhere within the frovedis implementation.  

**Constructing frovedis DataFrame from a pandas DataFrame:**  

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

**Performing groupby operation on the frovedis dataframe:**  

For example,  

    fdf1.groupby('Qualification')
    
This will perform groupby operation on the dataframe over **'Qualification'** column data.  

**Using groupby() to perform aggregation on resultant grouped dataframe. Also dropna = True:**  

For example,  

    fdf1.groupby('Qualification', dropna = True).agg({'Score': 'count'})

Output  

    Qualification   count_Score
    B.Tech  3
    Phd     2
    
Here, it excludes **NULL** group since missing values were dropped during groupby().  

**Using groupby() to perform aggregation on resultant grouped dataframe. Also dropna = False:**  

For example,  

    fdf1.groupby('Qualification', dropna = False).agg({'Score': 'count'})

Output  

    Qualification   count_Score
    B.Tech  3
    Phd     2
    NULL    1
    
Here, it includes **NULL** as new group since missing values were not dropped during groupby().  

__Return Value__  
It returns a FrovedisGroupedDataFrame instance. This instance is then further used to perform aggregate operations.  

### 5. DataFrame.max(numeric_only = True, min_count = -1)  

__Parameters__  
**_numeric\_only_**: It accepts a boolean parameter that specifies whether or not to use only numeric 
columns for aggregation. (Default: True)  
**_min\_count_**: This is an unused parameter. (Default: -1)  

__Purpose__  
It computes the maximum of group values.  

The parameter: "min_count" is simply kept in to to make the interface uniform to the pandas GroupBy.max().  
This is not used anywhere within the frovedis implementation.  

**Currently, this method only displays result for dataframe having atleast one numeric column.**  

**Constructing frovedis DataFrame from a pandas DataFrame:**  

For example,  
    
    import pandas as pd
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

                
    # convert to pandas dataframe
    pdf1 = pd.DataFrame(peopleDF)

    # convert to frovedis dataframe
    fdf1 = fdf.DataFrame(pdf1)
    
    # displaying created frovedis dataframe
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

**To calculate maximum value in each group of grouped dataframe:**  

For example,  

    # max() demo using FrovedisGroupedDataframe instance 
    fdf1.groupby('Qualification').max().show()
    
Output  

    Qualification   max_Age max_Score
    B.Tech          36      50
    Phd             33      52

It displays a frovedis dataframe containing numeric column(s) with newly computed maximum of each groups.   

__Return Value__  
It returns a new frovedis DataFrame instance with the result of the specified aggregate functions.  

### 6. FrovedisGroupedDataFrame.mean(numeric_only = True)  
 
__Parameters__  
**_numeric\_only_**: It accepts a boolean parameter that specifies whether or not to use only numeric 
column data as input. (Default: True)  

__Purpose__  
It computes mean of groups, excluding the missing values.  

**Currently, this method only displays result for dataframe having atleast one numeric column.**  

**Constructing frovedis DataFrame from a pandas DataFrame:**  

For example,  
    
    import pandas as pd
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
                
    # convert to pandas dataframe
    pdf1 = pd.DataFrame(peopleDF)

    # convert to frovedis dataframe
    fdf1 = fdf.DataFrame(pdf1)
    
    # displaying created frovedis dataframe
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

**Compute mean of each group:**  

For example,  

    # mean() demo using FrovedisGroupedDataframe instance 
    fdf1.groupby('Qualification').mean().show()
    
Output  

    Qualification   mean_Age  mean_Score
    B.Tech          29.25     36
    Phd             29        43.6666

It displays a frovedis dataframe containing numeric column(s) with newly computed mean of each groups.  

Also, it excludes the missing value(s) in **'Score'** column while computing mean 
of groups **'B.Tech'** and **'Phd'**.  

__Return Value__  
It returns a new frovedis DataFrame instance with the result of the specified aggregate functions.  

### 7. FrovedisGroupedDataFrame.min(numeric_only = True, min_count = -1)  

__Parameters__  
**_numeric\_only_**: It accepts a boolean parameter that specifies whether or not to use only numeric 
columns for aggregation. (Default: True)  
**_min\_count_**: This is an unused parameter. (Default: -1)  
  
__Purpose__  
It computes the minimum of group values.  

The parameter: "min_count" is simply kept in to to make the interface uniform to the pandas GroupBy.min(). 
This is not used anywhere within the frovedis implementation.  

**Currently, this method only displays result for dataframe having atleast one numeric column.**  

**Constructing frovedis DataFrame from a pandas DataFrame:**  

For example,  
    
    import pandas as pd
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
                
    # convert to pandas dataframe
    pdf1 = pd.DataFrame(peopleDF)

    # convert to frovedis dataframe
    fdf1 = fdf.DataFrame(pdf1)
    
    # displaying created frovedis dataframe
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

**To calculate minimum value in each group of grouped dataframe:**  

For example,  
    
    # min() demo using FrovedisGroupedDataframe instance 
    fdf1.groupby('Qualification').min().show()
    
Output  

    Qualification   min_Age min_Score
    B.Tech          22      23
    Phd             24      34

It displays a frovedis dataframe containing numeric column(s) with newly computed minimum of each groups.   

__Return Value__  
It returns a new frovedis DataFrame instance with the result of the specified aggregate functions.  

### 8. FrovedisGroupedDataFrame.sem(ddof = 1.0)  

__Parameters__  
**_ddof_**: It accepts an integer parameter that specifies the delta degrees of freedom. (Default: 1.0)  

__Purpose__  
It computes standard error of the mean of groups, excluding missing values.  

**Currently, this method only displays result for dataframe having atleast one numeric column.**  

**Constructing frovedis DataFrame from a pandas DataFrame:**  

For example,  
    
    import pandas as pd
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
                
    # convert to pandas dataframe
    pdf1 = pd.DataFrame(peopleDF)

    # convert to frovedis dataframe
    fdf1 = fdf.DataFrame(pdf1)
    
    # displaying created frovedis dataframe
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

**Compute standard error of the mean of each group:**  

For example,

    # sem() demo using FrovedisGroupedDataframe instance and default ddof value 
    fdf1.groupby('Qualification').sem().show()
    
Output  

    Qualification   sem_Age  sem_Score
    B.Tech          3.03795  7.81024
    Phd             2.12132  5.23874

It displays a frovedis dataframe with numeric column(s) containing the newly computed 
standard error of mean for each groups.  

Also, it excludes the missing value(s) in **'Score'** column while computing standard error of mean 
for the groups **'B.Tech'** and **'Phd'**.  

**Using ddof parameter to compute standard error of the mean of each group:**  

For example,

    # sem() demo using ddof = 2 
    fdf1.groupby('Qualification').sem(ddof = 2).show()
    
Output  

    Qualification   sem_Age  sem_Score
    B.Tech          3.72071  11.0453
    Phd             2.59807  7.4087

__Return Value__  
It returns a new frovedis DataFrame instance with the result of the specified aggregate functions.  

### 9. FrovedisGroupedDataFrame.size(numeric_only = True)  

__Parameters__  
**_numeric\_only_**: It accepts a boolean parameter that specifies whether or not to use only numeric 
column data as input. (Default: True)  

__Purpose__  
It computes group sizes, including the missing values.  

**Constructing frovedis DataFrame from a pandas DataFrame:**  

For example,  
    
    import pandas as pd
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
                
    # convert to pandas dataframe
    pdf1 = pd.DataFrame(peopleDF)

    # convert to frovedis dataframe
    fdf1 = fdf.DataFrame(pdf1)
    
    # displaying created frovedis dataframe
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

**To compute size of groups for the grouped dataframe:**  

For example,  
    
    # size() demo using FrovedisGroupedDataframe instance 
    fdf1.groupby('Qualification').size().show()
    
Output

    Qualification   size_Qualification
    B.Tech          4
    Phd             4

It displays a frovedis dataframe containing a column with the newly computed size of each group.  

Also, it does not exclude the missings values while computing group size.  

__Return Value__  
It returns a new frovedis DataFrame instance with the result of the specified aggregate functions.  

### 10. FrovedisGroupedDataFrame.sum(numeric_only = True, min_count = 0)  

__Parameters__  
**_numeric\_only_**: It accepts a boolean parameter that specifies whether or not to use only numeric 
column data as input. (Default: True)  
**_min\_count_**: This is an unused parameter. (Default: 0)  

__Purpose__  
It computes the sum of group values.  

The parameter: "min_count" is simply kept in to to make the interface uniform to the pandas GroupBy.sum().  
This is not used anywhere within the frovedis implementation.  

**Currently, this method only displays result for dataframe having atleast one numeric column.**  

**Constructing frovedis DataFrame from a pandas DataFrame:**  

For example,  
    
    import pandas as pd
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
                
    # convert to pandas dataframe
    pdf1 = pd.DataFrame(peopleDF)

    # convert to frovedis dataframe
    fdf1 = fdf.DataFrame(pdf1)
    
    # displaying created frovedis dataframe
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

**To compute sum of group values for the grouped dataframe:**  

For example,  

    # sum() demo using FrovedisGroupedDataframe instance 
    fdf1.groupby('Qualification').sum().show()
    
Output  

    Qualification   sum_Age sum_Score
    B.Tech          117     108
    Phd             116     131

It displays a frovedis dataframe containing numeric column(s) with newly computed sum of each groups.   

__Return Value__  
It returns a new frovedis DataFrame instance with the result of the specified aggregate functions.  

### 11. FrovedisGroupedDataFrame.var(ddof = 1.0)  

__Parameters__  
**_ddof_**: It accepts an integer parameter that specifies the delta degrees of freedom. (Default: 1.0)  

__Purpose__  
It computes the variance of groups, excluding missing values.  

**Currently, this method only displays result for dataframe having atleast one numeric column.**  

**Constructing frovedis DataFrame from a pandas DataFrame:**  

For example,  
    
    import pandas as pd
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
                
    # convert to pandas dataframe
    pdf1 = pd.DataFrame(peopleDF)

    # convert to frovedis dataframe
    fdf1 = fdf.DataFrame(pdf1)
    
    # displaying created frovedis dataframe
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

**To compute variance of groups in the grouped dataframe:**  

For example,  

    # var() demo using FrovedisGroupedDataframe instance and default ddof value
    fdf1.groupby('Qualification').var().show()
    
Output

    Qualification   var_Age  var_Score
    B.Tech          36.9166  183
    Phd             18       82.3333

It displays a frovedis dataframe with numeric column(s) containing the newly computed variance 
for each groups.  

Also, it excludes the missing value in **'Score'** column while computing variance of groups **'B.Tech'** 
and **'Phd'**.  

**Using ddof parameter to compute variance of each group:**  

For example,  

    # var() demo using ddof = 2 
    fdf1.groupby('Qualification').var(ddof = 2).show()
    
Output

    Qualification   var_Age var_Score
    B.Tech          55.375  366
    Phd             27      164.666

__Return Value__  
It returns a new frovedis DataFrame instance with the result of the specified aggregate functions.  

# SEE ALSO  

- **[Introduction to FrovedisGroupedDataFrame](./grouped_df_intro.md)**  
- **[Introduction to frovedis DataFrame](./df_intro.md)**  