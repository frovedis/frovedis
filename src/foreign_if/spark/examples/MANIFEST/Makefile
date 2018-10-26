include ../../../../../Makefile.in.x86
#include ../Makefile.in.x86

all: dtransfer.mf frov_mat.mf frov_df.mf pblas_scalapack.mf svd.mf pca.mf als.mf kmeans.mf lrm.mf lrsgd.mf nb.mf fm.mf dt.mf 

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

nb.mf: nb.mf.tmpl append
	cp nb.mf.tmpl nb.mf
	cat append >> nb.mf

fm.mf: fm.mf.tmpl append
	cp fm.mf.tmpl fm.mf
	cat append >> fm.mf

dt.mf: dt.mf.tmpl append
	cp dt.mf.tmpl dt.mf
	cat append >> dt.mf

frov_mat.mf: frov_mat.mf.tmpl append
	cp frov_mat.mf.tmpl frov_mat.mf
	cat append >> frov_mat.mf

frov_df.mf: frov_df.mf.tmpl append
	cp frov_df.mf.tmpl frov_df.mf
	cat append >> frov_df.mf

pblas_scalapack.mf: pblas_scalapack.mf.tmpl append
	cp pblas_scalapack.mf.tmpl pblas_scalapack.mf
	cat append >> pblas_scalapack.mf

svd.mf: svd.mf.tmpl append
	cp svd.mf.tmpl svd.mf
	cat append >> svd.mf

pca.mf: pca.mf.tmpl append
	cp pca.mf.tmpl pca.mf
	cat append >> pca.mf

clean:
	rm -f *~ *.mf append
