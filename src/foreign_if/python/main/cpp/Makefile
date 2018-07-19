include ../../../../../Makefile.in.x86

LIBS := ../../../exrpc/libfrovedis_exrpc.a ../../../../frovedis/core/libfrovedis_core.a ../../../../frovedis/matrix/libfrovedis_matrix.a ../../../../frovedis/ml/libfrovedis_ml.a ../../../../frovedis/dataframe/libfrovedis_dataframe.a ../../../server/libfrovedis_server.a

INCLUDES := -I../../../../../third_party/cereal-1.2.2/include/ -I../../../.. -I../../../exrpc -I../../../server -I${PYTHON_INC}

CXXFLAGS := ${CXXFLAGS_COMMON} ${INCLUDES}

TARGETS := libfrovedis_client_python.so

libfrovedis_client_python.so: client.o ${LIBS}
	${MPICXX} -fopenmp -shared -o libfrovedis_client_python.so -Wl,-whole-archive client.o ${LIBS} -Wl,-no-whole-archive -L../../../../../third_party/lib ${LIBS_COMMON}
	install -d ../../lib
	cp libfrovedis_client_python.so ../../lib

client.o: client.cc 

clean:
	${RM} ${TO_CLEAN} ${TARGETS}

