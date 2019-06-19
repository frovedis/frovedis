include ./Makefile.conf
include ./Makefile.in.${TARGET}

JOBS := $(shell /usr/bin/nproc)

all:
	cd third_party; make all
	cd src/frovedis/core; make -f Makefile.${TARGET} -j ${JOBS}
	cd src/frovedis/matrix; make -f Makefile.${TARGET} -j ${JOBS}
	cd src/frovedis/ml; make -f Makefile.${TARGET} -j ${JOBS}
	cd src/frovedis/dataframe; make -f Makefile.${TARGET} -j ${JOBS}
ifeq (${BUILD_FOREIGN_IF},true)
	cd src/foreign_if; make all
endif
ifeq (${BUILD_SAMPLES},true)
	cd samples; make all
endif

install:
	cd third_party; make install
	cd src/frovedis/core; make -f Makefile.${TARGET} install
	cd src/frovedis/matrix; make -f Makefile.${TARGET} install
	cd src/frovedis/ml; make -f Makefile.${TARGET} install
	cd src/frovedis/dataframe; make -f Makefile.${TARGET} install
	cd doc/tutorial; make -f Makefile.${TARGET} install
	cd doc/tutorial_python; make -f Makefile.${TARGET} install
	cd doc/tutorial_spark; make -f Makefile.${TARGET} install
	cd doc/manual; make install
ifeq (${BUILD_FOREIGN_IF},true)
	cd src/foreign_if; make install
endif
	cd samples; make install
	install -d ${INSTALLPATH}/licenses
	cp LICENSE ${INSTALLPATH}/licenses

clean:
	cd third_party; make clean
	cd src/frovedis/core; make -f Makefile.${TARGET} clean
	cd src/frovedis/matrix; make -f Makefile.${TARGET} clean
	cd src/frovedis/ml; make -f Makefile.${TARGET} clean
	cd src/frovedis/dataframe; make -f Makefile.${TARGET} clean
ifeq (${BUILD_FOREIGN_IF},true)
	cd src/foreign_if; make clean
endif
	cd samples; make clean
	${RM} *~
