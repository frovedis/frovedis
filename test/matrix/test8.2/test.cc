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

    // swapping two distributed std::vectors'
    std::vector<float> x = {1,2,3,4}; 
    std::vector<float> y = {5,6,7,8};
    std::vector<float> x1 = x, y1 = y;
    auto xbm = vec_to_bcm<float> (x); // distributed vector x
    auto ybm = vec_to_bcm<float> (y); // distrubuted vector y
    swap<float> (xbm,ybm); // ybm <=> xbm;
    BOOST_CHECK (xbm.moveto_vector() == y1);
    BOOST_CHECK (ybm.moveto_vector() == x1);

    auto bm = make_blockcyclic_matrix_load<float> ("./sample_4x4");
  
    // checking swap() operation  
    auto row1 = make_row_vector<float> (bm,1);
    auto row2 = make_row_vector<float> (bm,2);
    swap<float>(row1,row2); // row2 <=> row1
    bm.save("./out_4x4");

    // checking whether the above operation successfully taken place 
    BOOST_CHECK (system("diff ./out_4x4 ./ref_4x4") == 0);
    system("rm -f ./out_4x4");
}

