Frovedis:  
NEC **fr**amework **o**f **ve**ctorized and **dis**tributed data analytics
===

# 1. Introduction

Frovedis is high-performance middleware for data analytics. It is
written in C++ and utilizes MPI for communication between the servers.

It provides

- Spark-like API for distributed processing
- Matrix library using above API
- Machine learning algorithm library
- Dataframe for preprocessing
- Spark/Python interface for easy utilization

Our primary target architecture is SX-Aurora TSUBASA, which is NEC's
vector computer; these libraries are carefully written to support
vectorization. However, they are just standard C++ programs and can
run efficiently on other architectures like x86.

The machine learning algorithm library performs really well on sparse
datasets, especially on SX-Aurora TSUBASA. In the case of logistic
regression, it performed 10x faster on x86, and 100x faster on
SX-Aurora TSUBASA, compared to Spark on x86.

In addition, it provides Spark/Python interface that is mostly
compatible with Spark MLlib and Python scikit-learn. If you are using
these libraries, you can easily utilize it. In the case of SX-Aurora
TSUBASA, Spark/Python runs on x86 side and the middleware runs on VE
(Vector Engine); therefore, users can enjoy the high-performance
without noticing the hardware details.


# 2. Installation

If you want prebuilt binary, please check "release", which includes
rpm file. If your environment is supported, using rpm is the easiest
way to install.

If you want to build the framework on SX-Aurora TSUBASA, we recommend
to utilize our build tools (https://github.com/frovedis/packaging)
together with VE version of boost (https://github.com/frovedis/boost-ve).
Please follow the instructions in README.md of the build tools.

On other platforms, please follow the instructions below.

## 2.1. Environment

The system depends on 

- C++11 compiler
- Boost library
- MPI
- Numerial libraries: LAPACK, BLAS, Parallel ARPACK, ScaLAPACK

In Linux environment, it is easy to install them using package manager
like apt or yum. 
On SX-Aurora TSUBASA, numerical libraries and MPI are provided by the
system. Boost library is provided separately. 

Alternatively, the distribution contains BLAS, LAPACK, Parallel
ARPACK, and ScaLAPACK source code. You can build them by yourself
during the build process.

## 2.2. Configuration and install

First, please edit Makefile.conf in this directory to specify the
architecture. If `TARGET := x86`, target architecture is Linux/x86.
If `TARGET := sx`, target architecture is SX-ACE (cross compiler on Linux). 
`TARGET := icpc` is for using Intel compiler on Linux (depends on OpenMPI).
If `TARGET := ve`, target architecture is SX-Aurora TSUBASA (cross
compiler on Linux). 

If `BUILD_FOREIGN_IF := true`, foreign interface to Spark and Python
will be compiled. The interface itself makes sense only on x86 (of
course, the server can run on SX-ACE or VE of SX-Aurora TSUBASA,
though). To build this, you need to install Spark and Java beforehand
on x86. 

Next, please edit Makefile.in.[x86, sx, icpc, ve]. You would want to
change `INSTALLPATH`. You might need to change `LIBS_COMMON` if you
use different Linux distribution where the library names are
different. On SX-ACE or SX-Aurora TSUBASA, you would need to change
`BOOST_INC` and `BOOST_LIB`, which will be defined after installing
Boost.

If you build foreign interface on x86, you also need to specify 
JAVA_HOME, SCALA_HOME, SPARK_HOME, HDPATH (path for hadoop jar file),
and PYTHON_INC in Makefile.in.x86 (even if TARGET is icpc, you need to
edit Makefile.in.x86).

After editing `Makefile.in.[x86, sx, icpc, ve]`, please type

    $ make

Then, libraries will be compiled.

To copy the compiled libraries and headers to the specified directory,
please type

    $ make install

If the specified directory is not your home directory, you might need
to `sudo`. 


# 3. Getting started

Tutorial is in `${INSTALLPATH}`/doc/tutorial/tutorial.[md,pdf]. 
The directory also contains small programs that are explained in the
tutorial. You can copy the source files into your home directory and
compile them by yourself. The Makefile and Makefile.in.[x86, etc.]
contains configurations for compilation, like compilation options,
path to include files, libraries, etc. 
You can re-use it for your own programs.

The small programs in the tutorial directory looks like this:

    #include <frovedis.hpp>
    
    int two_times(int i) {return i*2;}
    
    int main(int argc, char* argv[]){
      frovedis::use_frovedis use(argc, argv);
      
      std::vector<int> v;
      for(size_t i = 1; i <= 8; i++) v.push_back(i);
      auto d1 = frovedis::make_dvector_scatter(v);
      auto d2 = d1.map(two_times);
      auto r = d2.gather();
      for(auto i: r) std::cout << i << std::endl;
    }

This program creates distributed vector from std::vector, and doubles
its elements in a distribited way; then gathers to std::vector again.
As you can see, you can write distributed program quite easily and
consicely compared to MPI program.

In addition, there are also sample programs in `${INSTALLPATH}`/samples
directory. You can also use them as reference when you write your own
programs.

Manuals are in `${INSTALLPATH}`/doc/manual/manual.pdf, which are more in
detail than the tutorial. Manuals for `man` command is in
`${INSTALLPATH}`/man; you can do like `man dvector` after setting the
directory into your MANPATH.


# 4. Spark/Python interface

You can utilize the predefined functionalities from Spark/Python,
which includes machine learning algorithms and matrix operations.
This is implemented as a server; the server accepts RPC (remote
procedure call) to provide the above functionalities from Spark or
Python. The server can run on both x86 and VE.

To run the server, the network should be configured properly.
Please check if

    $ hostname -i

returns the correct IP address.

Even if you want to use VE, you need to build x86 version, because it
builds dynamic link libraries for Spark/Python interface.

If the insall path of x86 version is `${INSTALLPATH}`, 
`${INSTALLPATH}`/foreign_if_demo/[spark,python] includes the demo.

Please copy these directories into your home directory (since it
creates files). The scripts ./foreign_if_demo/spark/run_demo.sh and 
./foreign_if_demo/python/run_demo.sh run demos. 

As for Spark, compiled jar files are used; as for Python, Python
scripts are used. The server is invoked from the script and shutdown
at the end of the program. 

To run the demo, you need to set following environment variables:

    export INSTALLPATH=...
    export FROVEDIS_SERVER=${INSTALLPATH}/bin/frovedis_server
    export PYTHONPATH=${INSTALLPATH}/lib/python
    export LD_LIBRARY_PATH=${INSTALLPATH}/lib

`FROVEDIS_SERVER` is used in the both run_demo.sh scripts.
`INSTALLPATH` is used to specify options in `spark-submit` in the
script. `PYTHONPATH` is needed to load the interface library written
in Python. `LD_LIBRARY_PATH` is used to load dynamic link library that
is used in the Python library.

You can modify the Spark programs and Python scripts; there is a
Makefile to build jar file from Spark/Scala programs.

If you want to run the server on VE, you need to change the
environment varaiable like:

    export FROVEDIS_SERVER="-x ${INSTALLPATH}/bin/frovedis_server"

Here, `${INSTALLPATH}` is the installation path of VE version.
Please note "-x"; it is required because `mpirun` of SX-Aurora TSUBASA
requires "-x" option to specify the binary.


# 5. License

License of this software is in LICENSE file. 

This software includes third party software. The third_party directory
includes third party software together with their licenses. 
For convenience, src/frovedis/core and src/foreign_if/exrpc inclues a
few third party files whose license is Boost Software License (shown
in the header of the files), and src/frovedis/ml/w2v is based on the
software whose license is Apache License Version 2.0. 
