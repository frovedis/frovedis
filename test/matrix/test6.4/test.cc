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

    std::vector<float> y = {1,2,3,4};
    std::vector<float> x = {1,2,3,4};
    std::vector<float> out = {3,6,9,12};
    axpy<float> (x,y,2); // y = 2*x + y;
    BOOST_CHECK (y == out);

    // creating a colmajor matrix local from file
    colmajor_matrix_local<float> cm (
           make_rowmajor_matrix_local_load<float>("./sample_4x4"));
  
    // checking axpy() operation  
    auto row1 = make_row_vector<float> (cm,1);
    auto row2 = make_row_vector<float> (cm,2);
    axpy<float>(row1,row2,3); // row2 = 3*row1 + row2
    cm.to_rowmajor().save("./out_4x4");

    // checking whether the above operation successfully taken place 
    BOOST_CHECK (system("diff ./out_4x4 ./ref_4x4") == 0);
    system("rm -f ./out_4x4");
}

