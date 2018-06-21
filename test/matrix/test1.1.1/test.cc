#include <frovedis.hpp>
#include <frovedis/matrix/blockcyclic_matrix.hpp>

#define BOOST_TEST_MODULE FrovedisTest
#include <boost/test/unit_test.hpp>

using namespace frovedis;
using namespace std;

bool check(blockcyclic_matrix_local<float>& m) {
  return m.is_valid();
}

BOOST_AUTO_TEST_CASE( frovedis_test )
{
    int argc = 1;
    char** argv = NULL;
    use_frovedis use(argc, argv);

    // creating blockcyclic matrix loading data from sample text file
    auto bm = make_blockcyclic_matrix_load<float> ("./sample_4x4");

    // checking the validity of the created blockcyclic matrix
    BOOST_CHECK (bm.data.map(check).get(0));

    // transposing the matrix and checking its dimension
    auto t_bm = bm.transpose();
    BOOST_CHECK (t_bm.get_nrows() == bm.get_ncols());
    BOOST_CHECK (t_bm.get_ncols() == bm.get_nrows());

    // transposing back and saving in text file
    t_bm.transpose().save("./tt_bm");

    // checking whether saved file have same data as with loaded file
    // this will confirm whether transpose operation was successful
    BOOST_CHECK (system("diff ./sample_4x4 ./tt_bm") == 0);

    // deleting the saved file
    system("rm -f ./tt_bm");
}

