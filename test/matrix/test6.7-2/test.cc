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
  
    // slicing rows
    auto row1 = make_row_vector<float> (cm1,0);
    auto row2 = make_row_vector<float> (cm1,1);
    auto row3 = make_row_vector<float> (cm1,2);
    auto row4 = make_row_vector<float> (cm1,3);

    // row3-of-cm1 = 5*cm2*row1-of-cm1
    gemv<float>(cm2,row1,row3,'N',5.0);

    // row4-of-cm1 = 2*trans(cm2)*row2-of-cm1 + 3*row4-of-cm1
    gemv<float>(cm2,row2,row4,'T',2.0,3.0);

    double tol = 0.01;
    colmajor_matrix_local<float> ref (
           make_rowmajor_matrix_local_load<float> ("./ref_4x4"));
    BOOST_CHECK (calc_rms_err<float> (cm1.val, ref.val) < tol);
}

