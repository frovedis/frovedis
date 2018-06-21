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

    // creating slice blockcyclic matrix from entire matrix
    sliced_blockcyclic_matrix<float> sm (bm);

    // scaling entire slice with 2
    auto col = make_col_vector(sm,0);
    scal<float> (col, 2);

    // scaling entire slice with 2
    auto row = make_row_vector(sm,3);
    scal<float> (row, 5);

    // checking whether operation can be done on sliced matrix correctly
    bm.save("./out_4x4");
    BOOST_CHECK (system("diff ./out_4x4 ./ref_4x4") == 0);
    system("rm -f ./out_4x4");
}

