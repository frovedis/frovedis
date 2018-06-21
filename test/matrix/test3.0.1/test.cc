#include <frovedis.hpp>
#include <frovedis/matrix/sliced_vector.hpp>
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

    // creating slice colmajor vector from entire std::vector
    sliced_colmajor_vector_local<float> sv (v_in);

    // scaling entire slice with 2
    scal<float> (sv, 2);

    // if slice was correctly created, this check should pass
    BOOST_CHECK (v_in == v_out);
}

