#ifndef SPECTRAL_CLUSTERING_MODEL
#define SPECTRAL_CLUSTERING_MODEL

namespace frovedis {

template <class T>
struct spectral_clustering_model {
  
  spectral_clustering_model() {}
  spectral_clustering_model(rowmajor_matrix<T>& aff,
                            std::vector<int>& lbl, int ncluster):
    affinity_matrix(aff), labels(lbl), nclusters(ncluster) {}

  void __create_dir_struct (const std::string& dir) {
    struct stat sb;
    if(stat(dir.c_str(), &sb) != 0) { // no file directory
      mode_t mode = S_IRWXU | S_IRWXG | S_IRWXO; // man 2 stat
      if(mkdir(dir.c_str(), mode) != 0) {
        perror("mkdir failed:");
        throw std::runtime_error("mkdir failed");
      }
    } else if(!S_ISDIR(sb.st_mode)) {
      throw std::runtime_error(dir + " is not a directory");
    }
  }
  void save(const std::string& dir) {
    __create_dir_struct(dir);
    std::string affinity_file = dir + "/affinity";
    affinity_matrix.save(affinity_file); //affinity: rowmajor_matrix<T>
    std::string label_file = dir + "/label";
    std::string cluster_file = dir + "/cluster_size";
    std::ofstream label_str, cluster_str;
    label_str.exceptions(std::ofstream::failbit | std::ofstream::badbit);
    cluster_str.exceptions(std::ofstream::failbit | std::ofstream::badbit);
    label_str.open(label_file.c_str()); for(auto& e: labels) label_str << e << std::endl;
    cluster_str.open(cluster_file.c_str()); cluster_str << nclusters << std::endl;
  }
  void savebinary(const std::string& dir) {
    __create_dir_struct(dir);
    std::string affinity_file = dir + "/affinity";
    affinity_matrix.savebinary(affinity_file); //affinity: rowmajor_matrix<T>
    std::string label_file = dir + "/label";
    std::string cluster_file = dir + "/cluster_size";
    make_dvector_scatter(labels).savebinary(label_file);
    std::ofstream cluster_str;
    cluster_str.exceptions(std::ofstream::failbit | std::ofstream::badbit);
    cluster_str.open(cluster_file.c_str()); cluster_str << nclusters << std::endl;
  }
  void load(const std::string& dir) {
    std::string affinity_file = dir + "/affinity";
    affinity_matrix = make_rowmajor_matrix_load<T>(affinity_file);
    std::string label_file = dir + "/label";
    labels = make_dvector_loadline<int>(label_file).gather();
    std::string cluster_file = dir + "/cluster_size";
    std::ifstream cluster_str(cluster_file.c_str()); cluster_str >> nclusters;
  }
  void loadbinary(const std::string& dir) {
    std::string affinity_file = dir + "/affinity";
    affinity_matrix = make_rowmajor_matrix_loadbinary<T>(affinity_file);
    std::string label_file = dir + "/label";
    std::string cluster_file = dir + "/cluster_size";
    labels = make_dvector_loadbinary<int>(label_file).gather();
    std::ifstream cluster_str(cluster_file.c_str()); cluster_str >> nclusters;
  }
  void debug_print() {
    std::cout << "affinity matrix: \n";
    affinity_matrix.debug_print();
    std::cout << "labels: \n";
    for(auto& i: labels) std::cout << i << " "; std::cout << std::endl;
    std::cout << "ncluster: " << nclusters << std::endl;
  }  

  rowmajor_matrix<T> affinity_matrix;
  std::vector<int> labels;
  int nclusters;
  SERIALIZE(affinity_matrix,labels,nclusters)
};

}

#endif
