#ifndef TEXT_UTILITY_HPP
#define TEXT_UTILITY_HPP

#include <cstddef>
#include <vector>
#include <string>

namespace frovedis {

// inplace update
void tolower(int* vp, size_t size);
void tolower(std::vector<int>& v);
void toupper(int* vp, size_t size);
void toupper(std::vector<int>& v);
/* during tokenization, utf-8 char (e.g. –, ’) can't be recognized as separator
   so just replace it to space */
void replace_to_spc(int* vp, size_t size, const std::string& s);
void replace_to_spc(std::vector<int>& v, const std::string& s);

}

#endif
