#include "parsedatetime.hpp"
#include "parseint.hpp"
#include "char_int_conv.hpp"
#include "../text/datetime_utility.hpp"

using namespace std;

namespace frovedis {

void parseabbmonth(const std::vector<int>& chars,
                   const std::vector<size_t>& starts,
                   const std::vector<size_t>& lens,
                   size_t num_words,
                   int* retp) {
  vector<string> months = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
                           "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
  for(int m = 0; m < 12; m++) {
    auto to_search = months[m];
    auto pos = like(chars, starts, lens, to_search);
    auto pos_size = pos.size();
    auto posp = pos.data();
#pragma _NEC ivdep
    for(size_t i = 0; i < pos_size; i++) {
      retp[posp[i]] = m + 1; // Jan == 1, etc.
    }
  }
}

std::vector<datetime_t>
parsedatetime(const std::vector<int>& chars,
              const std::vector<size_t>& starts,
              const std::vector<size_t>& lens,
              const std::string& format) {
  auto num_words = starts.size();
  auto charsp = chars.data();
  auto startsp = starts.data();
  auto format_size = format.size();
  auto yearpos = format.find("%Y");
  auto abbmonthpos = format.find("%b");
  auto lensp = lens.data();
  auto max_size = format_size;
  if(yearpos != string::npos) max_size += 2;
  if(abbmonthpos != string::npos) max_size += 1;
  int ok = true;
  for(size_t i = 0; i < num_words; i++) {
    if(lensp[i] < max_size) ok = false;
  }
  if(!ok)
    throw std::runtime_error("parsedatetime: word length is less than format");

  std::vector<datetime_t> ret(num_words);
  auto retp = ret.data();

  std::vector<size_t> newstarts(num_words), newlens(num_words);
  auto newstartsp = newstarts.data();
  auto newlensp = newlens.data();
  std::vector<int> year(num_words), month(num_words), day(num_words);
  std::vector<int> hour(num_words), minute(num_words), second(num_words);
  std::vector<int> nanosecond(num_words);
  auto yearp = year.data();
  auto monthp = month.data();
  auto dayp = day.data();
  auto hourp = hour.data();
  auto minutep = minute.data();
  auto secondp = second.data();
  auto nanosecondp = nanosecond.data();
  if(yearpos != string::npos) {
    auto pos = yearpos;
    if(pos > abbmonthpos) pos += 1;
    for(size_t i = 0; i < num_words; i++) {
      newstartsp[i] = startsp[i] + pos;
      newlensp[i] = 4;
    }
    parseint(charsp, newstartsp, newlensp, num_words, yearp);
  }
  if(abbmonthpos != string::npos) {
    auto pos = abbmonthpos;
    if(pos > yearpos) pos += 2;
    for(size_t i = 0; i < num_words; i++) {
      newstartsp[i] = startsp[i] + pos;
      newlensp[i] = 3;
    }
    parseabbmonth(chars, newstarts, newlens, num_words, monthp);
  }
  auto pos = format.find("%m");
  if(pos != string::npos) {
    if(pos > yearpos) pos += 2;
    if(pos > abbmonthpos) pos += 1;
    for(size_t i = 0; i < num_words; i++) {
      newstartsp[i] = startsp[i] + pos;
      newlensp[i] = 2;
    }
    parseint(charsp, newstartsp, newlensp, num_words, monthp);
  }
  pos = format.find("%d");
  if(pos != string::npos) {
    if(pos > yearpos) pos += 2;
    if(pos > abbmonthpos) pos += 1;
    for(size_t i = 0; i < num_words; i++) {
      newstartsp[i] = startsp[i] + pos;
      newlensp[i] = 2;
    }
    parseint(charsp, newstartsp, newlensp, num_words, dayp);
  }
  pos = format.find("%H");
  if(pos != string::npos) {
    if(pos > yearpos) pos += 2;
    if(pos > abbmonthpos) pos += 1;
    for(size_t i = 0; i < num_words; i++) {
      newstartsp[i] = startsp[i] + pos;
      newlensp[i] = 2;
    }
    parseint(charsp, newstartsp, newlensp, num_words, hourp);
  }
  pos = format.find("%M");
  if(pos != string::npos) {
    if(pos > yearpos) pos += 2;
    if(pos > abbmonthpos) pos += 1;
    for(size_t i = 0; i < num_words; i++) {
      newstartsp[i] = startsp[i] + pos;
      newlensp[i] = 2;
    }
    parseint(charsp, newstartsp, newlensp, num_words, minutep);
  }
  pos = format.find("%S");
  if(pos != string::npos) {
    if(pos > yearpos) pos += 2;
    if(pos > abbmonthpos) pos += 1;
    for(size_t i = 0; i < num_words; i++) {
      newstartsp[i] = startsp[i] + pos;
      newlensp[i] = 2;
    }
    parseint(charsp, newstartsp, newlensp, num_words, secondp);
  }
  for(size_t i = 0; i < num_words; i++) {
    retp[i] = makedatetime(yearp[i], monthp[i], dayp[i], hourp[i], minutep[i],
                           secondp[i], nanosecondp[i]);
  }
  return ret;
}

std::vector<datetime_t>
parsedatetime(const words& w, const std::string& format) {
  return parsedatetime(w.chars, w.starts, w.lens, format);
}

// TODO: this uses vectorized implementation, which is slow for just one parse
datetime_t parsedatetime(const std::string& datetime,
                         const std::string& format) {
  auto intdatetime = char_to_int(datetime);
  vector<size_t> starts = {0};
  vector<size_t> lens = {intdatetime.size()};
  auto r = parsedatetime(intdatetime, starts, lens, format);
  return r[0];
}

}
