#include <frovedis.hpp>
#include <frovedis/matrix/rowmajor_matrix.hpp>
#include <frovedis/ml/nb/naive_bayes.hpp>

#define BOOST_TEST_MODULE FrovedisTest
#include <boost/test/unit_test.hpp>

using namespace frovedis;

BOOST_AUTO_TEST_CASE( frovedis_test )
{
    int argc = 1;
    char** argv = NULL;
    use_frovedis use(argc, argv);

    std::string m_matf = "./input/m_mat";
    std::string m_lblf = "./input/m_lbl";
    std::string b_matf = "./input/b_mat";
    std::string b_lblf = "./input/b_lbl";

    auto b_mat = colmajor_matrix<double>(make_rowmajor_matrix_load<double>(b_matf));
    auto b_lbl = make_dvector_loadline<double>(b_lblf);
    auto m1 = bernoulli_nb<double,colmajor_matrix<double>,colmajor_matrix_local<double>>(b_mat,b_lbl);
    auto b_tmat = make_rowmajor_matrix_local_load<double>(b_matf);
    auto pd1 = m1.predict(b_tmat);
    //for(auto& e: pd1) std::cout << e << " "; std::cout << std::endl;

    auto m_mat = colmajor_matrix<double>(make_rowmajor_matrix_load<double>(m_matf));
    auto m_lbl = make_dvector_loadline<double>(m_lblf);
    auto m2 = multinomial_nb<double,colmajor_matrix<double>,colmajor_matrix_local<double>>(m_mat,m_lbl);
    auto m_tmat = make_rowmajor_matrix_local_load<double>(m_matf);
    auto pd2 = m2.predict(m_tmat);
    //for(auto& e: pd2) std::cout << e << " "; std::cout << std::endl;

    // confirming correctness of result
    std::vector<double> expected = {1,0,1,1,1,0,1,1};
    BOOST_CHECK (pd1 == expected && pd2 == expected);
}

