#include <frovedis.hpp>
#include <frovedis/ml/clustering/spectral_embedding.hpp>
#include <boost/program_options.hpp>

using namespace boost;
using namespace frovedis;

int main(int argc, char** argv) {
  use_frovedis use(argc,argv);

  using namespace boost::program_options;

  options_description opt("option");
  opt.add_options()
      ("help,h", "produce help message")
      ("input,i" , value<std::string>(), "input rowmajor matrix data.")
      ("n-components" , value<int>(), "number of eigenvectors to use for the spectral embedding.[default = 2]")
      ("gamma,g", value<double>(), "kernel coefficient for rbf, poly, sigmoid, laplacian and chi2 kernels.[default = 1.0]")
      ("norm-laplacian", value<bool>(), "[default: true]")
      ("precomputed", value<bool>(), "[default: false]")
      ("drop-first", value<bool>(), "[default: false]")
      ("mode", value<int>(), "[default: 1]")
      ("verbose", "set loglevel to DEBUG")
      ("verbose2", "set loglevel to TRACE");

  variables_map argmap;
  store(command_line_parser(argc,argv).options(opt).allow_unregistered().
        run(), argmap);
  notify(argmap);

  int n_comp = 2;
  double gamma = 1.0;
  bool norm_laplacian = true;
  bool precomputed = false;
  bool drop_first = false;
  int mode = 1;
  std::string data_p;

  if(argmap.count("help")){
    std::cerr << opt << std::endl;
    exit(1);
  }  
  if(argmap.count("n-components")){
    n_comp = argmap["n-components"].as<int>();
  }
  if(argmap.count("gamma")){
    gamma = argmap["gamma"].as<double>();
  }
  if(argmap.count("norm-laplacian")){
    norm_laplacian = argmap["norm-laplacian"].as<bool>();
  }
  if(argmap.count("precomputed")){
    precomputed = argmap["precomputed"].as<bool>();
  }  
  if(argmap.count("drop-first")){
    drop_first = argmap["drop-first"].as<bool>();
  }
  if(argmap.count("mode")){
    mode = argmap["mode"].as<int>();
  }
  if(argmap.count("input")){
    data_p = argmap["input"].as<std::string>();
  }else {
    std::cerr << "input is not specified" << std::endl;
    std::cerr << opt << std::endl;
    exit(1);
  }

  auto mat = make_rowmajor_matrix_load<double>(data_p); //./test_data

  // hyper-parameters
  time_spent embed(INFO);
  embed.lap_start();
  auto model = spectral_embedding<double>(std::move(mat), n_comp, norm_laplacian,
                                          precomputed, drop_first, gamma, mode);  
  embed.lap_stop();
  embed.show_lap("embedding time: ");

  std::cout << "\n embedding matrix: \n";
  model.embed_matrix.debug_print();
  return 0;
}
