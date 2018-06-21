#include <frovedis.hpp>
#include <frovedis/matrix/crs_matrix.hpp>

#include <boost/program_options.hpp>

using namespace boost;
using namespace frovedis;
using namespace std;

template <class T>
crs_matrix_local<T> calc_tf(crs_matrix_local<T>& mat) {
  crs_matrix_local<T> ret;
  ret.val.resize(mat.val.size());
  ret.idx = mat.idx;
  ret.off = mat.off;
  ret.local_num_col = mat.local_num_col;
  ret.local_num_row = mat.local_num_row;
  for(size_t row = 0; row < mat.local_num_row; row++) {
    T sum = 0;
    for(size_t col = mat.off[row]; col < mat.off[row + 1]; col++) {
      sum += mat.val[col];
    }
    T norm = 1.0/sum;
    for(size_t col = mat.off[row]; col < mat.off[row + 1]; col++) {
      ret.val[col] = mat.val[col] * norm;
    }
  }
  return ret;
}

template <class T>
vector<T> local_df(crs_matrix_local<T>& mat) {
  vector<T> ret(mat.local_num_col);
  T* retp = &ret[0];
  size_t* idxp = &mat.idx[0];

  for(size_t row = 0; row < mat.local_num_row; row++) {
#pragma cdir nodep
    for(size_t col = mat.off[row]; col < mat.off[row + 1]; col++) {
      if(idxp[col] >= mat.local_num_col) {
        std::cerr << "local_num_col = " << mat.local_num_col
                  << ", idxp[col] = " << idxp[col] << std::endl;
        throw std::runtime_error("error");
      }
      retp[idxp[col]]++;
    }
  }
  return ret;
}

template <class T>
vector<T> reduce_df(const vector<T>& l, const vector<T>& r) {
  vector<T> ret(l.size());
  for(size_t i = 0; i < l.size(); i++) {
    ret[i] = l[i] + r[i];
  }
  return ret;
}

template <class T>
vector<T> calc_idf(crs_matrix<T>& mat) {
  double total_doc = static_cast<double>(mat.num_row);
  auto local_dfs = mat.data.map(local_df<T>);
  auto df = local_dfs.reduce(reduce_df<T>);
  vector<T> idf(df.size());
  for(size_t i = 0; i < df.size(); i++) {
    idf[i] = log(total_doc / static_cast<double>(df[i]));
  }
  return idf;
}

template <class T>
void mul_idf(crs_matrix_local<T>& mat, vector<T>& idf) {
  T* matvalp = &mat.val[0];
  T* idfp = &idf[0];
  size_t* idxp = &mat.idx[0];
  for(size_t row = 0; row < mat.local_num_row; row++) {
#pragma cdir nodep
    for(size_t col = mat.off[row]; col < mat.off[row + 1]; col++) {
      matvalp[col] *= idfp[idxp[col]];
    }
  }
}

/*
  assume that document x term matrix
  (i.e. document is row, term is column)

  Though TF-IDF has vaious definitions, we adopted definition in
  Japanese wikipedia (2015/8/4)
  tf: number of term / all number of terms in the doc
  idf: log (number of all docs / number of docs that includes the term)
 */
template <class T>
void tfidf(const string& input, const string& output) {
  auto mat = make_crs_matrix_load<T>(input);
  crs_matrix<T> tf(mat.data.map(calc_tf<T>));
  auto idf = calc_idf<T>(mat);
  auto bcast_idf = make_node_local_broadcast(idf);
  tf.data.mapv(mul_idf<T>, bcast_idf);
  tf.save(output);
}

int main(int argc, char* argv[]){
  use_frovedis use(argc, argv);

  using namespace boost::program_options;

  options_description opt("option");
  opt.add_options()
    ("input,i", value<string>(), "input matrix file")
    ("output,o", value<string>(), "output matrix file");

  variables_map argmap;
  store(command_line_parser(argc,argv).options(opt).allow_unregistered().
        run(), argmap);
  notify(argmap);

  string input, output;
  
  if(argmap.count("input")){
    input = argmap["input"].as<string>();
  } else {
    cerr << "input matrix file is not specified" << endl;
    exit(1);
  }

  if(argmap.count("output")){
    output = argmap["output"].as<string>();
  } else {
    cerr << "output matrix file is not specified" << endl;
    exit(1);
  }

  tfidf<double>(input, output);
}
