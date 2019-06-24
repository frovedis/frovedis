#include <frovedis.hpp>
#include <frovedis/ml/clustering/agglomerative.hpp>
#include <boost/program_options.hpp>

using namespace boost;
using namespace frovedis;

template <class T>
std::vector<int>
agglomerative_clustering(const std::string& input, 
                         int& ncluster, 
                         const std::string& link,
                         const std::string& output) {
  auto mat = make_rowmajor_matrix_load<T>(input);
  auto model = agglomerative_training<T>(mat, link);
  model.save(output);
  return agglomerative_assign_cluster<T>(model, ncluster);
}

int main(int argc, char** argv) {
  use_frovedis use(argc,argv);
  time_spent t(DEBUG);

  using namespace boost::program_options;

  options_description opt("option");
  opt.add_options()
    ("help,h", "print help")
    ("input,i", value<std::string>(), "input dense matrix")
    ("output,o", value<std::string>(), "output model")
    ("nclus,nc", value<int>(), "number of clusters(default is 2)")
    ("linkage,l", value<std::string>() , "linkage type")
    ("float,f", "for float type input")
    ("double,d","for double type input(default input type is double)")
    ("verbose", "set loglevel to DEBUG")
    ("verbose2", "set loglevel to TRACE")
    ("clustering_verbose", "set loglevel to INFO (default loglevel is INFO)");
    
  variables_map argmap;
  store(command_line_parser(argc,argv).options(opt).allow_unregistered().
        run(), argmap);
  notify(argmap);

  std::string input, output;
  std::string linkage = "average";
  int nclus = 2;
  
  if(argmap.count("help")){
    std::cerr << opt << std::endl;
    exit(1);
  }

  if(argmap.count("input")){
    input = argmap["input"].as<std::string>();
  } else {
    std::cerr << "input is not specified" << std::endl;
    std::cerr << opt << std::endl;
    exit(1);
  }
  
  if(argmap.count("output")){
    output = argmap["output"].as<std::string>();
  } else {
    std::cerr << "output is not specified" << std::endl;
    std::cerr << opt << std::endl;
    exit(1);
  }

  if(argmap.count("nclus")){
    nclus = argmap["nclus"].as<int>();
  }
  
  if(argmap.count("linkage")){
    linkage = argmap["linkage"].as<std::string>();
    if(linkage != "average")
       REPORT_ERROR(USER_ERROR,"Only average linkage is supported!\n");    
  }

  if(argmap.count("verbose")) set_loglevel(DEBUG);
  if(argmap.count("verbose2")) set_loglevel(TRACE);
  if(argmap.count("clustering_verbose")) set_loglevel(INFO);

  std::vector<int> label;    
  if(argmap.count("float")) 
    label = agglomerative_clustering<float>(input,nclus,linkage,output);
  else
    label = agglomerative_clustering<double>(input,nclus,linkage,output);

  std::cout << "[frovedis] label: "; show<int>(label);
  return 0;
}

