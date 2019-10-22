#include <frovedis.hpp>
#include <frovedis/ml/neighbors/knn.hpp>

using namespace frovedis ;

int main (int argc, char** argv) {
  use_frovedis use(argc, argv);

  try {
    int k = 4;
    std::string metric = "euclidean";
    bool need_distance = false;

    // will be modified with program options
    if (argc > 1) metric = argv[1];
    if (argc > 2) need_distance = std::stoi(argv[2]) == 1;

    auto tr_mat = make_rowmajor_matrix_load<double>("train_data");
    //auto ts_mat = make_rowmajor_matrix_local_load<double>("test_data");
  
    time_spent calc_knn(INFO);
    calc_knn.lap_start();
    //auto ret = knn(tr_mat, ts_mat, k, metric, need_distance);
    auto ret = knn(tr_mat, k, metric, need_distance); // find knn on train data itself
    calc_knn.lap_stop();
    calc_knn.show_lap("total knn computation: ");

    ret.indices.save("indices");
    if (need_distance) ret.distances.save("distances");
  }
  catch(std::exception& e) {
    std::cout << "exception caught: " << e.what() << "\n";
  }

  return 0;
}
