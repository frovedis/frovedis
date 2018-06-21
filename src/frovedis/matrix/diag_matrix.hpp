#ifndef DIAG_MATRIX_HPP
#define DIAG_MATRIX_HPP

// diagonal n x n sparse matrix

#include "../core/serializehelper.hpp"

namespace frovedis {

template <class T>
struct diag_matrix_local {
  diag_matrix_local() {}
  diag_matrix_local(size_t size) {
    val.resize(size);
  }
  diag_matrix_local(diag_matrix_local<T>& m) {
    val = m.val;
  }
  diag_matrix_local<T>& operator=(diag_matrix_local<T>& m) {
    val = m.val;
    return *this;
  }
  diag_matrix_local(diag_matrix_local<T>&& m) {
    val.swap(m.val);
  }
  diag_matrix_local<T>& operator=(diag_matrix_local<T>&& m) {
    val.swap(m.val);
    return *this;
  }
  std::vector<T> val;
  size_t local_num() const {return val.size();}
  void save(const std::string& file) {
    std::ofstream str(file.c_str());
    str << *this;
  }
  void savebinary(const std::string&);
  void debug_print();

  SERIALIZE(val)
};

template <class T>
std::ostream& operator<<(std::ostream& str, const diag_matrix_local<T>& mat) {
  for(size_t i = 0; i < mat.val.size(); i++) {
    str << mat.val[i] << "\n";
  }
  return str;
}

template <class T>
diag_matrix_local<T>
make_diag_matrix_local_readstream(std::istream& str) {
  diag_matrix_local<T> ret;
  for(T e; str >> e;) ret.val.push_back(e);
  return ret;
}

template <class T>
std::istream& operator>>(std::istream& str,
                         diag_matrix_local<T>& mat) {
  mat = make_diag_matrix_local_readstream<T>(str);
  return str;
}

template <class T>
diag_matrix_local<T>
make_diag_matrix_local_load(const std::string& file) {
  std::ifstream str(file.c_str());
  return make_diag_matrix_local_readstream<T>(str);
}


template <class T>
void diag_matrix_local<T>::debug_print() {
  std::cout << "node = " << get_selfid()
            << ", local_num() = " << local_num()
            << ", val = ";
  for(auto i: val){ std::cout << i << " "; }
  std::cout << std::endl;
}

/*
  The format is just plain binary file
 */
template <class T>
diag_matrix_local<T>
make_diag_matrix_local_loadbinary(const std::string& input) {
  diag_matrix_local<T> ret;
  auto vec = make_dvector_loadbinary<T>(input);
  auto tmp = vec.gather();
  ret.val.swap(tmp);
  return ret;
}

template <class T>
void diag_matrix_local<T>::savebinary(const std::string& output) {
  auto tmp = make_dvector_scatter(val);
  tmp.savebinary(output);
}

template <class T>
struct diag_matrix {
  diag_matrix(){}
  diag_matrix(frovedis::node_local<diag_matrix_local<T>>&& d) :
    data(std::move(d)) {}
  diag_matrix_local<T> gather();
  void save(const std::string& file) {
    std::ofstream str(file.c_str());
    str << *this;
  }
  void savebinary(const std::string&);
  frovedis::node_local<diag_matrix_local<T>> data;  
};

template <class T>
std::vector<T> to_vector(diag_matrix_local<T>& m) {
  return m.val;
}

template <class T>
diag_matrix_local<T> diag_matrix<T>::gather() {
  auto vec = data.map(to_vector<T>).template moveto_dvector<T>().gather();
  diag_matrix_local<T> ret;
  ret.val.swap(vec);
  return ret;
}

template <class T>
diag_matrix_local<T> moveto_diag_matrix_local(std::vector<T>& v) {
  diag_matrix_local<T> ret;
  ret.val.swap(v);
  return ret;
}

template <class T>
diag_matrix<T> make_diag_matrix_scatter(diag_matrix_local<T>& m) {
  auto dvec = make_dvector_scatter(m.val);
  diag_matrix<T> ret;
  ret.data = dvec.moveto_node_local().map(moveto_diag_matrix_local<T>);
  return ret;
}

template <class T>
std::ostream& operator<<(std::ostream& str, const diag_matrix<T>& mat) {
  // node_local<T>::gather() is not const...
  auto& mat2 = const_cast<diag_matrix<T>&>(mat);
  auto gmat = mat2.data.gather();
  for(auto& l: gmat) str << l;
  return str;
}

template <class T>
void diag_matrix<T>::savebinary(const std::string& file) {
  data.map(to_vector<T>).template moveto_dvector<T>().savebinary(file);
}

}
#endif
