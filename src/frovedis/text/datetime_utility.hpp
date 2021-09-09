#ifndef DATETIME_UTILITY_HPP
#define DATETIME_UTILITY_HPP

#include "parsedatetime.hpp"
#include "../core/node_local.hpp"

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

/* Following utility functions should be inline, because they might be
   used in loop like calc */

inline datetime_t makedatetime(int year, int month, int day,
                               int hour = 0, int minute = 0, int second = 0) {
  datetime_t ret = ((datetime_t)year << 5 * 8) + ((datetime_t)month << 4 * 8) +
    ((datetime_t)day << 3 * 8) + ((datetime_t)hour << 2 * 8) +
    ((datetime_t)minute << 1 * 8) + (datetime_t)second;
    
  return ret;
}

inline int year_from_datetime(datetime_t t) {
  return t >> 5 * 8;
}
inline int month_from_datetime(datetime_t t) {
  return (t >> 4 * 8) & 0xFF;
}
inline int day_from_datetime(datetime_t t) {
  return (t >> 3 * 8) & 0xFF;
}
inline int hour_from_datetime(datetime_t t) {
  return (t >> 2 * 8) & 0xFF;
}  
inline int minute_from_datetime(datetime_t t) {
  return (t >> 1 * 8) & 0xFF;
}
inline int second_from_datetime(datetime_t t) {
  return t & 0xFF;
}

inline void datetime_to_ymdHMS(datetime_t t, int& year, int& month, int& day,
                               int& hour, int& minute, int& second) {
  year = year_from_datetime(t);
  month = month_from_datetime(t);
  day = hour_from_datetime(t);
  hour = hour_from_datetime(t);
  minute = minute_from_datetime(t);
  second = second_from_datetime(t);
}

// for orthogonality...
inline datetime_t ymdHMS_to_datetime(int y, int m, int d, int H, int M, int S) {
  return makedatetime(y,m,d,H,M,S);
}

// http://www.gem.hi-ho.ne.jp/joachim/floorandceiling/julian.html
// numdays : Gregorian calendar year = 0, month = 3, day = 1 is 0
inline void numdays_to_ymd(int numdays, int& y, int& m, int& d) {
  int yc = ((4 * numdays) + 3) / 146097;
  int rc = numdays - 36524 * yc - yc/4;
  int yi = (4 * rc + 3) / 1461;
  int ri = rc - 365 * yi - yi/4;
  y = 100 * yc + yi;
  m = (5 * ri + 2)/153 + 3;
  d = ri - (153 * (m - 3) + 2)/5 + 1;
}

inline int ymd_to_numdays(int y, int m, int d) {
  return 365 * y + y/4 - y/100 + y/400 + (153 * (m - 3) + 2)/5 + d - 1;
}

//  Please note that time_t related functions are not affected by locale
inline void time_t_to_ymdHMS(time_t t, int& y, int& m, int& d,
                             int& H, int& M, int& S) {
  int td = t/86400;
  int rd = t - time_t(td) * time_t(86400);
  H = rd/3600;
  int rH = rd - H * 3600;
  M = rH/60;
  S = rH - M * 60;
  int numdays = td + 719468;
  numdays_to_ymd(numdays, y, m, d);
}

inline int ymd_to_dayofweek(int y, int m, int d) {
  return (ymd_to_numdays(y, m, d) + 3) % 7;
}

inline time_t ymdHMS_to_time_t(int y, int m, int d, int H, int M, int S) {
  auto days = ymd_to_numdays(y, m, d);
  return time_t(days - 719468) * time_t(86400) + H * 3600 + M * 60 + S;
}

inline time_t datetime_t_to_time_t(datetime_t t) {
  int y, m, d, H, M, S;
  datetime_to_ymdHMS(t, y, m, d, H, M, S);
  return ymdHMS_to_time_t(y, m, d, H, M, S);
}

inline datetime_t time_t_to_datetime_t(time_t t) {
  int y, m, d, H, M, S;
  time_t_to_ymdHMS(t, y, m, d, H, M, S);
  return makedatetime(y, m, d, H, M, S);
}

node_local<std::vector<time_t>>
datetime_t_to_time_t(node_local<std::vector<datetime_t>>& dt);

node_local<std::vector<datetime_t>>
time_t_to_datetime_t(node_local<std::vector<time_t>>& t);

}

#endif
