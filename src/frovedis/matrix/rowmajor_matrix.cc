#include "rowmajor_matrix.hpp"

namespace frovedis {

std::vector<size_t> get_block_sizes(size_t num_row,
                                    size_t wsize) {
  size_t each_num_row = ceil_div(num_row, wsize);
  std::vector<size_t> rows(wsize);
  size_t left = num_row;
  auto rowsp = rows.data();
  for(size_t i = 0; i < wsize; i++) {
    if(left < each_num_row) {
      rowsp[i] = left;
      break;
    } else {
      rowsp[i] = each_num_row;
      left -= each_num_row;
    }
  }
  return rows;
}

std::vector<size_t> get_block_sizes(size_t num_row) {
  return get_block_sizes(num_row,get_nodesize());
}

template <>
void make_rowmajor_matrix_local_parseline(std::string& line,
                                          std::vector<double>& retval,
                                          size_t& width) {
  char* s = const_cast<char*>(line.c_str());
  if(width == 0) {
    while(*s != '\0') {
      char* next;
      double val = strtod(s, &next);
      if(next == s) break;
      retval.push_back(val);
      s = next;
      width++;
    }
  } else {
    while(*s != '\0') {
      char* next;
      double val = strtod(s, &next);
      if(next == s) break;
      retval.push_back(val);
      s = next;
    }
  }
}

template <>
void make_rowmajor_matrix_local_parseline(std::string& line,
                                          std::vector<float>& retval,
                                          size_t& width) {
  char* s = const_cast<char*>(line.c_str());
  if(width == 0) {
    while(*s != '\0') {
      char* next;
      float val = strtof(s, &next);
      if(next == s) break;
      retval.push_back(val);
      s = next;
      width++;
    }
  } else {
    while(*s != '\0') {
      char* next;
      float val = strtof(s, &next);
      if(next == s) break;
      retval.push_back(val);
      s = next;
    }
  }
}

template <>
void make_rowmajor_matrix_local_parseline(std::string& line,
                                          std::vector<int>& retval,
                                          size_t& width) {
  char* s = const_cast<char*>(line.c_str());
  if(width == 0) {
    while(*s != '\0') {
      char* next;
      int val = strtol(s, &next, 10);
      if(next == s) break;
      retval.push_back(val);
      s = next;
      width++;
    }
  } else {
    while(*s != '\0') {
      char* next;
      int val = strtol(s, &next, 10);
      if(next == s) break;
      retval.push_back(val);
      s = next;
    }
  }
}

template <>
void make_rowmajor_matrix_local_parseline(std::string& line,
                                          std::vector<unsigned int>& retval,
                                          size_t& width) {
  char* s = const_cast<char*>(line.c_str());
  if(width == 0) {
    while(*s != '\0') {
      char* next;
      unsigned int val = strtol(s, &next, 10);
      if(next == s) break;
      retval.push_back(val);
      s = next;
      width++;
    }
  } else {
    while(*s != '\0') {
      char* next;
      unsigned int val = strtol(s, &next, 10);
      if(next == s) break;
      retval.push_back(val);
      s = next;
    }
  }
}

template <>
void make_rowmajor_matrix_local_parseline(std::string& line,
                                          std::vector<long>& retval,
                                          size_t& width) {
  char* s = const_cast<char*>(line.c_str());
  if(width == 0) {
    while(*s != '\0') {
      char* next;
      long val = strtol(s, &next, 10);
      if(next == s) break;
      retval.push_back(val);
      s = next;
      width++;
    }
  } else {
    while(*s != '\0') {
      char* next;
      long val = strtol(s, &next, 10);
      if(next == s) break;
      retval.push_back(val);
      s = next;
    }
  }
}

template <>
void make_rowmajor_matrix_local_parseline(std::string& line,
                                          std::vector<unsigned long>& retval,
                                          size_t& width) {
  char* s = const_cast<char*>(line.c_str());
  if(width == 0) {
    while(*s != '\0') {
      char* next;
      unsigned long val = strtol(s, &next, 10);
      if(next == s) break;
      retval.push_back(val);
      s = next;
      width++;
    }
  } else {
    while(*s != '\0') {
      char* next;
      unsigned long val = strtol(s, &next, 10);
      if(next == s) break;
      retval.push_back(val);
      s = next;
    }
  }
}

template <>
void make_rowmajor_matrix_local_parseline(std::string& line,
                                          std::vector<long long>& retval,
                                          size_t& width) {
  char* s = const_cast<char*>(line.c_str());
  if(width == 0) {
    while(*s != '\0') {
      char* next;
      long long val = strtoll(s, &next, 10);
      if(next == s) break;
      retval.push_back(val);
      s = next;
      width++;
    }
  } else {
    while(*s != '\0') {
      char* next;
      long long val = strtoll(s, &next, 10);
      if(next == s) break;
      retval.push_back(val);
      s = next;
    }
  }
}

template <>
void make_rowmajor_matrix_local_parseline(std::string& line,
                                          std::vector<unsigned long long>& retval,
                                          size_t& width) {
  char* s = const_cast<char*>(line.c_str());
  if(width == 0) {
    while(*s != '\0') {
      char* next;
      unsigned long long val = strtoll(s, &next, 10);
      if(next == s) break;
      retval.push_back(val);
      s = next;
      width++;
    }
  } else {
    while(*s != '\0') {
      char* next;
      unsigned long long val = strtoll(s, &next, 10);
      if(next == s) break;
      retval.push_back(val);
      s = next;
    }
  }
}

}
