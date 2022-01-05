# tpch-spark

TPC-H queries implemented in Spark using the Frovedis DataFrame APIs.
Tested under Spark 3.1.2

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

```
./02_compile_and_run.sh
```

Default QID=0, which means all the queries would be executed in order.
If you need a specific query to be executed, modify the value of QID before executing the above script.

Make sure you set the INPUT_DIR and OUTPUT_DIR in `TpchQuery` class before compiling to point to the
location the of the input data and where the output should be saved.

