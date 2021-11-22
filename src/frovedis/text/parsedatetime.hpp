#ifndef PARSEDATETIME_HPP
#define PARSEDATETIME_HPP

#include "words.hpp"

typedef int64_t datetime_t;

/*
  only supports %Y(year), %m(month), %d(day), %H(hour), %M(minute), %S(second)
  and %b(abbreviated month name: Jan, Feb...)
  result is 64bit signed integer, which is nano second from unix epoch
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

}
#endif
