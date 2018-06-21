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
           make_rowmajor_matrix_local_load<float>("./sample_4x4"));

    // creating slices from matrix
    auto sm1 = make_sliced_colmajor_matrix_local<float>(cm,0,0,2,2);
    auto sm2 = make_sliced_colmajor_matrix_local<float>(cm,2,2,2,2);
    auto sm3 = make_sliced_colmajor_matrix_local<float>(cm,0,2,2,2);

    // <cm> top-right = top-left * bottom-right
    gemm<float>(sm1,sm2,sm3);
    cm.to_rowmajor().save("./out_4x4");

    // checking whether slices have been created properly and 
    // top-right sliced square is correctly modified with result
    BOOST_CHECK (system("diff ./out_4x4 ./ref_4x4") == 0);
    system("rm -f ./out_4x4");
}

