#include <string>
#include <iostream>
#include <cassert>

#include <omp.h>

#include <boost/lexical_cast.hpp>

#include <frovedis.hpp>
#include <frovedis/matrix/crs_matrix.hpp>
#include <frovedis/ml/kernel/kernel_svm.hpp>

using kernel_t = float;


int main(int argc, char** argv) {
  frovedis::use_frovedis use(argc, argv);

  // log
  //frovedis::set_loglevel(DEBUG);
  
  bool multiple_mpi_procs = frovedis::get_nodesize() > 1;
  if (multiple_mpi_procs) {
    std::cerr << "---------- CAUTION ----------" << std::endl
	      << "Kernel SVM uses openmp parallelization." << std::endl
	      << "This program is launched with multiple MPI processes, but other than one (master) is not actually working." << std::endl
	      << "You should assign each process in different nodes to prevent interference" << std::endl;
  }

  std::cerr << "number of openmp threads = " << omp_get_max_threads() << std::endl;

  auto train_x = frovedis::make_crs_matrix_local_load<kernel_t>("./train.mat").to_rowmajor();
  auto train_y = frovedis::make_dvector_loadline<kernel_t>("./train.label").gather();

  double tol = 0.001;
  double C = 1.0;
  int cache_size = 4;  // in unit of MB
  int max_iter = 100;
  std::string kernel_ty_str = "rbf";  // or "poly", "sigmoid" 
  double gamma = 0.01;
  
  frovedis::kernel_csvc_model<kernel_t> model(tol, C, cache_size, max_iter, kernel_ty_str, gamma);
  model.train(train_x, train_y);

  model.save("./model");
  frovedis::kernel_csvc_model<kernel_t> model2;
  model2.load("./model");

  auto test_x = frovedis::make_crs_matrix_local_load<kernel_t>("./test.mat").to_rowmajor();
  auto test_y = frovedis::make_dvector_loadline<kernel_t>("./test.label").gather();
  
  auto pred_y = model2.predict(test_x);

  size_t num_correct = 0;
  for (size_t i = 0; i < pred_y.size(); i++) {
    num_correct += int(test_y[i] == pred_y[i]);
  }
  double acc = 1.0 * num_correct / pred_y.size();
  std::cout << "Accuracy: " << acc << std::endl;

  return 0;
}
