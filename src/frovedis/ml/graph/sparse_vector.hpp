

#ifndef GRAPH_SPARSE_VECTOR_HPP
#define	GRAPH_SPARSE_VECTOR_HPP


#include <frovedis.hpp>
#include <frovedis/matrix/crs_matrix.hpp>
#include <frovedis/matrix/ccs_matrix.hpp>
#include <frovedis/matrix/jds_crs_hybrid.hpp>
#include<frovedis/matrix/shrink_matrix.hpp>

namespace frovedis {

    
//    //Should move to crs_matrix.hpp 
//template <class T, class I = size_t>
//struct sparse_vector {
//  ~sparse_vector(){}
//  sparse_vector(){}
//  sparse_vector(I vsize){
//      val.resize(vsize);
//      idx.resize(vsize);
//  }  
//  sparse_vector(I vsize, T initv){
//      val.assign(vsize, initv);
//      idx.assign(vsize, initv);
//  }   
//  sparse_vector(const std::vector<I>&vidx, const std::vector<T>& vval){
//      val = vval;
//      idx = vidx;
//  }
//  sparse_vector(const sparse_vector<T,I>& s) {
//    val = s.val;
//    idx = s.idx;
//  }
//  sparse_vector<T,I>& operator=(const sparse_vector<T,I>& s) {
//    val = s.val;
//    idx = s.idx;
//    return *this;
//  }
//  sparse_vector(sparse_vector<T,I>&& s) {
//    val.swap(s.val);
//    idx.swap(s.idx);
//  }
//  sparse_vector<T,I>& operator=(sparse_vector<T,I>&& s) {
//    val.swap(s.val);
//    idx.swap(s.idx);
//    return *this;
//  }
//  void debug_print() const {
//    std::cout << "val : ";
//    for(auto i: val) std::cout << i << " ";
//    std::cout << std::endl;
//    std::cout << "idx : ";
//    for(auto i: idx) std::cout << i << " ";
//    std::cout << std::endl;
//  }
//  std::vector<T> val;
//  std::vector<I> idx;
//private:
//  friend class boost::serialization::access;
//  template<class Archive>
//  void serialize(Archive& ar, unsigned int ver){
//    ar & val;
//    ar & idx;
//  }  
//};

//for sssp
//idx,val,pred.
template <class T, class I = size_t>
struct sparse_vector_tri {
   ~sparse_vector_tri(){}
  sparse_vector_tri(){}
  sparse_vector_tri(I vsize){
      val.resize(vsize);
      idx.resize(vsize);
      pred.resize(vsize);
  }
  sparse_vector_tri(I vsize, T vinit){
      val.assign(vsize,vinit);
      idx.assign(vsize,vinit);
      pred.assign(vsize,vinit);
  }    
  sparse_vector_tri(const std::vector<I>&vidx, const std::vector<T>& vval,const std::vector<T>& vpred){
      val = vval;
      idx = vidx;
      pred = vpred;
  }
  sparse_vector_tri(const sparse_vector_tri<T,I>& s) {
    val = s.val;
    idx = s.idx;
    pred = s.pred;
  }
  sparse_vector_tri(const sparse_vector<T,I>& s, const std::vector<I>& vpred) {
    val = s.val;
    idx = s.idx;
    pred = vpred;
  }  
  sparse_vector_tri<T,I>& operator=(const sparse_vector_tri<T,I>& s) {
    idx = s.idx;
    val = s.val;
    pred = s.pred;
    return *this;
  }
  sparse_vector_tri(sparse_vector_tri<T,I>&& s) {
    idx.swap(s.idx);
    val.swap(s.val);
    pred.swap(s.pred);
  }
  sparse_vector_tri<T,I>& operator=(sparse_vector_tri<T,I>&& s) {
    idx.swap(s.idx);
    val.swap(s.val);
    pred.swap(s.pred);
    return *this;
  }
  void debug_print() const {
    std::cout << "idx("<<idx.size()<<"): ";
    for(auto i: idx) std::cout << i << " ";
    std::cout << std::endl;      
    std::cout << "val("<<val.size()<<"): ";
    for(auto i: val) std::cout << i << " ";
    std::cout << std::endl;
    std::cout << "pred("<<pred.size()<<"): ";
    for(auto i: pred) std::cout << i << " ";
    std::cout << std::endl;    
  }
  std::vector<I> idx;
  std::vector<T> val;
  std::vector<I> pred;
private:
#if 0
  friend class boost::serialization::access;
  template<class Archive>
  void serialize(Archive& ar, unsigned int ver){
    ar & idx;
    ar & val;
    ar & pred;
  }  
#endif
  SERIALIZE(idx, val, pred)
};



template <class T, class I, class O>
std::vector<T> operator*(frovedis::ccs_matrix_local<T,I,O>& mat, sparse_vector<T, I>& v) {

//    size_t TE = 0;
  T* vvalp = &v.val[0];  
  I* vidxp = &v.idx[0];
  auto num_vnnz = v.idx.size(); 

  std::vector<T> ret(mat.local_num_row,0);//upper bound of the output vector size

  T* retp = &ret[0];
  
  T* mvalp = &mat.val[0];
  I* midxp = &mat.idx[0];
  O* moffp = &mat.off[0];


    for(size_t c=0; c<num_vnnz;c++){ 

    #pragma cdir nodep
    #pragma _NEC ivdep
    #pragma cdir on_adb(vvalp)
    #pragma cdir on_adb(vidxp)
        for(O r = moffp[ vidxp[c]]; r<moffp[ vidxp[c]+1];r++){ 
            retp[midxp[r]] = retp[midxp[r]] + mvalp[r] * vvalp[c];
        }
    }

  return ret;
}

template <class T, class I, class O, class B=bool>
std::vector<B> operator&(frovedis::ccs_matrix_local<T,I,O>& mat, sparse_vector<T, I>& v) {


  T* vvalp = &v.val[0];  
  I* vidxp = &v.idx[0];
  auto num_vnnz = v.val.size(); 
  size_t retvsize = mat.local_num_row;
  std::vector<T> ret(retvsize);//upper bound of the output vector size

  T* retp = &ret[0];
  std::vector<B> ret_bl(retvsize);
  T* mvalp = &mat.val[0];
  I* midxp = &mat.idx[0];
  O* moffp = &mat.off[0];

    for(size_t c=0; c<num_vnnz;c++){
        auto col_idx = vidxp[c]; 
        for(O r = moffp[col_idx]; r<moffp[col_idx+1];r++){
            retp[midxp[r]] = retp[midxp[r]] + mvalp[r] * vvalp[c];
        }
    }
  
  for(size_t i=0;i<retvsize;i++){
      ret_bl[i] = (B)(retp[i]);
  }
  return ret_bl;
}

}
#endif	/* GRAPH_SPARSE_VECTOR_HPP*/

