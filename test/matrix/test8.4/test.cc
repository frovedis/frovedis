#include <frovedis.hpp>
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

    // axpy'ing two distributed std::vectors'
    std::vector<float> x = {1,2,3,4}; 
    std::vector<float> y = {1,2,3,4};
    std::vector<float> out = {3,6,9,12};
    auto xbm = vec_to_bcm<float> (x); // distributed vector x
    auto ybm = vec_to_bcm<float> (y); // distrubuted vector y
    axpy<float> (xbm,ybm,2); // ybm = 2*xbm + ybm;
    BOOST_CHECK (ybm.moveto_vector() == out);

    auto bm = make_blockcyclic_matrix_load<float> ("./sample_4x4");
  
    // checking axpy() operation  
    auto row1 = make_row_vector<float> (bm,1);
    auto row2 = make_row_vector<float> (bm,2);
    axpy<float>(row1,row2,3); // row2 <=> 3*row1 + row2
    bm.save("./out_4x4");

    // checking whether the above operation successfully taken place 
    BOOST_CHECK (system("diff ./out_4x4 ./ref_4x4") == 0);
    system("rm -f ./out_4x4");
}

