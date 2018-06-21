#include <frovedis.hpp>
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

    // scaling a std::vector
    std::vector<float> in = {1,2,3,4};
    std::vector<float> out = {2,4,6,8};
    scal<float>(in,2); // in = 2*in
    BOOST_CHECK (in == out);

    // creating a colmajor matrix local from file
    colmajor_matrix_local<float> cm (
           make_rowmajor_matrix_local_load<float>("./sample_4x4"));
  
    // checking scal() operation  
    auto row = make_row_vector<float> (cm,1);
    scal<float>(row,5); // row = 5 * row
    cm.to_rowmajor().save("./out_4x4");

    // checking whether the above operation successfully taken place 
    BOOST_CHECK (system("diff ./out_4x4 ./ref_4x4") == 0);
    system("rm -f ./out_4x4");
}

