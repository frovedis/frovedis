/*
  This is used for showing topic when SVD is used for LSA
 */
#include <frovedis.hpp>
#include <frovedis/matrix/colmajor_matrix.hpp>
#include <boost/program_options.hpp>

using namespace boost;
using namespace std;

void show_topic(const string& input, const string& dictionary, int num) {
  ifstream dicstr(dictionary.c_str());
  string line;
  vector<string> dic;
  while(getline(dicstr, line)) {
    dic.push_back(line);
  }
  auto matrix = frovedis::colmajor_matrix_local<double>
    (frovedis::make_rowmajor_matrix_local_load<double>(input));
  vector<vector<pair<string,double>>> topic(matrix.local_num_col);
  for(size_t c = 0; c < matrix.local_num_col; c++) {
    vector<pair<double,int>> v;
    for(size_t r = 0; r < matrix.local_num_row; r++) {
      v.push_back(make_pair(abs(matrix.val[matrix.local_num_row * c + r]), r));
    }
    sort(v.begin(), v.end(), greater<pair<double,int>>());
    for(auto it = v.begin(); it != v.end(); ++it) {
      topic[c].push_back(make_pair(dic[it->second], it->first));
    }
  }
  for(size_t t = 0; t < topic.size(); t++) {
    cout << "topic " << t << ": ";
    for(int i = 0; i < num; i++) {
      cout << topic[t][i].first << "(" << topic[t][i].second << ") ";
    }
    cout << endl;
  }
}

int main(int argc, char* argv[]){
  using namespace boost::program_options;

  options_description opt("option");
  opt.add_options()
    ("input,i", value<string>(), "input vector file")
    ("dictionary,d", value<string>(), "dictionary file")
    ("num,n", value<int>(), "number of words per topic to show");

  variables_map argmap;
  store(command_line_parser(argc,argv).options(opt).allow_unregistered().
        run(), argmap);
  notify(argmap);

  string input, dictionary;
  int num;
  
  if(argmap.count("input")){
    input = argmap["input"].as<string>();
  } else {
    cerr << "input file is not specified" << endl;
    exit(1);
  }

  if(argmap.count("dictionary")){
    dictionary = argmap["dictionary"].as<string>();
  } else {
    cerr << "dictionary file is not specified" << endl;
    exit(1);
  }

  if(argmap.count("num")){
    num = argmap["num"].as<int>();
  } else {
    num = 10;
  }

  show_topic(input, dictionary, num);
}
