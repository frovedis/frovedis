#ifndef DATETIME_TO_WORDS_HPP
#define DATETIME_TO_WORDS_HPP

#include "parsedatetime.hpp"
#include "datetime_utility.hpp"
#include "int_to_words.hpp"

/*
  only supports %Y(year), %m(month), %d(day), %H(hour), %M(minute), %S(second)
*/

namespace frovedis {

words datetime_to_words(const datetime_t* srcp, size_t src_size,
                        const std::string& format);

words datetime_to_words(const std::vector<datetime_t>& src,
                        const std::string& format);

}
#endif
