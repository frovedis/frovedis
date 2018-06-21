#ifndef TIME_EXTRACT_HPP
#define TIME_EXTRACT_HPP

#include <time.h>

namespace frovedis {

enum time_extractor {
  SECOND,
  MINUTE,
  HOUR,
  DAY,
  MONTH,
  YEAR,
  WDAY,
  YDAY
};

struct time_extract {
  time_extract(){}
  time_extract(enum time_extractor to_extract) : to_extract(to_extract) {}
  int operator()(time_t time) {
    auto* tm = localtime(&time);
    switch(to_extract) {
    case SECOND:
      return tm->tm_sec;
      break;
    case MINUTE:
      return tm->tm_min;
      break;
    case HOUR:
      return tm->tm_hour;
      break;
    case DAY:
      return tm->tm_mday;
      break;
    case MONTH:
      return tm->tm_mon;
      break;
    case YEAR:
      return tm->tm_year;
      break;
    case WDAY:
      return tm->tm_wday;
      break;
    case YDAY:
      return tm->tm_yday;
      break;
    default:
      throw std::runtime_error("time_extract: invalid extractor");
    }
  }
  enum time_extractor to_extract;
  SERIALIZE(to_extract)
};

struct string_to_time {
  string_to_time(){}
  string_to_time(const std::string& format) : format(format) {}
  time_t operator()(const std::string& datetime) {
    struct tm tm;
    memset(&tm, 0, sizeof(struct tm));
    auto r = strptime(datetime.c_str(), format.c_str(), &tm);
    if(r == nullptr)
      throw std::runtime_error("string_to_time: invalid data/format"
                              + datetime + " " + format);
    return mktime(&tm);
  }
  std::string format;
  SERIALIZE(format)
};

}
#endif
