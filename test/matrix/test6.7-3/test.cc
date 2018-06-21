#include <frovedis.hpp>
#include <frovedis/matrix/blas_wrapper.hpp>

#define BOOST_TEST_MODULE FrovedisTest
#include <boost/test/unit_test.hpp>
#include "../../rmse.hpp"

using namespace frovedis;
using namespace std;

BOOST_AUTO_TEST_CASE( frovedis_test )
{
    int argc = 1;
    char** argv = NULL;
    use_frovedis use(argc, argv);

    // creating a colmajor matrix local from file
    colmajor_matrix_local<float> cm1 (
    make_rowmajor_matrix_local_load<float>("./sample_3x4"));
    auto cm2 = cm1;
  
    // slicing row and col
    auto row1 = make_row_vector<float> (cm1,0); // (4x1)
    auto col1 = make_col_vector<float> (cm1,0); // (3x1)

    // col1-of-cm1 = cm2*row1-of-cm1
    gemv<float>(cm2,row1,col1); // (3x4) * (4x1) => (OK)

    bool isError = false;
    // row1-of-cm1 = cm2*col1-of-cm1
    try {
      gemv<float>(cm2,col1,row1); // (3x4) * (3x1) => (Error)
    }
    catch (exception& e) {
      isError = true;
    }
    BOOST_CHECK (isError);

    // row1-of-cm1 = trans(cm2)*col1-of-cm1
    gemv<float>(cm2,col1,row1,'T'); // (4x3) * (3x1) => (OK)

    double tol = 0.01;
    colmajor_matrix_local<float> ref (
    make_rowmajor_matrix_local_load<float> ("./ref_3x4"));
    BOOST_CHECK (calc_rms_err<float> (cm1.val, ref.val) < tol);
}

