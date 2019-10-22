#include <frovedis.hpp>
#include <frovedis/matrix/pca.hpp>

#include <boost/program_options.hpp>

using namespace boost;
using namespace frovedis;
using namespace std;

void do_pca(const string& input_matrix,
            const string& pca_directions_file,
            const string& pca_scores_file,
            const string& eigen_values_file,
            const string& explained_variance_ratio_file,
            const string& singular_values_file,
            int k,
            bool to_standardize,
            bool binary) {
  time_spent t(DEBUG);
  rowmajor_matrix<double> matrix;
  if(binary) {
    matrix = make_rowmajor_matrix_loadbinary<double>(input_matrix);
  } else {
    matrix = make_rowmajor_matrix_load<double>(input_matrix);
  }
  t.show("load time: ");
  colmajor_matrix<double> pca_directions;
  colmajor_matrix<double> pca_scores;
  std::vector<double> eigen_values;
  std::vector<double> explained_variance_ratio;
  std::vector<double> singular_values;
  std::vector<double> mean; // not used
  double noise_variance; // not used

  pca(std::move(matrix), pca_directions, pca_scores, eigen_values,
      explained_variance_ratio, singular_values, mean, noise_variance,
      k, to_standardize);
  t.show("PCA: ");
  if(binary) {
    pca_directions.savebinary(pca_directions_file);
    if(pca_scores_file != "") {
      pca_scores.savebinary(pca_scores_file);
    }
    if(eigen_values_file != "") {
      make_dvector_scatter(eigen_values).savebinary(eigen_values_file);
    }
    if(explained_variance_ratio_file != "") {
      make_dvector_scatter(explained_variance_ratio).
        savebinary(explained_variance_ratio_file);
    }
    if(singular_values_file != "") {
      make_dvector_scatter(singular_values).
        savebinary(singular_values_file);
    }
  } else {
    pca_directions.save(pca_directions_file);
    if(pca_scores_file != "") {
      pca_scores.save(pca_scores_file);
    }
    if(eigen_values_file != "") {
      make_dvector_scatter(eigen_values).saveline(eigen_values_file);
    }
    if(explained_variance_ratio_file != "") {
      make_dvector_scatter(explained_variance_ratio).
        saveline(explained_variance_ratio_file);
    }
    if(singular_values_file != "") {
      make_dvector_scatter(singular_values).
        saveline(singular_values_file);
    }
  }
  t.show("save time: ");
}

int main(int argc, char* argv[]) {
  use_frovedis use(argc, argv);

  using namespace boost::program_options;

  options_description opt("option");
  opt.add_options()
    ("help,h", "print help")
    ("input,i", value<string>(), "input matrix")
    ("pca_directions,d", value<string>(),
     "PCA directions (components in scikit-learn)")
    ("pca_scores,c", value<string>(),
     "PCA score (no counter part in scikit-learn) [option]")
    ("eigen_values,e", value<string>(),
     "eigen values (explained_variance in scikit-learn) [option]")
    ("explained_variance_ratio,r", value<string>(),
     "explained variance ratio [option]")
    ("singular_values,s", value<string>(), "singular values [option]")
    ("to_standardize,t", "standardize the input [default: false]")
    ("k,k", value<int>(), "number of principal components to compute")
    ("verbose", "set loglevel DEBUG")
    ("verbose2", "set loglevel TRACE")
    ("binary,b", "use binary input/output");

  variables_map argmap;
  store(command_line_parser(argc,argv).options(opt).allow_unregistered().
        run(), argmap);
  notify(argmap);
  string input, pca_directions, pca_scores, eigen_values,
    explained_variance_ratio, singular_values;
  int k;
  bool binary = false;
  bool to_standardize = false;
  
  if(argmap.count("help")){
    cerr << opt << endl;
    return 1;
  }

  if(argmap.count("input")){
    input = argmap["input"].as<string>();
  } else {
    cerr << "input is not specified" << endl;
    cerr << opt << endl;
    return 1;
  }

  if(argmap.count("pca_directions")){
    pca_directions = argmap["pca_directions"].as<string>();
  } else {
    cerr << "file to store PCA directions is not specified" << endl;
    cerr << opt << endl;
    return 1;
  }

  if(argmap.count("pca_scores")){
    pca_scores = argmap["pca_scores"].as<string>();
  }

  if(argmap.count("eigen_values")){
    eigen_values = argmap["eigen_values"].as<string>();
  }

  if(argmap.count("explained_variance_ratio")){
    explained_variance_ratio = argmap["explained_variance_ratio"].as<string>();
  }

  if(argmap.count("singular_values")){
    singular_values = argmap["singular_values"].as<string>();
  }

  if(argmap.count("to_standardize")){
    to_standardize = true;
  }

  if(argmap.count("k")){
    k = argmap["k"].as<int>();
  } else {
    cerr << "number of singular value to compute is not specified" << endl;
    cerr << opt << endl;
    return 1;
  }

  if(argmap.count("binary")){
    binary = true;
  }

  if(argmap.count("verbose")){
    set_loglevel(DEBUG);
  }

  if(argmap.count("verbose2")){
    set_loglevel(TRACE);
  }

  try {
    do_pca(input, pca_directions, pca_scores, eigen_values,
           explained_variance_ratio, singular_values, k, to_standardize, binary);
  } catch (std::exception& e) {
    cerr << e.what() << endl;
    return 1;
  }
}
