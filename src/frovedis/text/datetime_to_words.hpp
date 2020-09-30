#ifndef DATETIME_TO_WORDS_HPP
#define DATETIME_TO_WORDS_HPP

#include "parsedatetime.hpp"
#include "int_to_words.hpp"

/*
  only supports %Y(year), %m(month), %d(day), %H(hour), %M(minute), %S(second)
  input is 64bit unsigned integer.
  each bytes are: |0|Y|Y|m|d|H|M|S|
*/

namespace frovedis {

void year_from_datetime(const datetime_t* srcp, size_t size, int* dstp);
void month_from_datetime(const datetime_t* srcp, size_t size, int* dstp);
void day_from_datetime(const datetime_t* srcp, size_t size, int* dstp);
void hour_from_datetime(const datetime_t* srcp, size_t size, int* dstp);
void minute_from_datetime(const datetime_t* srcp, size_t size, int* dstp);
void second_from_datetime(const datetime_t* srcp, size_t size, int* dstp);
std::vector<int> year_from_datetime(const std::vector<datetime_t>& src);
std::vector<int> month_from_datetime(const std::vector<datetime_t>& src);
std::vector<int> day_from_datetime(const std::vector<datetime_t>& src);
std::vector<int> hour_from_datetime(const std::vector<datetime_t>& src);
std::vector<int> minute_from_datetime(const std::vector<datetime_t>& src);
std::vector<int> second_from_datetime(const std::vector<datetime_t>& src);

words datetime_to_words(const datetime_t* srcp, size_t src_size,
                        const std::string& format);

words datetime_to_words(const std::vector<datetime_t>& src,
                        const std::string& format);

}
#endif
