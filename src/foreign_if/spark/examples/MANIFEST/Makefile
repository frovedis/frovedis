include ../../../../../Makefile.in.x86
#include ../Makefile.in.x86

all: dtransfer.mf frov_mat.mf frov_df.mf pblas_scalapack.mf svd.mf pca.mf als.mf kmeans.mf lrm.mf lrsgd.mf nb.mf fm.mf dt.mf fp.mf ac.mf se.mf sc.mf w2v.mf knn.mf knc.mf knr.mf page_rank.mf sssp.mf bfs.mf cc.mf dbscan.mf lda.mf rf.mf gbt.mf svr.mf

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

fp.mf: fp.mf.tmpl append
	cp fp.mf.tmpl fp.mf
	cat append >> fp.mf

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

knn.mf: knn.mf.tmpl append
	cp knn.mf.tmpl knn.mf
	cat append >> knn.mf

knc.mf: knc.mf.tmpl append
	cp knc.mf.tmpl knc.mf
	cat append >> knc.mf

knr.mf: knr.mf.tmpl append
	cp knr.mf.tmpl knr.mf
	cat append >> knr.mf

ac.mf: ac.mf.tmpl append
	cp ac.mf.tmpl ac.mf
	cat append >> ac.mf

se.mf: se.mf.tmpl append
	cp se.mf.tmpl se.mf
	cat append >> se.mf

sc.mf: sc.mf.tmpl append
	cp sc.mf.tmpl sc.mf
	cat append >> sc.mf

w2v.mf: w2v.mf.tmpl append
	cp w2v.mf.tmpl w2v.mf
	cat append >> w2v.mf

dbscan.mf: dbscan.mf.tmpl append
	cp dbscan.mf.tmpl dbscan.mf
	cat append >> dbscan.mf

page_rank.mf: page_rank.mf.tmpl append
	cp page_rank.mf.tmpl page_rank.mf
	cat append >> page_rank.mf

sssp.mf: sssp.mf.tmpl append
	cp sssp.mf.tmpl sssp.mf
	cat append >> sssp.mf

bfs.mf: bfs.mf.tmpl append
	cp bfs.mf.tmpl bfs.mf
	cat append >> bfs.mf

cc.mf: cc.mf.tmpl append
	cp cc.mf.tmpl cc.mf
	cat append >> cc.mf

lda.mf: lda.mf.tmpl append
	cp lda.mf.tmpl lda.mf
	cat append >> lda.mf

rf.mf: rf.mf.tmpl append
	cp rf.mf.tmpl rf.mf
	cat append >> rf.mf

gbt.mf: gbt.mf.tmpl append
	cp gbt.mf.tmpl gbt.mf
	cat append >> gbt.mf

svr.mf: svr.mf.tmpl append
	cp svr.mf.tmpl svr.mf
	cat append >> svr.mf

clean:
	rm -f *~ *.mf append
