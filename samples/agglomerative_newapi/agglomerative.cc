#include <frovedis.hpp>
#include <frovedis/ml/clustering/agglomerative.hpp>
#include <boost/program_options.hpp>

using namespace boost;
using namespace frovedis;

template <class T>
void do_agglomerative_clustering(const std::string& input_file, 
                              int ncluster, T threshold,
                              const std::string& link,
                              const std::string& model_file,
                              const std::string& label_file) {

  auto ac = agglomerative_clustering<T>(ncluster, link, threshold);  
  time_spent data_load(INFO), train(INFO);
  data_load.lap_start();  
  auto mat = make_rowmajor_matrix_load<T>(input_file);  
  data_load.lap_stop();
  data_load.show_lap("data loading time: ");
  train.lap_start();
  ac.fit(std::move(mat));  
  train.lap_stop();
  train.show_lap("training time: ");
  auto ret = ac.labels_();
  ac.save(model_file);    
  make_dvector_scatter(ret).saveline(label_file);
}

int main(int argc, char** argv) {
  use_frovedis use(argc,argv);
  time_spent t(DEBUG);

  using namespace boost::program_options;

  options_description opt("option");
  opt.add_options()
    ("help,h", "print help")
    ("input,i", value<std::string>(), "input dense matrix")
    ("model,m", value<std::string>(), "output model")
    ("label,o", value<std::string>(), "output label")
    ("nclus,n", value<int>(), "number of clusters (default: 2)")
    ("linkage,l", value<std::string>() , "linkage type - supported: average, single, complete (default: average)")
    ("float,f", "for float type input")
    ("double,d","for double type input(default input type is double)")
    ("threshold,t","threshold distance above which points will not be clustered (default: 0)")      
    ("verbose", "set loglevel to DEBUG")
    ("verbose2", "set loglevel to TRACE")
    ("clustering_verbose", "set loglevel to INFO (default loglevel is INFO)");
    
  variables_map argmap;
  store(command_line_parser(argc,argv).options(opt).allow_unregistered().
        run(), argmap);
  notify(argmap);

  std::string input, output_m, output_l;
  std::string linkage = "average";
  int nclus = 2;
  float threshold = 0;  
  
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
  
  if(argmap.count("model")){
    output_m = argmap["model"].as<std::string>();
  } else {
    std::cerr << "output model is not specified" << std::endl;
    std::cerr << opt << std::endl;
    exit(1);
  }

  if(argmap.count("label")){
    output_l = argmap["label"].as<std::string>();
  } else {
    std::cerr << "output label is not specified" << std::endl;
    std::cerr << opt << std::endl;
    exit(1);
  }

  if(argmap.count("nclus")){
    nclus = argmap["nclus"].as<int>();
  }

  if(argmap.count("threshold")){
    threshold = argmap["threshold"].as<float>();
  }    
  
  if(argmap.count("linkage")){
    linkage = argmap["linkage"].as<std::string>();
  }

  if(argmap.count("verbose")) set_loglevel(DEBUG);
  if(argmap.count("verbose2")) set_loglevel(TRACE);
  if(argmap.count("clustering_verbose")) set_loglevel(INFO);

  if(argmap.count("float")) 
    do_agglomerative_clustering<float>(input,nclus,threshold,linkage,output_m,output_l);
  else
    do_agglomerative_clustering<double>(input,nclus,threshold,linkage,output_m,output_l); 

  return 0;
}

