include ../../../../../Makefile.in.x86
#include ../Makefile.in.x86

all: frov_df.mf 

append:
	echo ${SPARK_HOME}/jars/*:${HDPATH} > tmp
	sed -e "s/:/\n /g" < tmp > append
	rm tmp

frov_df.mf: frov_df.mf.tmpl append
	cp frov_df.mf.tmpl frov_df.mf
	cat append >> frov_df.mf

clean:
	rm -f *~ *.mf append
