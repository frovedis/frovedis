#include <frovedis.hpp>
#include <frovedis/matrix/crs_matrix.hpp>

#include <boost/program_options.hpp>

using namespace boost;
using namespace frovedis;
using namespace std;

vector<size_t> reduce_word_count(const vector<size_t>& l,
                                 const vector<size_t>& r) {
  vector<size_t> ret(l.size());
  for(size_t i = 0; i < l.size(); i++) {
    ret[i] = l[i] + r[i];
  }
  return ret;
}

vector<size_t> each_word_count(crs_matrix_local<int>& mat) {
  vector<size_t> ret(mat.local_num_col);
  size_t* retp = &ret[0];
  size_t* idxp = &mat.idx[0];
  for(size_t r = 0; r < mat.local_num_row; r++) {
#pragma cdir nodep    
    for(size_t c = mat.off[r]; c < mat.off[r+1]; c++) {
      retp[idxp[c]]++;
    }
  }
  return ret;
}

struct shrink {
  shrink(){}
  shrink(int m, vector<size_t> w) :
    min_article_length(m), word_to_remove(w) {}
  crs_matrix_local<int> operator()(crs_matrix_local<int>& mat,
                                   vector<size_t>& removed_article) {
    vector<size_t> id_change(mat.local_num_col);
    size_t remove_idx = 0;
    size_t current_idx = 0;
    for(size_t i = 0; i < id_change.size(); i++) {
      if(remove_idx < word_to_remove.size() &&
         i == word_to_remove[remove_idx]) {
        id_change[i] = -1;
        remove_idx++;
      } else {
        id_change[i] = current_idx++;
      }
    }
    crs_matrix_local<int> ret; // already ret.off[0] == 0
    for(size_t r = 0; r < mat.local_num_row; r++) {
      size_t word_count = 0;
      for(size_t c = mat.off[r]; c < mat.off[r+1]; c++) {
        if(!binary_search(word_to_remove.begin(), word_to_remove.end(),
                          mat.idx[c])) word_count++;
      }
      if(word_count < static_cast<size_t>(min_article_length)) {
        removed_article.push_back(r);
      } else {
        size_t tmpoff = 0;
        for(size_t c = mat.off[r]; c < mat.off[r+1]; c++) {
          if(!binary_search(word_to_remove.begin(), word_to_remove.end(),
                            mat.idx[c])) {
            ret.idx.push_back(id_change[mat.idx[c]]);
            ret.val.push_back(mat.val[c]);
            tmpoff++;
          }
        }
        ret.off.push_back(tmpoff + *(ret.off.end() - 1));
      }
    }
    ret.set_local_num(mat.local_num_col - word_to_remove.size());
    return ret;
  }
  int min_article_length;
  vector<size_t> word_to_remove;
  SERIALIZE(min_article_length, word_to_remove);
};

/*
  assume that document x term matrix
  (i.e. document is row, term is column)

  contents of matrix are all int
 */
void shrink_mat(const string& input_matrix,
                const string& input_dictionary,
                const string& input_article,
                const string& output_matrix,
                const string& output_dictionary,
                const string& output_article,
                int min_word_used,
                int min_article_length) {
  vector<string> dictionary, article;
  ifstream dicstr(input_dictionary.c_str());
  string line;
  while(getline(dicstr, line)) {
    dictionary.push_back(line);
  }
  ifstream artstr(input_article.c_str());
  while(getline(artstr, line)) {
    article.push_back(line);
  }
  auto mat = make_crs_matrix_load<int>(input_matrix, dictionary.size());
  auto word_count = mat.data.map(each_word_count).reduce(reduce_word_count);
  vector<size_t> word_to_remove;
  for(size_t i = 0; i < word_count.size(); i++) {
    if(word_count[i] < static_cast<size_t>(min_word_used))
      word_to_remove.push_back(i);
  }
  auto removed_article_nloc = make_node_local_allocate<vector<size_t>>();
  crs_matrix<int> shrinked
    (mat.data.map<crs_matrix_local<int>>(shrink(min_article_length,
                                                word_to_remove),
                                         removed_article_nloc));
  // crs_get_local_num_row<T> is defined in crs_matrix.hpp
  auto local_num_rows = shrinked.data.map(crs_get_local_num_row<int,size_t,size_t>).gather();
  size_t num_row = 0;
  for(size_t i = 0; i < local_num_rows.size(); i++)
    num_row += local_num_rows[i];
  shrinked.set_num(num_row, mat.num_col - word_to_remove.size());

  vector<string> shrinked_dictionary, shrinked_article;
  size_t remove_idx = 0;
  for(size_t i = 0; i < dictionary.size(); i++) {
    if(remove_idx < word_to_remove.size() && i == word_to_remove[remove_idx]) {
      remove_idx++;
    } else {
      shrinked_dictionary.push_back(dictionary[i]);
    }
  }

  auto original_local_num_rows =
    mat.data.map(crs_get_local_num_row<int,size_t,size_t>).gather();
  auto local_removed_articles = removed_article_nloc.gather();
  vector<size_t> global_removed_article;
  size_t offset = 0;
  for(size_t i = 0; i < local_removed_articles.size(); i++) {
    for(size_t j = 0; j < local_removed_articles[i].size(); j++) {
      global_removed_article.push_back(local_removed_articles[i][j] + offset);
    }
    offset += original_local_num_rows[i];
  }
  remove_idx = 0;
  for(size_t i = 0; i < article.size(); i++) {
    if(remove_idx < global_removed_article.size()
       && i == global_removed_article[remove_idx]) {
      remove_idx++;
    } else {
      shrinked_article.push_back(article[i]);
    }
  }
  ofstream shrinked_dicstr(output_dictionary.c_str());
  for(auto i: shrinked_dictionary) shrinked_dicstr << i << "\n";
  ofstream shrinked_artstr(output_article.c_str());
  for(auto i: shrinked_article) shrinked_artstr << i << "\n";
  shrinked.save(output_matrix);
}

int main(int argc, char* argv[]){
  use_frovedis use(argc, argv);

  using namespace boost::program_options;

  options_description opt("option");
  opt.add_options()
    ("help,h", "print help")
    ("input-matrix", value<string>(), "input matrix file")
    ("input-dictionary", value<string>(), "input dictionary file")
    ("input-article", value<string>(), "input article file")
    ("output-matrix", value<string>(), "output matrix file")
    ("output-dictionary", value<string>(), "output dictionary file")
    ("output-article", value<string>(), "output article file")
    ("min-word-used", value<int>(), "words used less than this is omitted")
    ("min-article-length", value<int>(), "article whose words is less than this is omitted ");

  variables_map argmap;
  store(command_line_parser(argc,argv).options(opt).allow_unregistered().
        run(), argmap);
  notify(argmap);

  string input_matrix, input_dictionary, input_article;
  string output_matrix, output_dictionary, output_article;
  int min_word_used, min_article_length;
  
  if(argmap.count("help")){
    cerr << opt << endl;
    exit(1);
  }

  if(argmap.count("input-matrix")){
    input_matrix = argmap["input-matrix"].as<string>();
  } else {
    cerr << "input matrix file is not specified" << endl;
    cerr << opt << endl;
    exit(1);
  }

  if(argmap.count("input-dictionary")){
    input_dictionary = argmap["input-dictionary"].as<string>();
  } else {
    cerr << "input dictionary file is not specified" << endl;
    cerr << opt << endl;
    exit(1);
  }

  if(argmap.count("input-article")){
    input_article = argmap["input-article"].as<string>();
  } else {
    cerr << "input article file is not specified" << endl;
    cerr << opt << endl;
    exit(1);
  }

  if(argmap.count("output-matrix")){
    output_matrix = argmap["output-matrix"].as<string>();
  } else {
    cerr << "output matrix file is not specified" << endl;
    cerr << opt << endl;
    exit(1);
  }

  if(argmap.count("output-dictionary")){
    output_dictionary = argmap["output-dictionary"].as<string>();
  } else {
    cerr << "output dictionary file is not specified" << endl;
    cerr << opt << endl;
    exit(1);
  }

  if(argmap.count("output-article")){
    output_article = argmap["output-article"].as<string>();
  } else {
    cerr << "output article file is not specified" << endl;
    cerr << opt << endl;
    exit(1);
  }

  if(argmap.count("min-word-used")){
    min_word_used = argmap["min-word-used"].as<int>();
  } else {
    cerr << "min-word-used is not specified" << endl;
    cerr << opt << endl;
    exit(1);
  }

  if(argmap.count("min-article-length")){
    min_article_length = argmap["min-article-length"].as<int>();
  } else {
    cerr << "min-article-length is not specified" << endl;
    cerr << opt << endl;
    exit(1);
  }

  shrink_mat(input_matrix, input_dictionary, input_article,
             output_matrix, output_dictionary, output_article,
             min_word_used, min_article_length);
}
