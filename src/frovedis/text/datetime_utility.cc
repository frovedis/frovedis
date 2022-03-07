#include "datetime_utility.hpp"
#include "datetime_to_words.hpp"

namespace frovedis {

int lastday_of_month[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

std::string datetime_type_to_string(datetime_type type) {
  if(type == datetime_type::year) return "year";
  else if(type == datetime_type::month) return "month";
  else if(type == datetime_type::day) return "day";
  else if(type == datetime_type::hour) return "hour";
  else if(type == datetime_type::minute) return "minute";
  else if(type == datetime_type::second) return "second";
  else if(type == datetime_type::nanosecond) return "nanosecond";
  else if(type == datetime_type::quarter) return "quarter";
  else if(type == datetime_type::dayofweek) return "dayofweek";
  else if(type == datetime_type::dayofyear) return "dayofyear";
  else if(type == datetime_type::weekofyear) return "weekofyear";
  else throw std::runtime_error("unknown datetime_type");
}

std::string datetime_to_string(datetime_t& src, const std::string& format) {
  auto ws = datetime_to_words({src}, format);
  return words_to_vector_string(ws)[0];
}

}
