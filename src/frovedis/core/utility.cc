#include "utility.hpp"
#include <stdexcept>
#include <string>
#include <sys/time.h>
#include <sys/stat.h>
#include <mpi.h>

namespace frovedis {

bool is_bigendian() {
  int i = 1;
  if(*((char*)&i)) return false;
  else return true;
}

double get_dtime(){
  return MPI_Wtime();
/*
  struct timeval tv;
  gettimeofday(&tv, 0);
  return ((double)(tv.tv_sec) + (double)(tv.tv_usec) * 0.001 * 0.001);
*/
}

void make_directory(const std::string& path) {
  struct stat sb;
  if (stat(path.c_str(), &sb) != 0) {
    // there is no file/directory, let's make a directory
    mode_t mode = S_IRWXU | S_IRWXG | S_IRWXO;
    if (mkdir(path.c_str(), mode) != 0) {
      perror("mkdir failed:");
      throw std::runtime_error("mkdir failed");
    }
  } else if (!S_ISDIR(sb.st_mode)) {
    // something already exists, but is not a directory
    throw std::runtime_error(path + " is not a directory");
  }
}

bool directory_exists(const std::string& path) {
  struct stat sb;
  return (stat(path.c_str(), &sb) == 0) && S_ISDIR(sb.st_mode);
}

}
