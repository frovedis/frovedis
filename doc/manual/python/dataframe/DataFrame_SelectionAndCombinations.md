% DataFrame  Selection and Combinations Functions
  
# NAME
  
DataFrame Selection and Combinations Functions - the functions used for selection and combinations are illustrated here.  
  
## SYNOPSIS  
  
    frovedis.dataframe.df.DataFrame(df = None, is_series = False)  

## Public Member Functions  
    1. add_index(name)
    2. append(other, ignore_index = False, verify_integrity = False, sort = False)
    3. between(left, right, inclusive="both")
    4. drop(labels = None, axis = 0, index = None, columns = None, level = None, 
            inplace = False, errors = 'raise')
    5. drop_cols(targets, inplace = False)
    6. drop_duplicates(subset = None, keep = 'first', inplace = False, ignore_index = False)
    7. drop_rows(targets, inplace = False)
    8. filter(items = None, like = None, regex = None, axis = None)
    9. get_index_loc(value)
    10. head(n = 5)
    11. insert(loc, column, value, allow_duplicates = False)
    12. join(right, on, how = 'inner', lsuffix = '\_left', rsuffix = '\_right', 
             sort = False, join_type = 'bcast')
    13. merge(right, on = None, how = 'inner', left_on = None, right_on = None, 
              left_index = False, right_index = False, sort = False, suffixes = ('\_x', '\_y'), 
              copy = True, indicator = False, join_type = 'bcast')
    14. rename(columns, inplace = False)
    15. rename_index(new_name, inplace = False)
    16. reset_index(drop = False, inplace = False)
    17. set_index(keys, drop = True, append = False, inplace = False, verify_integrity = False)
    18. show()
    19. tail(n = 5)
    20. update_index(value, key = None, verify_integrity = False, inplace = False) 

## Detailed Description   
  
### 1. add_index(name)  

__Parameters__  
**_name_**: It accepts a string object as parameter which represents the name of the index label.  

__Purpose__  
It adds index column to the Frovedis DataFrame object. The index added is zero based i.e. [0, 1, ... n-1] where n is the number of rows.  

For example:

	# example to show use of add_index method
    
    import pandas as pd
    import frovedis.dataframe as fdf
    
    peopleDF = {
				'Ename' : ['Michael', 'Andy', 'Tanaka', 'Raul', 'Yuta'],
				'Age' : [29, 30, 27, 19, 31],
				'Country' : ['USA', 'England', 'Japan', 'France', 'Japan'],
				'isMale': [False, False, False, False, True]
			   }
	# create a pandas dataframe
    pd_df = pd.DataFrame(peopleDF)
    
	# create a frovedis dataFrame
    fd_df = fdf.DataFrame(pd_df)
	
    # display the frovedis dataframe
    fd_df.show()
    
    # display the frovedis dataframe when new index is added
    fd_df.add_index("SNo").show()
	
Output,    
    
    index   Ename   Age     Country isMale
	0       Michael 29      USA     0
	1       Andy    30      England 0
	2       Tanaka  27      Japan   0
	3       Raul    19      France  0
	4       Yuta    31      Japan   1
	
	SNo   index   Ename   Age     Country isMale
	0     0       Michael 29      USA     0
	1     1       Andy    30      England 0
	2     2       Tanaka  27      Japan   0
	3     3       Raul    19      France  0
	4     4       Yuta    31      Japan   1  

__Return Value__  
It returns a self reference.  
  
### 2. append(other, ignore_index = False, verify_integrity = False, sort = False)   

__Parameters__  
**_other_**: It accepts a Frovedis DataFrame instance or a Pandas DataFrame instance or a list of Frovedis DataFrame objects which are to be appended.  
**_ignore\_index_**: It accepts a boolean type parameter. If True, old index axis is ignored and a new index axis is added with values 0 to n - 1, where n is the number of rows in the DataFrame. (Default: False)  
**_verify\_integrity_**: It accepts a boolean type as parameter. If it is set to True, it checks 'index' label for duplicate entries before appending and when there are duplicate entries in the DataFrame, it doesn't append. Otherwise duplicate entries in the 'index' label will be appended. It will also append duplicate entries when 'ignore_index' = True. (Default: False)  
**_sort_**: It accepts a boolean type as parameter. It sorts the columns, if the columns of the given DataFrame and other DataFrame are not aligned. (Default: False)  
  
__Purpose__  
It is used to append entries of DataFrame(s) at the end of another DataFrame. The columns of other DataFrame object that are not in the calling DataFrame object are added as new columns.  

For example:  

	# example of using append method with default values
    
    import pandas as pd
    import frovedis.dataframe as fdf
    
    # creating a pandas dataframe
    pd_df1 = pd.DataFrame([[1, 2], [3, 4]], columns = list('AB'), index = ['x', 'y'])
	
    # create a frovedis dataframe from pandas dataframe
    fd_df1 = fdf.DataFrame(pd_df1)
    
    # display frovedis dataframe
	fd_df1.show()
    
	pd_df2 = pd.DataFrame([[5, 6], [7, 8]], columns = list('AB'), index = ['x', 'y'])
    fd_df2 = fdf.DataFrame(pd_df2)
    fd_df2.show()
    
    # appending fd_df2 to fd_df1
    fd_df1.append(fd_df2).show() 
    
Output,    
    
    index   A       B
	x       1       2
	y       3       4
	
	index   A       B
	x       5       6
	y       7       8
	
	index   A  		B
	x  		1  		2
	y  		3  		4
	x  		5  		6
	y  		7  		8 
    
For example:  

	# examples to show append method when there are no common columns
    
    import pandas as pd
    import frovedis.dataframe as fdf
    
    pd_df1 = pd.DataFrame([[1, 2], [3, 4]], columns = list('AB'), index = ['x', 'y'])
	fd_df1 = fdf.DataFrame(pd_df1)
	fd_df1.show()
    
	pd_df2 = pd.DataFrame([[5, 6], [7, 8]], columns = list('CD'), index = ['x', 'y'])
    fd_df2 = fdf.DataFrame(pd_df2)
    fd_df2.show()
    
    # appending fd_df2 to fd_df1
    fd_df1.append(fd_df2).show() 
    
Output,    
    
    index   A       B
    x       1       2
    y       3       4

    index   C       D
    x       5       6
    y       7       8

    index   A       B       D       C
    x       1       2       NULL    NULL
    y       3       4       NULL    NULL
    x       NULL    NULL    6       5
    y       NULL    NULL    8       7

For example:  

	# example of using append with parameter ignore_index
    
    import pandas as pd
    import frovedis.dataframe as fdf
    
    pd_df1 = pd.DataFrame([[1, 2], [3, 4]], columns = list('AB'), index = ['x', 'y'])
	fd_df1 = fdf.DataFrame(pd_df1)
	fd_df1.show()
    
	pd_df2 = pd.DataFrame([[5, 6], [7, 8]], columns = list('AB'), index = ['x', 'y'])
    fd_df2 = fdf.DataFrame(pd_df2)
    fd_df2.show()
	
    # With ignore_index set to True
	fd_df1.append(fd_df2, ignore_index = True).show()
    
Output,    
    
	index   A       B
	x       1       2
	y       3       4
	
	index   A       B
	x       5       6
	y       7       8
    
    index   A  		B
	0  		1  		2
	1  		3  		4
	2  		5  		6
	3  		7  		8
	
For example:  

	# example of using append with parameter verify_integrity
    
    import pandas as pd
    import frovedis.dataframe as fdf

	# With verify_integrity
	pd_df1 = pd.DataFrame([[1, 2], [3, 4]], columns=list('AB'), index=['x', 'y'])
	fd_df1 = fdf.DataFrame(pd_df1)
	fd_df1.show()
    
    pd_df2 = pd.DataFrame([[1, 2], [7, 8]], columns=list('AB'), index=['u', 'v'])    
	fd_df2 = fdf.DataFrame(pd_df2)
	fd_df2.show()
    
    # appending fd_df2 to fd_df1 with verify_integrity = True
    fd_df1.append(fd_df2, verify_integrity = True).show()
    
Output,

    index   A       B
	x       1       2
	y       3       4
	
	index   A       B
	u       1       2
	v       7       8
    
    index   A       B
    x       1       2
    y       3       4
    u       1       2
    v       7       8

For example:  

	# example of using append with paramter sort = True
    
    import pandas as pd
    import frovedis.dataframe as fdf

	# With sort = True
	pd_df1 = pd.DataFrame([[1, 2], [3, 4]], columns=list('YZ'), index=['x', 'y'])
	fd_df1 = fdf.DataFrame(pd_df1)
	fd_df1.show()
    
    pd_df2 = pd.DataFrame([[1, 2], [7, 8]], columns=list('AB'), index=['x', 'y'])    
	fd_df2 = fdf.DataFrame(pd_df2)
	fd_df2.show()
    
    # appending fd_df2 with fd_df1 with sort = True
    fd_df1.append(fd_df2, sort = True).show()
    
Output,

    index   Y       Z
    x       1       2
    y       3       4

    index   A       B
    x       1       2
    y       7       8

    index   A       B       Y       Z
    x       NULL    NULL    1       2
    y       NULL    NULL    3       4
    x       1       2       NULL    NULL
    y       7       8       NULL    NULL

__Return Value__  
It returns a new Frovedis DataFrame consisting of the rows of original DataFrame object and the rows of other DataFrame object.  

### 3. between(left, right, inclusive = "both")  

__Parameters__  
**_left_**: It accepts scalar values as parameter.  
**_right_**: It accepts scalar values as parameter.  
**_inclusive_**: It accepts string object as parameter tha specifies which boundaries to include, whether to set 
bounds as open or closed. (Default: 'both')  
- **'left'**: left boundary value is included.  
- **'right'**: right boundary value is included.  
- **'both'**: boundary values are included.  
- **'neither'**: boundary values are excluded.  


__Purpose__  
This method performs filtering of rows according to the specified bound over a single column at a time.  

**Currently, this medthod filters data for numeric column data only.**  

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

    index   Name    Age     City       Qualification   Score
    0       Jai     27      Nagpur     B.Tech          23
    1       Anuj    24      Kanpur     Phd             34
    2       Jai     22      Allahabad  B.Tech          35
    3       Princi  32      Kannuaj    Phd             45
    4       Gaurav  33      Allahabad  Phd             NULL
    5       Anuj    36      Kanpur     B.Tech          50
    6       Princi  27      Kanpur     Phd             52
    7       Abhi    32      Kanpur     B.Tech          NULL

For example,  

    # between() demo used with given column of dataframe
    # Also, both boundaries are included
    fdf1['Score'].between(40,50)
    
Unlike pandas, it returns an instance of dfoperator **(present in frovedis only)** which contains masked boolean data as result of between operation. In order to unmask the data, **to_mask_array()** must be used.

    fdf1['Score'].between(40,50).to_mask_array
    
Output  

    [False False False  True False  True False False]

This returns a boolean array containing True wherever the corresponding element is between the boundary values 'left' and 'right'. The missing values are treated as False.  

Also, this method can be used in the form of given expression below:  

For example,  
    
    # between() demo used with given column of dataframe
    # Also, both boundaries are included
    res = fdf1[fdf1['Score'].between(40,50)]
    print(res)

Output  

    index   Name    Age     City    Qualification   Score
    3       Princi  32      Kannuaj Phd     45
    5       Anuj    36      Kanpur  B.Tech  50
    
It can also be expressed as follows:  

For example,  

    print(fdf1[fdf1.Score.between(40,50)])

Output  

    index   Name    Age     City    Qualification   Score
    3       Princi  32      Kannuaj Phd     45
    5       Anuj    36      Kanpur  B.Tech  50

Now, when 'inclusive' parameter is provided, then,  

For example,  

    # between() demo and only left boundary included
    print(fdf1['Score'].between(40,50,inclusive = 'left').to_mask_array())
    
Output  

    [False False False  True False False False False]

For example,  

    # between() demo and only right boundary included
    print(fdf1['Score'].between(40,50,inclusive = 'right').to_mask_array())
    
Output  

    [False False False  True False  True False False]

For example,  

    # between() demo and both boundaries excluded
    print(fdf1['Score'].between(40,50,inclusive = 'neither').to_mask_array())
    
Output  

    [False False False  True False False False False]
    
__Return Value__  
It returns a dfoperator instance.  

### 4. drop(labels = None, axis = 0, index = None, columns = None, level = None, inplace = False, errors = 'raise')  

__Parameters__  
**_labels_**: It takes an integer or string type as argument. It represents column labels and integer represent index values of rows to be dropped. If any of the label is not found in the selected axis, it will raise an exception. (Default: None)  
When it is None (not specified explicitly), 'index' or 'columns' parameter must be provided. Otherwise it will drop specific rows of the DataFrame.  
**_axis_**: It accepts an integer type parameter where the value indicates the direction of drop operation according to below conditions:  
- 0 : The corresponding index labels will be dropped.  
- 1 : The corresponding columns labels will be dropped.  

**_index_**: It accepts an integer or string object as parameter. It is equivalent to dropping indices along the axis = 0. (Default: None)  
When it is None (not specified explicitly), 'columns' or 'labels' must be provided.  
**_columns_**: It accepts a string object or list of strings. It is equivalent to dropping columns along the axis = 1. (Default: None)  
When it is None (not specified explicitly), 'index' or 'labels' must be provided.  
**_level_**: This is an unsed parameter. (Default: None)   
**_inplace_**: It accepts a boolean as a parameter. It return a copy of DataFrame instance by default but when explicitly set to True, it performs operation on original DataFrame. (Default: False)  
**_errors_**: This is an unsed parameter. (Default: ‘raise’)  

__Purpose__  
It is used to drop specified labels from rows or columns.  
Rows or columns can be removed by specifying label names and corresponding axis, or by specifying index or column names.  

For example:

    # example of using drop with label and axis parameter
    
    import pandas as pd
    import numpy as np
    import frovedis.dataframe as fdf
    
    pd_df = pd.DataFrame(np.arange(12).reshape(3, 4),
                      columns = ['A', 'B', 'C', 'D'])
    
    fd_df = fdf.DataFrame(pd_df).
    # display frovedis dataframe
    fd_df.show()
    
    # Drop columns
    fd_df.drop(labels = ['B', 'C'], axis = 1).show()
    
Output,
    
    index   A       B       C       D
    0       0       1       2       3
    1       4       5       6       7
    2       8       9       10      11

    index   A       D
    0       0       3
    1       4       7
    2       8       11
    
For example:

    # example of using drop with columns parameter
    
    import pandas as pd
    import numpy as np
    import frovedis.dataframe as fdf
    
    pd_df = pd.DataFrame(np.arange(12).reshape(3, 4),
                      columns = ['A', 'B', 'C', 'D'])
    fd_df = fdf.DataFrame(pd_df)
    fd_df.show()   
    
    fd_df.drop(columns = ['B', 'C']).show()
    
Output,

    index   A       B       C       D
    0       0       1       2       3
    1       4       5       6       7
    2       8       9       10      11

    index   A       D
    0       0       3
    1       4       7
    2       8       11

For example:

    # example of using drop with label parameter and other default values

    import pandas as pd
    import numpy as np
    import frovedis.dataframe as fdf
    
    pd_df = pd.DataFrame(np.arange(12).reshape(3, 4),
                      columns = ['A', 'B', 'C', 'D'])
    fd_df = fdf.DataFrame(pd_df)
    fd_df.show()       
    
    # Drop a row by index
    fd_df.drop(labels = [0, 1]).show()
    
Output,

    index   A       B       C       D
    0       0       1       2       3
    1       4       5       6       7
    2       8       9       10      11

    index   A       B       C       D
    2       8       9       10      11

__Return Value__  
1. It returns a new of Frovedis DataFrame having remaining entries.  
2. It returns None when parameter 'inplace' = True.  

### 5. drop_cols(targets, inplace = False)

__Parameters__  
**_targets_**: It accepts a string or a list of strings as parameter. It is list of names of column labels to drop from the dataframe.  
**_inplace_**: It accepts a boolean as a parameter. It return a copy of DataFrame instance by default but when explicitly set to True, it performs operation on original DataFrame. (Default: False)  

__Purpose__  
It is used to drop specified columns.  

For example:

    # example of using drop_cols with inplace = True as parameter
    
    import pandas as pd
    import frovedis.dataframe as fdf
    
    peopleDF = {
                'Ename' : ['Michael', 'Andy', 'Tanaka', 'Raul', 'Yuta'],
                'Age' : [29, 30, 27, 19, 31],
                'Country' : ['USA', 'England', 'Japan', 'France', 'Japan'],
                'isMale': [False, False, False, False, True]
               }
    pd_df = pd.DataFrame(peopleDF)
    fd_df = fdf.DataFrame(pd_df)
    fd_df.show()
    
    # dropping 'Age' columns and modifying original dataframe object
    fd_df.drop_cols('Age', inplace = True)
    
    # display the frovedis dataframe
    fd_df.show()
    
Output,

    index   Ename   Age     Country isMale
    0       Michael 29      USA     0
    1       Andy    30      England 0
    2       Tanaka  27      Japan   0
    3       Raul    19      France  0
    4       Yuta    31      Japan   1

    index   Ename   Country isMale
    0       Michael USA     0
    1       Andy    England 0
    2       Tanaka  Japan   0
    3       Raul    France  0
    4       Yuta    Japan   1

For example:      
    
    # example to drop multiple columns using drop_cols
    
    import pandas as pd
    import frovedis.dataframe as fdf
    
    peopleDF = {
                'Ename' : ['Michael', 'Andy', 'Tanaka', 'Raul', 'Yuta'],
                'Age' : [29, 30, 27, 19, 31],
                'Country' : ['USA', 'England', 'Japan', 'France', 'Japan'],
                'isMale': [False, False, False, False, True]
               }
    pd_df = pd.DataFrame(peopleDF)
    fd_df = fdf.DataFrame(pd_df)
    
    # display frovedis dataframe
    fd_df.show()

    # drop 'Age' and 'Country' columns
    fd_df.drop_cols(['Age', 'Country']).show()
    
    # display original frovedis dataframe
    # No change in original dataframe as inplace is by default = False
    fd_df.show()
    
Output,

    index   Ename   Age     Country isMale
    0       Michael 29      USA     0
    1       Andy    30      England 0
    2       Tanaka  27      Japan   0
    3       Raul    19      France  0
    4       Yuta    31      Japan   1
    
    index   Ename   isMale
    0       Michael 0
    1       Andy    0
    2       Tanaka  0
    3       Raul    0
    4       Yuta    1
    
    index   Ename   Age     Country isMale
    0       Michael 29      USA     0
    1       Andy    30      England 0
    2       Tanaka  27      Japan   0
    3       Raul    19      France  0
    4       Yuta    31      Japan   1

__Return Value__  
1. It returns a new of Frovedis DataFrame having remaining entries.  
2. It returns None when parameter 'inplace' = True.  

### 6. drop_duplicates(subset = None, keep = 'first', inplace = False, ignore_index = False)

__Parameters__  
**_subset_**:  It accepts a string object or a list of strings which only consider certain columns for identifying duplicates. (Default: None)  
When it is None (not specified explicitly), it will consider all of the columns.  
**_keep_**: It accepts a string object which is used to determine which duplicates values to keep. (Default: 'first')  
1. **'first'** : Drop duplicates except for the first occurrence.  
2. **'last'** : Drop duplicates except for the last occurrence.  

**_inplace_**: It accepts a boolean as a parameter. It return a copy of DataFrame instance by default but when explicitly set to True, it performs operation on original DataFrame. (Default: False)  
**_ignore\_index_**: It accepts a boolean type parameter. If True, old index axis is ignored and a new index axis is added with values 0 to n - 1, where n is the number of rows in the DataFrame. (Default: False)  

__Purpose__  
It is used to remove duplicate rows.  

For example:

	# example to use drop_duplicates with default values
    
    import pandas as pd
    import frovedis.dataframe as fdf
    
    # Consider dataset containing ramen rating.
    pd_df = pd.DataFrame({
            'brand': ['Yum Yum', 'Yum Yum', 'Indomie', 'Indomie', 'Indomie'],
            'style': ['cup', 'cup', 'cup', 'pack', 'pack'],
            'rating': [4, 4, 3.5, 15, 5]})
	fd_df = fdf.DataFrame(pd_df)
	# display frovedis dataframe
    fd_df.show()
    
    # By default, it removes duplicate rows based on all columns.
	fd_df.drop_duplicates()
    
Output,

	index   brand   style   rating
	0       Yum Yum cup     4
	1       Yum Yum cup     4
	2       Indomie cup     3.5
	3       Indomie pack    15
	4       Indomie pack    5

    index   brand   style   rating
	0       Yum Yum cup     4
	2       Indomie cup     3.5
	3       Indomie pack    15
	4       Indomie pack    5
	
For example:

	# example to use drop_duplicates with parameter subset
    
    import pandas as pd
    import frovedis.dataframe as fdf
    
    # Consider dataset containing ramen rating.
    pd_df = pd.DataFrame({
            'brand': ['Yum Yum', 'Yum Yum', 'Indomie', 'Indomie', 'Indomie'],
            'style': ['cup', 'cup', 'cup', 'pack', 'pack'],
            'rating': [4, 4, 3.5, 15, 5]})
	fd_df = fdf.DataFrame(pd_df)
	fd_df.show()
    
	# To remove duplicates on specific column(s), use subset.
	fd_df.drop_duplicates(subset = ['brand']).show()

Output,    
    
    index   brand   style   rating
	0       Yum Yum cup     4
	1       Yum Yum cup     4
	2       Indomie cup     3.5
	3       Indomie pack    15
	4       Indomie pack    5
    
    index   brand   style   rating
	0       Yum Yum cup     4
	2       Indomie cup     3.5
	
For example:

	# example to use drop_duplicates with parameters subset and keep = 'last'
    
    import pandas as pd
    import frovedis.dataframe as fdf
    
    # Consider dataset containing ramen rating.
    pd_df = pd.DataFrame({
            'brand': ['Yum Yum', 'Yum Yum', 'Indomie', 'Indomie', 'Indomie'],
            'style': ['cup', 'cup', 'cup', 'pack', 'pack'],
            'rating': [4, 4, 3.5, 15, 5]})
	fd_df = fdf.DataFrame(pd_df)
    # display frovedis dataframe
	fd_df.show()
    
    # To remove duplicates and keep last occurrences, use keep.
	fd_df.drop_duplicates(subset = ['brand', 'style'], keep = 'last').show()
    
Output,    

    index   brand   style   rating
	0       Yum Yum cup     4
	1       Yum Yum cup     4
	2       Indomie cup     3.5
	3       Indomie pack    15
	4       Indomie pack    5
    
    index   brand   style   rating
	1       Yum Yum cup     4
	2       Indomie cup     3.5
	4       Indomie pack    5

__Return Value__  
1. It returns a new of Frovedis DataFrame having remaining entries.  
2. It returns None when parameter 'inplace' = True.  

### 7. drop_rows(targets, inplace = False)

__Parameters__  
**_targets_**: It accepts a integer or string type as parameter. It is the name of the indices to drop from the dataframe.  
**_inplace_**: It accepts a boolean as a parameter. It return a copy of DataFrame instance by default but when explicitly set to True, it performs operation on original DataFrame. (Default: False)  

__Purpose__  
It is used to drop specified rows.  

For example:

    # example to drop_rows with index = 2
    
    import pandas as pd
    import frovedis.dataframe as fdf
    
    peopleDF = {
                'Ename' : ['Michael', 'Andy', 'Tanaka', 'Raul', 'Yuta'],
                'Age' : [29, 30, 27, 19, 31],
                'Country' : ['USA', 'England', 'Japan', 'France', 'Japan'],
                'isMale': [False, False, False, False, True]
               }
    pd_df = pd.DataFrame(peopleDF)
    fd_df = fdf.DataFrame(pd_df)
    
    # display frovedis dataframe
    fd_df.show()
    
    # dropping a row when inplace is False
    fd_df.drop_rows(2).show()
    
    # No change in original dataframe
    fd_df.show()
    
Output,

    index   Ename   Age     Country isMale
    0       Michael 29      USA     0
    1       Andy    30      England 0
    2       Tanaka  27      Japan   0
    3       Raul    19      France  0
    4       Yuta    31      Japan   1

    index   Ename   Age     Country isMale
    0       Michael 29      USA     0
    1       Andy    30      England 0
    3       Raul    19      France  0
    4       Yuta    31      Japan   1
    
    index   Ename   Age     Country isMale
    0       Michael 29      USA     0
    1       Andy    30      England 0
    2       Tanaka  27      Japan   0
    3       Raul    19      France  0
    4       Yuta    31      Japan   1

For example:

    # example to use drop_rows with mulitple rows and inplace = True parameter
    
    import pandas as pd
    import frovedis.dataframe as fdf
    
    peopleDF = {
                'Ename' : ['Michael', 'Andy', 'Tanaka', 'Raul', 'Yuta'],
                'Age' : [29, 30, 27, 19, 31],
                'Country' : ['USA', 'England', 'Japan', 'France', 'Japan'],
                'isMale': [False, False, False, False, True]
               }
    pd_df = pd.DataFrame(peopleDF)
    fd_df = fdf.DataFrame(pd_df)
    fd_df.show()

    # dropping rows when inplace is True
    fd_df.drop_rows([2, 3], inplace = True)
    
    # display the frovedis dataframe
    fd_df.show()
    
Output,

    index   Ename   Age     Country isMale
    0       Michael 29      USA     0
    1       Andy    30      England 0
    2       Tanaka  27      Japan   0
    3       Raul    19      France  0
    4       Yuta    31      Japan   1
    
    index   Ename   Age     Country isMale
    0       Michael 29      USA     0
    1       Andy    30      England 0
    4       Yuta    31      Japan   1

__Return Value__  
1. It returns a new of Frovedis DataFrame having remaining entries.  
2. It returns None when parameter 'inplace' = True.  

### 8. filter(items = None, like = None, regex = None, axis = None)

__Parameters__  
**_items_**: It accepts a list of string as parameter. It filters only those labels which are mentioned. (Default: None)  
When it is None (not specified explicitly), 'like' or 'regex' must be provided.  
**_like_**: It accepts a string object parameter. It keeps the column labels if 'like in label == True'. (Default: None)   
When it is None (not specified explicitly), 'items' or 'regex' must be provided.  
**_regex_**: It accepts a regular expression as a string parameter. It keeps the column labels if 're.search(regex, label) == True'. (Default: None)  
When it is None (not specified explicitly), 'items' or 'like' must be provided.  
**_axis_**: It accepts an integer or string type parameter. It specifies the axis on which filter operation will be performed.  (Default: None)  
- 0 or 'index' : The corresponding index labels will be filtered. Currently this is not supported in Frovedis.  
- 1 or 'columns': The corresponding columns labels will be filtered.  

When it is None (not specified explicitly), it will filter along axis = 1 or 'columns'.  

__Purpose__  
It is used to filter the DataFrame according to the specified column labels.  

For example:

    # example to use filter with items parameter
    
    import pandas as pd
    import frovedis.dataframe as fdf
    
    pd_df = pd.DataFrame(np.array(([1, 2, 3], [4, 5, 6])),
                      index=['mouse', 'rabbit'],
                      columns=['one', 'two', 'three'])
    fd_df = fdf.DataFrame(pd_df)
    
    # display the frovedis dataframe
    fd_df.show()
    
    # select columns by name
    fd_df.filter(items = ['one', 'three']).show()
    
Output,

    index   one     two     three
    mouse   1       2       3
    rabbit  4       5       6

    index   one     three
    mouse   1       3
    rabbit  4       6

For example:

    # example to use filter with regex parameter along axis = 1
    
    import pandas as pd
    import frovedis.dataframe as fdf
    
    pd_df = pd.DataFrame(np.array(([1, 2, 3], [4, 5, 6])),
                      index=['mouse', 'rabbit'],
                      columns=['one', 'two', 'three'])
    fd_df = fdf.DataFrame(pd_df)
    fd_df.show()

    # select columns by regular expression
    # will display only those columns whose label ends with 'e'
    fd_df.filter(regex='e$', axis=1).show()
    
Output,

    index   one     two     three
    mouse   1       2       3
    rabbit  4       5       6
    
    index   one     three
    mouse   1       3
    rabbit  4       6

For example:

    # example to use filter with like parameter along axis = 1
    
    import pandas as pd
    import frovedis.dataframe as fdf
    
    pd_df = pd.DataFrame(np.array(([1, 2, 3], [4, 5, 6])),
                      index=['mouse', 'rabbit'],
                      columns=['one', 'two', 'three'])
    fd_df = fdf.DataFrame(pd_df)
    fd_df.show()

    # select column containing 'hre'
    fd_df.filter(like='hre', axis=1)
    
Output,

    index   one     two     three
    mouse   1       2       3
    rabbit  4       5       6
    
    index   three
    mouse   3
    rabbit  6

__Return Value__  
It returns a new Frovedis DataFrame with the column labels that matches the given conditions.  

### 9. get_index_loc(value)  

__Parameters__  
**_value_**: It accepts an integer or string parameter. It is the index value whose location is to be determined.  

__Purpose__  
It provides integer location, slice or boolean mask for requested label.  

For example:

    # example to use get_index_loc when there are unique index
    
    import pandas as pd
    import frovedis.dataframe as fdf
    
    peopleDF = {
                'Ename' : ['Michael', 'Andy', 'Tanaka', 'Raul', 'Yuta'],
                'Age' : [29, 30, 27, 19, 31],
                'Country' : ['USA', 'England', 'Japan', 'France', 'Japan'],
                'isMale': [False, False, False, False, True]
               }
    pd_df = pd.DataFrame(peopleDF)
    pd_df.index = ['a', 'b', 'c', 'd', 'e']
    fd_df = fdf.DataFrame(pd_df)
    
    # display frovedis dataframe
    fd_df.show()
    
    # getting index location of 'd' index value
    fd_df.get_index_loc('d').show()
    
Output,

    index   Ename   Age     Country isMale
    a       Michael 29      USA     0
    b       Andy    30      England 0
    c       Tanaka  27      Japan   0
    d       Raul    19      France  0
    e       Yuta    31      Japan   1

    3

For example:

    # example to use get_index_loc when there are duplicate index but consecutively
    
    import pandas as pd
    import frovedis.dataframe as fdf
    
    peopleDF = {
                'Ename' : ['Michael', 'Andy', 'Tanaka', 'Raul', 'Yuta'],
                'Age' : [29, 30, 27, 19, 31],
                'Country' : ['USA', 'England', 'Japan', 'France', 'Japan'],
                'isMale': [False, False, False, False, True]
               }
    pd_df = pd.DataFrame(peopleDF)

    pd_df.index = ['a', 'a', 'd', 'd', 'e']
    fd_df = fdf.DataFrame(pd_df)
    fd_df.show()
    
    # getting index location of 'd' index value
    fd_df.get_index_loc('d').show()
    
Output,

    index   Ename   Age     Country isMale
    a       Michael 29      USA     0
    a       Andy    30      England 0
    d       Tanaka  27      Japan   0
    d       Raul    19      France  0
    e       Yuta    31      Japan   1

    slice(2, 4, None)
    
For example:

    # example to use get_index_loc when there are duplicate index but randomly
    
    import pandas as pd
    import frovedis.dataframe as fdf
    
    peopleDF = {
                'Ename' : ['Michael', 'Andy', 'Tanaka', 'Raul', 'Yuta'],
                'Age' : [29, 30, 27, 19, 31],
                'Country' : ['USA', 'England', 'Japan', 'France', 'Japan'],
                'isMale': [False, False, False, False, True]
               }
    pd_df = pd.DataFrame(peopleDF)   
    
    pd_df.index = ['a', 'b', 'c', 'd', 'a']
    fd_df = fdf.DataFrame(pd_df)
    fd_df.show()
    
    # getting index location of 'a' index value
    fd_df.get_index_loc('a').show()
    
Output,

    index   Ename   Age     Country isMale
    a       Michael 29      USA     0
    b       Andy    30      England 0
    c       Tanaka  27      Japan   0
    d       Raul    19      France  0
    a       Yuta    31      Japan   1

    [ True False False False  True]

__Return Value__  
It returns the following values:  
1. integer: when there is a unique index  
2. slice: when there is a monotonic index i.e. repetitive values in index  
3. mask: it returns a list of boolean values otherwise  

### 10. head(n = 5)

__Parameters__  
**_n_**: It accepts an integer parameter which represents the number of rows to select. (Default: 5)  

__Purpose__   
This function returns the first n rows for the object based on position. It is useful for quickly testing if your object has the right type of data in it.  

For negative values of n, this function returns all rows except the last n rows, equivalent to **df[:-n]**.  

For example:  
 
    # example to use head with default value
    
    import pandas as pd
    import frovedis.dataframe as fdf
    
    # Creating the dataframe    
	pd_df = pd.DataFrame({'animal': ['alligator', 'bee', 'falcon', 'lion',
                   'monkey', 'parrot', 'shark', 'whale', 'zebra']})
	fd_df = fdf.DataFrame(pd_df)
    
    # display frovedis dataframe
    fd_df.show()
    
	# for default value of n
	fd_df.head().show()
    
Output,

	index   animal
    0       alligator
    1       bee
    2       falcon
    3       lion
    4       monkey
    5       parrot
    6       shark
    7       whale
    8       zebra
    
    index   animal
	0       alligator
	1       bee
	2       falcon
	3       lion
	4       monkey 

For example:  
 
    # example to use head with provided value
    
    import pandas as pd
    import frovedis.dataframe as fdf
    
    # Creating the dataframe    
	pd_df = fd.DataFrame({'animal': ['alligator', 'bee', 'falcon', 'lion',
                   'monkey', 'parrot', 'shark', 'whale', 'zebra']})
	fd_df = fdf.DataFrame(pd_df)
    
    fd_df.show()

	# for custom values of n = 2
	fd_df.head(2).show()
    
Output,

    index   animal
    0       alligator
    1       bee
    2       falcon
    3       lion
    4       monkey
    5       parrot
    6       shark
    7       whale
    8       zebra

	index   animal
	0       alligator
	1       bee
	
For example:  
 
    # example to use head with negative value
    
    import pandas as pd
    import frovedis.dataframe as fdf
    
    # Creating the dataframe    
	pd_df = fd.DataFrame({'animal': ['alligator', 'bee', 'falcon', 'lion',
                   'monkey', 'parrot', 'shark', 'whale', 'zebra']})
	fd_df = fdf.DataFrame(pd_df)
    
    fd_df.show()

    # for negative values of n = -3
	fd_df.head(-3).show()
    
Output,

    index   animal
    0       alligator
    1       bee
    2       falcon
    3       lion
    4       monkey
    5       parrot
    6       shark
    7       whale
    8       zebra

	index   animal
	0       alligator
	1       bee
	2       falcon
	3       lion
	4       monkey
	5       parrot

__Return Value__  
1. It n is positive integer, it returns a new DataFrame with the first n rows.  
2. If n is negative integer, it returns a new DataFrame with all rows except last n rows.  

### 11. insert(loc, column, value, allow_duplicates = False)

__Parameters__  
**_loc_**: It accepts an integer as parameter which represents the Insertion index. It must be in range 0 to n-1 where n is number of columns in DataFrame.  
**_column_**: It accepts a string object as parameter. It is the label of the inserted column.  
**_value_**: It accepts an integer or a pandas-Series instance or python ndarray as parameter. These are the values to be inserted in the specified 'column'.  
**_allow\_duplicates_**: It accepts a boolean value as parameter. Currently, Frovedis does not support duplicate column names. (Default: False)  

__Purpose__  
It is used to insert column into DataFrame at specified location.  

For example:

    # example to use insert
    
    import pandas as pd
    import frovedis.dataframe as fdf
    
    pd_df = pd.DataFrame({'col1': [1, 2], 'col2': [3, 4]})
    fd_df = fdf.DataFrame(pd_df)
    fd_df.show()
    
    # inserting "newcol" as position 1 with values [99, 99]
    fd_df.insert(1, "newcol", [99, 99])
    
    # display frovedis dataframe after insertion
    fd_df.show()
    
Output,

    index   col1    col2
    0       1       3
    1       2       4

    index   col1    newcol  col2
    0       1       99      3
    1       2       99      4

For example:

    # example to insert a Series object into frovedis object
    
    import pandas as pd
    import frovedis.dataframe as fdf
    
    pd_df = pd.DataFrame({'col1': [1, 2], 'col2': [3, 4]})
    fd_df = fdf.DataFrame(pd_df)
    
    # display frovedis dataframe object
    fd_df.show()    
    
    # inserting "col0" at 0 position having values [5,6]
    fd_df.insert(0, "col0", pd.Series([5, 6])).show()
    
Output,

    index   col1    col2
    0       1       3
    1       2       4

    index   col0    col1    col2
    0       5       1       3
    1       6       2       4

__Return Value__  
It returns a self reference.  


### 12. join(right, on, how = 'inner', lsuffix = '\_left', rsuffix = '\_right', sort = False, join_type = 'bcast')  

__Parameters__  
**_right_**: It accepts a Frovedis DataFrame or a pandas DataFrame or a list of DataFrame objects as parameter. Index should be similar to one of the columns in this one. If a Series is passed, its name attribute must be set, and that will be used as the column name in the resulting joined DataFrame.  
**_on_**: It accepts a string object or a list of strings as parameter. It is the column or index name(s) in the caller to join on the index in other, otherwise joins index-on-index. This parameter must be provided. It can not be None.  
**_how_**: It accepts a string object as parameter that specifies how to handle the operation of the two DataFrames. (Default: 'inner')  
1. **'left'**: form union of calling DataFrame’s index (or column if 'on' is specified) with other DataFrame’s index, and sort it lexicographically.  
2. **'inner'**: form intersection of calling DataFrame’s index (or column if 'on' is specified) with other DataFrame’s index, preserving the order of the calling’s one.  

**_lsuffix_**: It accepts a string object as parameter. It adds the suffix to left DataFrame’s overlapping columns. (Default: '\_left')  
**_rsuffix_**: It accepts a string object as parameter. It adds the suffix to right DataFrame’s overlapping columns. (Default: '\_right')  
**_sort_**: It accepts a boolean type value. It orders resultant DataFrame lexicographically by the join key. If False, the order of the join key depends on the join type ('how' keyword). (Default: False)  
**_join\_type_**: It accepts a string type object as parameter. It represents the type of join to be used internally. It can be specified as 'bcast' for broadcast join or 'hash' for hash join. (Default: 'bcast')  

__Purpose__  
Join columns of another DataFrame.  

Join columns with other DataFrame either on index or on a key column. Efficiently join multiple DataFrame objects by index at once by passing a list.  

**Note:- Parameters 'on', 'lsuffix', and 'rsuffix' are not supported when passing a list of DataFrame objects.**  

For example:

    # example to use join with parameters lsuffix and rsuffix
    
    import pandas as pd
    import frovedis.dataframe as fdf
    
    pd_df1 = pd.DataFrame({'key': ['K0', 'K1', 'K2', 'K3', 'K4', 'K5'],
                           'A': ['A0', 'A1', 'A2', 'A3', 'A4', 'A5']})
    fd_df1 = fdf.DataFrame(pd_df1)
    # display frovedis dataframe
    fd_df1.show()
    
    pd_df2 = pd.DataFrame({'key': ['K0', 'K1', 'K2'],
                          'B': ['B0', 'B1', 'B2']})
    fd_df2 = fdf.DataFrame(pd_df2)
    # display frovedis dataframe
    fd_df2.show()
    
    # Join DataFrames using their indexes
    fd_df1.join(fd_df2, 'index', lsuffix = '_caller', rsuffix = '_other').show()
    
Output,

    index   key     A
    0       K0      A0
    1       K1      A1
    2       K2      A2
    3       K3      A3
    4       K4      A4
    5       K5      A5

    index   key     B
    0       K0      B0
    1       K1      B1
    2       K2      B2

    index   key_caller      A       key_other       B
    0       K0              A0      K0              B0
    1       K1              A1      K1              B1
    2       K2              A2      K2              B2

For example:

    # example to use join with default values and on = 'key'
    
    import pandas as pd
    import frovedis.dataframe as fdf
    
    pd_df1 = pd.DataFrame({'key': ['K0', 'K1', 'K2', 'K3', 'K4', 'K5'],
                           'A': ['A0', 'A1', 'A2', 'A3', 'A4', 'A5']})
    fd_df1 = fdf.DataFrame(pd_df1)
    # display frovedis dataframe
    fd_df1.show()
    
    pd_df2 = pd.DataFrame({'key': ['K0', 'K1', 'K2'],
                          'B': ['B0', 'B1', 'B2']})
    fd_df2 = fdf.DataFrame(pd_df2)
    # display frovedis dataframe
    fd_df2.show()   
    
    # join using the 'key' columns, 'key' to be available in both fd_df1 and fd_df2
    fd_df1.join(fd_df2, 'key').show()
    
Output,

    index   key     A
    0       K0      A0
    1       K1      A1
    2       K2      A2
    3       K3      A3
    4       K4      A4
    5       K5      A5

    index   key     B
    0       K0      B0
    1       K1      B1
    2       K2      B2

    index   key     A       B
    0       K0      A0      B0
    1       K1      A1      B1
    2       K2      A2      B2
    
For example:

    # example of join method with parameter how = 'left'
    
    import pandas as pd
    import frovedis.dataframe as fdf
    
    pd_df1 = pd.DataFrame({'key': ['K0', 'K1', 'K2', 'K3', 'K4', 'K5'],
                           'A': ['A0', 'A1', 'A2', 'A3', 'A4', 'A5']})
    fd_df1 = fdf.DataFrame(pd_df1)
    fd_df1.show()
    
    pd_df2 = pd.DataFrame({'key': ['K0', 'K1', 'K2'],
                          'B': ['B0', 'B1', 'B2']})
    fd_df2 = fdf.DataFrame(pd_df2)
    fd_df2.show()   
    
    # left join on 'key' column and display
    fd_df1.join(fd_df2, 'key', how = 'left').show()
    
Output,

    index   key     A
    0       K0      A0
    1       K1      A1
    2       K2      A2
    3       K3      A3
    4       K4      A4
    5       K5      A5

    index   key     B
    0       K0      B0
    1       K1      B1
    2       K2      B2

    index   key     A       B
    0       K0      A0      B0
    1       K1      A1      B1
    2       K2      A2      B2
    3       K3      A3      NULL
    4       K4      A4      NULL
    5       K5      A5      NULL

__Return Value__  
It returns a new Frovedis DataFrame containing columns from both the DataFrame instances.  

### 13. merge(right, on = None, how = 'inner', left_on = None, right_on = None, left_index = False, right_index = False, sort = False, suffixes = ('\_x', '\_y'), copy = True, indicator = False, join_type = 'bcast')  

__Parameters__  
**_right_**: It accepts a Frovedis DataFrame or a pandas DataFrame or a list of Frovedis DataFrame objects as parameter. Index should be similar to one of the columns in this one. If a Series is passed, its name attribute must be set, and that will be used as the column name in the resulting joined DataFrame.  
**_on_**: It accepts a string object or a list of strings as parameter. It is the column or index level names to join on. These must be present in both DataFrames. (Default: None)  
When it is None and not merging on indexes then this defaults to the intersection of the columns in both DataFrames.  
**_how_**: It accepts a string object as parameter. It informs the type of merge operation on the two objects. (Default: 'inner')  
1. **'left'**: form union of calling DataFrame’s index (or column if 'on' is specified) with other’s index and sort it lexicographically.  
2. **'inner'**: form intersection of calling DataFrame’s index (or column if 'on' is specified) with other’s index, preserving the order of the  calling’s one.  

**_left\_on_**: It accepts a string object or a list of strings as parameter. It represents column names to join on in the left DataFrame. It can also be an array or list of arrays of the length of the left DataFrame. These arrays are treated as if they are columns. (Default: None)  
**_right\_on_**: It accepts a string object or a list of strings as parameter. It represents column names to join on in the right DataFrame. It can also be an array or list of arrays of the length of the right DataFrame. These arrays are treated as if they are columns. (Default: None)  
**_left\_index_**: It accepts a boolean value as parameter. It is used to specify whether to use the index from the left DataFrame as the join key. (Default: False)  

**Note:- Either parameter 'left_on' or 'left_index' can be used but not combination of both.**  

**_right\_index_**: It accepts a boolean value as parameter. It is used to specify whether to use the index from the right DataFrame as the join key. (Default: False)  

**Note:- Either parameter 'right_on' or 'right_index' can be used but not combination of both.**  

**_sort_**: It accepts a boolean value. When this is explicitly set to True, it sorts the join keys lexicographically in the resultant DataFrame. When it is False, the order of the join keys depends on the join type ('how' parameter). (Default: False)  
**_suffixes_**: It accepts a list like (list or tuple) object of strings of length two as parameter. It indicates the suffix to be added to the overlapping column names in left and right respectively. Need to explicitly pass a value of None instead of a string to indicate that the column name from left or right should be left as-it is, with no suffix. At least one of the values must not be None. (Default: (“\_x”, “\_y”))  

**Note:- During merging two DataFrames, the overlapping column names should be different. For example: suffixes = (False, False), then the overlapping columns would have the same name so merging operation will fail.  
Also when there is no overlapping column, then this parameter is ignored automatically.**  

**_copy_**: It is an unused parameter. (Default: True)  
**_indicator_**: It is an unused parameter. (Default: False)  
**_join\_type_**: It accepts a string type object as parameter. It represents the type of join to be used internally. It can be specified as 'bcast' for broadcast join or 'hash' for hash join. (Default: 'bcast')  

__Purpose__  
It is a utility to merge DataFrame objects with a database-style join.  

The join is done on columns or indexes. If joining columns on columns, the DataFrame indexes will be ignored. Otherwise if joining indexes on indexes or indexes on a column or columns, the index will be passed on.  

For example:

    # example to merge with parameters left_on and right_on
    
    import pandas as pd
    import frovedis.dataframe as fdf
    
    pd_df1 = pd.DataFrame({'lkey': ['foo', 'bar', 'baz', 'foo'],
                        'value': [1, 2, 3, 5]})
    pd_df2 = pd.DataFrame({'rkey': ['foo', 'bar', 'baz', 'foo'],
                        'value': [5, 6, 7, 8]})
    fd_df1 = fdf.DataFrame(pd_df1)
    # display frovedis dataframe
    fd_df1.show()
    
    fd_df2 = fdf.DataFrame(pd_df2)
    # display frovedis dataframe
    fd_df2.show()
    
    # Merge fd_df1 and fd_df2 on the lkey and rkey columns.
    # The value columns have the default suffixes, _x and _y, appended.
    fd_df1.merge(fd_df2, left_on = 'lkey', right_on = 'rkey').show()
    
Output,

    index   lkey    value
    0       foo     1
    1       bar     2
    2       baz     3
    3       foo     5

    index   rkey    value
    0       foo     5
    1       bar     6
    2       baz     7
    3       foo     8

    index   lkey    value_x rkey    value_y
    0       foo     1       foo     8
    1       foo     1       foo     5
    2       bar     2       bar     6
    3       baz     3       baz     7
    4       foo     5       foo     8
    5       foo     5       foo     5

For example:

    # example to merge with parameters left_on , right_on and suffixes
    
    import pandas as pd
    import frovedis.dataframe as fdf
    
    pd_df1 = pd.DataFrame({'lkey': ['foo', 'bar', 'baz', 'foo'],
                        'value': [1, 2, 3, 5]})
    pd_df2 = pd.DataFrame({'rkey': ['foo', 'bar', 'baz', 'foo'],
                        'value': [5, 6, 7, 8]})
    fd_df1 = fdf.DataFrame(pd_df1)
    fd_df1.show()
    
    fd_df2 = fdf.DataFrame(pd_df2)
    fd_df2.show()     
    
    # Merge DataFrames fd_df1 and fd_df2 with specified left and right suffixes 
    # appended to any overlapping columns
    fd_df1.merge(fd_df2, left_on = 'lkey', right_on = 'rkey', suffixes = ('_left', '_right')).show()
    
Output,

    index   lkey    value
    0       foo     1
    1       bar     2
    2       baz     3
    3       foo     5

    index   rkey    value
    0       foo     5
    1       bar     6
    2       baz     7
    3       foo     8

    index   lkey    value_left      rkey    value_right
    0       foo     1               foo     8
    1       foo     1               foo     5
    2       bar     2               bar     6
    3       baz     3               baz     7
    4       foo     5               foo     8
    5       foo     5               foo     5
    
For example:

    # example to merge when column labels are different with parameter left_on and right_on
    
    import pandas as pd
    import frovedis.dataframe as fdf
    
    pd_df1 = pd.DataFrame({'lkey': ['foo', 'bar', 'baz', 'foo'],
                        'left_value': [1, 2, 3, 5]})
    pd_df2 = pd.DataFrame({'rkey': ['foo', 'bar', 'baz', 'foo'],
                        'right_value': [5, 6, 7, 8]})
    fd_df1 = fdf.DataFrame(pd_df1)
    fd_df1.show()
    
    fd_df2 = fdf.DataFrame(pd_df2)
    fd_df2.show()     
    
    # Merge DataFrames fd_df1 and fd_df2 with same suffixes on 'lkey' and 'rkey'
    fd_df1.merge(fd_df2, left_on = 'lkey', right_on = 'rkey', suffixes = ('_test', '_test')).show()
    
Output,

    index   lkey    left_value
    0       foo     1
    1       bar     2
    2       baz     3
    3       foo     5

    index   rkey    right_value
    0       foo     5
    1       bar     6
    2       baz     7
    3       foo     8

    index   lkey    left_value      rkey    right_value
    0       foo     1               foo     8
    1       foo     1               foo     5
    2       bar     2               bar     6
    3       baz     3               baz     7
    4       foo     5               foo     8
    5       foo     5               foo     5

**Note:-** In above example, suffix is ignored as merging column labels are different. To have suffix, the column name must be same in both DataFrames but then the suffixes must be different.  

For example:

    # example to use merge with parameters left_index and right_index
    
    import pandas as pd
    import frovedis.dataframe as fdf
    
    pd_df1 = pd.DataFrame({'lkey': ['foo', 'bar', 'baz', 'foo'],
                        'value': [1, 2, 3, 5]})
    pd_df2 = pd.DataFrame({'rkey': ['foo', 'bar', 'baz', 'foo'],
                                            'value': [5, 6, 7, 8]})
    fd_df1 = fdf.DataFrame(pd_df1)

    # update index values of fd_df1
    fd_df1.update_index(['a','b', 'c', 'd'], key = 'index', inplace = True)
    
    # display frovedis dataframe
    fd_df1.show()

    fd_df2 = fdf.DataFrame(pd_df2)

    # update index values of fd_df2
    fd_df2.update_index(['a','b', 'c', 'd'], key = 'index', inplace = True)
    
    # display frovedis dataframe
    fd_df2.show()

    # Merge DataFrames fd_df1 and fd_df2 with left_index and right_index = True
    fd_df1.merge(fd_df2, left_index = True, right_index = True).show()
    
Output,

    index   lkey    value
    a       foo     1
    b       bar     2
    c       baz     3
    d       foo     5

    index   rkey    value
    a       foo     5
    b       bar     6
    c       baz     7
    d       foo     8

    index   lkey    value_x rkey    value_y
    a       foo     1       foo     5
    b       bar     2       bar     6
    c       baz     3       baz     7
    d       foo     5       foo     8

For example:

    # example to use merge with parameters how = 'inner' and 'on'
    
    import pandas as pd
    import frovedis.dataframe as fdf
    
    pd_df1 = pd.DataFrame({'a': ['foo', 'bar'], 'b': [1, 2]})
    pd_df2 = pd.DataFrame({'a': ['foo', 'baz'], 'c': [3, 4]})
    fd_df1 = fdf.DataFrame(pd_df1)
    fd_df2 = fdf.DataFrame(pd_df2)
    
    # display forvedis dataframes
    fd_df1.show()
    fd_df2.show()
    
    # merge using 'how' = inner and 'on' parameter
    fd_df1.merge(fd_df2, how = 'inner', on = 'a').show()
    
Output,

    index   a       b
    0       foo     1
    1       bar     2

    index   a       c
    0       foo     3
    1       baz     4

    index   a       b       c
    0       foo     1       3

For example:

    # example to use merge with how = 'left' and 'on' parameters

    import pandas as pd
    import frovedis.dataframe as fdf
    
    pd_df = pd.DataFrame({'a': ['foo', 'bar'], 'b': [1, 2]})
    pd_df2 = pd.DataFrame({'a': ['foo', 'baz'], 'c': [3, 4]})
    fd_df1 = fdf.DataFrame(pd_df)
    fd_df2 = fdf.DataFrame(pd_df2)
    
    # display forvedis dataframes
    fd_df1.show()  
    fd_df2.show()    
        
    # merge using 'how' = left and 'on' parameter
    fd_df1.merge(fd_df2, how = 'left', on = 'a').show()
    
Output,

    index   a       b
    0       foo     1
    1       bar     2

    index   a       c
    0       foo     3
    1       baz     4
    
    index   a       b       c
    0       foo     1       3
    1       bar     2       NULL

__Return Value__  
It returns a new Frovedis DataFrame with the merged entries of the two DataFrame instances.  

### 14. rename(columns, inplace = False)  

__Parameters__  
**_columns_**: It accepts a dictionary object as parameter. It contains the key as the name of the labels to be renamed and values as the final names.  
**_inplace._**: It accepts a boolean object as parameter which specify whether to modify the original DataFrame object or to return a copy. When it is set to True then the original DataFrame object is modified.  (Default: False)  

__Purpose__  
It is used to set the name of the columns.  

For example:

    # example to use rename on multiple columns
    
    import pandas as pd
    import frovedis.dataframe as fdf
    
    pd_df = pd.DataFrame({"A": [1, 2, 3], "B": [4, 5, 6]})
    fd_df = fdf.DataFrame(pd_df)
    fd_df.show()
    
    # rename columns 'A' and 'B' to 'a' and 'c' respectively
    fd_df.rename(columns={"A": "a", "B": "c"}).show()
    
Output,

    index   A       B
    0       1       4
    1       2       5
    2       3       6

    index   a       c
    0       1       4
    1       2       5
    2       3       6

For example:

    # example to use rename with inplace parameter
    
    import pandas as pd
    import frovedis.dataframe as fdf
    
    pd_df = pd.DataFrame({"A": [1, 2, 3], "B": [4, 5, 6]})
    fd_df = fdf.DataFrame(pd_df)
    
    fd_df.show()

    # Rename with implace = True
    fd_df.rename(columns={"A": "X", "B": "Y"}, inplace = True)
    
    fd_df.show()
    
Output,

    index   A       B
    0       1       4
    1       2       5
    2       3       6
    
    index   X       Y
    0       1       4
    1       2       5
    2       3       6

__Return Value__  
1. It returns a new Frovedis DataFrame with the updated label name for the specified columns.  
2. It returns None when 'inplace' parameter is set to True.  


### 15. rename_index(new_name, inplace = False)  

__Parameters__  
**_new\_name_**: It accepts a string object as parameter. It renames the index for which the value is provided.  
**_inplace_**: It accepts a boolean values as parameter. When it is explicitly set to True, it modifies the original object directly instead of creating a copy of DataFrame object. (Default: False)  

__Purpose__   
It is used to rename the index label.  

For example:

    # example to use rename_index
    
    import pandas as pd
    import frovedis.dataframe as fdf
    
    peopleDF = {
                'Ename' : ['Michael', 'Andy', 'Tanaka', 'Raul', 'Yuta'],
                'Age' : [29, 30, 27, 19, 31],
                'Country' : ['USA', 'England', 'Japan', 'France', 'Japan'],
                'isMale': [False, False, False, False, True]
               }
    pd_df = pd.DataFrame(peopleDF)
    fd_df = fdf.DataFrame(pd_df)
    fd_df.show()
    
    # rename index to "SNo"
    fd_df.rename_index("SNo").show()
    
Output,

    index   Ename   Age     Country isMale
    0       Michael 29      USA     0
    1       Andy    30      England 0
    2       Tanaka  27      Japan   0
    3       Raul    19      France  0
    4       Yuta    31      Japan   1

    SNo     Ename   Age     Country isMale
    0       Michael 29      USA     0
    1       Andy    30      England 0
    2       Tanaka  27      Japan   0
    3       Raul    19      France  0
    4       Yuta    31      Japan   1

__Return Value__  
1. It returns a new Frovedis DataFrame with the updated label for the Index label.  
2. It returns None when inplace parameter is set to True.  


### 16. reset_index(drop = False, inplace = False)  

__Parameters__  
**_drop_**: It accepts a boolean value as parameter. Do not try to insert index into dataframe columns. This resets the index to the default integer index. (Default: False)  
**_inplace_**: It accepts a boolean values as parameter. When it is explicitly set to True, it modifies the original object directly instead of creating a copy of DataFrame object. (Default: False)  

__Purpose__   
It is used to reset the Index label of the DataFrame. A new Index label is inserted with default integer values from 0 to n-1 where n is the number of rows.  

**Note:- MultiIndex is not supported by Frovedis DataFrame.**  

For example:

    # example to use reset_index
    
    import pandas as pd
    import frovedis.dataframe as fdf
    
    peopleDF = {
                'Ename' : ['Michael', 'Andy', 'Tanaka', 'Raul', 'Yuta'],
                'Age' : [29, 30, 27, 19, 31],
                'Country' : ['USA', 'England', 'Japan', 'France', 'Japan'],
                'isMale': [False, False, False, False, True]
               }
    pd_df = pd.DataFrame(peopleDF)
   
    # updating the index values of pandas dataframe
    pd_df.index = ['a', 'b', 'c', 'd', 'a']
    
    # creating frovedis dataframe from pandas dataframe
    fd_df = fdf.DataFrame(pd_df)
    
    # display the frovedid dataframe
    fd_df.show()
    
    # When we reset the index, the old index is added as a column, and a new sequential index is used:
    fd_df.reset_index().show
    
Output,

    index   Ename   Age     Country isMale
    a       Michael 29      USA     0
    b       Andy    30      England 0
    c       Tanaka  27      Japan   0
    d       Raul    19      France  0
    a       Yuta    31      Japan   1

    label_0 index_col       Ename   Age     Country isMale
    0       a               Michael 29      USA     0
    1       b               Andy    30      England 0
    2       c               Tanaka  27      Japan   0
    3       d               Raul    19      France  0
    4       a               Yuta    31      Japan   1

For example:

    # example to use reset_index with drop parameter
    
    import pandas as pd
    import frovedis.dataframe as fdf
    
    peopleDF = {
                'Ename' : ['Michael', 'Andy', 'Tanaka', 'Raul', 'Yuta'],
                'Age' : [29, 30, 27, 19, 31],
                'Country' : ['USA', 'England', 'Japan', 'France', 'Japan'],
                'isMale': [False, False, False, False, True]
               }
    pd_df = pd.DataFrame(peopleDF)
    pd_df.index = ['a', 'b', 'c', 'd', 'a']
    fd_df = fdf.DataFrame(pd_df)
    
    fd_df.show()    
       
    # We can use the drop parameter to avoid the old index being added as a column:
    fd_df.reset_index(drop = True).show()
    
Output,

    index   Ename   Age     Country isMale
    a       Michael 29      USA     0
    b       Andy    30      England 0
    c       Tanaka  27      Japan   0
    d       Raul    19      France  0
    a       Yuta    31      Japan   1
    
    index   Ename   Age     Country isMale
    0       Michael 29      USA     0
    1       Andy    30      England 0
    2       Tanaka  27      Japan   0
    3       Raul    19      France  0
    4       Yuta    31      Japan   1

__Return Value__   
1. It returns a new Frovedis DataFrame with the default sequence in Index label.  
2. It returns None if 'inplace' parameter is set to True.  
  
### 17. set_index(keys, drop = True, append = False, inplace = False, verify_integrity = False)  

__Parameters__  
**_keys_**: It accepts a string object as parameter. This parameter can be a single column key.  
**_drop_**: It accepts a boolean value as parameter. When it is set to True, it will remove the column which is selected as new index. (Default: True)  
Currently, Frovedis doesn't support drop = False.  
**_append_**: It accepts a boolean value as parameter. It will decide whether to append columns to existing index. (Default: False)  
Currently, Frovedis doesn't support append = True.  
**_inplace_**: It accepts a boolean values as parameter which is when explicitly set to True, it modifies the original object directly instead of creating a copy of DataFrame object. (Default: False)  
**_verify\_integrity_**: It accepts a boolean value as parameter. When it is set to True, it checks the new index for duplicates. Performance of this method will be better when it is set to False. (Default: False)  

__Purpose__  
It is used to set the Frovedis DataFrame index using existing columns. The index will replace the existing index.  

**Note:- Frovedis DataFrame does not support Multi Index.**  

For example:  

    # example to use set_index
    
    import pandas as pd
    import frovedis.dataframe as fdf
    
    pd_df = pd.DataFrame({'month': [1, 4, 1, 10],
                       'year': [2012, 2014, 2013, 2014],
                       'sale': [55, 40, 84, 31]})
    fd_df = fdf.DataFrame(pd_df)
    
    # display frovedis dataframe
    fd_df.show()
    
    # Set the index to become the ‘month’ column:
    fd_df.set_index('month').show()
    
Output,

    index   month   year    sale
    0       1       2012    55
    1       4       2014    40
    2       1       2013    84
    3       10      2014    31

    month   year    sale
    1       2012    55
    4       2014    40
    1       2013    84
    10      2014    31
    
For example:  

    # example to use set_index with verify_integrity parameter
    
    import pandas as pd
    import frovedis.dataframe as fdf
    
    pd_df = pd.DataFrame({'month': [1, 4, 7, 10],
                       'year': [2012, 2014, 2013, 2014],
                       'sale': [55, 40, 84, 31]})
    fd_df = fdf.DataFrame(pd_df)
    fd_df.show()
    
    # Set the index to become the ‘month’ column:
    fd_df.set_index('month', verify_integrity = True).show()
    
Output,

    index   month   year    sale
    0       1       2012    55
    1       4       2014    40
    2       7       2013    84
    3       10      2014    31

    month   year    sale
    1       2012    55
    4       2014    40
    7       2013    84
    10      2014    31

**Note:-** In above example, the column which is being selected as index must have unique values.  

__Return Value__  
1. It returns a new Frovedis DataFrame where the Index column label is replace with specified column label.  
2. It returns None when 'inplace' parameter is set to True.  


### 18. show()  

__Purpose__  
This method is used to display the Frovedis DataFrame on the console. It can display full dataframe or some selected columns of the DataFrame (single or multi-column).  
It can be used either with any method which returns a Frovedis DataFrame object type compatible with string type.  
  
For example:
 
    # example to use show with various combinations
    
    import pandas as pd
    import frovedis.dataframe as fdf
    
    # Creating the dataframe    
	peopleDF = {
            'Ename' : ['Michael', 'Andy', 'Tanaka', 'Raul', 'Yuta'], 
            'Age' : [29, 30, 27, 19, 31],
            'Country' : ['USA', 'England', 'Japan', 'France', 'Japan'],
            'isMale': [False, False, False, False, True]
           }
	pdf = pd.DataFrame(peopleDF)
	fd_df = fdf.DataFrame(pdf)
	
    # display frovedis dataframe
    print("Displaying complete frovedis dataframe")
    fd_df.show() 
    
    # Selecting single column
    print("Displaying frovedis dataframe with just Ename column")
    fd_df["Ename"].show()         # single column
    
    # Selecting multiple columns
    print("Displaying frovedis dataframe with Ename  and Age columns")
    fd_df[["Ename","Age"]].show() # multiple column
    
    # With slicing operator
    print("Displaying frovedis dataframe using condition based slicing operator")
    fd_df[fd_df.Age > 19].show()  
    
    # With slicing operator
    print("Displaying frovedis dataframe using chaining of methods")
    fd_df[fd_df.Country.str.contains("a")].show()
    
Output,
    
    Displaying complete frovedis dataframe
    index   Ename   Age     Country isMale
    0       Michael 29      USA     0
    1       Andy    30      England 0
    2       Tanaka  27      Japan   0
    3       Raul    19      France  0
    4       Yuta    31      Japan   1

    Displaying frovedis dataframe with just Ename column
    index   Ename
    0       Michael
    1       Andy
    2       Tanaka
    3       Raul
    4       Yuta 

    Displaying frovedis dataframe with Ename  and Age columns
    index   Ename   Age
    0       Michael 29
    1       Andy    30
    2       Tanaka  27
    3       Raul    19
    4       Yuta    31

    Displaying frovedis dataframe using condition based slicing operator
    index   Ename   Age     Country isMale
    0       Michael 29      USA     0
    1       Andy    30      England 0
    2       Tanaka  27      Japan   0
    4       Yuta    31      Japan   1

    Displaying frovedis dataframe using chaining of methods
    index   Ename   Age     Country isMale
    1       Andy    30      England 0
    2       Tanaka  27      Japan   0
    3       Raul    19      France  0
    4       Yuta    31      Japan   1
  
__Return Value__  
It return nothing.  

### 19. tail(n = 5)  

__Parameters__  
**_n_**: It accepts an integer as parameter. It represents the number of rows which is to be selected. (Default: 5)  

__Purpose__  
This utility is used to return the last **n** rows from the DataFrame.  
It is useful for quickly verifying data, for example, after sorting or appending rows.  

For negative values of n, this function returns all rows except the first n rows, equivalent to **fdf[n:]**.  

For example:

	# example to use tail with default value
    
    import pandas as pd
    import frovedis.dataframe as fdf
    
    pd_df = pd.DataFrame({'animal': ['alligator', 'bee', 'falcon', 'lion',
                   'monkey', 'parrot', 'shark', 'whale', 'zebra']})
	fd_df = fdf.DataFrame(pd_df)
    
    print("Viewing all entries of dataframe")
	fd_df.show()
    
    print("Viewing the last 5 lines")
	fd_df.tail().show()
    
Output,

	Viewing all entries of dataframe
    index   animal
	0       alligator
	1       bee
	2       falcon
	3       lion
	4       monkey
	5       parrot
	6       shark
	7       whale
	8       zebra

	Viewing the last 5 lines
    index   animal
	4       monkey
	5       parrot
	6       shark
	7       whale
	8       zebra    
    
For example:    
    
    # example to use tail with custom value
    
    import pandas as pd
    import frovedis.dataframe as fdf
    
    pd_df = pd.DataFrame({'animal': ['alligator', 'bee', 'falcon', 'lion',
                   'monkey', 'parrot', 'shark', 'whale', 'zebra']})
	fd_df = fdf.DataFrame(pd_df)
    
    print("Viewing all entries of dataframe")
	fd_df.show()
    
    # n = 3
    print("Viewing the last 3 lines")
	fd_df.tail(3).show()
    
Output,

	Viewing all entries of dataframe
    index   animal
	0       alligator
	1       bee
	2       falcon
	3       lion
	4       monkey
	5       parrot
	6       shark
	7       whale
	8       zebra

	Viewing the last 3 lines
    index   animal
	6       shark
	7       whale
	8       zebra
    
For example:    
    
    # example to use tail with negative value
    
    import pandas as pd
    import frovedis.dataframe as fdf
    
    pd_df = pd.DataFrame({'animal': ['alligator', 'bee', 'falcon', 'lion',
                   'monkey', 'parrot', 'shark', 'whale', 'zebra']})
	fd_df = fdf.DataFrame(pd_df)
    
    print("Viewing all entries of dataframe")
	fd_df.show()
    
    # n = -2
    print("For negative values of n")
	fd_df.tail(-2).show()
    
Output,

	Viewing all entries of dataframe
    index   animal
	0       alligator
	1       bee
	2       falcon
	3       lion
	4       monkey
	5       parrot
	6       shark
	7       whale
	8       zebra
    
    For negative values of n
    index   animal
	2       falcon
	3       lion
	4       monkey
	5       parrot
	6       shark
	7       whale
	8       zebra

__Return Value__   
It returns a new Frovedis DataFrame instance with last **n** rows.  

### 20. update_index(value, key = None, verify_integrity = False, inplace = False)  

__Parameters__  
**_value_**: It accepts a list-like object (list or tuple) as parameter. It contains the entries of the new index label.  
**_key_**: It accepts a string object as parameter. It is the name of the Index label. (Default: None)  
When it is None (not specified explicitly), it will not update the index entries and it will raise an exception.  
**_verify\_integrity_**: It accepts a boolean value as parameter. When it is explicitly set to True, it checks the new index for duplicates values before update and it will not update the index when there are duplicate values. Performance of this method will be improved when it is set to False. (Default: False)  
**_inplace_**: It accepts a boolean as a parameter. It return a copy of DataFrame instance by default but when explicitly set to True, it performs operation on original DataFrame. (Default: False)  

__Purpose__  
It sets the DataFrame index using existing columns. The index can replace the existing index or can expand it.  

For example:
    
    # example to use update_index (normal case)
    
    import pandas as pd
    import frovedis.dataframe as fdf
    
    pd_df = pd.DataFrame({'month': [1, 4, 7, 10],
                         'year': [2012, 2014, 2013, 2014],
                         'sale': [55, 40, 84, 31]})
    fd_df = fdf.DataFrame(pd_df)
    # display frovedis dataframe
    fd_df.show()
    
    # update the index values:
    fd_df.update_index(['a','b','a','d'], key = 'index').show()
    
Output,

    index   month   year    sale
    0       1       2012    55
    1       4       2014    40
    2       7       2013    84
    3       10      2014    31

    index   month   year    sale
    a       1       2012    55
    b       4       2014    40
    a       7       2013    84
    d       10      2014    31
    
For example:
    
    # example to use update_index with verify_integrity parameter.
    
    import pandas as pd
    import frovedis.dataframe as fdf
    
    pd_df = pd.DataFrame({'month': [1, 4, 7, 10],
                         'year': [2012, 2014, 2013, 2014],
                         'sale': [55, 40, 84, 31]})
    fd_df = fdf.DataFrame(pd_df)
    fd_df.show()
    
    # update the index values and verify_integrity = True
    fd_df.update_index(['a','b','c','d'], key = 'index', verify_integrity = True).show()
    
Output,

    index   month   year    sale
    0       1       2012    55
    1       4       2014    40
    2       7       2013    84
    3       10      2014    31

    index   month   year    sale
    a       1       2012    55
    b       4       2014    40
    c       7       2013    84
    d       10      2014    31

**Note:-** Values used to update 'index' must be different when parameter 'verify_integrity' = True.  

__Return Value__  
1. It returns a new Frovedis DataFrame with updated row values for index field along with specified label.  
2. It returns None when 'inplace' parameter is set to True.  


# SEE ALSO  

- **[DataFrame - Introduction](./DataFrame_Introduction.md)**  
- **[DataFrame - Conversion, Missing data handling, Sorting Functions](./DataFrame_ConversionAndSorting.md)**  
- **[DataFrame - Function Application, Groupby](./DataFrame_FunctionApplicationGroupby.md)**  
- **[DataFrame - Binary Operators](./DataFrame_BinaryOperators.md)**  
- **[DataFrame - Aggregate Functions](./DataFrame_AggregateFunctions.md)**  