include ../Makefile.conf
include ../Makefile.in.${TARGET}

all:
	cd svd; make -f Makefile.${TARGET} all
	cd glm; make -f Makefile.${TARGET} all
	cd kmeans; make -f Makefile.${TARGET} all
	cd document_matrix; make -f Makefile.${TARGET} all
	cd matrixfactorization; make -f Makefile.${TARGET} all
	cd factorization_machine; make -f Makefile.${TARGET} all
	cd eigen; make -f Makefile.${TARGET} all
	cd tree; make -f Makefile.${TARGET} all
	cd graph; make -f Makefile.${TARGET} all
	cd word2vec; make -f Makefile.${TARGET}
ifeq (${TARGET},x86)
	cd matrix_tools; make -f Makefile.${TARGET}
	cd wikipedia2matrix; make -f Makefile.${TARGET}
	cd dataframe_tools; make -f Makefile.${TARGET}
endif

install:
	install -d ${INSTALLPATH}/samples/
	cat Makefile.common | sed -e /^LIBS/d | sed -e /^INCLUDES/d |sed -e s/^\#LIBS/LIBS/ |  sed -e s/^#INCLUDES/INCLUDES/g | sed -e /^install/,'$$'d > tmp; install tmp ${INSTALLPATH}/samples/Makefile.common; rm tmp
	install Makefile.in.${TARGET} ${INSTALLPATH}/samples/
	cd svd; make -f Makefile.${TARGET} install
	cd glm; make -f Makefile.${TARGET} install
	cd kmeans; make -f Makefile.${TARGET} install
	cd document_matrix; make -f Makefile.${TARGET} install
	cd matrixfactorization; make -f Makefile.${TARGET} install
	cd factorization_machine; make -f Makefile.${TARGET} install
	cd eigen; make -f Makefile.${TARGET} install
	cd tree; make -f Makefile.${TARGET} install
	cd graph; make -f Makefile.${TARGET} install
	cd word2vec; make -f Makefile.${TARGET} install
ifeq (${TARGET},x86)
	cd matrix_tools; make -f Makefile.${TARGET} install
	cd wikipedia2matrix; make -f Makefile.${TARGET} install
	cd dataframe_tools; make -f Makefile.${TARGET} install
endif

clean:
	cd svd; make -f Makefile.${TARGET} clean
	cd glm; make -f Makefile.${TARGET} clean
	cd kmeans; make -f Makefile.${TARGET} clean
	cd document_matrix; make -f Makefile.${TARGET} clean
	cd matrixfactorization; make -f Makefile.${TARGET} clean
	cd factorization_machine; make -f Makefile.${TARGET} clean
	cd eigen; make -f Makefile.${TARGET} clean
	cd tree; make -f Makefile.${TARGET} clean
	cd graph; make -f Makefile.${TARGET} clean
	cd word2vec; make -f Makefile.${TARGET} clean
ifeq (${TARGET},x86)
	cd matrix_tools; make -f Makefile.${TARGET} clean
	cd wikipedia2matrix; make -f Makefile.${TARGET} clean
	cd dataframe_tools; make -f Makefile.${TARGET} clean
endif
	${RM} *~
