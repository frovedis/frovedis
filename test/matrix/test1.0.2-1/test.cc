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

    // constructing blockcylic matrix from colmajor matrix
    colmajor_matrix<float> cm (
             make_rowmajor_matrix_load<float>("./sample_4x4"));
    blockcyclic_matrix<float> bm1 (cm);

    // checking the validity of the created blockcyclic matrix
    BOOST_CHECK (bm1.data.map(check).get(0));

    // default constructor to create empty matrix
    blockcyclic_matrix<float> bm2, bm3;

    // overloaded copy =
    bm2 = bm1;

    // overloaded move =
    bm3 = std::move(bm1);

    // converting back to colmajor matrix and save in a text file
    bm2.to_colmajor().save("./cm");

    // saving these matrices in sample text files
    bm2.save("./bm2");
    bm3.save("./bm3");

    // checking whether saved files are having same data as with loaded file
    BOOST_CHECK (system("diff ./sample_4x4 ./cm") == 0);
    BOOST_CHECK (system("diff ./sample_4x4 ./bm2") == 0);
    BOOST_CHECK (system("diff ./sample_4x4 ./bm3") == 0);

    // deleting the saved files
    system("rm -f ./cm ./bm2 ./bm3");
}

