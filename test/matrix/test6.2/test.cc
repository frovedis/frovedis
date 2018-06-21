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

    // swapping two std::vectors'
    std::vector<float> v1 = {1,2,3,4};
    std::vector<float> v2 = {5,6,7,8}; 
    std::vector<float> v3 = v1, v4 = v2;
    swap<float>(v1, v2); // v2 <=> v1
    BOOST_CHECK( v1 == v4);
    BOOST_CHECK( v2 == v3);

    // creating a colmajor matrix local from file
    colmajor_matrix_local<float> cm (
           make_rowmajor_matrix_local_load<float>("./sample_4x4"));
  
    // checking swap() operation  
    auto row1 = make_row_vector<float> (cm,1);
    auto row2 = make_row_vector<float> (cm,2);
    swap<float>(row1,row2); // row2 <=> row1
    cm.to_rowmajor().save("./out_4x4");

    // checking whether teh above operation successfully taken place 
    BOOST_CHECK (system("diff ./out_4x4 ./ref_4x4") == 0);
    system("rm -f ./out_4x4");
}

