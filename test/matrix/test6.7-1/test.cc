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
           make_rowmajor_matrix_local_load<float>("./sample_4x4"));
    auto cm2 = cm1;
  
    // slicing rows and cols
    auto row1 = make_row_vector<float> (cm1,0);
    auto row2 = make_row_vector<float> (cm1,1);
    auto col1 = make_col_vector<float> (cm1,0);
    auto col2 = make_col_vector<float> (cm1,1);

    // updating row2 and col2 of cm1, using gemv (matrix-vector multiplication)
    gemv<float>(cm2,row1,row2); // row2-of-cm1 = cm2 * row1-of-cm1
    gemv<float>(cm2,col1,col2); // col2-of-cm1 = cm2 * col1-of-cm1

    double tol = 0.01;
    colmajor_matrix_local<float> ref (
           make_rowmajor_matrix_local_load<float> ("./ref_4x4"));
    BOOST_CHECK (calc_rms_err<float> (cm1.val, ref.val) < tol);
}

