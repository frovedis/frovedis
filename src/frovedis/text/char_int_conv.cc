#include "char_int_conv.hpp"
#include <stdexcept>
#include <iostream>

using namespace std;

namespace frovedis {

void char_to_int(const char* s, size_t size, int* v) {
  if(is_bigendian()) throw runtime_error("big endian is not supported");
  auto isize = size / 4;
  auto rest = size - isize * 4;
  const unsigned int* is = reinterpret_cast<const unsigned int*>(s);
  for(size_t i = 0; i < isize; i++) {
    auto tmp = is[i];
    v[i*4] = tmp & 0x000000FF;
    v[i*4 + 1] = (tmp >> 8) & 0x000000FF;
    v[i*4 + 2] = (tmp >> 16) & 0x000000FF;
    v[i*4 + 3] = tmp >> 24;
  }
  auto off = isize * 4;
  for(size_t i = 0; i < rest; i++) {
    v[off + i] = s[off + i];
  }
}

vector<int> char_to_int(const std::string& s) {
  auto size = s.size();
  vector<int> r(size);
  char_to_int(s.data(), size, r.data());
  return r;
}

vector<int> vchar_to_int(const std::vector<char>& s) {
  auto size = s.size();
  vector<int> r(size);
  char_to_int(s.data(), size, r.data());
  return r;
}

void int_to_char(const int* v, size_t size, char* s) {
  if(is_bigendian()) throw runtime_error("big endian is not supported");
  auto isize = size / 4;
  auto rest = size - isize * 4;
  unsigned int* is = reinterpret_cast<unsigned int*>(s);
  for(size_t i = 0; i < isize; i++) {
    unsigned int tmp = v[i*4];
    tmp |= v[i*4 + 1] << 8;
    tmp |= v[i*4 + 2] << 16;
    tmp |= v[i*4 + 3] << 24;
    is[i] = tmp;
  }
  auto off = isize * 4;
  for(size_t i = 0; i < rest; i++) {
    s[off + i] = static_cast<char>(v[off + i]);
  }
}

string int_to_char(const vector<int>& v) {
  auto size = v.size();
  string r;
  r.resize(size);
  int_to_char(v.data(), size, const_cast<char*>(r.data()));
  return r;
}

vector<char> int_to_vchar(const vector<int>& v) {
  auto size = v.size();
  vector<char> r(size);
  int_to_char(v.data(), size, r.data());
  return r;
}

}
