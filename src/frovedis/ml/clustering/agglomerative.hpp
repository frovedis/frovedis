#ifndef _HIER_HPP_
#define _HIER_HPP_

//#define UNION_FIND
#ifdef UNION_FIND
#define VEC_VEC
#endif

#include <cmath>
#include <iostream>
#include <vector>
#include <algorithm>
#include <string>
#include <frovedis/ml/clustering/common.hpp>
#include <frovedis/core/radix_sort.hpp>

namespace frovedis {

#ifdef UNION_FIND
void set_val(std::vector<size_t>& vec){
  for(size_t i = 0; i < vec.size(); ++i) vec[i] = i;
}

void set_one(std::vector<size_t>& vec){
  for(size_t i = 0; i < vec.size(); ++i) vec[i] = 1;
}

// structure for fast cluster labeling in unsorted dendrogram
struct LinkageUnionFind {
  LinkageUnionFind(size_t n){
    parent.resize(2 * n - 1);
    set_val(parent);
    next_label = n;
    size.resize(2 * n - 1);
    set_one(size);
  }
  size_t merge(size_t x, size_t y) {
    parent[x] = next_label;
    parent[y] = next_label;
    size_t sz = size[x] + size[y];
    size[next_label] = sz;
    next_label += 1;
    return sz ;
  }
  size_t find(size_t x){
    size_t p = x;
    while(parent[x] != x)  x =  parent[x];
    while(parent[p] != x) {
      p = parent[p];
      parent[p] = x;
    }
    return x;
  }
  std::vector<size_t> parent;
  std::vector<size_t> size;
  size_t next_label;
};

template<class T>
void label(std::vector<std::vector<T>> &Z, size_t n){
  //Correctly label clusters in unsorted dendrogram
  LinkageUnionFind uf(n);//  =  new LinkageUnionFind(n);
  size_t i, x, y, x_root, y_root;
  for(i = 0; i < n-1; ++i){
    x = size_t(Z[i][0]);
    y = size_t(Z[i][1]);
    x_root = uf.find(x);
    y_root = uf.find(y);
    if(x_root < y_root){
      Z[i][0] = x_root,
      Z[i][1] = y_root;
    }
    else{
      Z[i][0] = y_root;
      Z[i][1] = x_root;
    }
    Z[i][3] = uf.merge(x_root, y_root);
  }
}
#else

#ifdef VEC_VEC
template<class T>
void label(std::vector<std::vector<T>>& Z, size_t n){
  std::vector<int> label(n);
  auto lptr = label.data();
  for(size_t i = 0; i < n; ++i) lptr[i] = i;
  for(size_t i = 0; i < Z.size(); ++i) {
    auto x = (int) Z[i][0];
    auto y = (int) Z[i][1];
    auto xx = lptr[x];
    auto yy = lptr[y];
    if(xx > yy) std::swap(xx, yy);
    Z[i][0] = xx;
    Z[i][1] = yy;
    lptr[y] = n++;
  }
}
#else
template<class T>
void label(rowmajor_matrix_local<T>& Z, size_t n){
  std::vector<int> label(n);
  auto lptr = label.data();
  auto zptr = Z.val.data();
  auto size = Z.local_num_row;
  for(size_t i = 0; i < n; ++i) lptr[i] = i;
  for(size_t i = 0; i < size; ++i) {
    auto x = (int ) zptr[i*4+0];
    auto y = (int) zptr[i*4+1];
    auto xx = lptr[x];
    auto yy = lptr[y];
    if(xx > yy) std::swap(xx, yy);
    zptr[i*4+0] = xx;
    zptr[i*4+1] = yy;
    lptr[y] = n++;
  }
}
#endif
#endif

template <class T>
void show(std::vector<std::vector<T>>& Z){
  for(auto i: Z) {
    for(auto j: i) std::cout << j << " ";
    std::cout << "\n";
  }
  std::cout << "\n";
}

template <class T>
void show(std::vector<T>& vec) {
  for(auto e: vec) std::cout << e << " "; std::cout << std::endl;
}

inline size_t condensed_index(size_t n, size_t i, size_t j){
  //calculate the condensed index of element (i, j) in an n x n condensed matrix
  if(i < j) return (n * i - (i * (i + 1) / 2) + (j - i - 1));
  else      return (n * j - (j * (j + 1) / 2) + (i - j - 1));
}

template <class T>
inline double new_dist(T d_xi, T d_yi, T d_xy,
                       size_t size_x, size_t size_y, size_t size_i){
  return (size_x * d_xi + size_y * d_yi) / (size_x + size_y);
}

#ifdef VEC_VEC
template<class T>
bool sortcol(const std::vector<T>& v1,
             const std::vector<T>& v2 ) {
  return v1[2] < v2[2];
}
#else
template<class T>
void vectorized_sort(rowmajor_matrix_local<T>& Z) {
  auto zptr = Z.val.data();
  auto nrow = Z.local_num_row;
  auto ncol = Z.local_num_col; // ncol = 4
  std::vector<T> dist(nrow); auto dptr = dist.data();
  std::vector<int> ind(nrow); auto iptr = ind.data();
  // creating key (dist), value (ind) for radix sort
  for(size_t i = 0; i < nrow; ++i) {
    dptr[i] = zptr[i * ncol + 2]; // extracting 3rd col (index: 2)
    iptr[i] = i;
  }
  //std::cout << "before sort\n"; show(dist); show(ind); std::cout << "\n"; 
  radix_sort(dist, ind);
  //std::cout << "after sort\n"; show(dist); show(ind);
  rowmajor_matrix_local<T> ret(nrow, 4);
  auto rptr = ret.val.data();
  for(size_t i = 0; i < nrow; ++i) {
    int row_id = iptr[i];
    rptr[i * 4 + 0] = zptr[row_id * 4 + 0]; 
    rptr[i * 4 + 1] = zptr[row_id * 4 + 1]; 
    rptr[i * 4 + 2] = zptr[row_id * 4 + 2]; 
    rptr[i * 4 + 3] = zptr[row_id * 4 + 3]; 
  }
  Z.val.swap(ret.val); // update Z in-place
}
#endif

#ifdef VEC_VEC
template<class T>
std::vector<std::vector<T>>
#else
template<class T>
rowmajor_matrix_local<T>
#endif 
compute_dendogram (std::vector<T>& D, size_t n) {
  //create an empty dendogram for storing the future information
  //nrows rows and 4 columns
#ifdef VEC_VEC 
  std::vector<std::vector<T>> Z(n-1, std::vector<T>(4));
#else
  rowmajor_matrix_local<T> Z(n-1, 4);
  auto zptr = Z.val.data();
#endif
  std::vector<size_t> size(n,1);
  std::vector<int> cluster_chain(n);
  auto dptr = D.data();
  auto szptr = size.data();
  auto cptr = cluster_chain.data();

  size_t cond_index = 0;
  size_t chain_length = 0;
  size_t i, k, x, y, nx, ny, ni;
  T dist, current_min;

  for(k = 0; k < n-1; ++k) {
    if(chain_length == 0) {
      // search for active cluster
      for(i = 0; i < n; ++i) if(szptr[i] > 0) break;
      cptr[chain_length++] = i; // push
    }

    // go through chain of neighbors until two mutual neighbors are found.
    while(true) {
      x = cptr[chain_length - 1];

      // we want to prefer the previous element in the chain as the
      // minimum, to avoid potentially going in cycles.
      if(chain_length > 1){
        y = cptr[chain_length - 2];
        if (x < y) cond_index = (n * x - (x * (x + 1) / 2) + (y - x - 1)); 
        else       cond_index = (n * y - (y * (y + 1) / 2) + (x - y - 1));
        current_min = dptr[cond_index];
      }
      else {
        y = 0;
        current_min = INFINITY;
      }
      std::vector<T> tmp(n); auto tptr = tmp.data();
      for(i = 0; i < n; ++i) {
        if(szptr[i] == 0 || x == i ) tptr[i] = INFINITY;
        else {
          if (x < i) cond_index = (n * x - (x * (x + 1) / 2) + (i - x - 1)); 
          else       cond_index = (n * i - (i * (i + 1) / 2) + (x - i - 1));
          tptr[i] = dptr[cond_index];
        }
      }

      for(i = 0; i < n; ++i) {
        dist = tptr[i];
        if(current_min > dist){
          current_min = dist;
          y = i;
        }
      }
      if(chain_length > 1 && y == cptr[chain_length - 2]) break;
      else cptr[chain_length++] = y; //push
    }

    //Merge clusters x and y and pop them from stack.
    chain_length -= 2;

    //This is a convention used in fastcluster.
    if(x > y)  std::swap(x,y);
    //std::cout << x << ", " << y << std::endl;

    //get the original numbers of posize_ts in clusters x and y
    nx = szptr[x];
    ny = szptr[y];

    //Record the new node.
#ifdef VEC_VEC
    Z[k][0] = (T)x;
    Z[k][1] = (T)y;
    Z[k][2] = current_min;
    Z[k][3] = (T) nx + ny;
#else
    zptr[k*4+0] = (T)x;
    zptr[k*4+1] = (T)y;
    zptr[k*4+2] = current_min;
    zptr[k*4+3] = (T) nx + ny;
#endif

    szptr[x] = 0;        // cluster x will be dropped.
    szptr[y] = nx + ny;  // cluster y will be replaced with the new cluster

    size_t x_indx = 0, y_indx = 0;
    //Update the distance matrix.
#pragma _NEC ivdep
    for(i = 0; i < n; ++i){
      ni = szptr[i];
      if(ni == 0 || i == y) continue;
      if (i < x) x_indx = (n * i - (i * (i + 1) / 2) + (x - i - 1)); 
      else       x_indx = (n * x - (x * (x + 1) / 2) + (i - x - 1));
      if (i < y) y_indx = (n * i - (i * (i + 1) / 2) + (y - i - 1)); 
      else       y_indx = (n * y - (y * (y + 1) / 2) + (i - y - 1));
      dptr[y_indx] =  ((nx * dptr[x_indx] + ny * dptr[y_indx]) / (nx + ny));
    }
  }

  //Sort Z by cluster distances.
#ifdef VEC_VEC
  std::sort(Z.begin(), Z.end(), sortcol);
#else
  vectorized_sort(Z);
#endif

  //Find correct cluster labels inplace.
  label(Z, n);
  return Z;
}

template<class T>
std::vector<int>
#ifdef VEC_VEC
assign_labels_vectorized(std::vector<std::vector<T>>& Z,
#else
assign_labels_vectorized(rowmajor_matrix_local<T>& Z,
#endif
                         int nsamples, int ncluster) {
  auto size = 2 * nsamples - 1;
  std::vector<int> e_ret(size, -1);// initialize with -1
  auto eptr = e_ret.data();
#ifdef VEC_VEC
  size_t start = Z.size() - ncluster;
#else
  size_t start = Z.local_num_row - ncluster;
  auto zptr = Z.val.data();
#endif
  int label = 0;
  for(int i = start; i >= 0; i--) { // i should be int (decreasing loop)
#ifdef VEC_VEC
    auto x = (int) Z[i][0];
    auto y = (int) Z[i][1];
#else
    auto x = (int) zptr[i*4+0];
    auto y = (int) zptr[i*4+1];
#endif
    auto e_indx = nsamples + i;
    if(eptr[e_indx] == -1) eptr[x] = eptr[y] = eptr[e_indx] = label++;
    else eptr[x] = eptr[y] = eptr[e_indx];
  }
  std::vector<int> ret(nsamples);
  auto rptr = ret.data();
  for(size_t i = 0; i < nsamples; ++i) {
    rptr[i] = (eptr[i] == -1) ? label++ : eptr[i];
  }
  return ret;
}

template <class T>
void label_nodes(rowmajor_matrix_local<T>& children,
                 std::vector<int>& out,
                 int start, int label, int nsamples) {
  auto cptr = children.val.data();
  auto optr = out.data();
  auto x = cptr[start*3+0];
  auto y = cptr[start*3+1];
  if(x < nsamples) optr[x] = label;
  else label_nodes(children, out, x - nsamples, label, nsamples);
  if(y < nsamples) optr[y] = label;
  else label_nodes(children, out, y - nsamples, label, nsamples);
  cptr[start*3+2] = 1; // marked as visited
  //std::cout << "start: " << start << " "; show(out);
}

template<class T>
std::vector<int>
#ifdef VEC_VEC
assign_labels_recursive(std::vector<std::vector<T>>& Z,
#else
assign_labels_recursive(rowmajor_matrix_local<T>& Z,
#endif
                        int nsamples, int ncluster) {
#ifdef VEC_VEC
  auto size = Z.size();
#else
  auto zptr = Z.val.data();
  auto size = Z.local_num_row;
#endif
  rowmajor_matrix_local<int> children(size, 3);
  std::vector<int> ret(nsamples, -1); // marked all labels with -1
  auto cptr = children.val.data();
  auto rptr = ret.data();
  for(size_t i = 0; i < size; ++i) {
#ifdef VEC_VEC
     cptr[i*3+0] = (int) Z[i][0];
     cptr[i*3+1] = (int) Z[i][1];
#else
     cptr[i*3+0] = (int) zptr[i*4+0];
     cptr[i*3+1] = (int) zptr[i*4+1];
#endif
     cptr[i*3+2] = 0; // marked as not visited
  }
  int start = size - ncluster;
  int label = 0;
  while(start >= 0) {
    if(cptr[start*3+2] == 0) label_nodes(children,ret,
                             start,label++,nsamples);
    --start;
  }
  for(size_t i = 0; i < nsamples; ++i) {
    if(rptr[i] == -1) rptr[i] = label++; // FIXME: marked as visited required (?)
  }
  return ret;
}

template<class T>
std::vector<int>
#ifdef VEC_VEC
assign_labels_impl(std::vector<std::vector<T>>& Z,
#else
assign_labels_impl(rowmajor_matrix_local<T>& Z,
#endif
              int nsamples, int ncluster) {
  //return assign_labels_recursive(Z,nsamples,ncluster);
  return assign_labels_vectorized<T>(Z,nsamples,ncluster);
}

template <class T>
#ifdef VEC_VEC
std::vector<std::vector<T>>
#else
rowmajor_matrix_local<T>
#endif
agglomerative_impl(rowmajor_matrix_local<T>& mat, 
                   const std::string& linkage,
                   bool inputMovable = false) {
  if(linkage != "average") REPORT_ERROR(USER_ERROR, "Frovedis supports only average linkage!\n");
  auto nsamples = mat.local_num_row;
  time_spent dist(DEBUG), nn(DEBUG);
  dist.lap_start();
  auto dist_vec = construct_condensed_distance_matrix(mat);
  dist.lap_stop();
  dist.show_lap("condensed dist computation: ");
  if(inputMovable) mat.clear();
  nn.lap_start();
  auto tree = compute_dendogram(dist_vec, nsamples);
  nn.lap_stop();
  nn.show_lap("nn chain computation: ");
  return tree;
}

// --- definition of user APIS starts here ---

template <class T>
#ifdef VEC_VEC
std::vector<std::vector<T>>
#else
rowmajor_matrix_local<T>
#endif
agglomerative_training(rowmajor_matrix<T>& mat,
                       const std::string& linkage = "average") {
  auto lmat = mat.gather();
  return agglomerative_impl(lmat, linkage, true);
}
              
template <class T>
#ifdef VEC_VEC
std::vector<std::vector<T>>
#else
rowmajor_matrix_local<T>
#endif
agglomerative_training(rowmajor_matrix<T>&& mat,
                       const std::string& linkage = "average") {
  auto lmat = mat.gather();
  mat.clear(); // mat is rvalue
  return agglomerative_impl(lmat, linkage, true);
}
              
template <class T>
#ifdef VEC_VEC
std::vector<std::vector<T>>
#else
rowmajor_matrix_local<T>
#endif
agglomerative_training(rowmajor_matrix_local<T>& mat,
                       const std::string& linkage = "average") {
  return agglomerative_impl(mat, linkage, false); // mat is lvalue
}

template <class T>
#ifdef VEC_VEC
std::vector<std::vector<T>>
#else
rowmajor_matrix_local<T>
#endif
agglomerative_training(rowmajor_matrix_local<T>&& mat,
                       const std::string& linkage = "average") {
  return agglomerative_impl(mat, linkage, true); // mat is rvalue
}

template <class T>
std::vector<int>
#ifdef VEC_VEC
agglomerative_assign_cluster(std::vector<std::vector<T>>& tree, 
#else
agglomerative_assign_cluster(rowmajor_matrix_local<T>& tree, 
#endif
                             int ncluster) {
  time_spent assign(DEBUG);
  auto nsamples = tree.local_num_row + 1;
  if ((ncluster <= 0) || (ncluster > nsamples))
    REPORT_ERROR(USER_ERROR,"Number of clusters should be greater than 0 and less than nsamples.\n");
  std::vector<int> label;
  assign.lap_start();
  if(ncluster == 1) label = alloc_vector<int>(nsamples,false);
  else if (ncluster == nsamples) label = alloc_vector<int>(nsamples,true);
  else label = assign_labels_impl(tree, nsamples, ncluster); 
  assign.lap_stop();
  assign.show_lap("cluster assign: ");
  return label;
}

}

#endif
