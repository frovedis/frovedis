#ifndef DFSCALAR_HPP
#define DFSCALAR_HPP

#include <memory>
#include "dfutil.hpp"

namespace frovedis {

struct dfscalar {
  virtual std::shared_ptr<dfscalar> type_cast(const std::string& to_type) = 0;
  virtual std::string dtype() const = 0;  
};

template <class T>
struct typed_dfscalar : public dfscalar {
  typed_dfscalar(){}
  typed_dfscalar(T val) : val(val) {}
  virtual std::shared_ptr<dfscalar> type_cast(const std::string& to_type);
  virtual std::string dtype() const {
    return get_dftype_name<T>();
  }

  T val;
};

template <>
struct typed_dfscalar<std::string> : public dfscalar {
  typed_dfscalar(){}
  typed_dfscalar(std::string val) : val(val) {}
  virtual std::shared_ptr<dfscalar> type_cast(const std::string& to_type) {
    throw std::runtime_error("unsupported type: std::string");
  }
  virtual std::string dtype() const { 
    return get_dftype_name<std::string>();
  }

  std::string val;
};

template <class T>
std::shared_ptr<dfscalar>
typed_dfscalar<T>::type_cast(const std::string& to_type) {
  if(to_type == "int") {
    return std::make_shared<typed_dfscalar<int>>(static_cast<int>(val));
  } else if(to_type == "unsigned int") {
    return std::make_shared<typed_dfscalar<unsigned int>>
      (static_cast<unsigned int>(val));
  } else if(to_type == "long") {
    return std::make_shared<typed_dfscalar<long>>(static_cast<long>(val));
  } else if(to_type == "unsigned long") {
    return std::make_shared<typed_dfscalar<unsigned long>>
      (static_cast<unsigned long>(val));
  } else if(to_type == "float") {
    return std::make_shared<typed_dfscalar<float>>(static_cast<float>(val));
  } else if(to_type == "double") {
    return std::make_shared<typed_dfscalar<double>>(static_cast<double>(val));
  } else {
    throw std::runtime_error("unsupported type: " + to_type);
  }
}


}
#endif
