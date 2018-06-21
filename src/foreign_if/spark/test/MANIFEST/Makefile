include ../../../../../Makefile.in.x86
#include ../Makefile.in.x86

all: lr_frovedis.mf svm_frovedis.mf lnr_frovedis.mf lasso_frovedis.mf ridge_frovedis.mf lr_spark.mf svm_spark.mf lnr_spark.mf lasso_spark.mf ridge_spark.mf als_frovedis.mf als_spark.mf kmeans_frovedis.mf kmeans_spark.mf

append:
	echo ${SPARK_HOME}/jars/*:${HDPATH} > tmp
	sed -e "s/:/\n /g" < tmp > append
	rm tmp

lr_frovedis.mf: lr_frovedis.mf.tmpl append
	cp lr_frovedis.mf.tmpl lr_frovedis.mf
	cat append >> lr_frovedis.mf

lr_spark.mf: lr_spark.mf.tmpl append
	cp lr_spark.mf.tmpl lr_spark.mf
	cat append >> lr_spark.mf

svm_frovedis.mf: svm_frovedis.mf.tmpl append
	cp svm_frovedis.mf.tmpl svm_frovedis.mf
	cat append >> svm_frovedis.mf

svm_spark.mf: svm_spark.mf.tmpl append
	cp svm_spark.mf.tmpl svm_spark.mf
	cat append >> svm_spark.mf

lnr_frovedis.mf: lnr_frovedis.mf.tmpl append
	cp lnr_frovedis.mf.tmpl lnr_frovedis.mf
	cat append >> lnr_frovedis.mf

lnr_spark.mf: lnr_spark.mf.tmpl append
	cp lnr_spark.mf.tmpl lnr_spark.mf
	cat append >> lnr_spark.mf

lasso_frovedis.mf: lasso_frovedis.mf.tmpl append
	cp lasso_frovedis.mf.tmpl lasso_frovedis.mf
	cat append >> lasso_frovedis.mf

lasso_spark.mf: lasso_spark.mf.tmpl append
	cp lasso_spark.mf.tmpl lasso_spark.mf
	cat append >> lasso_spark.mf

ridge_frovedis.mf: ridge_frovedis.mf.tmpl append
	cp ridge_frovedis.mf.tmpl ridge_frovedis.mf
	cat append >> ridge_frovedis.mf

ridge_spark.mf: ridge_spark.mf.tmpl append
	cp ridge_spark.mf.tmpl ridge_spark.mf
	cat append >> ridge_spark.mf

als_frovedis.mf: als_frovedis.mf.tmpl append
	cp als_frovedis.mf.tmpl als_frovedis.mf
	cat append >> als_frovedis.mf

als_spark.mf: als_spark.mf.tmpl append
	cp als_spark.mf.tmpl als_spark.mf
	cat append >> als_spark.mf

kmeans_frovedis.mf: kmeans_frovedis.mf.tmpl append
	cp kmeans_frovedis.mf.tmpl kmeans_frovedis.mf
	cat append >> kmeans_frovedis.mf

kmeans_spark.mf: kmeans_spark.mf.tmpl append
	cp kmeans_spark.mf.tmpl kmeans_spark.mf
	cat append >> kmeans_spark.mf

clean:
	rm -f *~ *.mf append
