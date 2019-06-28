#ifndef _W2V_COMMON_HPP_
#define _W2V_COMMON_HPP_


#include <sys/types.h>
#include <sys/stat.h>

#include <cstdio>
#include <vector>
#include <string>
#include <fstream>
#include <iterator>


namespace w2v {


typedef float real;
typedef unsigned int uint;
typedef unsigned long long ulonglong;

struct vocab_word {
    uint cn;
    char *word;
};


template <class T>
void write_vector_to_binary_file(const std::vector<T>& vec, const std::string& filename)
{
  FILE* fp = fopen(filename.c_str(), "wb");
  fwrite(vec.data(), sizeof(T), vec.size(), fp);
  fclose(fp);
}

template <class T>
std::vector<T>
read_vector_from_binary_file(const std::string& filename)
{
  std::vector<T> vec;
  
  struct stat stat_buf;
  stat(filename.c_str(), &stat_buf);
  vec.resize(stat_buf.st_size / sizeof(T));

  FILE *fp = fopen(filename.c_str(), "rb");
  fseek(fp, 0, SEEK_SET);
  fread(vec.data(), sizeof(T), stat_buf.st_size / sizeof(T), fp);
  fclose(fp);
  
  return vec;
}

}  // namespace w2v

#endif  // _W2V_COMMON_HPP_

