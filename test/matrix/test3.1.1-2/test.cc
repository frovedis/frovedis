#include <frovedis.hpp>
#include <frovedis/matrix/sliced_vector.hpp>
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
             make_rowmajor_matrix_local_load<float>("./sample_6x6"));

    // creating slice matrix from colmajor matrix
    auto sm = make_sliced_colmajor_matrix_local<float>(cm,1,1,4,4);

    // creating slice vectors from slice matrix
    auto row = make_row_vector<float>(sm,3);
    auto col = make_col_vector<float>(sm,0);

    // scaling the slices
    scal<float> (row, 5);
    scal<float> (col, 2);
    cm.to_rowmajor().save("./out_6x6");

    // if slice was correctly created, this check should pass
    BOOST_CHECK (system("diff ./out_6x6 ref_6x6") == 0);
    system("rm -f ./out_6x6");
}

