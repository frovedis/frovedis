#include <frovedis.hpp>
#include <frovedis/matrix/crs_matrix.hpp>
#include <frovedis/ml/nb/naive_bayes.hpp>

using namespace frovedis;

int main(int argc, char* argv[]) {
  try {
    use_frovedis use(argc, argv);
    std::string datafile = argv[1];
    std::cout << "training data: " << datafile << std::endl;
  
    dvector<double> lbl;
    crs_matrix<double> mat;
    //mat = make_crs_matrix_loadlibsvm<double>(datafile, lbl); 
    mat = make_crs_matrix_load<double>(datafile + "/feature.sparse");
    lbl = make_dvector_loadline<double>(datafile + "/label");
 
    time_spent train_t(INFO);
    train_t.lap_start(); 
    auto model = multinomial_nb(mat, lbl);
    train_t.lap_stop(); 
    train_t.show_lap("total train time: ");
    //model.debug_print(); 
  }
  catch (std::exception& e) {
    std::cout << "exception caught: " << e.what() << std::endl;
  }
  return 0;
}
