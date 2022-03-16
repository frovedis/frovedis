#ifndef DFSCALAR_HPP
#define DFSCALAR_HPP

#include <memory>
#include <boost/lexical_cast.hpp>
#include "dfutil.hpp"
#include "../text/parsedatetime.hpp"
#include "../text/datetime_utility.hpp"

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

struct datetime {}; // for tag

template <>
struct typed_dfscalar<datetime> : public typed_dfscalar<datetime_t> {
  typed_dfscalar(): typed_dfscalar<datetime_t>() {}
  typed_dfscalar(datetime_t val) : typed_dfscalar<datetime_t>(val) {}
  virtual std::shared_ptr<dfscalar> type_cast(const std::string& to_type);
  virtual std::string dtype() const { return "datetime"; }
};

template <>
struct typed_dfscalar<std::string> : public dfscalar {
  typed_dfscalar(){}
  typed_dfscalar(std::string val) : val(val) {}
  virtual std::shared_ptr<dfscalar> type_cast(const std::string& to_type) {
    if(to_type == "int") {
      auto c_val = boost::lexical_cast<int>(val);
      return std::make_shared<typed_dfscalar<int>>(c_val);
    } else if(to_type == "unsigned int") {
      auto c_val = boost::lexical_cast<unsigned int>(val);
      return std::make_shared<typed_dfscalar<unsigned int>>(c_val);
    } else if(to_type == "long") {
      auto c_val = boost::lexical_cast<long>(val);
      return std::make_shared<typed_dfscalar<long>>(c_val);
    } else if(to_type == "unsigned long") {
      auto c_val = boost::lexical_cast<unsigned long>(val);
      return std::make_shared<typed_dfscalar<unsigned long>>(c_val);
    } else if(to_type == "float") {
      auto c_val = boost::lexical_cast<float>(val);
      return std::make_shared<typed_dfscalar<float>>(c_val);
    } else if(to_type == "double") {
      auto c_val = boost::lexical_cast<double>(val);
      return std::make_shared<typed_dfscalar<double>>(c_val);
    } else if(to_type == "datetime") { // default format: "%Y-%m-%d"
      auto c_val = parsedatetime(val, "%Y-%m-%d");
      return std::make_shared<typed_dfscalar<datetime>>(c_val);
    } else if(to_type.find("datetime:") == 0) {
      auto fmt = to_type.substr(9);
      auto c_val = parsedatetime(val, fmt);
      return std::make_shared<typed_dfscalar<datetime>>(c_val);
    } else {
      throw std::runtime_error("dfscalar<string>::type_cast: unsupported type: " + to_type);
    }
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
  } else if(to_type == "string") {
    return std::make_shared<typed_dfscalar<std::string>>(std::to_string(val));
  } else if(to_type == "datetime") {
    return std::make_shared<typed_dfscalar<datetime>>(static_cast<datetime_t>(val));
  } else {
    throw std::runtime_error("dfscalar<T>::type_cast: unsupported type: " + to_type);
  }
}

}
#endif
