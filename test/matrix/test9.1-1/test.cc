#include <frovedis.hpp>
#include <frovedis/matrix/pblas_wrapper.hpp>
#include <frovedis/matrix/scalapack_wrapper.hpp>


#define BOOST_TEST_MODULE FrovedisTest
#include <boost/test/unit_test.hpp>

using namespace frovedis;
using namespace std;

BOOST_AUTO_TEST_CASE( frovedis_test )
{
    int argc = 1;
    char** argv = NULL;
    use_frovedis use(argc, argv);

    auto bm = make_blockcyclic_matrix_load<float> ("./sample_3x3");
    auto inv_bm = inv(bm);

    frovedis::lvec<int> ipiv; // empty ipiv array
    getrf<float> (bm,ipiv); // bm will be factorized and ipiv will contain pivoting info
    getri<float> (bm,ipiv); // bm will be overwritten with inversed matrix

    // checking whether the above operations successfully taken place 
    bm.save("./out_3x3_1");
    inv_bm.save("./out_3x3_2");
    BOOST_CHECK (system("diff ./out_3x3_1 ./ref_3x3") == 0);
    BOOST_CHECK (system("diff ./out_3x3_2 ./ref_3x3") == 0);
    system("rm -f ./out_3x3_1");
    system("rm -f ./out_3x3_2");
}

