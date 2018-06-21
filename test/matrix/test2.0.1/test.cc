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

    std::vector<float> v_in = {1,2,3,4};
    std::vector<float> v_out = {2,4,6,8};

    // creating slice colmajor matrix from entire vector
    sliced_colmajor_matrix_local<float> sm (v_in);

    // scaling entire slice with 2
    auto col = make_col_vector(sm,0);
    scal<float> (col, 2);

    // if slice was correctly created, this check should pass
    BOOST_CHECK (v_in == v_out);
}

