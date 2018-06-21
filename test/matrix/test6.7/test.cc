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
  
    // input-output vectors
    std::vector<float> v1 = {1, 2, 3, 4}; //input
    std::vector<float> v2(4); //output

    // v2 = cm1 * v1
    gemv<float> (cm1,v1,v2);
    //for(auto &i: v2) cout << i << " "; cout << endl;

    double tol = 0.01;
    std::vector<float> e_out = {1.0, 32.0, 5.0, 4.0};
    BOOST_CHECK (calc_rms_err<float> (v2, e_out) < tol);
}

