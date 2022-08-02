% DataFrame Indexing Operations

# NAME

Indexing in frovedis Dataframe - it means simply selecting particular rows and columns of data from a dataframe.  

# DESCRIPTION  
There are a lot of ways to pull the rows and columns from a dataframe. Frovedis DataFrame class provides some 
indexing methods which help in filtering data from a dataframe. These indexing methods appear very similar but 
behave very differently. **Currently, frovedis supports the below mentioned types of indexing**. They are as follows:  

- **Dataframe.[ ]**: This function is also known as __getitem__. It helps in filtering rows and columns from a dataframe.  
- **Dataframe.loc[]**: It is primarily label based, but may also be used with a boolean array.  
- **Dataframe.iloc[]**: It is primarily integer position based, but may also be used with a boolean array.  
- **Dataframe.at[]**: Similar to loc, both provide label-based lookups. It is used when only needed to get a 
single value in a DataFrame.  
- **Dataframe.iat[]**: Similar to iloc, both provide integer-based lookups. It is used when only needed to get 
a single value in a DataFrame.  
- **Dataframe.take()**: This function is used to return the elements in the given positional indices along an axis.  


## Detailed description  

**1. Indexing using [ ]**  

Indexing operator is used to refer to the square brackets following an object.  

In order to select a single column, we simply put the name of the column in-between the brackets.  

**Selecting a single column:**  

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

    print('selecting single column: ')
    fdf1['Bonus'].show()

Output  

    index   Last Bonus   Bonus   Last Salary  Salary
    John    5            5       58           60
    Marry   2            2       59           60
    Sam     2            2       63           64
    Jo      4            4       58           59

    selecting single column:
    index   Bonus
    John    5
    Marry   2
    Sam     2
    Jo      4

**Note:- Frovedis also supports use of attribute operators to filter/select column(s) in dataframe**  

In previous example, **Bonus** column can also be selected from the dataframe as follows:  

For example,  

    fdf1.Bonus

This returns a FrovedisColumn instance.  

**To select multiple columns:**  

For example,  

    # selecting multiple columns
    fdf1[['Bonus','Salary']].show()

Output  

    index   Bonus   Salary
    John    5       60
    Marry   2       60
    Sam     2       64
    Jo      4       59

Here, list of columns are passed in the indexing operator.  

**Filtering dataframe using slice operation with row numbers:**  

For example,  

    fdf1[1:2].show()

Output  

    index   Last Bonus   Bonus   Last Salary   Salary
    Marry   2            2       59            60

**Filtering dataframe using slice operation with row labels:**  

For example,  

    fdf1['John':'Sam'].show()

Output  

    index   Last Bonus   Bonus   Last Salary   Salary
    Marry   2            2       59            60

**Filtering can be done with help of attribute operators:**  

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

The above filtering expressions can also be done with the help of indexing operator.  

For example,  

    # filtering data using given condition
    fdf1[fdf1["Bonus"] == 2].show()

Output  

    index   Last Bonus  Bonus  Last Salary  Salary
    Marry   2           2      59           60
    Sam     2           2      63           64

For example,  

    # filtering data using '>' operator
    fdf1[fdf1["Bonus"] > 2].show()

Output  

    index   Last Bonus   Bonus   Last Salary   Salary
    John    5            5       58            60
    Jo      4            4       58            59

**2. Indexing a DataFrame using .loc[ ] :**  

This function selects data by the label of the rows and columns.  

Allowed inputs are:  

- A single label, e.g. 5 or 'a'. Here, 5 is interpreted as a label of the index, and not as an 
integer position along the index.  
- A list or array of labels, e.g. ['a', 'b', 'c'].  
- A slice object with labels, e.g. 'a':'f'. Here while using slices, both the start and the stop 
are included.  
- A boolean array of the same length as number of rows, e.g. [True, False, True].  

**Currently, .loc[] cannot be used to set values for items in dataframe.**   

For example,   

    fdf1.loc['0',:] = 'Jai'              #not supported
    fdf1.loc['0',:] = 12                 #not supported

The above expression will give an error.   

Also, it cannot be used to filter data using given condition.  

For example,  

    fdf1.loc[fdf1['Bonus'] > 2].show()   #not supported

The above expression will give an error.  

**Selecting a single column:**  

In order to select a single row using .loc[], we put a single row label in a .loc function.  

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

    print('selecting single column: ')
    fdf1.loc['John'].show()

Output  

    index   Last Bonus   Bonus   Last Salary  Salary
    John    5            5       58           60
    Marry   2            2       59           60
    Sam     2            2       63           64
    Jo      4            4       58           59

    selecting single column:
    index   Last Bonus   Bonus   Last Salary  Salary
    John    5            5       58           60

**To select multiple columns:**  

For example,  

    # selecting multiple columns
    fdf1.loc[['Marry','Sam','Jo']].show()

Output  

    index   Last Bonus   Bonus   Last Salary  Salary
    Marry   2            2       59           60
    Sam     2            2       63           64
    Jo      4            4       58           59

**Filtering dataframe using slice operation with row labels:**  

For example,  

    fdf1.loc['Marry':'Sam'].show()

Output  

    index   Last Bonus   Bonus   Last Salary  Salary
    Marry   2            2       59           60
    Sam     2            2       63           64

**Selecting two rows and three columns:**  

For example,  

    fdf1.loc['Marry':'Sam', ['Bonus', 'Last Salary', 'Salary']]

Output  

    index   Bonus    Last Salary   Salary
    Marry   2        59            60
    Sam     2        63            64

**Selecting all rows and some columns:**  

For example,  

    fdf1.loc[:, ['Bonus', 'Last Salary', 'Salary']]

Output  

    index   Bonus    Last Salary   Salary
    John    5        58            60
    Marry   2        59            60
    Sam     2        63            64
    Jo      4        58            59

**To select data using boolean list:**  

For example,  

    # selecting using boolean list
    fdf1.loc[[True, False, True, False]].show()

Output  

    index   Last Bonus   Bonus   Last Salary   Salary
    John    5            5       58            60
    Sam     2            2       63            64

In case a dataframe having boolean indices is used, then .loc[] must be used with such boolean labels only.  

For example,  

    import pandas as pd
    import frovedis.dataframe as fdf

    # a pandas dataframe from key value pair
    pdf2 = pd.DataFrame({ "Last Bonus": [5, 2, 2, 4],
                          "Bonus": [5, 2, 2, 4],
                          "Last Salary": [58, 59, 63, 58],
                          "Salary": [60, 60, 64, 59]
                        }, index= [True, False, False, True]
                       )
    # creating frovedis dataframe
    fdf1 = fdf.DataFrame(pdf2)

    # display created frovedis dataframe
    fdf1.show()

    fdf1.loc[[False, False, True, False]].show()

Output  

    index   Last Bonus   Bonus   Last Salary   Salary
    1       5            5       58            60
    0       2            2       59            60
    0       2            2       63            64
    1       4            4       58            59

    index   Last Bonus   Bonus   Last Salary   Salary
    0       2            2       63            64

**3. Indexing a DataFrame using .iloc[ ] :**  

This function allows us to retrieve rows and columns by position.  

The df.iloc indexer is very similar to df.loc but only uses integer locations 
to make its selections.  

Allowed inputs are:  

- An integer, e.g. 5.  
- A list or array of integers, e.g. [4, 3, 0].  
- A slice object with ints, e.g. 1:7.  
- A boolean array of the same length as number of rows, e.g. [True, False, True].  

**Currently, .iloc[] cannot be used to set values for items in dataframe.**  

For example,  

    fdf1.iloc[0,:] = 'Jai'               #not supported
    fdf1.iloc[0,:] = 12                  #not supported

The above expression will give an error.  

Also, it cannot be used to filter data using given condition.  

For example,  

    fdf1.iloc[fdf1['Bonus'] > 2].show()  #not supported

The above expression will give an error.  

**Selecting a single column:**  

In order to select a single row using .iloc[], we can pass a single integer to .iloc[] function.  

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

    print('selecting single column: ')
    fdf1.iloc[3].show()

Output  

    index   Last Bonus   Bonus   Last Salary  Salary
    John    5            5       58           60
    Marry   2            2       59           60
    Sam     2            2       63           64
    Jo      4            4       58           59

    selecting single column:
    index   Last Bonus   Bonus   Last Salary  Salary
    Jo      4            4       58           59

**To select multiple columns:**  

For example,  

    # selecting multiple columns
    fdf1.iloc[[0, 1 ,3]].show()

Output  

    index   Last Bonus   Bonus   Last Salary  Salary
    John    5            5       58           60
    Marry   2            2       59           60
    Jo      4            4       58           59

**Filtering dataframe using slice operation with row labels:**  

For example,  

    fdf1.iloc[1:3].show()

Output  

    index   Last Bonus   Bonus   Last Salary  Salary
    Marry   2            2       59           60
    Sam     2            2       63           64

**Selecting two rows and two columns:**  

For example,  

    fdf1.iloc[[1, 3],[2, 3]].show()

Output  

    index   Last Salary  Salary
    Marry   59           60
    Jo      58           59

**Selecting all rows and some columns:**  

For example,  

    fdf1.iloc[:, [2, 3]].show()

Output  

    index   Last Salary   Salary
    John    58            60
    Marry   59            60
    Sam     63            64
    Jo      58            59

**To select data using boolean list:**  

For example,  

    # selecting using boolean list
    fdf1.iloc[[True, False, True, False]].show()

Output  

    index   Last Bonus   Bonus   Last Salary   Salary
    John    5            5       58            60
    Sam     2            2       63            64


**4. Indexing a DataFrame using .at[ ] :**  

This function is used to access single value for a row/column label pair.  

This method works in a similar way to .loc[ ] but .at[ ] is used to return an only single value 
and hence it works faster than .loc[].  

**Currently, .at[] cannot be used to set values for items in dataframe.**  

For example,  

    fdf1.at['John', 'Bonus'] = 52       #not supported

The above expression will give an error.  

**Accesing a single value:**  

In order to access a single value, we can pass a pair of row and column labels to .at[] function.  

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

    print('selecting single value: ')
    fdf1.at['John', 'Bonus']

Output  

    index   Last Bonus   Bonus   Last Salary  Salary
    John    5            5       58           60
    Marry   2            2       59           60
    Sam     2            2       63           64
    Jo      4            4       58           59

    selecting single value:
    5

In case the given row/column label is not found then, it will give KeyError.  

Also, it does not allow to perform slicing and will give an error.  

**5. Indexing a DataFrame using .iat[ ] :**  

This function is used to access single value for a row/column pair by integer position.  

This method works in a similar way to .iloc[ ] but .iat[ ] is used to return an only single value
and hence it works faster than .iloc[].  

**Currently, .iat[] cannot be used to set values for items in dataframe.**  

For example,  

    fdf1.iat[0, 2] = 52       #not supported

The above expression will give an error.  

**Accesing a single value:**  

In order to access a single value, we can pass a pair of row and column index positions to .iat[] function.  

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

    print('selecting single value: ')
    fdf1.iat[0, 2]

Output  

    index   Last Bonus   Bonus   Last Salary  Salary
    John    5            5       58           60
    Marry   2            2       59           60
    Sam     2            2       63           64
    Jo      4            4       58           59

    selecting single value:
    58

In case the given row/column index position is not found then, it will give IndexError.  

Also, it does not allow to perform slicing and will give an error.  

**6. Indexing a DataFrame using take() :**  

**DataFrame.take(indices, axis = 0, is_copy = None, \*\*kwargs)**  

__Parameters__  
**_indices_**: An array of integers indicating which positions to take.  
**_axis_**: It accepts an integer or string object as parameter. It is used to select elements. (Default: 0)  
- 0 or 'index': used to select rows.  
- 1 or 'columns': used to select columns.  

**_is\_copy_**: This is an unused parameter. (Default: None)  
_**\*\*kwargs**_: This is an unused parameter.  

__Purpose__  
It returns the elements in the given positional indices along an axis.  

Here,  indexing is not according to actual values in the index attribute of the object rather 
indexing is done according to the actual position of the element in the object.  

**Selecting elements along axis = 0 (default) :**  

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

    print('selecting along axis = 0: ')
    fdf1.take(indices = [0, 2], axis = 0)

Output  

    index   Last Bonus   Bonus   Last Salary  Salary
    John    5            5       58           60
    Marry   2            2       59           60
    Sam     2            2       63           64
    Jo      4            4       58           59

    selecting along axis = 0:
    index   Last Bonus   Bonus   Last Salary  Salary
    John    5            5       58           60
    Sam     2            2       63           64

**Selecting elements along axis = 1 (column selection) :**  

For example,  

    fdf1.take(indices = [1, 2], axis = 1)

Output  

    index   Bonus   Last Salary
    John    5       58
    Marry   2       59
    Sam     2       63
    Jo      4       58

**Selecting elements from the end along axis = 0:**  

For example,  

    fdf1.take(indices = [-1, -2], axis = 0)

Output  

    index   Last Bonus  Bonus   Last Salary    Salary
    Jo      4           4       58             59
    Sam     2           2       63             64

__Return Value__  
It returns a frovedis DataFrame instance or FrovedisColumn instance.  

# SEE ALSO  

- **[DataFrame - Introduction](./df_intro.md)**  
- **[DataFrame - Generic Functions](./df_generic_func.md)**  
- **[DataFrame - Conversion Functions](./df_conversion.md)**  
- **[DataFrame - Sorting Functions](./df_sort.md)**  
- **[DataFrame - Math Functions](./df_math_func.md)**  
- **[DataFrame - Aggregate Functions](./df_agg_func.md)**  

