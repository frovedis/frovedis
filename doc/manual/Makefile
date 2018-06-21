include ../../Makefile.conf
include ../../Makefile.in.${TARGET}

all:
	cd core; make
	cd matrix; make
	cd ml; make
	cd spark/exrpc; make
	cd spark/matrix; make
	cd spark/ml; make
	cd python/matrix; make
	cd python/ml; make
	gs -dBATCH -dNOPAUSE -q -sDEVICE=pdfwrite -dPDFSETTINGS=/prepress -sOutputFile=manual.pdf core/dvector.pdf core/node_local.pdf core/dunordered_map.pdf matrix/rowmajor_matrix_local.pdf matrix/rowmajor_matrix.pdf matrix/colmajor_matrix_local.pdf matrix/colmajor_matrix.pdf matrix/sliced_colmajor_matrix_local.pdf matrix/sliced_colmajor_vector_local.pdf matrix/blas_wrapper.pdf matrix/lapack_wrapper.pdf matrix/blockcyclic_matrix.pdf  matrix/sliced_blockcyclic_matrix.pdf matrix/sliced_blockcyclic_vector.pdf matrix/pblas_wrapper.pdf matrix/scalapack_wrapper.pdf matrix/crs_matrix_local.pdf matrix/crs_matrix.pdf matrix/ccs_matrix_local.pdf matrix/ccs_matrix.pdf matrix/ell_matrix_local.pdf matrix/ell_matrix.pdf matrix/jds_matrix_local.pdf matrix/jds_matrix.pdf ml/linear_regression_model.pdf ml/linear_regression.pdf ml/lasso_regression.pdf ml/ridge_regression.pdf ml/logistic_regression_model.pdf ml/logistic_regression.pdf ml/svm_model.pdf ml/linear_svm.pdf ml/matrix_factorization_model.pdf ml/als.pdf ml/kmeans.pdf spark/exrpc/frovedis_sparse_data.pdf spark/matrix/blockcyclic_matrix.pdf spark/matrix/pblas_wrapper.pdf spark/matrix/scalapack_wrapper.pdf spark/matrix/arpack_wrapper.pdf spark/matrix/getrf_result.pdf spark/matrix/gesvd_result.pdf spark/ml/linear_regression_model.pdf spark/ml/linear_regression.pdf spark/ml/lasso_regression.pdf spark/ml/ridge_regression.pdf spark/ml/logistic_regression_model.pdf spark/ml/logistic_regression.pdf spark/ml/svm_model.pdf spark/ml/linear_svm.pdf spark/ml/matrix_factorization_model.pdf spark/ml/als.pdf spark/ml/kmeans_model.pdf spark/ml/kmeans.pdf python/matrix/dvector.pdf python/matrix/crs_matrix.pdf python/matrix/blockcyclic_matrix.pdf python/matrix/pblas_wrapper.pdf python/matrix/scalapack_wrapper.pdf python/matrix/arpack_wrapper.pdf python/matrix/getrf_result.pdf python/matrix/gesvd_result.pdf  python/ml/linear_regression.pdf python/ml/lasso_regression.pdf python/ml/ridge_regression.pdf python/ml/logistic_regression.pdf python/ml/linear_svm.pdf python/ml/als.pdf python/ml/kmeans.pdf


install:
	install -d ${INSTALLPATH}/doc/manual
	cp manual.pdf ${INSTALLPATH}/doc/manual
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
