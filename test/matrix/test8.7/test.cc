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
    auto v1 = make_blockcyclic_matrix_load<float> ("./sample_4x1");
    blockcyclic_matrix<float> v2(4,1); // output

    // v2 = bm1 * v1
    gemv<float> (bm1,v1,v2);
    v2.save("./out");

    double tol = 0.01;
    std::vector<float> e_out = {1.0, 32.0, 5.0, 4.0};
    auto m = make_rowmajor_matrix_local_load<float> ("./out");
    //for(auto &i: m.val) cout << i << " "; cout << endl;
    BOOST_CHECK (calc_rms_err<float> (m.val, e_out) < tol);
    system("rm -f ./out");
}

