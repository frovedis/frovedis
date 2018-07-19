include ../../../../../Makefile.in.x86

LIBS := ../../../exrpc/libfrovedis_exrpc.a ../../../../frovedis/core/libfrovedis_core.a ../../../../frovedis/matrix/libfrovedis_matrix.a ../../../../frovedis/ml/libfrovedis_ml.a ../../../../frovedis/dataframe/libfrovedis_dataframe.a ../../../server/libfrovedis_server.a
INCLUDES := -I../../../../../third_party/cereal-1.2.2/include/ -I../../../.. -I../../../exrpc -I../../../server -I${JAVA_HOME}/include -I${JAVA_HOME}/include/linux

CXXFLAGS := ${CXXFLAGS_COMMON} ${INCLUDES}

TARGETS := libfrovedis_client_spark.so

libfrovedis_client_spark.so: spark_backend_client.o ../../../server/libfrovedis_server.a
	${MPICXX} -fopenmp -shared -o libfrovedis_client_spark.so -Wl,-whole-archive spark_backend_client.o ${LIBS} -Wl,-no-whole-archive -L../../../../../third_party/lib ${LIBS_COMMON}
	cp libfrovedis_client_spark.so ../../lib

spark_backend_client.o: spark_backend_client.cc jre_config.hpp

clean:
	${RM} ${TO_CLEAN} ${TARGETS} ../../lib/${TARGETS}
