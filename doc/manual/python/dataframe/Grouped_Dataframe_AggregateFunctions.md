% FrovedisGroupedDataFrame Aggregate Functions
  
# NAME

FrovedisGroupedDataFrame Aggregate Functions - aggregate operations performed on grouped dataframe are being illustrated here.  
  
## SYNOPSIS  
    
    frovedis.dataframe.grouped_df.FrovedisGroupedDataframe(df = None)  
  
## Public Member Functions  

    1. agg(func, \*args, \*\*kwargs)
    2. aggregate(func, \*args, \*\*kwargs)
    3. count(numeric_only = True)
    4. max(numeric_only = True, min_count = -1)
    5. mean(numeric_only = True)
    6. min(numeric_only = True, min_count = -1)
    7. sem(ddof = 1.0)
    8. size(numeric_only = True)
    9. sum(numeric_only = True, min_count = 0)
    10. var(ddof = 1.0)

## Detailed Description  
  
### 1. agg(func, \*args, \*\*kwargs)  

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

For example,  

    # agg() demo with func as a function string name 
    fdf1.groupby('Qualification').agg('min').show()

Output  

    Qualification   min_Age min_Score
    B.Tech          22      23
    Phd             24      34

It displays a frovedis dataframe containing numeric column(s) with newly computed minimum of each groups.  

For example,  
    
    # agg() demo with func as a dictionary 
    fdf1.groupby('Qualification').agg({'Age': ['max','min','mean'], 'Score': ['sum']}).show()

Output  

    Qualification   max_Age min_Age mean_Age  sum_Score
    B.Tech          36      22      29.25     108
    Phd             33      24      29        13

For example,  
    
    # agg() demo where func is a list of functions 
    fdf1.groupby('Qualification').agg(['min', 'max']).show()

Output  

    Qualification   min_Age max_Age min_Score  max_Score
    B.Tech          22      36      23         50
    Phd             24      33      34         52

__Return Value__  
It returns a new frovedis DataFrame instance with the result of the specified aggregate functions.  

### 2. aggregate(func, \*args, \*\*kwargs)  

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

For example,  

    # aggregate() demo using FrovedisGroupedDataframe instance and 'func' as a function string name 
    fdf1.groupby('Qualification').agg('max').show()
    
Output  

    Qualification   max_Age max_Score
    B.Tech          36      50
    Phd             33      52

It displays a frovedis dataframe containing numeric column(s) with newly computed minimum of each groups.   

For example,  
    
    # aggregate() demo using FrovedisGroupedDataframe instance and 'func' as a dictionary 
    fdf1.groupby('Qualification').agg({'Age': ['count'], 'Score': ['max','min']}).show()

Output  

    Qualification   count_Age  max_Score  min_Score
    B.Tech          4          50         23
    Phd             4          52         34

For example,  
    
    # aggregate() demo using FrovedisGroupedDataframe instance and 'func' is a list of functions 
    fdf1.groupby('Qualification').agg(['mean','sum']).show()  

Output  

    Qualification   mean_Age  sum_Age mean_Score  sum_Score
    B.Tech          29.25     117     36          108
    Phd             29        116     43.6666     131

__Return Value__  
It returns a new frovedis DataFrame instance with the result of the specified aggregate functions.  

### 3. count(numeric_only = True)  

__Parameters__  
**_numeric\_only_**: It accepts a boolean parameter that specifies whether or not to use only numeric 
column data as input. (Default: True)  

__Purpose__  
It computes count of groups, excluding the missing values.  

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

### 4. max(numeric_only = True, min_count = -1)  

__Parameters__  
**_numeric\_only_**: It accepts a boolean parameter that specifies whether or not to use only numeric 
columns for aggregation. (Default: True)  
**_min\_count_**: This is an unused parameter. (Default: -1)  

__Purpose__  
It computes the maximum of group values.  

The parameter: "min_count" is simply kept in to to make the interface uniform to the pandas GroupBy.max().  
This is not used anywhere within the frovedis implementation.  

**Currently, this method only displays result for dataframe having atleast one numeric column.**  

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

### 5. mean(numeric_only = True)  
 
__Parameters__  
**_numeric\_only_**: It accepts a boolean parameter that specifies whether or not to use only numeric 
column data as input. (Default: True)  

__Purpose__  
It computes mean of groups, excluding the missing values.  

**Currently, this method only displays result for dataframe having atleast one numeric column.**  

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

### 6. min(numeric_only = True, min_count = -1)  

__Parameters__  
**_numeric\_only_**: It accepts a boolean parameter that specifies whether or not to use only numeric 
columns for aggregation. (Default: True)  
**_min\_count_**: This is an unused parameter. (Default: -1)  
  
__Purpose__  
It computes the minimum of group values.  

The parameter: "min_count" is simply kept in to to make the interface uniform to the pandas GroupBy.min(). 
This is not used anywhere within the frovedis implementation.  

**Currently, this method only displays result for dataframe having atleast one numeric column.**  

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

### 7. sem(ddof = 1.0)  

__Parameters__  
**_ddof_**: It accepts an integer parameter that specifies the delta degrees of freedom. (Default: 1.0)  

__Purpose__  
It computes standard error of the mean of groups, excluding missing values.  

**Currently, this method only displays result for dataframe having atleast one numeric column.**  

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

For example,

    # sem() demo using ddof = 2 
    fdf1.groupby('Qualification').sem(ddof = 2).show()
    
Output  

    Qualification   sem_Age  sem_Score
    B.Tech          3.72071  11.0453
    Phd             2.59807  7.4087

__Return Value__  
It returns a new frovedis DataFrame instance with the result of the specified aggregate functions.  

### 8. size(numeric_only = True)  

__Parameters__  
**_numeric\_only_**: It accepts a boolean parameter that specifies whether or not to use only numeric 
column data as input. (Default: True)  

__Purpose__  
It computes group sizes, including the missing values.  

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

### 9. sum(numeric_only = True, min_count = 0)  

__Parameters__  
**_numeric\_only_**: It accepts a boolean parameter that specifies whether or not to use only numeric 
column data as input. (Default: True)  
**_min\_count_**: This is an unused parameter. (Default: 0)  

__Purpose__  
It computes the sum of group values.  

The parameter: "min_count" is simply kept in to to make the interface uniform to the pandas GroupBy.sum().  
This is not used anywhere within the frovedis implementation.  

**Currently, this method only displays result for dataframe having atleast one numeric column.**  

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

### 10. var(ddof = 1.0)  

__Parameters__  
**_ddof_**: It accepts an integer parameter that specifies the delta degrees of freedom. (Default: 1.0)  

__Purpose__  
It computes the variance of groups, excluding missing values.  

**Currently, this method only displays result for dataframe having atleast one numeric column.**  

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

- **[Introduction to FrovedisGroupedDataFrame](./Grouped_Dataframe_Introduction.md)**  
- **[Introduction to frovedis DataFrame](./DataFrame_Introduction.md)**  