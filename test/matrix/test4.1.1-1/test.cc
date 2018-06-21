#include <frovedis.hpp>
#include <frovedis/matrix/sliced_matrix.hpp>
#include <frovedis/matrix/pblas_wrapper.hpp>

#define BOOST_TEST_MODULE FrovedisTest
#include <boost/test/unit_test.hpp>

using namespace frovedis;
using namespace std;

BOOST_AUTO_TEST_CASE( frovedis_test )
{
    int argc = 1;
    char** argv = NULL;
    use_frovedis use(argc, argv);

    auto bm = make_blockcyclic_matrix_load<float> ("./sample_4x4");

    // creating slices from above blockcyclic matrix
    auto sm1 = make_sliced_blockcyclic_matrix<float>(bm,0,0,2,2);
    auto sm2 = make_sliced_blockcyclic_matrix<float>(bm,2,2,2,2);
    auto sm3 = make_sliced_blockcyclic_matrix<float>(bm,0,2,2,2);

    // <bm> top-right = top-left * bottom-right
    gemm<float>(sm1,sm2,sm3);
    bm.save("./out_4x4");

    // checking whether slices have been created properly and 
    // top-right sliced square is correctly modified with result
    BOOST_CHECK (system("diff ./out_4x4 ./ref_4x4") == 0);
    system("rm -f ./out_4x4");
}

