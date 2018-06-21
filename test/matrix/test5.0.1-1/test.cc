#include <frovedis.hpp>
#include <frovedis/matrix/sliced_vector.hpp>
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

    auto bm = make_blockcyclic_matrix_load<float> ("./sample_4x1");

    // creating slice blockcyclic vector from entire "blockcyclic matrix with single column"
    sliced_blockcyclic_vector<float> sv (bm);

    // scaling entire slice with 2
    scal<float> (sv, 2);
    bm.save("./out_4x1");

    // if slice was correctly created, this check should pass
    BOOST_CHECK (system("diff ./out_4x1 ref_4x1") == 0);
    system("rm -f ./out_4x1");
}

