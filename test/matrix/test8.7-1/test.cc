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
    auto bm1 = make_blockcyclic_matrix_load<float> ("./sample_4x4");
    auto bm2 = bm1;

    // slicing rows and cols
    auto row1 = make_row_vector<float> (bm1,0);
    auto row2 = make_row_vector<float> (bm1,1);
    auto col1 = make_col_vector<float> (bm1,0);
    auto col2 = make_col_vector<float> (bm1,1);

    // updating row2 and col2 of bm1, using gemv (matrix-vector multiplication)
    gemv<float>(bm2,row1,row2); // row2-of-bm1 = bm2 * row1-of-bm1
    gemv<float>(bm2,col1,col2); // col2-of-bm1 = bm2 * col1-of-bm1
    bm1.save("./out");

    double tol = 0.01;
    auto out = make_rowmajor_matrix_local_load<float> ("./out");
    auto ref = make_rowmajor_matrix_local_load<float> ("./ref_4x4");
    BOOST_CHECK (calc_rms_err<float> (out.val, ref.val) < tol);
    system("rm -f ./out");
}

