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

    colmajor_matrix_local<float> cm (
             make_rowmajor_matrix_local_load<float>("./sample_4x1"));

    // creating slice colmajor vector from entire "colmajor matrix with single column"
    sliced_colmajor_vector_local<float> sv (cm);

    // scaling entire slice with 2
    scal<float> (sv, 2);
    cm.to_rowmajor().save("./out_4x1");

    // if slice was correctly created, this check should pass
    BOOST_CHECK (system("diff ./out_4x1 ref_4x1") == 0);
    system("rm -f ./out_4x1");
}

