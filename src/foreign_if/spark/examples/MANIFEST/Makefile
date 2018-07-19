include ../../../../../Makefile.in.x86
#include ../Makefile.in.x86

all: dtransfer.mf als.mf kmeans.mf lrm.mf lrsgd.mf frov_mat.mf frov_df.mf pblas_scalapack.mf arpack.mf

append:
	echo ${SPARK_HOME}/jars/*:${HDPATH} > tmp
	sed -e "s/:/\n /g" < tmp > append
	rm tmp

dtransfer.mf: dtransfer.mf.tmpl append
	cp dtransfer.mf.tmpl dtransfer.mf
	cat append >> dtransfer.mf

als.mf: als.mf.tmpl append
	cp als.mf.tmpl als.mf
	cat append >> als.mf

kmeans.mf: kmeans.mf.tmpl append
	cp kmeans.mf.tmpl kmeans.mf
	cat append >> kmeans.mf

lrm.mf: lrm.mf.tmpl append
	cp lrm.mf.tmpl lrm.mf
	cat append >> lrm.mf

lrsgd.mf: lrsgd.mf.tmpl append
	cp lrsgd.mf.tmpl lrsgd.mf
	cat append >> lrsgd.mf

frov_mat.mf: frov_mat.mf.tmpl append
	cp frov_mat.mf.tmpl frov_mat.mf
	cat append >> frov_mat.mf

frov_df.mf: frov_df.mf.tmpl append
	cp frov_df.mf.tmpl frov_df.mf
	cat append >> frov_df.mf

pblas_scalapack.mf: pblas_scalapack.mf.tmpl append
	cp pblas_scalapack.mf.tmpl pblas_scalapack.mf
	cat append >> pblas_scalapack.mf

arpack.mf: arpack.mf.tmpl append
	cp arpack.mf.tmpl arpack.mf
	cat append >> arpack.mf

clean:
	rm -f *~ *.mf append
