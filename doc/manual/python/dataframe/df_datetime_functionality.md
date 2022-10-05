
% Dataframe Datetime functionality

# NAME
Frovedis supports creation of datetime columns in dataframes and various functionalities that can be done with datetime columns.

# DESCRIPTION
The datetime columns in frovedis are internally stored as integers representing the amount of time in nanoseconds since January 1, 1970.
The missing values in datetime columns are present as NULL.


## Detailed description
Frovedis supports creation of columns of datetime type. This can be done by loading a pandas dataframe with datetime type columns. Consider the following pandas dataframe "df1" with a datetime column "c2".

	print(df1)
	Output:
	   c1         c2
	0   5 2022-01-02
	1  10 2022-04-22
	2  12 2022-09-11
	3  25 2022-02-24
    
    
	print(df1.dtypes)
	Output:
	c1             int64
	c2    datetime64[ns]


The frovedis dataframe is created by passing "df1" in the constructor for frovedis DataFrame class.

	fdf1 = fdf.DataFrame(df1)
	print(fdf1)
    Output:
	index   c1      c2
	0       5       2022-01-02
	1       10      2022-04-22
	2       12      2022-09-11
	3       25      2022-02-24

	print(fdf1.dtypes)
	Output:
	c1         int64
	c2    datetime64


In pandas the dataframe, different datetime columns can have different time-zones set. In such case, the time-zone related information is saved in FrovedisDataframe object as a dictionary: "datetime_cols_info".

	print(df2)
	Output:
	   c1                        c2                        c3
	0   5 2022-01-02 00:00:00+09:00 2021-01-01 00:00:00+05:30
	1  10 2022-04-22 00:00:00+09:00 2021-02-01 00:00:00+05:30
	2  12 2022-09-11 00:00:00+09:00 2021-03-01 00:00:00+05:30
	3  25 2022-02-24 00:00:00+09:00 2021-04-02 00:00:00+05:30


	print(df2.dtypes)
	Output:
	c1                           int64
	c2      datetime64[ns, Asia/Tokyo]
	c3    datetime64[ns, Asia/Kolkata]


	fdf2 = fdf.DataFrame(df2)
	print(fdf2)
	Output:
	index   c1      c2              c3
	0       5       2022-01-02      2021-01-01
	1       10      2022-04-22      2021-02-01
	2       12      2022-09-11      2021-03-01
	3       25      2022-02-24      2021-04-02

	print(fdf2.dtypes)
	Output:
	c1         int64
	c2    datetime64
	c3    datetime64

	print(fdf2.datetime_cols_info)
	Output:
	{'c2': 'Asia/Tokyo', 'c3': 'Asia/Kolkata'}


The time-zone information in "datetime_cols_info" is used internally while converting frovedis dataframe to pandas dataframe using to_pandas().

	converted_df = fdf2.to_pandas()
	print(converted_df)
	Output:
	       c1                        c2                        c3
	index
	0       5 2022-01-02 00:00:00+09:00 2021-01-01 00:00:00+05:30
	1      10 2022-04-22 00:00:00+09:00 2021-02-01 00:00:00+05:30
	2      12 2022-09-11 00:00:00+09:00 2021-03-01 00:00:00+05:30
	3      25 2022-02-24 00:00:00+09:00 2021-04-02 00:00:00+05:30

	print(converted_df.dtypes)
	Output:
	c1                           int64
	c2      datetime64[ns, Asia/Tokyo]
	c3    datetime64[ns, Asia/Kolkata]



### Support of missing values

The missing values in datetime columns in pandas are represented as NaT (Not a Time). Frovedis supports loading of datetime columns with NaT values. These would be represented as NULL in frovedis dataframes.

	print(df3)
	Output:
	   c1         c2
	0   5 2022-01-02
	1  10 2022-04-22
	2  12        NaT
	3  25 2022-02-24

	fdf3 = fdf.DataFrame(df3)
	print(fdf3)
	Output:
	index   c1      c2
	0       5       2022-01-02
	1       10      2022-04-22
	2       12      NULL
	3       25      2022-02-24

The missing values present as NULL would be converted to NaT when converting frovedis dataframe to pandas dataframe.

	converted_df = fdf3.to_pandas()
	print(converted_df)
	Output:
	      c1         c2
	index
	0       5 2022-01-02
	1      10 2022-04-22
	2      12        NaT
	3      25 2022-02-24

## Datetime extraction functions

Frovedis supports extraction of information like day, month, hour, etc. from datetime columns. This can be done through the "dt" accessor, which returns a FrovedisDatetimeProperties object. Consider the following dataframe:

	print(df4)
	Output:
	   c1                  c2
	0   1 2022-04-01 11:32:45
	1   4 2021-11-01 12:45:43
	2  10 2023-04-14 14:23:00
	3  22 2020-01-12 07:01:24


	fdf4 = fdf.DataFrame(df4)
	print(fdf4)
	Output:
	index   c1      c2
	0       1       2022-04-01
	1       4       2021-11-01
	2       10      2023-04-14
	3       22      2020-01-12


The following datetime properties are supported for extraction from datetime columns:

**day**: This extracts the day of month from the datetime column.

    fdf4["c2"].dt.day
    Output:
    index   c2
    0       1
    1       1
    2       14
    3       12


**dayofweek**: This extracts the day of week from the datetime column. It is assumed the week starts on Monday, which is denoted by 0 and ends on Sunday which is denoted by 6.

    fdf4["c2"].dt.dayofweek
    Output:
    index   c2
    0       4
    1       0
    2       4
    3       6



**hour**: This extracts the hour from the datetime column.

	fdf4["c2"].dt.hour
	Output:
	index   c2
	0       11
	1       12
	2       14
	3       7


**minute**: This extracts the minutes from the datetime column.

	fdf4["c2"].dt.minute
	Output:
	index   c2
	0       32
	1       45
	2       23
	3       1


**second**: This extracts the seconds from the datetime column.

	fdf4["c2"].dt.second
	Output:
	index   c2
	0       45
	1       43
	2       0
	3       24


**nanosecond**: This extracts the nanoseconds from the datetime column.

	fdf4["c2"].dt.nanosecond
	Output:
	index   c2
	0       0
	1       0
	2       0
	3       0


**year**: This extracts the year from the datetime column.

	fdf4["c2"].dt.year
	Output:
	index   c2
	0       2022
	1       2021
	2       2023
	3       2020


**month**: This extracts the month from the datetime column, with January=1, December=12.

	fdf4["c2"].dt.month
	Output:
	index   c2
	0       4
	1       11
	2       4
	3       1


**quarter**: This extracts the quarter in which the date falls, from the datetime column.

	fdf4["c2"].dt.quarter
	Output:
	index   c2
	0       2
	1       4
	2       2
	3       1


**dayofyear**: This extracts the ordinal day of the year from the datetime column.

	fdf4["c2"].dt.dayofyear
	Output:
	index   c2
	0       91
	1       305
	2       104
	3       12


 **weekofyear**: This extracts the ordinal week of the year from the datetime column.

	fdf4["c2"].dt.weekofyear
	Output:
	index   c2
	0       13
	1       44
	2       15
	3       2

## Loading datetime columns through read_csv

Frovedis supports loading datetime column in dataframe from a csv file. The name of the column to be parsed as datetime type has to specified in "parse_dates" parameter. The format for the datetime column can be spefied as a string in parameter "datetime_format".

Consider the following csv file:

[data1.csv]  
A,24/12/2022  
B,12/01/2021  
C,10/02/2020  
D,01/07/2023  


	fdf1 = fdf.read_csv("./data1.csv", names=["c1", "c2"] , parse_dates=["c2"], datetime_format="%d/%m/%Y")
	print(fdf1)
	Output:
	index   c1      c2
	0       A       2022-12-24
	1       B       2021-01-12
	2       C       2020-02-10
	3       D       2023-07-01


Instead of the name of column in "parse_dates" paramter, the position of column can be specified as an integer.

	fdf1 = fdf.read_csv("./data1.csv", names=["c1", "c2"] , parse_dates=[1], datetime_format="%d/%m/%Y")
	print(fdf1)
	Output:
	index   c1      c2
	0       A       2022-12-24
	1       B       2021-01-12
	2       C       2020-02-10
	3       D       2023-07-01


In the csv file, different datetime columns can be present in different formats. Consider the following csv file.

[data2.csv]  
A,24/12/2022,2020|05|15  
B,12/01/2021,2024|11|17  
C,10/02/2020,2021|04|21  
D,01/07/2023,2025|06|04  


In such case, the "datetime_format" can be specified as a list of formats.

	fdf2 = fdf.read_csv("./data2.csv", names=["c1", "c2", "c3"] , parse_dates=["c2", "c3"],  
	                    datetime_format=["%d/%m/%Y", "%Y|%m|%d"])
	print(fdf2)
	Output:
	index   c1      c2              c3
	0       A       2022-12-24      2020-05-15
	1       B       2021-01-12      2024-11-17
	2       C       2020-02-10      2021-04-21
	3       D       2023-07-01      2025-06-04

The "datetime_format" paramter can also be specified as a dictionary with keys as column names and values as the respective formats.

	fdf2 = fdf.read_csv("./data2.csv", names=["c1", "c2", "c3"] , parse_dates=["c2", "c3"],  
	                    datetime_format={"c2": "%d/%m/%Y", "c3":"%Y|%m|%d"})
	print(fdf2)
	Output:
	index   c1      c2              c3
	0       A       2022-12-24      2020-05-15
	1       B       2021-01-12      2024-11-17
	2       C       2020-02-10      2021-04-21
	3       D       2023-07-01      2025-06-04




The format can also be inferred from the data, by specifying "infer_datetime_format" paramter as True. Internally the first hundred rows would be used to infer the format for the column. In case the datetime format cannot be inferred the column would be loaded as a string column.

	fdf3 = fdf.read_csv("./data2.csv", names=["c1", "c2", "c3"] , parse_dates=["c2", "c3"],  
	                   infer_datetime_format=True)
	print(fdf3)
	Output:
	index   c1      c2              c3
	0       A       2023-12-15      2020|05|15
	1       B       2021-12-01      2024|11|17
	2       C       2020-10-02      2021|04|21
	3       D       2023-01-07      2025|06|04

	print(fdf3.dtypes)
	Output:
	c1        object
	c2    datetime64
	c3        object
	dtype: object

In the above example, format for "c2" was inferred and it is loaded as datetime column, while format for "c3" could not be inferred and it is loaded as a string column.


In the csv file, datetime information can be present in separate columns. Consider the following csv file:

[data3.csv]  
A,12/2022,Jan  
B,08/2024,Jul  
C,22/2021,Dec  
D,04/2025,Sep  

In such case the, frovedis supports loading a combination of columns as a single datetime column. The combination needs to be specified as a list in the "parse_dates" parameter.

	fdf4 = fdf.read_csv("./data3.csv", names=["c1", "c2", "c3"] , parse_dates=[["c2", "c3"]],  
	                   infer_datetime_format=True)
	print(fdf4)
	Output:
	index   c2_c3           c1
	0       2022-01-12      A
	1       2024-07-08      B
	2       2021-12-22      C
	3       2025-09-04      D

	print(fdf4.dtypes)
	Output:
	c2_c3    datetime64
	c1           object
	dtype: object



The original columns use for the combination are dropped by default. In case the original columns are required to be kept, the "keep_date_col" paramter can be specified as True.

	fdf5 = fdf.read_csv("./data3.csv", names=["c1", "c2", "c3"] , parse_dates=[["c2", "c3"]],  
	                    infer_datetime_format=True, keep_date_col=True)
	print(fdf5)
	Output:
	index   c2_c3           c1      c2      c3
	0       2022-01-12      A       12/2022 Jan
	1       2024-07-08      B       08/2024 Jul
	2       2021-12-22      C       22/2021 Dec
	3       2025-09-04      D       04/2025 Sep

	print(fdf5.dtypes)
	Output:
	c2_c3    datetime64
	c1           object
	c2           object
	c3           object
	dtype: object


# SEE ALSO  

- **[DataFrame - Introduction](./df_intro.md)**  
- **[DataFrame - Generic Functions](./df_generic_func.md)**  
- **[DataFrame - Conversion Functions](./df_conversion.md)**  
