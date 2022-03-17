#ifndef DATETIME_UTILITY_HPP
#define DATETIME_UTILITY_HPP

#include "parsedatetime.hpp"
#include "../core/utility.hpp"

/*
  datetime is 64bit signed integer, which is nano second from unix epoch.
*/
namespace frovedis {

// used in dataframe
enum datetime_type {
  year,
  month,
  day,
  hour,
  minute,
  second,
  nanosecond,
  quarter,
  dayofweek,
  dayofyear,
  weekofyear
};

std::string datetime_type_to_string(datetime_type type);

std::string datetime_to_string(datetime_t& src, const std::string& format);

/* Following utility functions should be in header as inline,
   because they are used in loop */

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
  if(m > 12) {
    m -= 12;
    y += 1;
  }
}

inline int ymd_to_numdays(int y, int m, int d) {
  if(m < 3) {
    m += 12;
    y -= 1;
  }
  return 365 * y + y/4 - y/100 + y/400 + (153 * (m - 3) + 2)/5 + d - 1;
}

// assume long is 64bit
inline datetime_t makedatetime(int year, int month, int day,
                               int hour = 0, int minute = 0, int second = 0,
                               int nanosecond = 0) {
  int numdays = ymd_to_numdays(year, month, day);
  datetime_t totalsecond = datetime_t(numdays - 719468) * datetime_t(86400) +
    hour * 3600 + minute * 60 + second;
  return totalsecond * 1000000000L + nanosecond;
}

inline void datetime_to_ymdHMSns(datetime_t t, int& year, int& month, int& day,
                                 int& hour, int& minute, int& second,
                                 int& nanosecond) {
  datetime_t ts = t / 1000000000L;
  nanosecond = t - ts * 1000000000L;
  datetime_t tm = ts / 60;
  second = ts - tm * 60;
  datetime_t th = tm / 60;
  minute = tm - th * 60;
  datetime_t td = th / 24;
  hour = th - td * 24;
  int numdays = td + 719468;
  numdays_to_ymd(numdays, year, month, day);
}

inline int year_from_datetime(datetime_t t) {
  int year, month, day, hour, minute, second, nanosecond;
  datetime_to_ymdHMSns(t, year, month, day, hour, minute, second, nanosecond);
  return year;
}

inline int month_from_datetime(datetime_t t) {
  int year, month, day, hour, minute, second, nanosecond;
  datetime_to_ymdHMSns(t, year, month, day, hour, minute, second, nanosecond);
  return month;
}

inline int day_from_datetime(datetime_t t) {
  int year, month, day, hour, minute, second, nanosecond;
  datetime_to_ymdHMSns(t, year, month, day, hour, minute, second, nanosecond);
  return day;
}

inline int hour_from_datetime(datetime_t t) {
  datetime_t ts = t / 1000000000L;
  datetime_t tm = ts / 60;
  datetime_t th = tm / 60;
  datetime_t td = th / 24;
  int hour = th - td * 24;
  return hour;
}  

inline int minute_from_datetime(datetime_t t) {
  datetime_t ts = t / 1000000000L;
  datetime_t tm = ts / 60;
  datetime_t th = tm / 60;
  int minute = tm - th * 60;
  return minute;
}

inline int second_from_datetime(datetime_t t) {
  datetime_t ts = t / 1000000000L;
  datetime_t tm = ts / 60;
  int second = ts - tm * 60;
  return second;
}

inline int nanosecond_from_datetime(datetime_t t) {
  datetime_t ts = t / 1000000000L;
  int nanosecond = t - ts * 1000000000L;
  return nanosecond;
}

// for orthogonality...
inline datetime_t ymdHMSns_to_datetime(int y, int m, int d,
                                       int H, int M, int S, int ns) {
  return makedatetime(y,m,d,H,M,S,ns);
}

// 1: Sun, 2: Mon, ..., 7: Sat; same as SQL/Spark
inline int ymd_to_dayofweek(int y, int m, int d) {
  return ((ymd_to_numdays(y, m, d) + 3) % 7) + 1;
}

inline int datetime_diff_nanosecond(datetime_t a, datetime_t b) {
  if(a < b) return b - a; else return a - b;
}

inline int datetime_diff_second(datetime_t a, datetime_t b) {
  auto ad = a / datetime_t(1000000000L);
  auto bd = b / datetime_t(1000000000L);
  return ad - bd;
}

inline int datetime_diff_minute(datetime_t a, datetime_t b) {
  auto ad = a / datetime_t(60L * 1000000000L);
  auto bd = b / datetime_t(60L * 1000000000L);
  return ad - bd;
}

inline int datetime_diff_hour(datetime_t a, datetime_t b) {
  auto ad = a / datetime_t(60L * 60L * 1000000000L);
  auto bd = b / datetime_t(60L * 60L * 1000000000L);
  return ad - bd;
}

inline int datetime_diff_day(datetime_t a, datetime_t b) {
  auto ad = a / datetime_t(24l * 60L * 60L * 1000000000L);
  auto bd = b / datetime_t(24l * 60L * 60L * 1000000000L);
  return ad - bd;
}

inline int datetime_diff_month(datetime_t a, datetime_t b) {
  int ay, am, ad, aH, aM, aS, ans;
  int by, bm, bd, bH, bM, bS, bns;
  datetime_to_ymdHMSns(a, ay, am, ad, aH, aM, aS, ans);
  datetime_to_ymdHMSns(b, by, bm, bd, bH, bM, bS, bns);
  return (ay - by) * 12 + (am - bm);
}

inline int datetime_diff_year(datetime_t a, datetime_t b) {
  int ay, am, ad, aH, aM, aS, ans;
  int by, bm, bd, bH, bM, bS, bns;
  datetime_to_ymdHMSns(a, ay, am, ad, aH, aM, aS, ans);
  datetime_to_ymdHMSns(b, by, bm, bd, bH, bM, bS, bns);
  return ay - by;
}

inline datetime_t datetime_add_nanosecond(datetime_t t, int val) {
  return t + val;
}

inline datetime_t datetime_sub_nanosecond(datetime_t t, int val) {
  return t - val;
}

inline datetime_t datetime_add_second(datetime_t t, int val) {
  return t + val * 1000000000L;
}

inline datetime_t datetime_sub_second(datetime_t t, int val) {
  return t - val * 1000000000L;
}

inline datetime_t datetime_add_minute(datetime_t t, int val) {
  return t + val * 60L * 1000000000L;
}

inline datetime_t datetime_sub_minute(datetime_t t, int val) {
  return t - val * 60L * 1000000000L;
}

inline datetime_t datetime_add_hour(datetime_t t, int val) {
  return t + val * 60L * 60L * 1000000000L;
}

inline datetime_t datetime_sub_hour(datetime_t t, int val) {
  return t - val * 60L * 60L * 1000000000L;
}

inline datetime_t datetime_add_day(datetime_t t, int val) {
  return t + val * 24L * 60L * 60L * 1000000000L;
}

inline datetime_t datetime_sub_day(datetime_t t, int val) {
  return t - val * 24L * 60L * 60L * 1000000000L;
}

extern int lastday_of_month[];

inline datetime_t datetime_add_month(datetime_t t, int val) {
  int y, m, d, H, M, S, ns;
  datetime_to_ymdHMSns(t, y, m, d, H, M, S, ns);
  int mtmp = m + val - 1; // 0-based
  if(mtmp >= 0) {
    int divided = mtmp / 12;
    y += divided;
    m = mtmp - divided * 12 + 1;
  } else {
    int divided = ceil_div(-mtmp, 12);
    y -= divided;
    m = mtmp + divided * 12 + 1;
  }
  int isleap = (y % 4  == 0) - (y % 100  == 0) + (y % 400 == 0);
  int thislastday = lastday_of_month[m-1];
  if(m == 2) thislastday += isleap;
  if(d > thislastday) d = thislastday;
  return makedatetime(y, m, d, H, M, S, ns);
}

inline datetime_t datetime_sub_month(datetime_t t, int val) {
  return datetime_add_month(t, -val);
}

inline datetime_t datetime_add_year(datetime_t t, int val) {
  int y, m, d, H, M, S, ns;
  datetime_to_ymdHMSns(t, y, m, d, H, M, S, ns);
  y += val;
  int isleap = (y % 4  == 0) - (y % 100  == 0) + (y % 400 == 0);
  if(m == 2 && !isleap && d == 29) d = 28;
  return makedatetime(y, m, d, H, M, S, ns);
}

inline datetime_t datetime_sub_year(datetime_t t, int val) {
  return datetime_add_year(t, -val);
}

inline int dayofweek_from_datetime(datetime_t t) {
  int y, m, d, H, M, S, ns;
  datetime_to_ymdHMSns(t, y, m, d, H, M, S, ns);
  return ymd_to_dayofweek(y, m, d);
}

inline int dayofyear_from_datetime(datetime_t t) {
  int y, m, d, H, M, S, ns;
  datetime_to_ymdHMSns(t, y, m, d, H, M, S, ns);
  int numdays = ymd_to_numdays(y,m,d);
  int firstday = ymd_to_numdays(y,1,1);
  return numdays - firstday + 1;
}

inline int quarter_from_datetime(datetime_t t) {
  int y, m, d, H, M, S, ns;
  datetime_to_ymdHMSns(t, y, m, d, H, M, S, ns);
  return (m-1)/3 + 1;
}

/*
  same specification as Spark:
  Returns the week of the year of the given date.
  A week is considered to start on a Monday and
  week 1 is the first week with >3 days.
 */
inline int weekofyear_from_datetime(datetime_t t) {
  int y, m, d, H, M, S, ns;
  datetime_to_ymdHMSns(t, y, m, d, H, M, S, ns);
  int numdays = ymd_to_numdays(y,m,d);
  int firstday = ymd_to_numdays(y,1,1);
  int jan1 = (firstday + 2) % 7; // 0 if Mon
  int num1stweek = 7-jan1;
  int weeks = (numdays - firstday - num1stweek) / 7;
  return weeks + 1 + (num1stweek > 3);
}

inline datetime_t datetime_truncate_year(datetime_t t) {
  int y, m, d, H, M, S, ns;
  datetime_to_ymdHMSns(t, y, m, d, H, M, S, ns);
  return makedatetime(y, 1, 1, 0, 0, 0, 0);
}

inline datetime_t datetime_truncate_month(datetime_t t) {
  int y, m, d, H, M, S, ns;
  datetime_to_ymdHMSns(t, y, m, d, H, M, S, ns);
  return makedatetime(y, m, 1, 0, 0, 0, 0);
}

inline datetime_t datetime_truncate_day(datetime_t t) {
  int y, m, d, H, M, S, ns;
  datetime_to_ymdHMSns(t, y, m, d, H, M, S, ns);
  return makedatetime(y, m, d, 0, 0, 0, 0);
}

inline datetime_t datetime_truncate_hour(datetime_t t) {
  int y, m, d, H, M, S, ns;
  datetime_to_ymdHMSns(t, y, m, d, H, M, S, ns);
  return makedatetime(y, m, d, H, 0, 0, 0);
}

inline datetime_t datetime_truncate_minute(datetime_t t) {
  int y, m, d, H, M, S, ns;
  datetime_to_ymdHMSns(t, y, m, d, H, M, S, ns);
  return makedatetime(y, m, d, H, M, 0, 0);
}

inline datetime_t datetime_truncate_second(datetime_t t) {
  int y, m, d, H, M, S, ns;
  datetime_to_ymdHMSns(t, y, m, d, H, M, S, ns);
  return makedatetime(y, m, d, H, M, S, 0);
}

inline datetime_t datetime_truncate_quarter(datetime_t t) {
  int y, m, d, H, M, S, ns;
  datetime_to_ymdHMSns(t, y, m, d, H, M, S, ns);
  return makedatetime(y, (m-1)/3 * 3 + 1, 1, 0, 0, 0, 0);
}

inline datetime_t datetime_truncate_weekofyear(datetime_t t) {
  int y, m, d, H, M, S, ns;
  datetime_to_ymdHMSns(t, y, m, d, H, M, S, ns);
  int thisdayofweek = ymd_to_dayofweek(y, m, d);
  if(thisdayofweek > 1) {
    return datetime_sub_day(makedatetime(y,m,d), thisdayofweek - 2);
  } else {
    return datetime_sub_day(makedatetime(y,m,d), 6);
  }
}

/*
  same specification as Spark:
  If timestamp1 and timestamp2 are on the same day of month,
  or both are the last day of month, time of day will be ignored.
  Otherwise, the difference is calculated based on 31 days per month.
 */
inline double datetime_months_between(datetime_t a, datetime_t b) {
  int ay, am, ad, aH, aM, aS, ans;
  int by, bm, bd, bH, bM, bS, bns;
  datetime_to_ymdHMSns(a, ay, am, ad, aH, aM, aS, ans);
  datetime_to_ymdHMSns(b, by, bm, bd, bH, bM, bS, bns);
  int aisleap = (ay % 4  == 0) - (ay % 100  == 0) + (ay % 400 == 0);
  int alastday = lastday_of_month[am-1];
  if(am == 2) alastday += aisleap;
  int bisleap = (by % 4  == 0) - (by % 100  == 0) + (by % 400 == 0);
  int blastday = lastday_of_month[bm-1];
  if(bm == 2) blastday += bisleap;
  if(ad == bd || (alastday == ad && blastday == bd)) {
    return (ay - by) * 12 + am - bm;
  } else {
    int diffmonth = (ay - by) * 12 + am - bm;
    int diffday = ad - bd;
    datetime_t difftime =
      aH * 60L * 60L * 1000000000L +
      aM * 60L * 1000000000L +
      aS * 1000000000L +
      ans -
      bH * 60L * 60L * 1000000000L -
      bM * 60L * 1000000000L -
      bS * 1000000000L -
      bns;
    return
      diffmonth +
      diffday / 31.0 +
      difftime / (31.0 * 24L * 60L * 60L * 1000000000L);
  }
}

/*
  dayofweek: Sun = 1, ..., Sat = 7
 */
inline datetime_t datetime_next_day(datetime_t t, int dayofweek) {
  int y, m, d, H, M, S, ns;
  datetime_to_ymdHMSns(t, y, m, d, H, M, S, ns);
  int thisdayofweek = ymd_to_dayofweek(y, m, d);
  if(dayofweek > thisdayofweek) {
    return datetime_add_day(makedatetime(y,m,d), dayofweek - thisdayofweek);
  } else {
    return datetime_add_day(makedatetime(y,m,d),
                            7 + dayofweek - thisdayofweek);
  }
}

}
#endif
