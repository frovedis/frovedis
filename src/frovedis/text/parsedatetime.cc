#include "parsedatetime.hpp"
#include "char_int_conv.hpp"

using namespace std;

namespace frovedis {

void parseabbmonth(const std::vector<int>& chars,
                   const std::vector<size_t>& starts,
                   const std::vector<size_t>& lens,
                   size_t num_words,
                   datetime_t* retp) {
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
  std::vector<size_t> parsed(num_words);
  auto parsedp = parsed.data();
  if(yearpos != string::npos) {
    auto pos = yearpos;
    if(pos > abbmonthpos) pos += 1;
    for(size_t i = 0; i < num_words; i++) {
      newstartsp[i] = startsp[i] + pos;
      newlensp[i] = 4;
    }
    parseint(charsp, newstartsp, newlensp, num_words, parsedp);
    for(size_t i = 0; i < num_words; i++) {
      retp[i] = parsedp[i] << 5 * 8;
    }
  }
  if(abbmonthpos != string::npos) {
    auto pos = abbmonthpos;
    if(pos > yearpos) pos += 2;
    for(size_t i = 0; i < num_words; i++) {
      newstartsp[i] = startsp[i] + pos;
      newlensp[i] = 3;
    }
    parseabbmonth(chars, newstarts, newlens, num_words, parsedp);
    for(size_t i = 0; i < num_words; i++) {
      retp[i] += parsedp[i] << 4 * 8;
    }
  }
  auto pos = format.find("%m");
  if(pos != string::npos) {
    if(pos > yearpos) pos += 2;
    if(pos > abbmonthpos) pos += 1;
    for(size_t i = 0; i < num_words; i++) {
      newstartsp[i] = startsp[i] + pos;
      newlensp[i] = 2;
    }
    parseint(charsp, newstartsp, newlensp, num_words, parsedp);
    for(size_t i = 0; i < num_words; i++) {
      retp[i] += parsedp[i] << 4 * 8;
    }
  }
  pos = format.find("%d");
  if(pos != string::npos) {
    if(pos > yearpos) pos += 2;
    if(pos > abbmonthpos) pos += 1;
    for(size_t i = 0; i < num_words; i++) {
      newstartsp[i] = startsp[i] + pos;
      newlensp[i] = 2;
    }
    parseint(charsp, newstartsp, newlensp, num_words, parsedp);
    for(size_t i = 0; i < num_words; i++) {
      retp[i] += parsedp[i] << 3 * 8;
    }
  }
  pos = format.find("%H");
  if(pos != string::npos) {
    if(pos > yearpos) pos += 2;
    if(pos > abbmonthpos) pos += 1;
    for(size_t i = 0; i < num_words; i++) {
      newstartsp[i] = startsp[i] + pos;
      newlensp[i] = 2;
    }
    parseint(charsp, newstartsp, newlensp, num_words, parsedp);
    for(size_t i = 0; i < num_words; i++) {
      retp[i] += parsedp[i] << 2 * 8;
    }
  }
  pos = format.find("%M");
  if(pos != string::npos) {
    if(pos > yearpos) pos += 2;
    if(pos > abbmonthpos) pos += 1;
    for(size_t i = 0; i < num_words; i++) {
      newstartsp[i] = startsp[i] + pos;
      newlensp[i] = 2;
    }
    parseint(charsp, newstartsp, newlensp, num_words, parsedp);
    for(size_t i = 0; i < num_words; i++) {
      retp[i] += parsedp[i] << 1 * 8;
    }
  }
  pos = format.find("%S");
  if(pos != string::npos) {
    if(pos > yearpos) pos += 2;
    if(pos > abbmonthpos) pos += 1;
    for(size_t i = 0; i < num_words; i++) {
      newstartsp[i] = startsp[i] + pos;
      newlensp[i] = 2;
    }
    parseint(charsp, newstartsp, newlensp, num_words, parsedp);
    for(size_t i = 0; i < num_words; i++) {
      retp[i] += parsedp[i];
    }
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

datetime_t makedatetime(int year, int month, int day,
                        int hour, int minute, int second) {
  datetime_t ret = ((datetime_t)year << 5 * 8) + ((datetime_t)month << 4 * 8) +
    ((datetime_t)day << 3 * 8) + ((datetime_t)hour << 2 * 8) +
    ((datetime_t)minute << 1 * 8) + (datetime_t)second;
    
  return ret;
}


}
