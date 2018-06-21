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
           make_rowmajor_matrix_local_load<float>("./sample_4x4"));

    // creating sliced matrix
    auto sm = make_sliced_colmajor_matrix_local<float> (cm,1,1,3,3);

    std::vector<int> ipiv; // empty ipiv array
    getrf<float> (sm,ipiv); // cm will be factorized and ipiv will contain pivoting info
    getri<float> (sm,ipiv); // cm will be overwritten with inversed matrix

    // checking whether the above operations successfully taken place 
    cm.to_rowmajor().save("./out_4x4");
    BOOST_CHECK (system("diff ./out_4x4 ./ref_4x4") == 0);
    system("rm -f ./out_4x4");
}

