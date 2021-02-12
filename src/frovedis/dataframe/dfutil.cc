#include "dfutil.hpp"

namespace frovedis {

template<>
std::string get_dftype_name<int>(){return std::string("int");}

template<>
std::string get_dftype_name<unsigned int>(){return std::string("unsigned int");}

template<>
std::string get_dftype_name<long>(){return std::string("long");}

template<>
std::string get_dftype_name<unsigned long>(){
  return std::string("unsigned long");
}

template<>
std::string get_dftype_name<float>(){return std::string("float");}

template<>
std::string get_dftype_name<double>(){return std::string("double");}

template<>
std::string get_dftype_name<std::string>(){return std::string("string");}

}
