include ../Makefile.conf
include ../Makefile.in.${TARGET}

all:
	cd svd; make -f Makefile.${TARGET} all
	cd pca; make -f Makefile.${TARGET} all
	cd glm; make -f Makefile.${TARGET} all
	cd knn; make -f Makefile.${TARGET} all
	cd kmeans; make -f Makefile.${TARGET} all
	cd agglomerative; make -f Makefile.${TARGET} all
	cd spectral; make -f Makefile.${TARGET} all
	cd dbscan; make -f Makefile.${TARGET} all
	cd document_matrix; make -f Makefile.${TARGET} all
	cd matrixfactorization; make -f Makefile.${TARGET} all
	cd factorization_machine; make -f Makefile.${TARGET} all
	cd eigen; make -f Makefile.${TARGET} all
	cd tree; make -f Makefile.${TARGET} all
	cd forest; make -f Makefile.${TARGET} all
	cd gbts; make -f Makefile.${TARGET} all
	cd graph; make -f Makefile.${TARGET} all
	cd word2vec; make -f Makefile.${TARGET}
	cd fp_growth; make -f Makefile.${TARGET} all
	cd lda; make -f Makefile.${TARGET} all
	cd nb; make -f Makefile.${TARGET} all
	cd tsne; make -f Makefile.${TARGET} all
	cd GridSearchCV; make -f Makefile.${TARGET} all
	cd gmm; make -f Makefile.${TARGET} all
	cd agglomerative_newapi; make -f Makefile.${TARGET} all
	cd arima; make -f Makefile.${TARGET} all
	cd glm_newapi; make -f Makefile.${TARGET} all
	cd kmeans_newapi; make -f Makefile.${TARGET} all
	cd spectral_embedding; make -f Makefile.${TARGET} all
	cd spectral_newapi; make -f Makefile.${TARGET} all
	cd standard_scaler; make -f Makefile.${TARGET} all
	cd vector_matrix_operations; make -f Makefile.${TARGET} all
ifeq (${TARGET},x86)
	cd matrix_tools; make -f Makefile.${TARGET}
	cd wikipedia2matrix; make -f Makefile.${TARGET}
	cd dataframe_tools; make -f Makefile.${TARGET}
endif

install:
	install -d ${INSTALLPATH}/samples/
	cat Makefile.common | sed -e /^LIBS/d | sed -e /^INCLUDES/d |sed -e s/^\#LIBS/LIBS/ |  sed -e s/^#INCLUDES/INCLUDES/g | sed -e /^install/,'$$'d > tmp; install tmp ${INSTALLPATH}/samples/Makefile.common; rm tmp
	install Makefile.in.${TARGET} ${INSTALLPATH}/samples/
	cd svd; make -f Makefile.${TARGET} install
	cd pca; make -f Makefile.${TARGET} install
	cd glm; make -f Makefile.${TARGET} install
	cd knn; make -f Makefile.${TARGET} install
	cd kmeans; make -f Makefile.${TARGET} install
	cd agglomerative; make -f Makefile.${TARGET} install
	cd spectral; make -f Makefile.${TARGET} install
	cd dbscan; make -f Makefile.${TARGET} install
	cd document_matrix; make -f Makefile.${TARGET} install
	cd matrixfactorization; make -f Makefile.${TARGET} install
	cd factorization_machine; make -f Makefile.${TARGET} install
	cd eigen; make -f Makefile.${TARGET} install
	cd tree; make -f Makefile.${TARGET} install
	cd forest; make -f Makefile.${TARGET} install
	cd gbts; make -f Makefile.${TARGET} install
	cd graph; make -f Makefile.${TARGET} install
	cd word2vec; make -f Makefile.${TARGET} install
	cd fp_growth; make -f Makefile.${TARGET} install
	cd lda; make -f Makefile.${TARGET} install
	cd nb; make -f Makefile.${TARGET} install
	cd tsne; make -f Makefile.${TARGET} install
	cd GridSearchCV; make -f Makefile.${TARGET} install
	cd gmm; make -f Makefile.${TARGET} install
	cd agglomerative_newapi; make -f Makefile.${TARGET} install
	cd arima; make -f Makefile.${TARGET} install
	cd glm_newapi; make -f Makefile.${TARGET} install
	cd kmeans_newapi; make -f Makefile.${TARGET} install
	cd spectral_embedding; make -f Makefile.${TARGET} install
	cd spectral_newapi; make -f Makefile.${TARGET} install
	cd standard_scaler; make -f Makefile.${TARGET} install
	cd vector_matrix_operations; make -f Makefile.${TARGET} install
ifeq (${TARGET},x86)
	cd matrix_tools; make -f Makefile.${TARGET} install
	cd wikipedia2matrix; make -f Makefile.${TARGET} install
	cd dataframe_tools; make -f Makefile.${TARGET} install
endif

clean:
	cd svd; make -f Makefile.${TARGET} clean
	cd pca; make -f Makefile.${TARGET} clean
	cd glm; make -f Makefile.${TARGET} clean
	cd knn; make -f Makefile.${TARGET} clean
	cd kmeans; make -f Makefile.${TARGET} clean
	cd agglomerative; make -f Makefile.${TARGET} clean
	cd spectral; make -f Makefile.${TARGET} clean
	cd dbscan; make -f Makefile.${TARGET} clean
	cd document_matrix; make -f Makefile.${TARGET} clean
	cd matrixfactorization; make -f Makefile.${TARGET} clean
	cd factorization_machine; make -f Makefile.${TARGET} clean
	cd eigen; make -f Makefile.${TARGET} clean
	cd tree; make -f Makefile.${TARGET} clean
	cd forest; make -f Makefile.${TARGET} clean
	cd gbts; make -f Makefile.${TARGET} clean
	cd graph; make -f Makefile.${TARGET} clean
	cd word2vec; make -f Makefile.${TARGET} clean
	cd fp_growth; make -f Makefile.${TARGET} clean
	cd lda; make -f Makefile.${TARGET} clean
	cd nb; make -f Makefile.${TARGET} clean
	cd tsne; make -f Makefile.${TARGET} clean
	cd GridSearchCV; make -f Makefile.${TARGET} clean
	cd gmm; make -f Makefile.${TARGET} clean
	cd agglomerative_newapi; make -f Makefile.${TARGET} clean
	cd arima; make -f Makefile.${TARGET} clean
	cd glm_newapi; make -f Makefile.${TARGET} clean
	cd kmeans_newapi; make -f Makefile.${TARGET} clean
	cd spectral_embedding; make -f Makefile.${TARGET} clean
	cd spectral_newapi; make -f Makefile.${TARGET} clean
	cd standard_scaler; make -f Makefile.${TARGET} clean
	cd vector_matrix_operations; make -f Makefile.${TARGET} clean
ifeq (${TARGET},x86)
	cd matrix_tools; make -f Makefile.${TARGET} clean
	cd wikipedia2matrix; make -f Makefile.${TARGET} clean
	cd dataframe_tools; make -f Makefile.${TARGET} clean
endif
	${RM} *~
