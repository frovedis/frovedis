#include "utility.hpp"
#include <stdexcept>
#include <string>
#include <regex>
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

int count_files_with_regex(const std::string& dir,
                           const std::string& exp) {
  if(!directory_exists(dir))
    throw std::runtime_error("count_non_hidden_files: directory does not exist!\n");
  int count = 0;
  auto dp = opendir(dir.c_str());
  if (dp != NULL) {
    struct dirent *cur = readdir(dp);
    while (cur) {
      if (std::regex_match(cur->d_name, std::regex(exp))) ++count;
      cur = readdir(dp);    
    }
  }
  closedir(dp);
  return count;
}

// https://stackoverflow.com/questions/2256945/removing-a-non-empty-directory-programmatically-in-c-or-c
int remove_directory_impl(const std::string& pathstr) {
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
            r2 = remove_directory_impl(buf);
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

void remove_directory(const std::string& pathstr) {
  auto r = remove_directory_impl(pathstr);
  if(r)
    throw std::runtime_error("remove_directory: " +
                             std::string(strerror(errno)));
}


// https://stackoverflow.com/questions/2180079/how-can-i-copy-a-file-on-unix-using-c
int cp(const char *from, const char *to) {
  int fd_to, fd_from;
  char buf[2 * 1024 * 1024];
  ssize_t nread;
  int saved_errno;

  fd_from = open(from, O_RDONLY);
  if (fd_from < 0)
    return -1;

  fd_to = open(to, O_WRONLY | O_CREAT | O_EXCL, 0666);
  if (fd_to < 0)
    goto out_error;

  while (nread = read(fd_from, buf, sizeof buf), nread > 0) {
    char *out_ptr = buf;
    ssize_t nwritten;
    do {
      nwritten = write(fd_to, out_ptr, nread);

      if (nwritten >= 0) {
        nread -= nwritten;
        out_ptr += nwritten;
      } else if (errno != EINTR) {
        goto out_error;
      }
    } while (nread > 0);
  }

  if (nread == 0) {
    if (close(fd_to) < 0) {
      fd_to = -1;
      goto out_error;
    }
    close(fd_from);
    /* Success! */
    return 0;
  }

out_error:
  saved_errno = errno;
  close(fd_from);
  if (fd_to >= 0)
    close(fd_to);
  errno = saved_errno;
  return -1;
}

void copy_file(const std::string& from, const std::string& to) {
  auto r = cp(from.c_str(), to.c_str());
  if(r)
    throw std::runtime_error("copy_file: " +
                             std::string(strerror(errno)));
}

int copy_directory_impl(const std::string& fromstr, const std::string& tostr) {
  auto from = fromstr.c_str();
  auto to = tostr.c_str();
  DIR *d = opendir(from);
  size_t from_path_len = strlen(from);
  size_t to_path_len = strlen(to);
  int r = -1;

  if (d) {
    make_directory(to);
    struct dirent *p;

    r = 0;
    while (!r && (p=readdir(d))) {
      int r2 = -1;

      /* Skip the names "." and ".." as we don't want to recurse on them. */
      if (!strcmp(p->d_name, ".") || !strcmp(p->d_name, ".."))
        continue;

      size_t fromlen = from_path_len + strlen(p->d_name) + 2; 
      size_t tolen = to_path_len + strlen(p->d_name) + 2; 
      char* frombuf = (char*)malloc(fromlen);
      char* tobuf = (char*)malloc(tolen);

      if (frombuf && tobuf) {
        struct stat statbuf;

        snprintf(frombuf, fromlen, "%s/%s", from, p->d_name);
        snprintf(tobuf, tolen, "%s/%s", to, p->d_name);
        if (!stat(frombuf, &statbuf)) {
          if (S_ISDIR(statbuf.st_mode)) {
            r2 = copy_directory_impl(frombuf, tobuf);
          } else {
            r2 = cp(frombuf, tobuf);
          }
        }
        free(frombuf);
        free(tobuf);
      }
      r = r2;
    }
    closedir(d);
  }

  return r;
}

void copy_directory(const std::string& fromstr, const std::string& tostr) {
  auto r = copy_directory_impl(fromstr, tostr);
  if(r)
    throw std::runtime_error("copy_directory: " +
                             std::string(strerror(errno)));
}


}
