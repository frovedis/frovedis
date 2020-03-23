#include "dftable.hpp"
#include "make_dftable_string.hpp"
#include <boost/tokenizer.hpp>
#include <boost/lexical_cast.hpp>

namespace frovedis {

using namespace std;
using namespace boost;

vector<vector<string>>
separate_dvector_helper(vector<string>& vs, size_t num_col,
                        bool skip_first_line,
                        string& sep) {
  vector<vector<string>> ret;
  ret.resize(num_col);
  size_t i;
  if(get_selfid() == 0 && skip_first_line) i = 1; else i = 0;
  for(; i < vs.size(); i++) {
    if(sep == "") {
      tokenizer<escaped_list_separator<char>> tok(vs[i]);
      size_t current = 0;
      auto it = tok.begin();
      for(; it != tok.end() && current < num_col; ++it, current++) {
        ret[current].push_back(*it);
      }
      if(it != tok.end() || current != num_col)
        throw
          std::runtime_error(string("invalid number of columns: ") + vs[i]);
    } else {
      escaped_list_separator<char> els("\\", sep, "\"");
      tokenizer<escaped_list_separator<char>> tok(vs[i], els);
      size_t current = 0;
      auto it = tok.begin();
      for(; it != tok.end() && current < num_col; ++it, current++) {
        ret[current].push_back(*it);
      }
      if(it != tok.end() || current != num_col)
        throw
          std::runtime_error(string("invalid number of columns: ") + vs[i]);
    }
  } 
  return ret;
}

struct pick_ith {
  pick_ith(){}
  pick_ith(size_t i) : i(i) {}
  void operator()(vector<vector<string>>& vs, vector<string>& r) {
    r.swap(vs[i]);
  }
  size_t i;
  SERIALIZE(i)
};

vector<dvector<string>> separate_dvector(dvector<string>& d,
                                         bool skip_first_line,
                                         const string& sep) {
  vector<dvector<string>> ret;
  if(d.size() == 0) return ret;
  else {
    string s = d.get(0);
    size_t num_col = 0;
    if(sep == "") {
      tokenizer<escaped_list_separator<char>> tok(s);
      for(auto it = tok.begin(); it != tok.end(); ++it) num_col++;
    } else {
      escaped_list_separator<char> els("\\", sep, "\"");
      tokenizer<escaped_list_separator<char>> tok(s, els);
      for(auto it = tok.begin(); it != tok.end(); ++it) num_col++;
    }
    auto nls = d.viewas_node_local().map(separate_dvector_helper,
                                         broadcast(num_col),
                                         broadcast(skip_first_line),
                                         broadcast(sep));
    ret.resize(num_col);
    for(size_t i = 0; i < num_col; i++) {
      node_local<vector<string>> tmp =
        make_node_local_allocate<vector<string>>();
      nls.mapv(pick_ith(i), tmp);
      ret[i] = tmp.moveto_dvector<string>();
    }
    return ret;
  }
}

template <class T>
struct parsetype {
  T operator()(string& t){return boost::lexical_cast<T>(t);}
  SERIALIZE_NONE
};

// assuming LP64: long is 64bit
// internally long long != long so used long for 64bit
std::shared_ptr<dfcolumn> parse_dvector(dvector<string>& dv,
                                        const string& type) {
  if(type == "string") {
    return std::make_shared<typed_dfcolumn<string>>(dv);
  } else if(type == "unsigned int") {
    auto pdv = dv.map<unsigned int>(parsetype<unsigned int>());
    return std::make_shared<typed_dfcolumn<unsigned int>>(pdv);
  } else if(type == "int") {
    auto pdv = dv.map<int>(parsetype<int>());
    return std::make_shared<typed_dfcolumn<int>>(pdv);
  } else if(type == "double") {
    auto pdv = dv.map<double>(parsetype<double>());
    return std::make_shared<typed_dfcolumn<double>>(pdv);
  } else if(type == "float") {
    auto pdv = dv.map<float>(parsetype<float>());
    return std::make_shared<typed_dfcolumn<float>>(pdv);
  } else if(type == "unsigned long") {
    auto pdv = dv.map<unsigned long>(parsetype<unsigned long>());
    return std::make_shared<typed_dfcolumn<unsigned long>>(pdv);
  } else if(type == "long") {
    auto pdv = dv.map<long>(parsetype<long>());
    return std::make_shared<typed_dfcolumn<long>>(pdv);
  } else throw std::runtime_error("unknown type: " + type);
}

dftable make_dftable_string(dvector<string>& d,
                            const vector<string>& types,
                            const vector<string>& names) {
  dftable ret;
  auto dvs = separate_dvector(d, false, string());
  if(dvs.size() != types.size() || dvs.size() != names.size())
    throw runtime_error("invalid number of colums, types, or names");
  for(size_t i = 0; i < dvs.size(); i++) {
    auto col = parse_dvector(dvs[i], types[i]);
    ret.append_column(names[i], col);
  }
  return ret;
}

// use first line as column name
dftable make_dftable_string(dvector<string>& d,
                            const vector<string>& types) {
  dftable ret;
  if(d.size() == 0) return ret;
  auto first_line = d.get(0);
  vector<string> names;
  tokenizer<escaped_list_separator<char>> tok(first_line);
  for(auto it = tok.begin(); it != tok.end(); ++it)
    names.push_back(*it);
  auto dvs = separate_dvector(d, true, string());
  if(dvs.size() != types.size() || dvs.size() != names.size())
    throw runtime_error("invalid number of colums, types, or names");
  for(size_t i = 0; i < dvs.size(); i++) {
    auto col = parse_dvector(dvs[i], types[i]);
    ret.append_column(names[i], col);
  }
  return ret;
}

dftable make_dftable_string(dvector<string>& d,
                            const vector<string>& types,
                            const vector<string>& names,
                            const string& sep) {
  dftable ret;
  time_spent t(DEBUG);
  auto dvs = separate_dvector(d, false, sep);
  t.show("separate_dvector: ");
  if(dvs.size() != types.size() || dvs.size() != names.size())
    throw runtime_error("invalid number of colums, types, or names");
  for(size_t i = 0; i < dvs.size(); i++) {
    auto col = parse_dvector(dvs[i], types[i]);
    ret.append_column(names[i], col);
  }
  t.show("parse_dvector: ");
  return ret;
}

// use first line as column name
dftable make_dftable_string(dvector<string>& d,
                            const vector<string>& types,
                            const string& sep) {
  dftable ret;
  if(d.size() == 0) return ret;
  auto first_line = d.get(0);
  vector<string> names;
  escaped_list_separator<char> els("\\", sep, "\"");
  tokenizer<escaped_list_separator<char>> tok(first_line, els);
  for(auto it = tok.begin(); it != tok.end(); ++it)
    names.push_back(*it);
  auto dvs = separate_dvector(d, true, sep);
  if(dvs.size() != types.size() || dvs.size() != names.size())
    throw runtime_error("invalid number of colums, types, or names");
  for(size_t i = 0; i < dvs.size(); i++) {
    auto col = parse_dvector(dvs[i], types[i]);
    ret.append_column(names[i], col);
  }
  return ret;
}

}
