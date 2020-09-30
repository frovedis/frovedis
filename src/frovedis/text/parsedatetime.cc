#include "parsedatetime.hpp"
#include "char_int_conv.hpp"

using namespace std;

namespace frovedis {

void parsedatetime(const int* charsp, const size_t* startsp,
                   const size_t* lensp, size_t num_words,
                   const std::string& format, datetime_t* retp) {
  std::vector<size_t> newstarts(num_words), newlens(num_words);
  auto newstartsp = newstarts.data();
  auto newlensp = newlens.data();
  std::vector<size_t> parsed(num_words);
  auto parsedp = parsed.data();
  auto yearpos = format.find("%Y");
  if(yearpos != string::npos) {
    for(size_t i = 0; i < num_words; i++) {
      newstartsp[i] = startsp[i] + yearpos;
      newlensp[i] = 4;
    }
    parseint(charsp, newstartsp, newlensp, num_words, parsedp);
    for(size_t i = 0; i < num_words; i++) {
      retp[i] = parsedp[i] << 5 * 8;
    }
  }
  auto pos = format.find("%m");
  if(pos != string::npos) {
    if(pos > yearpos) pos += 2;
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
    for(size_t i = 0; i < num_words; i++) {
      newstartsp[i] = startsp[i] + pos;
      newlensp[i] = 2;
    }
    parseint(charsp, newstartsp, newlensp, num_words, parsedp);
    for(size_t i = 0; i < num_words; i++) {
      retp[i] += parsedp[i];
    }
  }
}

std::vector<datetime_t>
parsedatetime(const std::vector<int>& chars,
              const std::vector<size_t>& starts,
              const std::vector<size_t>& lens,
              const std::string& format) {
  auto size = starts.size();
  std::vector<datetime_t> ret(size);
  parsedatetime(chars.data(), starts.data(), lens.data(), size,
                format, ret.data());
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
  size_t starts = 0;
  auto lens = intdatetime.size();
  datetime_t ret;
  parsedatetime(intdatetime.data(), &starts, &lens, 1, format, &ret);
  return ret;
}

datetime_t makedatetime(int year, int month, int day,
                        int hour, int minute, int second) {
  datetime_t ret = ((datetime_t)year << 5 * 8) + ((datetime_t)month << 4 * 8) +
    ((datetime_t)day << 3 * 8) + ((datetime_t)hour << 2 * 8) +
    ((datetime_t)minute << 1 * 8) + (datetime_t)second;
    
  return ret;
}


}
