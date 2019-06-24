#ifndef SPECTRAL_EMBEDDING_MODEL
#define SPECTRAL_EMBEDDING_MODEL

namespace frovedis {

template <class T>
struct spectral_embedding_model {
  
  spectral_embedding_model() {}
  spectral_embedding_model(rowmajor_matrix<T>& aff,
                           rowmajor_matrix<T>& embed): 
    affinity_matrix(aff), embed_matrix(embed) {}

  void __create_dir_struct (const std::string& dir) {
    struct stat sb;
    if(stat(dir.c_str(), &sb) != 0) { // no file/directory
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
    std::string embed_file = dir + "/embedding";
    embed_matrix.save(embed_file); //embedding: rowmajor_matrix<T>
  }
  void savebinary(const std::string& dir) {
    __create_dir_struct(dir);
    std::string affinity_file = dir + "/affinity";
    affinity_matrix.savebinary(affinity_file); //affinity: rowmajor_matrix<T>
    std::string embed_file = dir + "/embedding";
    embed_matrix.savebinary(embed_file); //embedding: rowmajor_matrix<T>
  }
  void load(const std::string& dir) {
    std::string affinity_file = dir + "/affinity";
    affinity_matrix = make_rowmajor_matrix_load<T>(affinity_file);
    std::string embed_file = dir + "/embedding";
    embed_matrix = make_rowmajor_matrix_load<T>(embed_file);
  }
  void loadbinary(const std::string& dir) {
    std::string affinity_file = dir + "/affinity";
    affinity_matrix = make_rowmajor_matrix_loadbinary<T>(affinity_file);
    std::string embed_file = dir + "/embedding";
    embed_matrix = make_rowmajor_matrix_loadbinary<T>(embed_file);
  }
  void debug_print() {
    std::cout << "affinity matrix: \n";
    affinity_matrix.debug_print();
    std::cout << "embed matrix: \n";
    embed_matrix.debug_print();
  }  

  rowmajor_matrix<T> affinity_matrix, embed_matrix;
  SERIALIZE(affinity_matrix, embed_matrix)
};

}

#endif
