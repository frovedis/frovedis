#include <frovedis.hpp>
#include <frovedis/ml/model_selection/k_fold.hpp>

using namespace frovedis;

int main(int argc, char* argv[]) {
  use_frovedis use(argc, argv);
  //set_loglevel(DEBUG);
  try {
    if(argc < 3) 
      throw std::runtime_error("Insufficient args! Syntax: ./search_cv <train-data> <train-label>\n");

    //auto mat = make_crs_matrix_load<double>(argv[1]); // for sparse data
    auto mat = make_rowmajor_matrix_load<double>(argv[1]); // for dense data
    auto label = make_dvector_loadline<double>(argv[2]);

    // frovedis::k_fold demo for logistic regression
    int nsplits = 3;
    k_fold<rowmajor_matrix<double>,double> kf(nsplits);
    kf.fit(mat, label);

    std::cout << "*** debug print of all splits *** \n";
    kf.debug_print(5);

    for(size_t i = 0; i < nsplits; ++i) {
      std::cout << "\n*** debug print of " << std::to_string(i)
                << "th fold for train/test data ***\n";
      auto train_test_split = kf[i];
      std::cout << "--------------------\n";
      std::cout << "TRAIN: \n";
      std::cout << "--------------------\n";
      train_test_split.first.debug_print(5);
      std::cout << "--------------------\n";
      std::cout << "TEST: \n";
      std::cout << "--------------------\n";
      train_test_split.second.debug_print(5);
      std::cout << "--------------------\n";
    }
  }
  catch (std::exception& e) {
    REPORT_ERROR(USER_ERROR, "exception caught: " + std::string(e.what()) + "\n");
  }
  return 0; 
}
