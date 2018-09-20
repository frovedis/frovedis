include ../../../../../Makefile.in.x86
#include ../Makefile.in.x86

all: gtest.mf

append:
	echo ${SPARK_HOME}/jars/*:${HDPATH} > tmp
	sed -e "s/:/\n /g" < tmp > append
	rm tmp

gtest.mf: gtest.mf.tmpl append
	cp gtest.mf.tmpl gtest.mf
	cat append >> gtest.mf

clean:
	rm -f *~ *.mf append
