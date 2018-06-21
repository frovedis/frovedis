#include <frovedis.hpp>
#include <frovedis/matrix/blas_wrapper.hpp>
#include <frovedis/matrix/lapack_wrapper.hpp>


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
           make_rowmajor_matrix_local_load<float>("./sample_3x3"));
    auto inv_cm = inv(cm);

    std::vector<int> ipiv; // empty ipiv array
    getrf<float> (cm,ipiv); // cm will be factorized and ipiv will contain pivoting info
    getri<float> (cm,ipiv); // cm will be overwritten with inversed matrix

    // checking whether the above operations successfully taken place 
    cm.to_rowmajor().save("./out_3x3_1");
    inv_cm.to_rowmajor().save("./out_3x3_2");
    BOOST_CHECK (system("diff ./out_3x3_1 ./ref_3x3") == 0);
    BOOST_CHECK (system("diff ./out_3x3_2 ./ref_3x3") == 0);
    system("rm -f ./out_3x3_1");
    system("rm -f ./out_3x3_2");
}

