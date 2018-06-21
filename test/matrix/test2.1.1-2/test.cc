#include <frovedis.hpp>
#include <frovedis/matrix/sliced_matrix.hpp>
#include <frovedis/matrix/blas_wrapper.hpp>

#define BOOST_TEST_MODULE FrovedisTest
#include <boost/test/unit_test.hpp>

using namespace frovedis;
using namespace std;

BOOST_AUTO_TEST_CASE( frovedis_test )
{
    int argc = 1;
    char** argv = NULL;
    use_frovedis use(argc, argv);

    // creating a colmajor matrix local from file
    colmajor_matrix_local<float> cm (
           make_rowmajor_matrix_local_load<float>("./sample_6x6"));

    // creating slice matrix
    auto sm = make_sliced_colmajor_matrix_local<float>(cm,1,1,4,4);

    // creating slices from slice matrix
    auto sm1 = make_sliced_colmajor_matrix_local<float>(sm,0,0,2,2);
    auto sm2 = make_sliced_colmajor_matrix_local<float>(sm,2,2,2,2);
    auto sm3 = make_sliced_colmajor_matrix_local<float>(sm,0,2,2,2);

    // <sm> top-right = top-left * bottom-right
    gemm<float>(sm1,sm2,sm3);
    cm.to_rowmajor().save("./out_6x6");

    // checking whether slices have been created properly and 
    // top-right sliced square is correctly modified with result
    BOOST_CHECK (system("diff ./out_6x6 ./ref_6x6") == 0);
    system("rm -f ./out_6x6");
}

