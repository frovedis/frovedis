include ../../../../../Makefile.in.x86

LIBS := ../../../exrpc/libfrovedis_exrpc.a ../../../../frovedis/core/libfrovedis_core.a ../../../../frovedis/text/libfrovedis_text.a ../../../../frovedis/matrix/libfrovedis_matrix.a ../../../../frovedis/ml/libfrovedis_ml.a ../../../../frovedis/dataframe/libfrovedis_dataframe.a ../../../server/libfrovedis_server.a

INCLUDES := -I../../../../../third_party/cereal-1.2.2/include/ -I../../../../../third_party/yas-7.0.5/include -I../../../.. -I../../../exrpc -I../../../server -I${PYTHON_INC}

HEADERS := python_client_headers.hpp
OBJS := utility.o exception_handling.o server_info.o dvector.o data_storage.o wrapper.o dataframe.o model.o ml.o graph.o

CXXFLAGS := ${CXXFLAGS_COMMON} ${INCLUDES} -fopenmp

TARGETS := libfrovedis_client_python.so

libfrovedis_client_python.so: ${OBJS} ${LIBS}
	${MPICXX} -fopenmp -shared -o libfrovedis_client_python.so -Wl,-whole-archive ${OBJS} ${LIBS} -Wl,-no-whole-archive -L../../../../../third_party/lib ${LIBS_COMMON}
	install -d ../../lib
	cp libfrovedis_client_python.so ../../lib

utility.o: utility.cc ${HEADERS}
exception_handling.o: exception_handling.cc ${HEADERS}
server_info.o: server_info.cc ${HEADERS}
dvector.o: dvector.cc ${HEADERS}
data_storage.o: data_storage.cc ${HEADERS}
wrapper.o: wrapper.cc ${HEADERS}
dataframe.o: dataframe.cc ${HEADERS}
model.o: model.cc ${HEADERS}
ml.o: ml.cc ${HEADERS}
graph.o: graph.cc ${HEADERS}

clean:
	${RM} ${TO_CLEAN} ${TARGETS} ${OBJS}

