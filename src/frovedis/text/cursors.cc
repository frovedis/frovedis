#include "cursors.hpp"
#include "char_int_conv.hpp"

using namespace std;

namespace frovedis {

struct is_char {
  is_char(int to_comp) : to_comp(to_comp) {}
  bool operator()(int a) const {return a == to_comp;}
  int to_comp;
};

void cursors::advance_string(const string& str) {
  for(size_t i = 0; i < str.size(); i++) {
    advance_if(is_char(static_cast<int>(str[i])));
  }
}

void cursors::merge(const cursors& to_merge) {
  std::vector<size_t> new_starts, new_ends;
  set_merge_pair(starts, ends, to_merge.starts, to_merge.ends,
                 new_starts, new_ends);
  starts.swap(new_starts);
  ends.swap(new_ends);
}

void cursors::debug_print() {
  cout << "starts: ";
  for(auto i: starts) cout << i << " ";
  cout << endl;
  cout << "ends: ";
  for(auto i: ends) cout << i << " ";
  cout << endl;
  
  auto size = starts.size();
  for(size_t i = 0; i < size; i++) {
    auto start = starts[i];
    auto len = ends[i] - starts[i];
    for(size_t j = 0; j < len; j++) {
      cout << static_cast<char>(chars[start+j]);
    }
    cout << " ";
  }
  cout << endl;
}

// Because the length (end[i] - begin[i]) is short,
// the vector length becomes short.
// it is possible to make it longer by loop raking,
// but this funciton is not the bottleneck anyway.
void mask_string(std::vector<int>& v,
                 const std::vector<size_t>& begin,
                 const std::vector<size_t>& end,
                 int mask_char) {
  auto size = begin.size();
  auto beginp = begin.data();
  auto endp = end.data();
  auto vp = v.data();
  for(size_t i = 0; i < size; i++) {
    auto start = vp + beginp[i];
    auto len = endp[i] - beginp[i];
    for(size_t j = 0; j < len; j++) {
      start[j] = mask_char;
    }
  }
}

std::vector<int> replace_string(const std::vector<int>& v,
                                const std::vector<size_t>& begin,
                                const std::vector<size_t>& end,
                                const std::string& replace_to) {
  auto vp = v.data();
  auto size = begin.size();
  auto beginp = begin.data();
  auto endp = end.data();
  auto replace_to_int = char_to_int(replace_to);
  auto replace_to_intp = replace_to_int.data();
  auto replace_to_size = replace_to_int.size();
  std::vector<ssize_t> diff(size);
  auto diffp = diff.data();
  for(size_t i = 0; i < size; i++) {
    diffp[i] =
      static_cast<ssize_t>(endp[i]) -
      static_cast<ssize_t>(beginp[i]) - 
      static_cast<ssize_t>(replace_to_size);
  }
  ssize_t diff_total = 0;
  for(size_t i = 0; i < size; i++) {
    diff_total += diffp[i];
  }
  auto retsize = static_cast<ssize_t>(v.size()) - diff_total;
  std::vector<int> ret(retsize);
  auto retp = ret.data();
  auto from = vp;
  auto to = retp;
  for(size_t i = 0; i < size; i++) {
    auto len = begin[i] - (from - vp);
    for(size_t j = 0; j < len; j++) {
      to[j] = from[j];
    }
    to += len;
    for(size_t j = 0; j < replace_to_size; j++) {
      to[j] = replace_to_intp[j];
    }
    from += (len + (endp[i] - beginp[i]));
    to += replace_to_size;
  }
  auto len = v.size() - endp[size-1];
  for(size_t j = 0; j < len; j++) {
    to[j] = from[j];
  }
  return ret;
}

}
