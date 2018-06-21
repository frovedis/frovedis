#include <frovedis.hpp>
#include <frovedis/matrix/crs_matrix.hpp>

#include <boost/program_options.hpp>

using namespace boost;
using namespace frovedis;
using namespace std;

/*
  assume that document x term matrix
  (i.e. document is row, term is column)
 */
void show_matrix(const string& matrix_file,
                 const string& dictionary_file,
                 const string& article_file) {

  vector<string> dictionary, article;
  ifstream dicstr(dictionary_file.c_str());
  string line;
  while(getline(dicstr, line)) {
    dictionary.push_back(line);
  }
  ifstream artstr(article_file.c_str());
  while(getline(artstr, line)) {
    article.push_back(line);
  }
  auto mat = make_crs_matrix_local_load<double>(matrix_file);
  for(size_t row = 0; row < mat.local_num_row; row++) {
    cout << article[row] << endl;
    for(size_t col = mat.off[row]; col < mat.off[row + 1]; col++) {
      cout << dictionary[mat.idx[col]] << ":" << mat.val[col] << " ";
    }
    cout << endl;
  }
}

int main(int argc, char* argv[]){
  use_frovedis use(argc, argv);

  using namespace boost::program_options;

  options_description opt("option");
  opt.add_options()
    ("help,h", "print help")
    ("matrix,m", value<string>(), "input matrix file")
    ("dictionary,d", value<string>(), "input dictionary file")
    ("article,a", value<string>(), "input article file");

  variables_map argmap;
  store(command_line_parser(argc,argv).options(opt).allow_unregistered().
        run(), argmap);
  notify(argmap);

  string matrix, dictionary, article;
  
  if(argmap.count("help")){
    cerr << opt << endl;
    exit(1);
  }

  if(argmap.count("matrix")){
    matrix = argmap["matrix"].as<string>();
  } else {
    cerr << "input matrix file is not specified" << endl;
    cerr << opt << endl;
    exit(1);
  }

  if(argmap.count("dictionary")){
    dictionary = argmap["dictionary"].as<string>();
  } else {
    cerr << "input dictionary file is not specified" << endl;
    cerr << opt << endl;
    exit(1);
  }

  if(argmap.count("article")){
    article = argmap["article"].as<string>();
  } else {
    cerr << "input article file is not specified" << endl;
    cerr << opt << endl;
    exit(1);
  }

  show_matrix(matrix, dictionary, article);
}
