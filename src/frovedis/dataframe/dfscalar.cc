#include "dfscalar.hpp"

namespace frovedis {

std::shared_ptr<dfscalar> 
typed_dfscalar<datetime>::type_cast(const std::string& to_type) {
  if(to_type == "datetime") {
    return std::make_shared<typed_dfscalar<datetime>>(*this);
  } else if(to_type == "string") {
    auto str = datetime_to_string(val, "%Y-%m-%d"); // default format
    return std::make_shared<typed_dfscalar<std::string>>(str);
  } else {
    throw std::runtime_error("dfscalar<datetime>::type_cast: unsupported type: " + to_type);
  }
}

}
