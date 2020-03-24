include ../../../../Makefile.in.x86

TARGETS := client server

LIBS := -L.. -lfrovedis_exrpc -L../../../frovedis/core -lfrovedis_core -L../../../../third_party/lib ${LIBS_COMMON}
INCLUDES := -I.. -I../../../ -I../../../../third_party/cereal-1.2.2/include/ -I../../../../third_party/yas-7.0.5/include/

CXXFLAGS := ${CXXFLAGS_COMMON} ${INCLUDES}

all: ${TARGETS}

server: server.o functions.o 
	${MPICXX} ${CXXFLAGS_COMMON} -o server server.o functions.o ${LIBS}

client: client.o functions.o
	${MPICXX} ${CXXFLAGS_COMMON} -o client client.o functions.o ${LIBS}

functions.o: functions.cc functions.hpp

clean:
	${RM} *.o *~ ${TARGETS}
