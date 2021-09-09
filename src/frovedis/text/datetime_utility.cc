#include "datetime_utility.hpp"

namespace frovedis {

void year_from_datetime(const datetime_t* srcp, size_t size, int* dstp) {
  for(size_t i = 0; i < size; i++) {
    dstp[i] = srcp[i] >> 5 * 8;
  }
}
void month_from_datetime(const datetime_t* srcp, size_t size, int* dstp) {
  for(size_t i = 0; i < size; i++) {
    dstp[i] = (srcp[i] >> 4 * 8) & 0xFF;
  }
}
void day_from_datetime(const datetime_t* srcp, size_t size, int* dstp) {
  for(size_t i = 0; i < size; i++) {
    dstp[i] = (srcp[i] >> 3 * 8) & 0xFF;
  }
}
void hour_from_datetime(const datetime_t* srcp, size_t size, int* dstp) {
  for(size_t i = 0; i < size; i++) {
    dstp[i] = (srcp[i] >> 2 * 8) & 0xFF;
  }
}
void minute_from_datetime(const datetime_t* srcp, size_t size, int* dstp) {
  for(size_t i = 0; i < size; i++) {
    dstp[i] = (srcp[i] >> 1 * 8) & 0xFF;
  }
}
void second_from_datetime(const datetime_t* srcp, size_t size, int* dstp) {
  for(size_t i = 0; i < size; i++) {
    dstp[i] = srcp[i] & 0xFF;
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

node_local<std::vector<time_t>>
datetime_t_to_time_t(node_local<std::vector<datetime_t>>& dt) {
  return dt.map(+[](std::vector<datetime_t>& dt) {
      auto size = dt.size();
      auto dtp = dt.data();
      std::vector<time_t> ret(size);
      auto retp = ret.data();
      for(size_t i = 0; i < size; i++) {
        auto t = dtp[i];
        int y = t >> 5 * 8;
        int m = (t >> 4 * 8) & 0xFF;
        int d = (t >> 3 * 8) & 0xFF;
        int H = (t >> 2 * 8) & 0xFF;
        int M = (t >> 1 * 8) & 0xFF;
        int S = t & 0xFF;
        int days =
          365 * y + y/4 - y/100 + y/400 + (153 * (m - 3) + 2)/5 + d - 1;
        retp[i] = time_t(days - 719468) * time_t(86400) + H * 3600 + M * 60 + S;
      }
      return ret;
    });
}

node_local<std::vector<datetime_t>>
time_t_to_datetime_t(node_local<std::vector<time_t>>& t) {
  return t.map(+[](std::vector<time_t>& t) {
      auto size = t.size();
      auto tp = t.data();
      std::vector<datetime_t> ret(size);
      auto retp = ret.data();
      for(size_t i = 0; i < size; i++) {
        auto t = tp[i];
        int td = t/86400;
        int rd = t - time_t(td) * time_t(86400);
        int H = rd/3600;
        int rH = rd - H * 3600;
        int M = rH/60;
        int S = rH - M * 60;
        int numdays = td + 719468;
        int yc = ((4 * numdays) + 3) / 146097;
        int rc = numdays - 36524 * yc - yc/4;
        int yi = (4 * rc + 3) / 1461;
        int ri = rc - 365 * yi - yi/4;
        int y = 100 * yc + yi;
        int m = (5 * ri + 2)/153 + 3;
        int d = ri - (153 * (m - 3) + 2)/5 + 1;
        retp[i] = ((datetime_t)y << 5 * 8) + ((datetime_t)m << 4 * 8) +
          ((datetime_t)d << 3 * 8) + ((datetime_t)H << 2 * 8) +
          ((datetime_t)M << 1 * 8) + (datetime_t)S;
      }
      return ret;
    });
}

}
