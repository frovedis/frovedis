#include "datetime_to_words.hpp"
#include "char_int_conv.hpp"

using namespace std;

namespace frovedis {

void
datetime_to_words_fill_helper(int* charsp, size_t entry_size,
                              size_t size, size_t fill_last_pos, words& w,
                              int max_len) {
  auto src_charsp = w.chars.data();
  auto src_startsp = w.starts.data();
  auto src_lensp = w.lens.data();
  for(size_t i = 0; i < size; i++) {
    src_startsp[i] += src_lensp[i] - 1; // last pos
  }
  std::vector<size_t> fill_pos(size);
  auto fill_posp = fill_pos.data();
  for(size_t i = 0; i < size; i++) {
    fill_posp[i] = entry_size * i + fill_last_pos;
  }
  for(int i = 0; i < max_len; i++) {
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
    for(size_t j = 0; j < size; j++) {
      if(src_lensp[j] > 0) {
        charsp[fill_posp[j]] = src_charsp[src_startsp[j]];
        src_lensp[j]--;
        src_startsp[j]--;
        fill_posp[j]--;
      }
    }
  }
}

words make_abbmonth_words(int* value, size_t size) {
  vector<string> months = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
                           "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
  words ret;
  ret.chars.resize(3 * 12);
  ret.starts.resize(size);
  ret.lens.resize(size);
  auto charsp = ret.chars.data();
  auto startsp = ret.starts.data();
  auto lensp = ret.lens.data();
  vector<vector<int>> months_int(12);
  for(int i = 0; i < 12; i++) {
    auto mon = char_to_int(months[i]);
    charsp[i * 3] = mon[0];
    charsp[i * 3 + 1] = mon[1];
    charsp[i * 3 + 2] = mon[2];
  }
  for(size_t i = 0; i < size; i++) lensp[i] = 3;
  for(size_t i = 0; i < size; i++) {
    startsp[i] = (value[i] - 1) * 3;
  }
  return ret;
}

void year_from_datetime(const datetime_t* srcp, size_t size, int* dstp) {
  for(size_t i = 0; i < size; i++) {
    dstp[i] = year_from_datetime(srcp[i]);
  }
}
void month_from_datetime(const datetime_t* srcp, size_t size, int* dstp) {
  for(size_t i = 0; i < size; i++) {
    dstp[i] = month_from_datetime(srcp[i]);
  }
}
void day_from_datetime(const datetime_t* srcp, size_t size, int* dstp) {
  for(size_t i = 0; i < size; i++) {
    dstp[i] = day_from_datetime(srcp[i]);
  }
}
void hour_from_datetime(const datetime_t* srcp, size_t size, int* dstp) {
  for(size_t i = 0; i < size; i++) {
    dstp[i] = hour_from_datetime(srcp[i]);
  }
}
void minute_from_datetime(const datetime_t* srcp, size_t size, int* dstp) {
  for(size_t i = 0; i < size; i++) {
    dstp[i] = minute_from_datetime(srcp[i]);
  }
}
void second_from_datetime(const datetime_t* srcp, size_t size, int* dstp) {
  for(size_t i = 0; i < size; i++) {
    dstp[i] = second_from_datetime(srcp[i]);
  }
}

void datetime_to_words(const datetime_t* srcp, size_t src_size,
                       int* charsp, size_t entry_size,
                       const std::string& format) {
  std::vector<int> value(src_size);
  std::vector<size_t> fill_last_pos(src_size);
  auto valuep = value.data();
  auto yearpos = format.find("%Y");
  auto abbmonthpos = format.find("%b");
  if(yearpos != string::npos) {
    auto pos = yearpos;
    if(pos > abbmonthpos) pos += 1;
    size_t fill_last_pos = pos + 3;
    year_from_datetime(srcp, src_size, valuep);
    auto value_words = int_to_words(valuep, src_size);
    datetime_to_words_fill_helper(charsp, entry_size, src_size, fill_last_pos,
                                  value_words, 4);
  }
  if(abbmonthpos != string::npos) {
    auto pos = abbmonthpos;
    if(pos > yearpos) pos += 2;
    size_t fill_last_pos = pos + 2;
    month_from_datetime(srcp, src_size, valuep);
    auto value_words = make_abbmonth_words(valuep, src_size);
    datetime_to_words_fill_helper(charsp, entry_size, src_size, fill_last_pos,
                                  value_words, 3);
  }
  auto pos = format.find("%m");
  if(pos != string::npos) {
    if(pos > yearpos) pos += 2;
    if(pos > abbmonthpos) pos += 1;
    size_t fill_last_pos = pos + 1;
    month_from_datetime(srcp, src_size, valuep);
    auto value_words = int_to_words(valuep, src_size);
    datetime_to_words_fill_helper(charsp, entry_size, src_size, fill_last_pos,
                                  value_words, 2);
  }
  pos = format.find("%d");
  if(pos != string::npos) {
    if(pos > yearpos) pos += 2;
    if(pos > abbmonthpos) pos += 1;
    size_t fill_last_pos = pos + 1;
    day_from_datetime(srcp, src_size, valuep);
    auto value_words = int_to_words(valuep, src_size);
    datetime_to_words_fill_helper(charsp, entry_size, src_size, fill_last_pos,
                                  value_words, 2);
  }
  pos = format.find("%H");
  if(pos != string::npos) {
    if(pos > yearpos) pos += 2;
    if(pos > abbmonthpos) pos += 1;
    size_t fill_last_pos = pos + 1;
    hour_from_datetime(srcp, src_size, valuep);
    auto value_words = int_to_words(valuep, src_size);
    datetime_to_words_fill_helper(charsp, entry_size, src_size, fill_last_pos,
                                  value_words, 2);
  }
  pos = format.find("%M");
  if(pos != string::npos) {
    if(pos > yearpos) pos += 2;
    if(pos > abbmonthpos) pos += 1;
    size_t fill_last_pos = pos + 1;
    minute_from_datetime(srcp, src_size, valuep);
    auto value_words = int_to_words(valuep, src_size);
    datetime_to_words_fill_helper(charsp, entry_size, src_size, fill_last_pos,
                                  value_words, 2);
  }
  pos = format.find("%S");
  if(pos != string::npos) {
    if(pos > yearpos) pos += 2;
    if(pos > abbmonthpos) pos += 1;
    size_t fill_last_pos = pos + 1;
    second_from_datetime(srcp, src_size, valuep);
    auto value_words = int_to_words(valuep, src_size);
    datetime_to_words_fill_helper(charsp, entry_size, src_size, fill_last_pos,
                                  value_words, 2);
  }
}

words datetime_to_words(const datetime_t* srcp, size_t src_size,
                        const std::string& format) {
  auto mod_format = format;
  auto pos = mod_format.find("%Y");
  if(pos != string::npos) mod_format = mod_format.replace(pos, 2, "0000");
  pos = mod_format.find("%b");
  if(pos != string::npos) mod_format = mod_format.replace(pos, 2, "   ");
  pos = mod_format.find("%m");
  if(pos != string::npos) mod_format = mod_format.replace(pos, 2, "00");
  pos = mod_format.find("%d");
  if(pos != string::npos) mod_format = mod_format.replace(pos, 2, "00");
  pos = mod_format.find("%H");
  if(pos != string::npos) mod_format = mod_format.replace(pos, 2, "00");
  pos = mod_format.find("%M");
  if(pos != string::npos) mod_format = mod_format.replace(pos, 2, "00");
  pos = mod_format.find("%S");
  if(pos != string::npos) mod_format = mod_format.replace(pos, 2, "00");
  auto mod_format_size = mod_format.size();
  words ret;
  auto chars_size = mod_format_size * src_size;
  ret.chars.resize(chars_size);
  ret.starts.resize(src_size);
  ret.lens.resize(src_size);
  auto charsp = ret.chars.data();
  auto startsp = ret.starts.data();
  auto lensp = ret.lens.data();
  auto intformat = char_to_int(mod_format);
  auto intformatp = intformat.data();
#pragma _NEC select_vector
  for(size_t i = 0; i < src_size; i++) {
    for(size_t j = 0; j < mod_format_size; j++) {
      charsp[i * mod_format_size + j] = intformatp[j];
    }
  }
  for(size_t i = 0; i < src_size; i++) startsp[i] = i * mod_format_size;
  for(size_t i = 0; i < src_size; i++) lensp[i] = mod_format_size;
  datetime_to_words(srcp, src_size, charsp, mod_format_size, format);
  return ret;
}

words datetime_to_words(const std::vector<datetime_t>& src,
                        const std::string& format) {
  return datetime_to_words(src.data(), src.size(), format);
}

}
