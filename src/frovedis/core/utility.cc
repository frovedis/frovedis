#include "utility.hpp"
#include <stdexcept>
#include <string>
#include <sys/time.h>
#include <sys/stat.h>
#include <mpi.h>
#include <dirent.h>

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

int count_non_hidden_files(const std::string& dir) {
  if(!directory_exists(dir))
    throw std::runtime_error("count_non_hidden_files: directory does not exist!\n");
  int count = 0;
  auto dp = opendir(dir.c_str());
  if (dp != NULL) {
    struct dirent *cur = readdir(dp);
    while (cur) {
      if (cur->d_name[0] != '.') ++count;
      cur = readdir(dp);    
    }
  }
  closedir(dp);
  return count;
}

// https://stackoverflow.com/questions/2256945/removing-a-non-empty-directory-programmatically-in-c-or-c
int remove_directory(const std::string& pathstr) {
  auto path = pathstr.c_str();
  DIR *d = opendir(path);
  size_t path_len = strlen(path);
  int r = -1;

  if (d) {
    struct dirent *p;

    r = 0;
    while (!r && (p=readdir(d))) {
      int r2 = -1;
      char *buf;
      size_t len;

      /* Skip the names "." and ".." as we don't want to recurse on them. */
      if (!strcmp(p->d_name, ".") || !strcmp(p->d_name, ".."))
        continue;

      len = path_len + strlen(p->d_name) + 2; 
      buf = (char*)malloc(len);

      if (buf) {
        struct stat statbuf;

        snprintf(buf, len, "%s/%s", path, p->d_name);
        if (!stat(buf, &statbuf)) {
          if (S_ISDIR(statbuf.st_mode))
            r2 = remove_directory(buf);
          else
            r2 = unlink(buf);
        }
        free(buf);
      }
      r = r2;
    }
    closedir(d);
  }

  if (!r)
    r = rmdir(path);

  return r;
}

}
