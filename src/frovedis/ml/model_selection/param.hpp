#ifndef _PARAM_T_
#define _PARAM_T_

#include <string>
#include <boost/lexical_cast.hpp>
#include <frovedis/ml/macro.hpp>

#ifdef USE_BOOST
#include <boost/variant.hpp>
#endif

namespace frovedis {

#ifdef USE_BOOST
struct param_t {
  param_t() {}
  param_t(int t): tt(t) {}
  param_t(long t): tt(t) {}
  param_t(float t): tt(t) {}
  param_t(double t): tt(t) {}
  param_t(unsigned int t): tt(t) {}
  param_t(unsigned long t): tt(t) {}
  param_t(long long t): tt(t) {}
  param_t(unsigned long long t): tt(t) {}
  param_t(const char* t): tt(std::string(t)) {} 
  param_t(const std::string& t): tt(t) {}
  param_t(char t): tt(t) {}
  param_t(bool t): tt(t) {}

  template <class T>
  T get() { 
    return boost::get<T>(tt);
  }

  boost::variant<int, long, float, double, 
                 unsigned int, // size_t
                 unsigned long, 
                 long long, unsigned long long, 
                 std::string, char, bool> tt;
  SERIALIZE(tt);
};

#else

struct param_t {
  param_t() {}
  param_t(int t): tt(STR(t)) {} 
  // {std::cout << "int: " << tt << std::endl;}
  param_t(long t): tt(STR(t)) {} 
  //  {std::cout << "long: " << tt << std::endl;}
  param_t(float t): tt(STR(t)) {} 
  // {std::cout << "float: " << tt << std::endl;}
  param_t(double t): tt(STR(t)) {} 
  // {std::cout << "double: " << tt << std::endl;}
  param_t(unsigned int t): tt(STR(t)) {} 
  // {std::cout << "ui: " << tt << std::endl;}
  param_t(unsigned long t): tt(STR(t)) {} 
  // {std::cout << "ul: " << tt << std::endl;}
  param_t(long long t): tt(STR(t)) {} 
  // {std::cout << "ll: " << tt << std::endl;}
  param_t(unsigned long long t): tt(STR(t)) {} 
  // {std::cout << "ull: " << tt << std::endl;}
  param_t(const char* t): tt(t) {} 
  // {std::cout << "const char*: " << tt << std::endl;}
  param_t(const std::string& t): tt(t) {} 
  // {std::cout << "str: " << tt << std::endl;}
  param_t(char t): tt(STR(t)) {} 
  // {std::cout << "char: " << tt << std::endl;}
  param_t(bool t): tt(STR(t)) {} 
  // {std::cout << "bool: " << tt << std::endl;}

  template <class T>
  T get() { 
    return boost::lexical_cast<T>(tt);
  }

  std::string tt;
  SERIALIZE(tt);
};
#endif

using paraml = std::vector<std::vector<std::pair<std::string, param_t>>>;

template <class T>
std::ostream& 
operator<<(std::ostream& str,
           param_t& t) {
  str << t.get<T>();
  return str;
}

}
#endif
