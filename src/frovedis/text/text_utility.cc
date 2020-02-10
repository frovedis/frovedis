#include "text_utility.hpp"
#include "find.hpp"

using namespace std;

namespace frovedis {

void tolower(int* vp, size_t size) {
  int off = 'a' - 'A';
  for(size_t i = 0; i < size; i++) {
    if(vp[i] >= 'A' && vp[i] <= 'Z') vp[i] += off;
  }
}

void tolower(vector<int>& v) {
  tolower(v.data(), v.size());
}

void toupper(int* vp, size_t size) {
  int off = 'a' - 'A';
  for(size_t i = 0; i < size; i++) {
    if(vp[i] >= 'a' && vp[i] <= 'z') vp[i] -= off;
  }
}

void toupper(vector<int>& v) {
  toupper(v.data(), v.size());
}

void replace_to_spc(int* vp, size_t size, const std::string& s) {
  auto ssize = s.size();
  auto found = find(vp, size, s);
  auto foundp = found.data();
  auto found_size = found.size();
  for(size_t len = 0; len < ssize; len++) {
    for(size_t i = 0; i < found_size; i++) {
      vp[foundp[i] + len] = ' ';
    }
  }
}

void replace_to_spc(std::vector<int>& v, const std::string& s) {
  auto vp = v.data();
  auto size = v.size();
  replace_to_spc(vp, size, s);
}

}
