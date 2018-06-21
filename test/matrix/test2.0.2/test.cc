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

    colmajor_matrix_local<float> cm (
             make_rowmajor_matrix_local_load<float>("./sample_4x4"));

    // creating slice colmajor matrix from entire matrix
    sliced_colmajor_matrix_local<float> sm (cm);

    // scaling entire slice with 2
    auto col = make_col_vector(sm,0);
    scal<float> (col, 2);

    // scaling entire slice with 2
    auto row = make_row_vector(sm,3);
    scal<float> (row, 5);

    // checking whether operation can be done on sliced matrix correctly
    cm.to_rowmajor().save("./out_4x4");
    BOOST_CHECK (system("diff ./out_4x4 ./ref_4x4") == 0);
    system("rm -f ./out_4x4");
}

