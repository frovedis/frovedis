#ifndef DFUTIL_HPP
#define DFUTIL_HPP

#include <string>
#include <stdexcept>

namespace frovedis {
template <class T>
std::string get_dftype_name(){throw std::runtime_error("unknown type");}

template<> std::string get_dftype_name<int>();
template<> std::string get_dftype_name<unsigned int>();
template<> std::string get_dftype_name<long>();
template<> std::string get_dftype_name<unsigned long>();
template<> std::string get_dftype_name<float>();
template<> std::string get_dftype_name<double>();
template<> std::string get_dftype_name<std::string>();
}
#endif
