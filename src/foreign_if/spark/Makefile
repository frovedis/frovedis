include ../../../Makefile.in.x86

JOBS := $(shell /usr/bin/nproc --all)

all:
	install -d ./bin
	install -d ./lib
	make -f Makefile.js
	cd main/cpp; make -j ${JOBS}

install:
	install -D lib/libfrovedis_client_spark.so ${INSTALLPATH}/lib/libfrovedis_client_spark.so
	install -D lib/frovedis_client.jar ${INSTALLPATH}/lib/spark/frovedis_client.jar

clean:
	cd main/cpp; make clean
	make -f Makefile.js clean
	rm -f *~
