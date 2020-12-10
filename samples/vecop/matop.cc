#include <frovedis.hpp>
#include <frovedis/matrix/rowmajor_matrix.hpp>
#include <frovedis/matrix/colmajor_matrix.hpp>
#include <frovedis/matrix/crs_matrix.hpp>
#include <frovedis/matrix/matrix_operations.hpp>

using namespace frovedis;

template <class T>
void show(const std::string& msg, const T& val) {
  std::cout << msg << val << std::endl;
}

template <class T>
void show(const std::string& msg,
          const std::vector<T>& vec,
          const int& limit = 10) {
  std::cout << msg; debug_print_vector(vec, limit);
}

template <class MATRIX>
void invoke_matrix_operations(MATRIX& m) {
  time_spent t(INFO);
  auto msumall = matrix_sum(m)[0];
  t.show("matrix_sum(): ");

  auto msum0 = matrix_sum(m, 0);
  t.show("matrix_sum(axis = 0): ");

  auto msum1 = matrix_sum(m, 1);
  t.show("matrix_sum(axis = 1): ");

  auto msqsumall = matrix_squared_sum(m)[0];
  t.show("matrix_squared_sum(): ");

  auto msqsum0 = matrix_squared_sum(m, 0);
  t.show("matrix_squared_sum(axis = 0): ");

  auto msqsum1 = matrix_squared_sum(m, 1);
  t.show("matrix_squared_sum(axis = 1): ");

  auto mmeanall = matrix_mean(m)[0];
  t.show("matrix_mean(): ");

  auto mmean0 = matrix_mean(m, 0);
  t.show("matrix_mean(axis = 0): ");

  auto mmean1 = matrix_mean(m, 1);
  t.show("matrix_mean(axis = 1): ");

  auto binmat = matrix_binarize(m, 4);
  t.show("matrix_binarize(): ");

/*
  std::cout << "-------- results -------- \n";
  show("sum: ", msumall);
  show("sum (axis = 0): ", msum0);
  show("sum (axis = 1): ", msum1);
  show("squared sum: ", msqsumall);
  show("squared sum (axis = 0): ", msqsum0);
  show("squared sum (axis = 1): ", msqsum1);
  show("mean: ", mmeanall); 
  show("mean (axis = 0): ", mmean0); 
  show("mean (axis = 1): ", mmean1); 
  std::cout << "binarize(thr = 4): "; binmat.debug_print(10);
*/
}

int main(int argc, char* argv[]) {
  use_frovedis use(argc, argv);
  try {
    if (argc < 3) 
      throw std::runtime_error("Use syntax: mpirun -np <x> ./matop <matrix-file> <mattype>");
    auto fname = argv[1];
    auto mattype = std::string(argv[2]);
    std::cout << "matrix operation demo for: " << mattype << std::endl;
    if (mattype == "rowmajor") {
      auto m = make_rowmajor_matrix_load<double>(fname);
      invoke_matrix_operations(m);
    }
    else if (mattype == "rowmajor-local") {
      auto m = make_rowmajor_matrix_local_load<double>(fname);
      invoke_matrix_operations(m);
    }
    else if (mattype == "colmajor") {
      auto m = colmajor_matrix<double>(make_rowmajor_matrix_load<double>(fname));
      invoke_matrix_operations(m);
    }
    else if (mattype == "colmajor-local") {
      auto m = colmajor_matrix_local<double>(make_rowmajor_matrix_local_load<double>(fname));
      invoke_matrix_operations(m);
    }
    else if (mattype == "crs") {
      auto m = make_crs_matrix_load<double>(fname);
      invoke_matrix_operations(m);
    }
    else if (mattype == "crs-local") {
      auto m = make_crs_matrix_local_load<double>(fname);
      invoke_matrix_operations(m);
    }
    else REPORT_ERROR(USER_ERROR, "Unknown mattype is encountered!\n");
  } 
  catch(std::exception& e) {
    std::cout <<  "exception caught: " << e.what() << std::endl;
  }
  return 0;
}
