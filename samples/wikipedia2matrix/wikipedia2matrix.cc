#include <frovedis.hpp>

#include <boost/program_options.hpp>
#include <boost/serialization/utility.hpp>

#include <fstream>

#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

using namespace boost;
using namespace frovedis;
using namespace std;

vector<string> dir_traverse(const string& input) {
  struct stat sb;
  if(stat(input.c_str(), &sb) != 0) {
    perror("error stat'ing input directory/file");
    throw runtime_error("error stat'ing input directory/file");
  }
  if(S_ISREG(sb.st_mode)) {
    vector<string> ret;
    ret.push_back(input);
    return ret;
  } else if(S_ISDIR(sb.st_mode)) {
    DIR *dp;
    dp = opendir(input.c_str());
    if(dp) {
      struct dirent *entry;
      vector<string> tmp;
      while((entry = readdir(dp))) {
        if(strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
          tmp.push_back(string(entry->d_name));
        }
      }
      closedir(dp);
      vector<string> ret;
      for(auto& i: tmp) {
        string input_tmp = *(input.end() - 1) == '/' ? input : input + "/";
        auto subdir = dir_traverse(input_tmp + i);
        ret.insert(ret.end(), subdir.begin(), subdir.end());
      }
      return ret;
    } else {
      perror("error opening directory in input directory/file");
      closedir(dp);
      throw runtime_error("error opening directory in input directory/file");
    }
  } else {
    return vector<string>();
  }
}

void append_file(string& input, vector<string>&raw) {
  auto t= input.find("<doc");
  if(t != string::npos) raw.push_back(std::move(input)); // omit null entry
}

vector<string> split(const string &str){
  vector<string> res;
  string tmp = str;
  char* sv;
  char* token = strtok_r(const_cast<char*>(tmp.c_str()), " \t\r\n", &sv);
  while(token != NULL) {
    res.push_back(string(token));
    token = strtok_r(NULL, " \t\r\n", &sv);
  }
  return res;
}

std::string trim(const std::string& str) {
  string::size_type start, end;
  for(start = 0; start < str.size(); start++) {
    if(isalpha(str[start])) break;
  }
  for(end = start; end < str.size(); end++) {
    if(!isalpha(str[end])) break;
  }
  return str.substr(start, end - start);
}

//<doc id="xx" url="xx" title="xx">
pair<string, vector<pair<string, int>>> parse_doc(const string& text, set<string>& ldic) {
  auto t = text.find("title=\"");
  if(t == string::npos) {
    RLOG(ERROR) << "title is not found" << endl;
    return pair<string, vector<pair<string,int>>>();
  }
  auto te = text.find("\">");
  if(te == string::npos) {
    RLOG(ERROR) << "doc tag is not closed?" << endl;
    return pair<string, vector<pair<string,int>>>();
  }
  string title = text.substr(t + 7, te - t - 7);
  auto words = split(text.substr(te + 2));
  std::map<string, int> wordmap;
  for(auto& i: words) {
    string tmp = trim(i);
    std::transform(tmp.begin(), tmp.end(), tmp.begin(), ::tolower);
    if(!tmp.empty()) wordmap[tmp]++;
  }
  vector<pair<string, int>> wc;
  wc.insert(wc.end(), wordmap.begin(), wordmap.end());
  for(auto& i: wc) {
    ldic.insert(i.first);
  }
  return make_pair(title, wc);
}

string get_article_name(pair<string, vector<pair<string, int>>>& doc) {
  return doc.first;
}

bool not_empty(const pair<string, vector<pair<string, int>>>& doc) {
  return doc.second.size() != 0;
}

vector<string> to_vector(const set<string>& dic) {
  vector<string> ret;
  ret.insert(ret.end(), dic.begin(), dic.end());
  return ret;
}

vector<string> string_set_union(const vector<string>& l,
                                const vector<string>& r) {
  vector<string> ret;
  set_union(l.begin(), l.end(),
            r.begin(), r.end(),
            inserter(ret, ret.begin()));
  return ret;
}

vector<pair<int,int>> make_mat(const pair<string, vector<pair<string, int>>>& doc,
                               const vector<string>& dic) {
  vector<pair<int,int>> ret;
  for(auto& i: doc.second) {
    int pos = lower_bound(dic.begin(), dic.end(), i.first) - dic.begin();
    ret.push_back(make_pair(pos, i.second));
  }
  return ret;
}

void wikipedia2matrix(const string& input, const string& matrix, const string& dictionary,
                      const string& article) {
  time_spent t;
  auto files = dir_traverse(input);
  auto raw = make_node_local_allocate<vector<string>>();
  for(auto& file: files) {
    auto onefile = make_dvector_load(file, "</doc>");
    onefile.mapv(append_file, raw);
  }
  t.show("file read: ");
  auto ldic = make_node_local_allocate<set<string>>();
  auto rawvec = raw.moveto_dvector<string>();
  auto parsed = rawvec.map(parse_doc, ldic);
  t.show("parse: ");
  parsed.inplace_filter(not_empty);
  parsed.sort();
  t.show("filter & sort: ");
  auto article_names = parsed.map(get_article_name).gather();
  t.show("get_article_name: ");
  auto vdic = ldic.map(to_vector).reduce(string_set_union);
  t.show("reduce: ");
  auto vldic = make_node_local_broadcast(vdic);
  t.show("dic bcast: ");
  auto mat = parsed.map(make_mat, vldic).gather();
  t.show("make_mat: ");

  ofstream dicstr(dictionary);
  for(auto& i: vdic) dicstr << i << endl;

  ofstream artstr(article);
  for(auto& i: article_names) artstr << i << endl;

  ofstream matstr(matrix);
  for(auto& doc: mat) {
    for(size_t i = 0; i < doc.size() - 1; i++) {
      matstr << doc[i].first << ":" << doc[i].second << " ";
    }
    matstr << doc[doc.size()-1].first << ":" << doc[doc.size()-1].second << endl;
  }
}

int main(int argc, char* argv[]){
  use_frovedis use(argc, argv);

  using namespace boost::program_options;

  options_description opt("option");
  opt.add_options()
    ("help,h", "print help")
    ("input,i", value<string>(), "input directory/file")
    ("matrix,m", value<string>(), "output matrix file")
    ("dictionary,d", value<string>(), "output dictionay file")
    ("article,a", value<string>(), "output article list file");

  variables_map argmap;
  store(command_line_parser(argc,argv).options(opt).allow_unregistered().
        run(), argmap);
  notify(argmap);

  string input, matrix, dictionary, article;

  if(argmap.count("help")){
    cerr << opt << endl;
    exit(1);
  }
  
  if(argmap.count("input")){
    input = argmap["input"].as<string>();
  } else {
    cerr << "input is not specified" << endl;
    cerr << opt << endl;
    exit(1);
  }

  if(argmap.count("matrix")){
    matrix = argmap["matrix"].as<string>();
  } else {
    cerr << "output matrix file is not specified" << endl;
    cerr << opt << endl;
    exit(1);
  }

  if(argmap.count("dictionary")){
    dictionary = argmap["dictionary"].as<string>();
  } else {
    cerr << "output dictionary file is not specified" << endl;
    cerr << opt << endl;
    exit(1);
  }

  if(argmap.count("article")){
    article = argmap["article"].as<string>();
  } else {
    cerr << "output article file is not specified" << endl;
    cerr << opt << endl;
    exit(1);
  }

  wikipedia2matrix(input, matrix, dictionary, article);
}
