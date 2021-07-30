#include <frovedis.hpp>
#include <frovedis/ml/model_selection/k_fold.hpp>
#include <boost/program_options.hpp>

using namespace frovedis;

int main(int argc, char* argv[]) {
  use_frovedis use(argc, argv);
  using namespace boost::program_options;

  options_description opt("option");
  opt.add_options()
      ("help,h", "produce help message")
      ("input-data,i" , value<std::string>(), "input rowmajor matrix data.")
      ("input-labels,l" , value<std::string>(), "input label data.")
      ("nsplits" , value<int>(), "[default = 5]")
      ("verbose", "set loglevel to DEBUG")
      ("verbose2", "set loglevel to TRACE");

  variables_map argmap;
  store(command_line_parser(argc,argv).options(opt).allow_unregistered().
        run(), argmap);
  notify(argmap);
  
  int nsplits = 5;
  std::string data_p, data_l;

  if(argmap.count("input-data")){
    data_p = argmap["input-data"].as<std::string>();
  }else {
    std::cerr << "input-data is not specified" << std::endl;
    std::cerr << opt << std::endl;
    exit(1);
  }
  if(argmap.count("input-labels")){
    data_l = argmap["input-labels"].as<std::string>();
  }else {
    std::cerr << "input-labels is not specified" << std::endl;
    std::cerr << opt << std::endl;
    exit(1);
  }
  if(argmap.count("nsplits")){
    nsplits = argmap["nsplits"].as<int>();
  }
  try {
    auto mat = make_rowmajor_matrix_load<double>(data_p); // for dense data
    auto label = make_dvector_loadline<double>(data_l);

    // frovedis::k_fold demo for logistic regression
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
