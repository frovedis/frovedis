# tpch-spark

TPC-H queries implemented in Spark using the Frovedis DataFrame APIs.
Tested under Spark 3.1.2 and SBT 1.4.7

### Generating tables

```
./01_dbgen.sh
```
which generates tables with extension `.tbl` with scale 1 (default) for a total of rougly 1GB size across all tables
and moved them inside the input directory used by the scala programs.

For different size tables you can provide the 'scale' option as first argument of the script:
```
./01_dbgen.sh 10
```
will generate roughly 10GB of input data.

### Running

There are two 'src' directories:

- src-frovedis: queries with frovedis dataframe APIs
- src-spark:    queries with spark dataframe APIs

The default symbolic link, 'src' points to the 'src-frovedis' directory
to build and execute queries with frovedis dataframe APIs.

If you need to build and execute queries with spark dataframe APIs, 
please re-link 'src' to point to the 'src-spark' directory.

The below script can be used to build and execute the target 'src':

```
./02_compile_and_run.sh
```

Default QID=0, which means all the queries would be executed in order.
If you need a specific query to be executed, modify the value of QID before executing the above script.

Make sure you set the INPUT_DIR and OUTPUT_DIR in `TpchQuery` class before compiling to point to the
location of the input data (specially when scale factor is other than default) 
and where the output should be saved.

### Report Creation

'src-frovedis' contains line-of-codes to profile each of the following sections 
during individual query execution:

- time for spark-to-frovedis data transfer
- time for query processsing by frovedis (on VE)
- time for result conversion (frovedis-to-spark dataframe)

During the exeution of the above script, if you redirect the stderr logs in some file as follows:

```
./02_compile_and_run.sh 2> stderr
```

Then the below script can be used to generate a report, 'eval.csv' containing 
times for each individual sections mentioned above for each execueted query:

```
./03_get_report.sh stderr
```

