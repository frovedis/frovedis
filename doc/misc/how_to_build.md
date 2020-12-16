How to build
===

If you want to build the framework on platforms other than SX-Aurora
TSUBASA, please follow the instructions below.

# 1. Environment

The system depends on 

- C++11 compiler
- Boost library
- MPI
- Numerical libraries: LAPACK, BLAS, Parallel ARPACK, ScaLAPACK

In Linux environment, it is easy to install them using package manager
like apt or yum. 
On SX-Aurora TSUBASA, numerical libraries and MPI are provided by the
system. Boost library is provided separately. 

Alternatively, the distribution contains BLAS, LAPACK, Parallel
ARPACK, and ScaLAPACK source code. You can build them by yourself
during the build process.

The system also depends on third party serialization tools like cereal
or YAS, but they are included in the distribution so you do not have
to worry about it.

# 2. Configuration and install

First, please edit Makefile.conf in this directory to specify the
architecture. If `TARGET := x86`, target architecture is Linux/x86.
`TARGET := icpc` is for using Intel compiler on Linux (it is assumed
to use OpenMPI as MPI). If `TARGET := ve`, target architecture is
SX-Aurora TSUBASA (cross compiler on Linux). 

If `BUILD_FOREIGN_IF := true`, foreign interface to Spark and Python
will be compiled. The interface itself makes sense only on x86 (of
course, the server can run on VE of SX-Aurora TSUBASA, though). To
build this, you need to install Spark and Java beforehand on x86. 

Next, please edit Makefile.in.[x86, icpc, ve]. You would want to
change `INSTALLPATH`. You might need to change `LIBS_COMMON` if you
use different Linux distribution where the library names are
different. On SX-Aurora TSUBASA, you would need to change `BOOST_INC`
and `BOOST_LIB`, which will be defined after installing Boost.

If you build foreign interface on x86, you also need to specify 
JAVA_HOME, SCALA_HOME, SPARK_HOME, HDPATH (path for hadoop jar file),
and PYTHON_INC in Makefile.in.x86 (even if TARGET is icpc, you need to
edit Makefile.in.x86).

After editing `Makefile.in.[x86, icpc, ve]`, please type

    $ make

Then, libraries will be compiled.

To copy the compiled libraries and headers to the specified directory,
please type

    $ make install

If the specified directory is not your home directory, you might need
to `sudo`. 

# 3. Spark/Python interface

Even if you want to use VE, you need to build x86 version, because it
builds dynamic link libraries for Spark/Python interface.

To run the demo, you need to set following environment variables
(if you are using prebuilt binary, veenv.sh or x86env.sh sets these
variables): 

    export INSTALLPATH=...
    export FROVEDIS_SERVER=${INSTALLPATH}/bin/frovedis_server
    export PYTHONPATH=${INSTALLPATH}/lib/python
    export LD_LIBRARY_PATH=${INSTALLPATH}/lib

`FROVEDIS_SERVER` is used in the run_demo.sh scripts of Spark/Python demo.
`INSTALLPATH` is used to specify options in `spark-submit` in the
script. `PYTHONPATH` is needed to load the interface library written
in Python. `LD_LIBRARY_PATH` is used to load dynamic link library that
is used in the Python library.
