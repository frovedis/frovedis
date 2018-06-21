#include <frovedis.hpp>
#include <frovedis/matrix/pblas_wrapper.hpp>

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

    // creating blockcyclic matrix from file
    auto bm1 = make_blockcyclic_matrix_load<float> ("./sample_3x4"); // (3x4)
    auto bm2 = bm1;     // (3x4)
 
    // slicing row and col
    auto row1 = make_row_vector<float> (bm1,0); // (4x1)
    auto col1 = make_col_vector<float> (bm1,0); // (3x1)

    // col1-of-bm1 = bm2*row1-of-bm1
    gemv<float>(bm2,row1,col1); // (3x4) * (4x1) => (OK)

    bool isError = false;
    // row1-of-bm1 = bm2*col1-of-bm1
    try {
      gemv<float>(bm2,col1,row1); // (3x4) * (3x1) => (Error)
    }
    catch (exception& e) {
      isError = true;
    }
    BOOST_CHECK (isError);

    // row1-of-bm1 = trans(bm2)*col1-of-bm1
    gemv<float>(bm2,col1,row1,'T'); // (4x3) * (3x1) => (OK)
    bm1.save("./out");

    double tol = 0.01;
    auto out = make_rowmajor_matrix_local_load<float> ("./out");
    auto ref = make_rowmajor_matrix_local_load<float> ("./ref_3x4");
    BOOST_CHECK (calc_rms_err<float> (out.val, ref.val) < tol);
    system("rm -f ./out");
}

