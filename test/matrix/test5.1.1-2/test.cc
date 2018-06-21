#include <frovedis.hpp>
#include <frovedis/matrix/sliced_vector.hpp>
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

    auto bm = make_blockcyclic_matrix_load<float> ("./sample_6x6");

    // creating slice matrix from blockcyclic matrix
    auto sm = make_sliced_blockcyclic_matrix<float>(bm,1,1,4,4);

    // creating slice vectors from slice matrix
    auto row = make_row_vector<float>(sm,3);
    auto col = make_col_vector<float>(sm,0);

    // scaling the slices
    scal<float> (row, 5);
    scal<float> (col, 2);
    bm.save("./out_6x6");

    // if slice was correctly created, this check should pass
    BOOST_CHECK (system("diff ./out_6x6 ref_6x6") == 0);
    system("rm -f ./out_6x6");
}

