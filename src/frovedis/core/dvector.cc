#include <utility>
#include <algorithm>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <stdlib.h>
#include "dvector.hpp"
#if defined(_SX) && !defined(__ve__)
#include <byteswap.h>
#endif


namespace frovedis {

using namespace std;

bool is_sequential_save() {
  auto envval = getenv("FROVEDIS_SEQUENTIAL_SAVE");
  if(envval == NULL) return false;
  else if(std::string(envval) == "true") return true;
  else return false;
}

void save_file_checker(const std::string& path) {
  struct stat sb;
  if(stat(path.c_str(), &sb) == 0) { // there is something
    if(S_ISREG(sb.st_mode)) { // file?
      if(unlink(path.c_str()) != 0) {
        throw std::runtime_error("unlink " + path + " error: " +
                                 std::string(strerror(errno)));
      }
    } else {
      throw std::runtime_error(path + " that is not a file exists");
    }
  }
  int fd = creat(path.c_str(), 0666);
  if(fd == -1)
    throw std::runtime_error("creat " + path + " error: " +
                             std::string(strerror(errno)));
  if(close(fd) == -1)
    throw std::runtime_error("close " + path + " error: " +
                             std::string(strerror(errno)));
}

std::vector<size_t> 
align_as_calc_alltoall_sizes(std::vector<size_t>& srcsizes,
                             std::vector<size_t>& dstsizes) {
  int self = get_selfid();
  int node_size = get_nodesize();
  size_t global_start = 0;
  auto srcsizesp = srcsizes.data();
  auto dstsizesp = dstsizes.data();
  for(int i = 0; i < self; i++) {
    global_start += srcsizesp[i];
  }
  size_t global_end = global_start + srcsizesp[self];
  vector<size_t> alltoall_sizes(node_size);
  auto alltoall_sizesp = alltoall_sizes.data();
  size_t dst_global_start = 0;
  for(int i = 0; i < node_size; i++) {
    size_t dst_global_end = dst_global_start + dstsizesp[i];
    if(dst_global_start < global_start &&
       dst_global_end < global_start) {
      ;
    } else if(dst_global_start < global_start &&
              dst_global_end >= global_start &&
              dst_global_end < global_end) {
      alltoall_sizesp[i] = dst_global_end - global_start;
    } else if(dst_global_start < global_start &&
              dst_global_end >= global_end) {
      alltoall_sizesp[i] = global_end - global_start;
    } else if(dst_global_start >= global_start &&
              dst_global_start < global_end &&
              dst_global_end >= global_start &&
              dst_global_end < global_end) {
      alltoall_sizesp[i] = dst_global_end - dst_global_start;
    } else if(dst_global_start >= global_start &&
              dst_global_start < global_end &&
              dst_global_end >= global_end) {
      alltoall_sizesp[i] = global_end - dst_global_start;
    } else if(dst_global_start >= global_end &&
              dst_global_end >= global_end) {
      ;
    } else {
      throw
        std::runtime_error("internal error in align_as_calc_alltoall_sizes");
    }
    dst_global_start += dstsizesp[i];
  }
  return alltoall_sizes;
}

template <>
void align_as_align(std::vector<double>& src, std::vector<double>& dst,
                    std::vector<std::vector<size_t>>& alltoall_sizes) {
  align_as_align_pod(src, dst, alltoall_sizes);
}

template <>
void align_as_align(std::vector<float>& src, std::vector<float>& dst,
                    std::vector<std::vector<size_t>>& alltoall_sizes) {
  align_as_align_pod(src, dst, alltoall_sizes);
}

template <>
void align_as_align(std::vector<int>& src, std::vector<int>& dst,
                    std::vector<std::vector<size_t>>& alltoall_sizes) {
  align_as_align_pod(src, dst, alltoall_sizes);
}

template <>
void align_as_align(std::vector<size_t>& src, std::vector<size_t>& dst,
                    std::vector<std::vector<size_t>>& alltoall_sizes) {
  align_as_align_pod(src, dst, alltoall_sizes);
}

template <>
DVID<std::vector<std::vector<unsigned int>>> 
alltoall_exchange(DVID<std::vector<std::vector<unsigned int>>>& src) {
  return alltoall_exchange_pod<unsigned int>(src);
}

template <>
DVID<std::vector<std::vector<int>>> 
alltoall_exchange(DVID<std::vector<std::vector<int>>>& src) {
  return alltoall_exchange_pod<int>(src);
}

template <>
DVID<std::vector<std::vector<float>>> 
alltoall_exchange(DVID<std::vector<std::vector<float>>>& src) {
  return alltoall_exchange_pod<float>(src);
}

template <>
DVID<std::vector<std::vector<double>>> 
alltoall_exchange(DVID<std::vector<std::vector<double>>>& src) {
  return alltoall_exchange_pod<double>(src);
}

template <>
DVID<std::vector<std::vector<unsigned long long>>> 
alltoall_exchange(DVID<std::vector<std::vector<unsigned long long>>>& src) {
  return alltoall_exchange_pod<unsigned long long>(src);
}

template <>
DVID<std::vector<std::vector<long long>>> 
alltoall_exchange(DVID<std::vector<std::vector<long long>>>& src) {
  return alltoall_exchange_pod<long long>(src);
}

template <>
DVID<std::vector<std::vector<unsigned long>>> 
alltoall_exchange(DVID<std::vector<std::vector<unsigned long>>>& src) {
  return alltoall_exchange_pod<unsigned long>(src);
}

template <>
DVID<std::vector<std::vector<long>>> 
alltoall_exchange(DVID<std::vector<std::vector<long>>>& src) {
  return alltoall_exchange_pod<long>(src);
}

#ifdef USE_THREAD
pthread_mutex_t align_as_lock = PTHREAD_MUTEX_INITIALIZER;
#endif

struct make_dvector_load_mpi_helper {
  make_dvector_load_mpi_helper(){}
  make_dvector_load_mpi_helper(const std::string& path,
                               const std::string& delim) :
    path(path), delim(delim) {}
  void operator()(std::vector<std::string>& ret) {
    MPI_File fh;
    int r = MPI_File_open(frovedis_comm_rpc, const_cast<char*>(path.c_str()),
                          MPI_MODE_RDONLY, MPI_INFO_NULL, &fh);
    if(r != 0) throw std::runtime_error("error in MPI_File_open");
    MPI_Offset file_size_;
    r = MPI_File_get_size(fh, &file_size_);
    size_t file_size = static_cast<size_t>(file_size_);
    if(r != 0) throw std::runtime_error("error in MPI_File_get_size");
    size_t nodes = get_nodesize();
    int self = get_selfid();
    size_t each_size = ceil_div(file_size, nodes);
    size_t delim_len = delim.size();
    if(each_size < delim_len)
      throw std::runtime_error("delimiter size is larger than each part");
    size_t myoff, myend;
    if(self == 0) myoff = 0;
    else myoff = std::min(each_size * self - delim_len, file_size);
    if(self == nodes - 1) myend = file_size;
    else myend = std::min(each_size * (self + 1) - delim_len, file_size);
    size_t mysize = myend - myoff;
    char datarep[] = "native";
    MPI_File_set_view(fh, myoff, MPI_CHAR, MPI_CHAR, datarep, MPI_INFO_NULL);

    MPI_Status st;
    std::string buf;
    buf.resize(mysize);
    char* bufp = &buf[0];
    size_t maxsize = 0;
    if(sizeof(size_t) == 8)
      MPI_Allreduce(&mysize, &maxsize, 1, MPI_UNSIGNED_LONG_LONG, MPI_MAX,
                    frovedis_comm_rpc);
    else
      MPI_Allreduce(&mysize, &maxsize, 1, MPI_UNSIGNED_LONG, MPI_MAX,
                    frovedis_comm_rpc);
    size_t numiter = ceil_div(maxsize, mpi_max_count);
    size_t to_read = mysize;
    size_t pos = 0;
    for(size_t i = 0; i < numiter; i++) {
      size_t read_size = std::min(to_read, mpi_max_count);
      r = MPI_File_read_all(fh, bufp + pos, read_size, MPI_CHAR, &st);
      if(r != 0) throw std::runtime_error("error in MPI_File_read");
      pos += read_size;
      if(to_read > mpi_max_count) to_read -= mpi_max_count;
      else to_read = 0;
    }
    size_t crnt = 0;
    // node 0 does not check the first delimiter
    bool first = self != 0 ? true : false;
    while(true) {
      size_t idx = buf.find(delim, crnt);
      if(idx == std::string::npos) break;
      else {
        // data for first delimiter is stored in the previous node        
        if(first) { 
          first = false;
          crnt = idx + delim_len;
          continue;
        } else {
          ret.push_back(buf.substr(crnt,idx-crnt));
          crnt = idx + delim_len;
        }
      }
    }
    // if first == true, there is no item for this node
    if(!first) { 
      if(self == nodes - 1) {
        if(crnt != buf.size())
          ret.push_back(buf.substr(crnt, buf.size()));
      } else {
        size_t file_remain = file_size - myend;
        std::string last = buf.substr(crnt, buf.size());
        while(true) { // search for last delimiter
          size_t read_size = std::min(file_remain, size_t(4096));
          std::string overread;
          overread.resize(read_size);
          char* overreadp = &overread[0];
          r = MPI_File_read(fh, overreadp, read_size, MPI_CHAR, &st);
          if(r != 0) throw std::runtime_error("error in MPI_File_read");
          last.insert(last.end(), overread.begin(), overread.end());
          size_t idx = last.find(delim);
          if(idx == std::string::npos) {
            if(file_remain == read_size) { // no last delimiter
              // might become 0 if end of intermediate node reaches flie end
              if(last.size() != 0) ret.push_back(last);
              break;
            } else {
              file_remain -= read_size;
              continue;
            }
          } else {
            ret.push_back(last.substr(0, idx));
            break;
          }
        }
      }
    }
    MPI_File_close(&fh);
  }
  std::string path;
  std::string delim;
  SERIALIZE(path, delim)
};

dvector<std::string> make_dvector_load_mpi(const std::string& path,
                                           const std::string& delim) {
  auto ret = make_node_local_allocate<std::vector<std::string>>();
  ret.mapv(make_dvector_load_mpi_helper(path, delim));
  return ret.moveto_dvector<std::string>();
}

/*
  TODO: support hdfs
 */
template <>
dvector<std::string> make_dvector_load<std::string>(const std::string& path,
                                                    const std::string& delim) {
  auto idx = path.find(':', 0);
  if(idx == std::string::npos) {
    return make_dvector_load_mpi(path, delim);
  } else {
    if(path.substr(0, idx) == "file") {
      if(path.substr(idx + 1, 2) == "//") {
        return make_dvector_load_mpi(path.substr(idx + 3), delim);
      } else {
        return make_dvector_load_mpi(path.substr(idx + 1), delim);
      } 
    } else {
      throw
        std::runtime_error("scheme " + path.substr(0, idx) +
                           " is not implemented yet.");
    }
  }
}

dvector<std::string> make_dvector_loadline(const std::string& path) {
  return make_dvector_load(path, string("\n"));
}

inline bool is_bigendian() {
  int i = 1;
  if(*((char*)&i)) return false;
  else return true;
}

inline bool no_conversion_needed() {
#if defined(BIG_ENDIAN_FILE)
  return is_bigendian();
#elif defined(LITTLE_ENDIAN_FILE)
  return !is_bigendian();
#else
#error please define BIG_ENDIAN_FILE or LITTLE_ENDIAN_FILE
#endif
}

inline uint32_t swap32(uint32_t val) {
  return ( (((val) >> 24) & 0x000000FF) | (((val) >>  8) & 0x0000FF00) | 
           (((val) <<  8) & 0x00FF0000) | (((val) << 24) & 0xFF000000) );
}

inline uint64_t swap64(uint64_t val) {
  return ( (((val) >> 56) & 0x00000000000000FF) |
           (((val) >> 40) & 0x000000000000FF00) |
           (((val) >> 24) & 0x0000000000FF0000) |
           (((val) >>  8) & 0x00000000FF000000) |
           (((val) <<  8) & 0x000000FF00000000) |
           (((val) << 24) & 0x0000FF0000000000) |
           (((val) << 40) & 0x00FF000000000000) |
           (((val) << 56) & 0xFF00000000000000) );
}

template <>
std::vector<char> convert_endian(std::vector<char>& vec) {return vec;}

union int_uint32_t {
  int src;
  uint32_t dst;
};

union long_uint32_t {
  long src;
  uint32_t dst;
};

union float_uint32_t {
  float src;
  uint32_t dst;
};

union long_uint64_t {
  long src;
  uint64_t dst;
};

union long_long_uint64_t {
  long long src;
  uint64_t dst;
};

union double_uint64_t {
  double src;
  uint64_t dst;
};

template <>
std::vector<int> convert_endian(std::vector<int>& vec) {
  std::vector<int> ret(vec.size());
  if(no_conversion_needed()) {
    auto retp = ret.data();
    auto vecp = vec.data();
    auto size = vec.size();
    for(size_t i = 0; i < size; i++) {
      retp[i] = vecp[i];
    }
  } else {
#if defined(_SX) && !defined(__ve__)
    void* vecp = reinterpret_cast<void*>(&vec[0]);
    void* retp = reinterpret_cast<void*>(&ret[0]);
    bswap_memcpy(retp, vecp, sizeof(int), vec.size());
#else
    auto retp = ret.data();
    auto vecp = vec.data();
    auto size = vec.size();
    for(size_t i = 0; i < size; i++) {
      int_uint32_t t1, t2;
      t1.src = vecp[i];
      t2.dst = swap32(t1.dst);
      retp[i] = t2.src;
    }
#endif
  }
  return ret;
}

template <>
std::vector<unsigned int> convert_endian(std::vector<unsigned int>& vec) {
  std::vector<unsigned int> ret(vec.size());
  if(no_conversion_needed()) {
    auto retp = ret.data();
    auto vecp = vec.data();
    auto size = vec.size();
    for(size_t i = 0; i < size; i++) {
      retp[i] = vecp[i];
    }
  } else {
#if defined(_SX) && !defined(__ve__)
    void* vecp = reinterpret_cast<void*>(&vec[0]);
    void* retp = reinterpret_cast<void*>(&ret[0]);
    bswap_memcpy(retp, vecp, sizeof(unsigned int), vec.size());
#else
    auto retp = ret.data();
    auto vecp = vec.data();
    auto size = vec.size();
    for(size_t i = 0; i < size; i++) {
      retp[i] = swap32(vecp[i]);
    }
#endif
  }
  return ret;
}

template <>
std::vector<long> convert_endian(std::vector<long>& vec) {
  std::vector<long> ret(vec.size());
  if(no_conversion_needed()) {
    auto retp = ret.data();
    auto vecp = vec.data();
    auto size = vec.size();
    for(size_t i = 0; i < size; i++) {
      retp[i] = vecp[i];
    }
  } else {
#if defined(_SX) && !defined(__ve__)
    void* vecp = reinterpret_cast<void*>(&vec[0]);
    void* retp = reinterpret_cast<void*>(&ret[0]);
    bswap_memcpy(retp, vecp, sizeof(long), vec.size());
#else
    auto retp = ret.data();
    auto vecp = vec.data();
    auto size = vec.size();
    if(sizeof(long) == 8) {
      for(size_t i = 0; i < size; i++) {
        long_uint64_t t1, t2;
        t1.src = vecp[i];
        t2.dst = swap64(t1.dst);
        retp[i] = t2.src;
      }
    } else { // sizeof(long) == 4
      for(size_t i = 0; i < size; i++) {
        long_uint32_t t1, t2;
        t1.src = vecp[i];
        t2.dst = swap32(t1.dst);
        retp[i] = t2.src;
      }
    }
#endif
  }
  return ret;
}

template <>
std::vector<unsigned long> convert_endian(std::vector<unsigned long>& vec) {
  std::vector<unsigned long> ret(vec.size());
  if(no_conversion_needed()) {
    auto retp = ret.data();
    auto vecp = vec.data();
    auto size = vec.size();
    for(size_t i = 0; i < size; i++) {
      retp[i] = vecp[i];
    }
  } else {
#if defined(_SX) && !defined(__ve__)
    void* vecp = reinterpret_cast<void*>(&vec[0]);
    void* retp = reinterpret_cast<void*>(&ret[0]);
    bswap_memcpy(retp, vecp, sizeof(unsigned long), vec.size());
#else
    auto retp = ret.data();
    auto vecp = vec.data();
    auto size = vec.size();
    if(sizeof(unsigned long) == 8) {
      for(size_t i = 0; i < size; i++) {
        retp[i] = swap64(vecp[i]);
      }
    } else {
      for(size_t i = 0; i < size; i++) {
        retp[i] = swap32(vecp[i]);
      }
    }
#endif
  }
  return ret;
}

template <>
std::vector<long long> convert_endian(std::vector<long long>& vec) {
  std::vector<long long> ret(vec.size());
  if(no_conversion_needed()) {
    auto retp = ret.data();
    auto vecp = vec.data();
    auto size = vec.size();
    for(size_t i = 0; i < size; i++) {
      retp[i] = vecp[i];
    }
  } else {
#if defined(_SX) && !defined(__ve__)
    void* vecp = reinterpret_cast<void*>(&vec[0]);
    void* retp = reinterpret_cast<void*>(&ret[0]);
    bswap_memcpy(retp, vecp, sizeof(long long), vec.size());
#else
    auto retp = ret.data();
    auto vecp = vec.data();
    auto size = vec.size();
    for(size_t i = 0; i < size; i++) {
      long_long_uint64_t t1, t2;
      t1.src = vecp[i];
      t2.dst = swap64(t1.dst);
      retp[i] = t2.src;
    }
#endif
  }
  return ret;
}

template <>
std::vector<unsigned long long>
convert_endian(std::vector<unsigned long long>& vec) {
  std::vector<unsigned long long> ret(vec.size());
  if(no_conversion_needed()) {
    auto retp = ret.data();
    auto vecp = vec.data();
    auto size = vec.size();
    for(size_t i = 0; i < size; i++) {
      retp[i] = vecp[i];
    }
  } else {
#if defined(_SX) && !defined(__ve__)
    void* vecp = reinterpret_cast<void*>(&vec[0]);
    void* retp = reinterpret_cast<void*>(&ret[0]);
    bswap_memcpy(retp, vecp, sizeof(unsigned long long), vec.size());
#else
    auto retp = ret.data();
    auto vecp = vec.data();
    auto size = vec.size();
    for(size_t i = 0; i < size; i++) {
      retp[i] = swap64(vecp[i]);
    }
#endif
  }
  return ret;
}

template <>
std::vector<float> convert_endian(std::vector<float>& vec) {
  std::vector<float> ret(vec.size());
  if(no_conversion_needed()) {
    auto retp = ret.data();
    auto vecp = vec.data();
    auto size = vec.size();
    for(size_t i = 0; i < size; i++) {
      retp[i] = vecp[i];
    }
  } else {
#if defined(_SX) && !defined(__ve__)
    void* vecp = reinterpret_cast<void*>(&vec[0]);
    void* retp = reinterpret_cast<void*>(&ret[0]);
    bswap_memcpy(retp, vecp, sizeof(float), vec.size());
#else
    auto retp = ret.data();
    auto vecp = vec.data();
    auto size = vec.size();
    for(size_t i = 0; i < size; i++) {
      float_uint32_t t1, t2;
      t1.src = vecp[i];
      t2.dst = swap32(t1.dst);
      retp[i] = t2.src;
    }
#endif
  }
  return ret;
}

template <>
std::vector<double> convert_endian(std::vector<double>& vec) {
  std::vector<double> ret(vec.size());
  if(no_conversion_needed()) {
    auto retp = ret.data();
    auto vecp = vec.data();
    auto size = vec.size();
    for(size_t i = 0; i < size; i++) {
      retp[i] = vecp[i];
    }
  } else {
#if defined(_SX) && !defined(__ve__)
    void* vecp = reinterpret_cast<void*>(&vec[0]);
    void* retp = reinterpret_cast<void*>(&ret[0]);
    bswap_memcpy(retp, vecp, sizeof(double), vec.size());
#else
    auto retp = ret.data();
    auto vecp = vec.data();
    auto size = vec.size();
    for(size_t i = 0; i < size; i++) {
      double_uint64_t t1, t2;
      t1.src = vecp[i];
      t2.dst = swap64(t1.dst);
      retp[i] = t2.src;
    }
#endif
  }
  return ret;
}

template <>
dvector<double> make_dvector_scatter(const std::vector<double>& src,
                                     const std::vector<size_t>& sizevec) {
  auto dvid = make_dvid_allocate<std::vector<double>>();
  intptr_t ptr = reinterpret_cast<intptr_t>(&src);
  dvid.mapv(make_dvector_scatter_helper2<double>(ptr, sizevec));
  dvector<double> dvec(dvid);
  dvec.set_sizes(sizevec);
  return dvec;
}

template <>
dvector<float> make_dvector_scatter(const std::vector<float>& src,
                                    const std::vector<size_t>& sizevec) {
  auto dvid = make_dvid_allocate<std::vector<float>>();
  intptr_t ptr = reinterpret_cast<intptr_t>(&src);
  dvid.mapv(make_dvector_scatter_helper2<float>(ptr, sizevec));
  dvector<float> dvec(dvid);
  dvec.set_sizes(sizevec);
  return dvec;
}

template <>
dvector<int> make_dvector_scatter(const std::vector<int>& src,
                                  const std::vector<size_t>& sizevec) {
  auto dvid = make_dvid_allocate<std::vector<int>>();
  intptr_t ptr = reinterpret_cast<intptr_t>(&src);
  dvid.mapv(make_dvector_scatter_helper2<int>(ptr, sizevec));
  dvector<int> dvec(dvid);
  dvec.set_sizes(sizevec);
  return dvec;
}

template <>
dvector<size_t> make_dvector_scatter(const std::vector<size_t>& src,
                                     const std::vector<size_t>& sizevec) {
  auto dvid = make_dvid_allocate<std::vector<size_t>>();
  intptr_t ptr = reinterpret_cast<intptr_t>(&src);
  dvid.mapv(make_dvector_scatter_helper2<size_t>(ptr, sizevec));
  dvector<size_t> dvec(dvid);
  dvec.set_sizes(sizevec);
  return dvec;
}

template <>
std::vector<double> gather(dvector<double>& dv) {
  std::vector<double> ret;
  dv.get_dvid().
    mapv(dvector_gather_helper2<double>(reinterpret_cast<intptr_t>(&ret)));
  return ret;
}

template <>
std::vector<float> gather(dvector<float>& dv) {
  std::vector<float> ret;
  dv.get_dvid().
    mapv(dvector_gather_helper2<float>(reinterpret_cast<intptr_t>(&ret)));
  return ret;
}

template <>
std::vector<int> gather(dvector<int>& dv) {
  std::vector<int> ret;
  dv.get_dvid().
    mapv(dvector_gather_helper2<int>(reinterpret_cast<intptr_t>(&ret)));
  return ret;
}

template <>
std::vector<size_t> gather(dvector<size_t>& dv) {
  std::vector<size_t> ret;
  dv.get_dvid().
    mapv(dvector_gather_helper2<size_t>(reinterpret_cast<intptr_t>(&ret)));
  return ret;
}
}
