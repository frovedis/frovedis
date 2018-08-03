include ../../Makefile.conf
include ../../Makefile.in.${TARGET}

all:
	cd exrpc; make -f Makefile.${TARGET}
	cd server; make -f Makefile.${TARGET}
ifeq (${TARGET},x86)
	cd spark; make
	cd spark/examples; make
	install -d python/examples/out
	cd python/main/cpp; make
endif

install:
	cd exrpc; make -f Makefile.${TARGET} install
	cd server; make -f Makefile.${TARGET} install
ifeq (${TARGET},x86)
	cd spark; make install
	install -d ${INSTALLPATH}/foreign_if_demo/spark
	cd spark/examples; tar cf - --exclude=.svn ./* | (cd ${INSTALLPATH}/foreign_if_demo/spark && tar xvf - --no-same-owner)
	cd ${INSTALLPATH}/foreign_if_demo/spark; cat run_demo.sh | sed -e /^LIBRARY_PATH/d | sed -e /^JARS/d | sed -e /^COMMAND/d | sed -e s/^\#COMMAND/COMMAND/g | sed -e s/^\#LIBRARY_PATH/LIBRARY_PATH/g | sed -e s/^\#JARS/JARS/g > tmp; mv tmp run_demo.sh; chmod +x run_demo.sh; chmod 777 out
	cp ../../Makefile.in.x86 ${INSTALLPATH}/foreign_if_demo/spark
	cd ${INSTALLPATH}/foreign_if_demo/spark; cat Makefile | sed -e /^include/d | sed -e /^CLIENT_JAR/d | sed -e s/^\#include/include/g | sed -e s/^\#CLIENT_JAR/CLIENT_JAR/g > tmp; mv tmp Makefile
	cd ${INSTALLPATH}/foreign_if_demo/spark/MANIFEST; cat Makefile | sed -e /^include/d | sed -e s/^\#include/include/g > tmp; mv tmp Makefile
	install -d ${INSTALLPATH}/lib/python/
	cp python/lib/libfrovedis_client_python.so ${INSTALLPATH}/lib/libfrovedis_client_python.so
	install -d ${INSTALLPATH}/lib/python
	cd ./python/main/python; tar cf - --exclude=.svn ./* | (cd ${INSTALLPATH}/lib/python && tar xvf - --no-same-owner)
	install -d ${INSTALLPATH}/foreign_if_demo/python
	cd ./python/examples; tar cf - --exclude=.svn ./* | (cd ${INSTALLPATH}/foreign_if_demo/python && tar xvf - --no-same-owner)
	cd ${INSTALLPATH}/foreign_if_demo/python; cat run_demo.sh | sed -e /^export/d | sed -e /^COMMAND/d | sed -e s/^\#COMMAND/COMMAND/g > tmp; mv tmp run_demo.sh; chmod +x run_demo.sh; chmod 777 out
endif

clean:
	cd exrpc; make -f Makefile.${TARGET} clean
	cd server; make -f Makefile.${TARGET} clean
ifeq (${TARGET},x86)
	cd spark; make clean
	cd spark/examples; make clean
	cd python/main/cpp; make clean
	rm -fr python/examples/out
endif

