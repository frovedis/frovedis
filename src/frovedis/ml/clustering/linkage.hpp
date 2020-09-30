#ifndef _LINKAGE_HPP_
#define _LINKAGE_HPP_

namespace frovedis {

// Average linkage
template <class T>
struct average_linkage {
  average_linkage() {}
  average_linkage(size_t n_): n(n_) {}
  
  void update_distance(std::vector<T>& D, 
                       std::vector<size_t>& size, 
                       size_t x, size_t y) { 
    size_t x_indx = 0, y_indx = 0;
    auto dptr = D.data();
    auto szptr = size.data();
    //get the original numbers of posize_ts in clusters x and y
    auto nx = szptr[x];
    auto ny = szptr[y];
    szptr[x] = 0;        // cluster x will be dropped.
    szptr[y] = nx + ny;  // cluster y will be replaced with the new cluster
#pragma _NEC ivdep
    for(size_t i = 0; i < n; ++i) {
      auto ni = szptr[i];
      if(ni == 0 || i == y) continue;
      if (i < x) x_indx = (n * i - (i * (i + 1) / 2) + (x - i - 1));
      else       x_indx = (n * x - (x * (x + 1) / 2) + (i - x - 1));
      if (i < y) y_indx = (n * i - (i * (i + 1) / 2) + (y - i - 1));
      else       y_indx = (n * y - (y * (y + 1) / 2) + (i - y - 1));
      dptr[y_indx] =  ((nx * dptr[x_indx] + ny * dptr[y_indx]) / (nx + ny));
    }
  }
  size_t n;
  SERIALIZE(n)
};

template <class T>
struct complete_linkage {
  complete_linkage() {}
  complete_linkage(size_t n_): n(n_) {}
  void update_distance(std::vector<T>& D, 
                       std::vector<size_t>& size, 
                       size_t x, size_t y) {
    size_t x_indx = 0, y_indx = 0;
    auto dptr = D.data();
    auto szptr = size.data();
    // cluster y will be replaced with the new cluster  
    szptr[y] = szptr[x] + szptr[y];
    // cluster x will be dropped.
    szptr[x] = 0;     
#pragma _NEC ivdep
    for(size_t i = 0; i < n; ++i){
      auto ni = szptr[i];
      if(ni == 0 || i == y) continue;
      if (i < x) x_indx = (n * i - (i * (i + 1) / 2) + (x - i - 1));
      else       x_indx = (n * x - (x * (x + 1) / 2) + (i - x - 1));
      if (i < y) y_indx = (n * i - (i * (i + 1) / 2) + (y - i - 1));
      else       y_indx = (n * y - (y * (y + 1) / 2) + (i - y - 1));
      dptr[y_indx] =  (dptr[x_indx] > dptr[y_indx]) ? 
                                      dptr[x_indx] : dptr[y_indx]; 
    }
  }
  size_t n;
  SERIALIZE(n)
};

template <class T>
struct single_linkage {
  single_linkage() {}
  single_linkage(size_t n_): n(n_) {}
  void update_distance(std::vector<T>& D, 
                       std::vector<size_t>& size, 
                       size_t x, size_t y) {
    size_t x_indx = 0, y_indx = 0;
    auto dptr = D.data();
    auto szptr = size.data();
    // cluster y will be replaced with the new cluster  
    szptr[y] = szptr[x] + szptr[y];
    // cluster x will be dropped.
    szptr[x] = 0;     
#pragma _NEC ivdep
    for(size_t i = 0; i < n; ++i){
      auto ni = szptr[i];
      if(ni == 0 || i == y) continue;
      if (i < x) x_indx = (n * i - (i * (i + 1) / 2) + (x - i - 1));
      else       x_indx = (n * x - (x * (x + 1) / 2) + (i - x - 1));
      if (i < y) y_indx = (n * i - (i * (i + 1) / 2) + (y - i - 1));
      else       y_indx = (n * y - (y * (y + 1) / 2) + (i - y - 1));
      dptr[y_indx] =  (dptr[x_indx] < dptr[y_indx]) ? 
                                      dptr[x_indx] : dptr[y_indx];
    }
  }
  size_t n;
  SERIALIZE(n)
};

}
#endif
