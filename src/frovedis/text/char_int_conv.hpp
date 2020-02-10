#ifndef CHAR_INT_CONV_HPP
#define CHAR_INT_CONV_HPP

#include <vector>
#include <string>
#include <cstddef>

namespace frovedis {

void char_to_int(const char* s, size_t size, int* v);
std::vector<int> char_to_int(const std::string&);

void int_to_char(const int* v, size_t size, char* s);
std::string int_to_char(const std::vector<int>&);
}

#endif
