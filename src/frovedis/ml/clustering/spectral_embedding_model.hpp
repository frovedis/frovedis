#ifndef SPECTRAL_EMBEDDING_MODEL
#define SPECTRAL_EMBEDDING_MODEL

namespace frovedis {

template <class T>
struct spectral_embedding_model {
  
  spectral_embedding_model() {}
  spectral_embedding_model(rowmajor_matrix<T>& aff,
                           rowmajor_matrix<T>& embed): 
    dense_affinity_matrix(aff), embed_matrix(embed) {
    is_dense_affinity = true;  
  }
  spectral_embedding_model(crs_matrix<T>& aff,
                           rowmajor_matrix<T>& embed): 
    sparse_affinity_matrix(aff), embed_matrix(embed) {
    is_dense_affinity = false;  
  }    
  void save(const std::string& dir) {
    make_directory(dir);
    //Affinity matrix 
    std::string affinity_file = dir + "/affinity";
    is_dense_affinity ? dense_affinity_matrix.save(affinity_file)  
                      : sparse_affinity_matrix.save(affinity_file);
    //Affinity type  
    std::string type_file = dir + "/aff_type";
    std::ofstream type_str;
    type_str.exceptions(std::ofstream::failbit | std::ofstream::badbit);
    type_str.open(type_file.c_str()); 
    std::string aff_type = is_dense_affinity ? "dense" : "sparse";
    type_str << aff_type << std::endl;      
    std::string embed_file = dir + "/embedding";
    embed_matrix.save(embed_file); //embedding: rowmajor_matrix<T>
  }
  void savebinary(const std::string& dir) {
    make_directory(dir);
    //Affinity matrix 
    std::string affinity_file = dir + "/affinity";
    is_dense_affinity ? dense_affinity_matrix.savebinary(affinity_file)  
                      : sparse_affinity_matrix.savebinary(affinity_file);
    //Affinity type  
    std::string type_file = dir + "/aff_type";
    std::ofstream type_str;
    type_str.exceptions(std::ofstream::failbit | std::ofstream::badbit);
    type_str.open(type_file.c_str()); 
    std::string aff_type = is_dense_affinity ? "dense" : "sparse";
    type_str << aff_type << std::endl;
    std::string embed_file = dir + "/embedding";
    embed_matrix.savebinary(embed_file); //embedding: rowmajor_matrix<T>
  }
  void load(const std::string& dir) {
    //check affinity type  
    std::string type_file = dir + "/aff_type";
    std::string aff_type;
    std::ifstream type_str(type_file.c_str()); type_str >> aff_type;
    is_dense_affinity = (aff_type == "dense");
    std::string affinity_file = dir + "/affinity";
    if(is_dense_affinity)
      dense_affinity_matrix = make_rowmajor_matrix_load<T>(affinity_file);
    else
      sparse_affinity_matrix = make_crs_matrix_load<T>(affinity_file);
    //Embedding  
    std::string embed_file = dir + "/embedding";
    embed_matrix = make_rowmajor_matrix_load<T>(embed_file);
  }
  void loadbinary(const std::string& dir) {
    //check affinity type  
    std::string type_file = dir + "/aff_type";
    std::string aff_type;
    std::ifstream type_str(type_file.c_str()); type_str >> aff_type;
    is_dense_affinity = (aff_type == "dense");
    std::string affinity_file = dir + "/affinity";
    if(is_dense_affinity)
      dense_affinity_matrix = make_rowmajor_matrix_loadbinary<T>(affinity_file);
    else
      sparse_affinity_matrix = make_crs_matrix_loadbinary<T>(affinity_file);
    //Embedding  
    std::string embed_file = dir + "/embedding";
    embed_matrix = make_rowmajor_matrix_loadbinary<T>(embed_file);
  }
  void debug_print() {
    std::cout << "affinity matrix: \n";
    if(is_dense_affinity) dense_affinity_matrix.debug_print();
    else sparse_affinity_matrix.debug_print();
    std::cout << "embed matrix: \n";
    embed_matrix.debug_print();
  }  

  rowmajor_matrix<T> dense_affinity_matrix;
  crs_matrix<T> sparse_affinity_matrix;
  bool is_dense_affinity;  
  rowmajor_matrix<T> embed_matrix;
  SERIALIZE(dense_affinity_matrix, sparse_affinity_matrix,
            is_dense_affinity, embed_matrix)
};

}

#endif
