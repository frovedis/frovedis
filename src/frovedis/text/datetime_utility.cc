#include "datetime_utility.hpp"

namespace frovedis {

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
void nanosecond_from_datetime(const datetime_t* srcp, size_t size, int* dstp) {
  for(size_t i = 0; i < size; i++) {
    dstp[i] = nanosecond_from_datetime(srcp[i]);
  }
}
std::vector<int> year_from_datetime(const std::vector<datetime_t>& src) {
  auto size = src.size();
  std::vector<int> ret(size);
  year_from_datetime(src.data(), size, ret.data());
  return ret;
}
std::vector<int> month_from_datetime(const std::vector<datetime_t>& src) {
  auto size = src.size();
  std::vector<int> ret(size);
  month_from_datetime(src.data(), size, ret.data());
  return ret;
}
std::vector<int> day_from_datetime(const std::vector<datetime_t>& src) {
  auto size = src.size();
  std::vector<int> ret(size);
  day_from_datetime(src.data(), size, ret.data());
  return ret;
}
std::vector<int> hour_from_datetime(const std::vector<datetime_t>& src) {
  auto size = src.size();
  std::vector<int> ret(size);
  hour_from_datetime(src.data(), size, ret.data());
  return ret;
}
std::vector<int> minute_from_datetime(const std::vector<datetime_t>& src) {
  auto size = src.size();
  std::vector<int> ret(size);
  minute_from_datetime(src.data(), size, ret.data());
  return ret;
}
std::vector<int> second_from_datetime(const std::vector<datetime_t>& src) {
  auto size = src.size();
  std::vector<int> ret(size);
  second_from_datetime(src.data(), size, ret.data());
  return ret;
}
std::vector<int> nanosecond_from_datetime(const std::vector<datetime_t>& src) {
  auto size = src.size();
  std::vector<int> ret(size);
  second_from_datetime(src.data(), size, ret.data());
  return ret;
}

int lastday_of_month[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

}
