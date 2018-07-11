include ../Makefile.conf
include ../Makefile.in.${TARGET}

all: 
	install -d ./lib
ifeq (${BUILD_BLAS},true)
	cd lapack-3.7.1; rm -f make.inc; ln -s make.inc.${TARGET} make.inc; make blaslib; cp BLAS/SRC/libblas.a ../lib; make cblaslib; cp CBLAS/src/libcblas.a ../lib; cp CBLAS/include/cblas.h CBLAS/include/cblas_mangling.h ../include
endif
ifeq (${BUILD_LAPACK},true)
	cd lapack-3.7.1; rm -f make.inc; ln -s make.inc.${TARGET} make.inc; make lapacklib; cp liblapack.a ../lib
endif
ifeq (${BUILD_PARPACK},true)
	cd arpack; make -f Makefile.${TARGET}; cp libparpack.a ../lib
endif
ifeq (${BUILD_SCALAPACK},true)
	cd scalapack-2.0.2; rm -f SLmake.inc; ln -s SLmake.inc.${TARGET} SLmake.inc; make lib; cp libscalapack.a ../lib
endif

install: 
ifeq (${BUILD_BLAS},true)
	install -d ${INSTALLPATH}/lib/; cp lib/libblas.a ${INSTALLPATH}/lib/libblas.a; cp lib/libcblas.a ${INSTALLPATH}/lib/libcblas.a
	install -d ${INSTALLPATH}/include/; cp include/cblas.h include/cblas_mangling.h ${INSTALLPATH}/include
	install -d ${INSTALLPATH}/licenses/third_party/; cp lapack-3.7.1/LICENSE ${INSTALLPATH}/licenses/third_party/LICENSE.blas_lapack
endif
ifeq (${BUILD_LAPACK},true)
	install -d ${INSTALLPATH}/lib/; cp lib/liblapack.a ${INSTALLPATH}/lib/liblapack.a
	install -d ${INSTALLPATH}/licenses/third_party/; cp lapack-3.7.1/LICENSE ${INSTALLPATH}/licenses/third_party/LICENSE.blas_lapack
endif
ifeq (${BUILD_PARPACK},true)
	install -d ${INSTALLPATH}/lib/; cp lib/libparpack.a ${INSTALLPATH}/lib/libparpack.a
	install -d ${INSTALLPATH}/licenses/third_party/; cp arpack/arpack-ng/COPYING ${INSTALLPATH}/licenses/third_party/COPYING.arpack
endif
ifeq (${BUILD_SCALAPACK},true)
	install -d ${INSTALLPATH}/lib/; cp lib/libscalapack.a ${INSTALLPATH}/lib/libscalapack.a
	install -d ${INSTALLPATH}/licenses/third_party/; cp scalapack-2.0.2/LICENSE ${INSTALLPATH}/licenses/third_party/LICENSE.scalapack
endif
	install -d ${INSTALLPATH}/include/
	cp -r cereal-1.2.2/include/cereal ${INSTALLPATH}/include
	install -d ${INSTALLPATH}/licenses/third_party/; cp cereal-1.2.2/LICENSE ${INSTALLPATH}/licenses/third_party/LICENSE.cereal

clean: 
ifeq (${BUILD_BLAS},true)
	cd lapack-3.7.1; rm -f make.inc; ln -s make.inc.${TARGET} make.inc; make clean; rm -f make.inc; rm -f ../lib/libblas.a ../lib/libcblas.a ../include/cblas*.h
endif
ifeq (${BUILD_LAPACK},true)
	cd lapack-3.7.1; rm -f make.inc; ln -s make.inc.${TARGET} make.inc; make clean; rm -f make.inc; rm -f ../lib/liblapack.a
endif
ifeq (${BUILD_PARPACK},true)
	cd arpack; make -f Makefile.${TARGET} clean; rm -f ../lib/libparpack.a
endif
ifeq (${BUILD_SCALAPACK},true)
	cd scalapack-2.0.2; rm -f SLmake.inc; ln -s SLmake.inc.${TARGET} SLmake.inc; make clean; rm -f SLmake.inc; rm -f ../lib/libscalapack.a
endif
