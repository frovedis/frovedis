#ifndef _MATRIX_CONVERSION_HPP_
#define _MATRIX_CONVERSION_HPP_

#include "../../matrix/crs_matrix.hpp"
#include "../../matrix/ell_matrix.hpp"
#include "../../matrix/jds_crs_hybrid.hpp"
//#include "../../core/exceptions.hpp"


namespace frovedis {

template <class T, class I = size_t, class O = size_t>
std::vector<ell_matrix_local<T,I>>
to_ell_vec(std::vector<crs_matrix_local<T,I,O>>& crs_vec) {
  std::vector<ell_matrix_local<T,I>> ret(crs_vec.size());
  for(size_t i = 0; i < crs_vec.size(); i++) {
    ret[i] = ell_matrix_local<T,I>(crs_vec[i]);
  }
  return ret;
}

template <class T, class I = size_t, class O = size_t>
ell_matrix_local<T,I>
to_ell_data(crs_matrix_local<T,I,O>& crs_data) {
  ell_matrix_local<T,I> ret(crs_data);
  return ret;
}

template <class T, class I = size_t, class O = size_t>
std::vector<crs_matrix_local<T,I,O>>
to_trans_crs_vec(std::vector<crs_matrix_local<T,I,O>>& crs_vec) {
  std::vector<crs_matrix_local<T,I,O>> ret(crs_vec.size());
  time_spent t(TRACE);
  for(size_t i = 0; i < crs_vec.size(); i++) {
    ret[i] = crs_vec[i].transpose();
    t.show("transpose crs: ");
  }
  return ret;
}

template <class T, class I = size_t, class O = size_t>
crs_matrix_local<T,I,O>
to_trans_crs_data(crs_matrix_local<T,I,O>& crs_data) {
  return crs_data.transpose();
}

template <class T, class I = size_t, class O = size_t, class P = size_t>
std::vector<jds_crs_hybrid_local<T,I,O,P>>
to_jds_crs_vec(std::vector<crs_matrix_local<T,I,O>>& crs_vec) {
  std::vector<jds_crs_hybrid_local<T,I,O,P>> ret(crs_vec.size());
  time_spent t(TRACE);
  for(size_t i = 0; i < crs_vec.size(); i++) {
    ret[i] = jds_crs_hybrid_local<T,I,O,P>(crs_vec[i]);
    t.show("convert to jds_crs_hybrid: ");
  }
  return ret;
}

template <class T, class I = size_t, class O = size_t, class P = size_t>
jds_crs_hybrid_local<T,I,O,P>
to_jds_crs_data(crs_matrix_local<T,I,O>& crs_data) {
  jds_crs_hybrid_local<T,I,O,P> ret(crs_data);
  return ret;
}

template <class T, class I = size_t, class O = size_t, class P = size_t>
std::vector<jds_crs_hybrid_local<T,I,O,P>>
to_trans_jds_crs_vec(std::vector<crs_matrix_local<T,I,O>>& crs_vec) {
  std::vector<jds_crs_hybrid_local<T,I,O,P>> ret(crs_vec.size());
  time_spent t(TRACE);
  for(size_t i = 0; i < crs_vec.size(); i++) {
    auto trans = crs_vec[i].transpose();
    t.show("transpose crs: ");
    ret[i] = jds_crs_hybrid_local<T,I,O,P>(std::move(trans));
    t.show("convert to jds_crs_hybrid: ");
  }
  return ret;
}

template <class T, class I = size_t, class O = size_t, class P = size_t>
jds_crs_hybrid_local<T,I,O,P>
to_trans_jds_crs_data(crs_matrix_local<T,I,O>& crs_data) {
  auto trans = crs_data.transpose();
  return jds_crs_hybrid_local<T,I,O,P>(std::move(trans));
}

template <class T, class I = size_t, class O = size_t, class P = size_t>
std::vector<jds_matrix_local<T,I,O,P>>
to_trans_jds_vec(std::vector<crs_matrix_local<T,I,O>>& crs_vec) {
  std::vector<jds_matrix_local<T,I,O,P>> ret(crs_vec.size());
  time_spent t(TRACE);
  for(size_t i = 0; i < crs_vec.size(); i++) {
    auto trans = crs_vec[i].transpose();
    t.show("transpose crs: ");
    ret[i] = jds_matrix_local<T,I,O,P>(std::move(trans));
    t.show("convert to jds_matrix: ");
  }
  return ret;
}

template <class T, class I = size_t, class O = size_t, class P = size_t>
std::vector<jds_matrix_local<T,I,O,P>>
to_jds_vec(std::vector<crs_matrix_local<T,I,O>>& crs_vec) {
  std::vector<jds_matrix_local<T,I,O,P>> ret(crs_vec.size());
  time_spent t(TRACE);
  for(size_t i = 0; i < crs_vec.size(); i++) {
    ret[i] = jds_matrix_local<T,I,O,P>(crs_vec[i]);
    t.show("convert to jds_matrix: ");
  }
  return ret;
}

}  // namespace frovedis

#endif  // _MATRIX_CONVERSION_HPP_
