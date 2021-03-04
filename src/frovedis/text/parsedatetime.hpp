#ifndef PARSEDATETIME_HPP
#define PARSEDATETIME_HPP

#include <cstdint>
#include "parseint.hpp"

typedef uint64_t datetime_t;

/*
  only supports %Y(year), %m(month), %d(day), %H(hour), %M(minute), %S(second)
  and %b(abbreviated month name: Jan, Feb...)
  result is 64bit unsigned integer.
  each bytes are: |0|Y|Y|m|d|H|M|S|
  This is similar to Oracle's DATETIME format
  We do not use time_t, since it makes things difficult (e.g. locale, timezone)
*/

namespace frovedis {

std::vector<datetime_t>
parsedatetime(const std::vector<int>& chars,
              const std::vector<size_t>& starts,
              const std::vector<size_t>& lens,
              const std::string& format);

std::vector<datetime_t>
parsedatetime(const words& w, const std::string& format);

datetime_t parsedatetime(const std::string& datetime,
                         const std::string& format);

datetime_t makedatetime(int year, int month, int day,
                        int hour = 0, int minute = 0, int second = 0);

}
#endif


