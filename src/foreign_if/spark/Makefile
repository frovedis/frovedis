include ../../../Makefile.in.x86

all:
	make -f Makefile.js
	cd main/cpp; make

install:
	install -D lib/libfrovedis_client_spark.so ${INSTALLPATH}/lib/libfrovedis_client_spark.so
	install -D lib/frovedis_client.jar ${INSTALLPATH}/lib/spark/frovedis_client.jar

clean:
	cd main/cpp; make clean
	make -f Makefile.js clean
	rm -f *~
