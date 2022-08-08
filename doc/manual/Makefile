include ../../Makefile.conf
include ../../Makefile.in.${TARGET}

CPPMAN := core/dvector.md core/node_local.md core/dunordered_map.md matrix/rowmajor_matrix_local.md matrix/rowmajor_matrix.md matrix/colmajor_matrix_local.md matrix/colmajor_matrix.md matrix/sliced_colmajor_matrix_local.md matrix/sliced_colmajor_vector_local.md matrix/blas_wrapper.md matrix/lapack_wrapper.md matrix/blockcyclic_matrix.md  matrix/sliced_blockcyclic_matrix.md matrix/sliced_blockcyclic_vector.md matrix/pblas_wrapper.md matrix/scalapack_wrapper.md matrix/crs_matrix_local.md matrix/crs_matrix.md matrix/ccs_matrix_local.md matrix/ccs_matrix.md matrix/ell_matrix_local.md matrix/ell_matrix.md matrix/jds_matrix_local.md matrix/jds_matrix.md ml/linear_regression_model.md ml/linear_regression.md ml/lasso_regression.md ml/ridge_regression.md ml/logistic_regression_model.md ml/logistic_regression.md ml/svm_model.md ml/linear_svm.md ml/matrix_factorization_model.md ml/als.md ml/kmeans.md ml/spectral_clustering_model.md ml/spectral_clustering.md ml/spectral_embedding_model.md ml/spectral_embedding.md ml/arima.md

SPARKMAN := spark/exrpc/frovedis_sparse_data.md spark/matrix/blockcyclic_matrix.md spark/matrix/pblas_wrapper.md spark/matrix/scalapack_wrapper.md spark/matrix/getrf_result.md spark/matrix/gesvd_result.md spark/ml/linear_regression_model.md spark/ml/linear_regression.md spark/ml/lasso_regression.md spark/ml/ridge_regression.md spark/ml/logistic_regression_model.md spark/ml/logistic_regression.md spark/ml/svm_model.md spark/ml/linear_svm.md spark/ml/matrix_factorization_model.md spark/ml/als.md spark/ml/kmeans_model.md spark/ml/kmeans.md spark/ml/arima.md spark/ml/arima_model.md

PYTHONMAN := python/ml/agglomerative_clustering.md python/ml/als.md python/ml/arima.md python/ml/bernoulli_naive_bayes.md python/ml/dbscan.md python/ml/decision_tree_classifier.md python/ml/decision_tree_regressor.md python/ml/fm_classifier.md python/ml/fm_regressor.md python/ml/fpgrowth.md python/ml/gaussian_mixture.md python/ml/gradient_boosting_classifier.md python/ml/gradient_boosting_regressor.md python/ml/kmeans.md python/ml/kneighbors_classifier.md python/ml/kneighbors_regressor.md python/ml/lasso_regression.md python/ml/lda.md python/ml/linear_regression.md python/ml/linear_svc.md python/ml/linear_svr.md python/ml/logistic_regression.md python/ml/multinomial_naive_bayes.md python/ml/nearest_neighbors.md python/ml/pca.md python/ml/random_forest_classifier.md python/ml/random_forest_regressor.md python/ml/ridge_regression.md python/ml/sgd_classifier.md python/ml/sgd_regressor.md python/ml/spectral_clustering.md python/ml/spectral_embedding.md python/ml/standard_scaler.md python/ml/svc.md python/ml/tsne.md python/ml/word2vec.md python/linalg/linalg.md python/linalg/scalapack.md python/graph/graph.md python/graph/pagerank.md python/graph/bfs.md python/graph/sssp.md python/graph/connected_components.md python/matrix/dvector.md python/matrix/crs_matrix.md python/matrix/blockcyclic_matrix.md python/matrix/pblas_wrapper.md python/matrix/scalapack_wrapper.md python/matrix/getrf_result.md python/matrix/gesvd_result.md python/dataframe/df_intro.md python/dataframe/df_indexing_operations.md python/dataframe/df_generic_func.md python/dataframe/df_conversion.md python/dataframe/df_sort.md python/dataframe/df_agg_func.md python/dataframe/df_math_func.md python/dataframe/grouped_df_intro.md python/dataframe/grouped_df_agg_func.md

all:
	cd core; make
	cd matrix; make
	cd ml; make
	(cp intro_cpp.md manual_cpp.md; for file in ${CPPMAN}; do sed -e 's/^\#/\#\#/' < $$file >> manual_cpp.md; done; pandoc -N --toc --toc-depth=1 -V documentclass=book -V geometry:margin=1in -o manual_cpp.pdf manual_cpp.md)
	(cp intro_cpp.md manual_cpp.md; for file in ${CPPMAN}; do sed -e 's/^\#/\#\#/' < $$file > tmp; sed -e 's/^%\(.*\)/\n\n\#\1/' < tmp >> manual_cpp.md; done; pandoc -N --toc --toc-depth=1 -V documentclass=book -V geometry:margin=1in -o manual_cpp.pdf manual_cpp.md; rm tmp)
	cd spark/exrpc; make
	cd spark/matrix; make
	cd spark/ml; make
	(cp intro_spark.md manual_spark.md; for file in ${SPARKMAN}; do sed -e 's/^\#/\#\#/' < $$file > tmp; sed -e 's/^%\(.*\)/\n\n\#\1/' < tmp >> manual_spark.md; done; pandoc -N --toc --toc-depth=1 -V documentclass=book -V geometry:margin=1in -o manual_spark.pdf manual_spark.md; rm tmp)
	cd python/ml; make
	cd python/linalg; make
	cd python/graph; make
	cd python/matrix; make
	cd python/dataframe; make
	(cp intro_python.md manual_python.md; for file in ${PYTHONMAN}; do sed -e 's/^\#/\#\#/' < $$file > tmp; sed -e 's/^%\(.*\)/\n\n\#\1/' < tmp >> manual_python.md; done; pandoc -N --toc --toc-depth=1 -V documentclass=book -V geometry:margin=1in -o manual_python.pdf manual_python.md; rm tmp)

install:
	install -d ${INSTALLPATH}/doc/manual
	cp manual_cpp.pdf manual_spark.pdf manual_python.pdf ${INSTALLPATH}/doc/manual
	install -d ${INSTALLPATH}/man/man3
	install -d ${INSTALLPATH}/man/man3s
	install -d ${INSTALLPATH}/man/man3p
	cp core/*.3 ${INSTALLPATH}/man/man3
	cp matrix/*.3 ${INSTALLPATH}/man/man3
	cp ml/*.3 ${INSTALLPATH}/man/man3
	cp spark/exrpc/*.3s ${INSTALLPATH}/man/man3s
	cp spark/matrix/*.3s ${INSTALLPATH}/man/man3s
	cp spark/ml/*.3s ${INSTALLPATH}/man/man3s
	cp python/matrix/*.3p ${INSTALLPATH}/man/man3p
	cp python/ml/*.3p ${INSTALLPATH}/man/man3p
