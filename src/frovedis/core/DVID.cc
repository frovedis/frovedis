#include <boost/serialization/vector.hpp>
#include <map>
#include "DVID.hpp"

namespace frovedis {

// valid only on root
dvid_t current_dvid = 1;
dvid_t get_new_dvid() {
  dvid_t ret = current_dvid;
  if(current_dvid == INT_MAX)
    throw std::runtime_error("dvid exceeded integer");
  else current_dvid++;
  return ret;
}
std::map<dvid_t,intptr_t> dvid_table;

/*
  for vector of double/float/int/size_t, serialization is skipped
 */
template <>
DVID<std::vector<double>> make_dvid_broadcast(const std::vector<double>& src) {
  std::vector<double>* newsrc = new std::vector<double>();
  *newsrc = src;
  auto d = get_new_dvid();
  intptr_t root_buf_ptr = reinterpret_cast<intptr_t>(&(*newsrc)[0]);
  size_t size = src.size() * sizeof(double);
  bcast_rpc_oneway(dvid_broadcast_helper2<double>, d, size, root_buf_ptr);
  set_data<std::vector<double>>(d, newsrc);
  return DVID<std::vector<double>>(d);
}

template <>
DVID<std::vector<float>> make_dvid_broadcast(const std::vector<float>& src) {
  std::vector<float>* newsrc = new std::vector<float>();
  *newsrc = src;
  auto d = get_new_dvid();
  intptr_t root_buf_ptr = reinterpret_cast<intptr_t>(&(*newsrc)[0]);
  size_t size = src.size() * sizeof(float);
  bcast_rpc_oneway(dvid_broadcast_helper2<float>, d, size, root_buf_ptr);
  set_data<std::vector<float>>(d, newsrc);
  return DVID<std::vector<float>>(d);
}

template <>
DVID<std::vector<int>> make_dvid_broadcast(const std::vector<int>& src) {
  std::vector<int>* newsrc = new std::vector<int>();
  *newsrc = src;
  auto d = get_new_dvid();
  intptr_t root_buf_ptr = reinterpret_cast<intptr_t>(&(*newsrc)[0]);
  size_t size = src.size() * sizeof(int);
  bcast_rpc_oneway(dvid_broadcast_helper2<int>, d, size, root_buf_ptr);
  set_data<std::vector<int>>(d, newsrc);
  return DVID<std::vector<int>>(d);
}

template <>
DVID<std::vector<size_t>> make_dvid_broadcast(const std::vector<size_t>& src) {
  std::vector<size_t>* newsrc = new std::vector<size_t>();
  *newsrc = src;
  auto d = get_new_dvid();
  intptr_t root_buf_ptr = reinterpret_cast<intptr_t>(&(*newsrc)[0]);
  size_t size = src.size() * sizeof(size_t);
  bcast_rpc_oneway(dvid_broadcast_helper2<size_t>, d, size, root_buf_ptr);
  set_data<std::vector<size_t>>(d, newsrc);
  return DVID<std::vector<size_t>>(d);
}

template <> std::vector<std::vector<double>> 
gather(DVID<std::vector<double>>& d) {
  std::vector<std::vector<double>> ret(get_nodesize());
  d.mapv(dvid_gather_helper2<double>(reinterpret_cast<intptr_t>(&ret)));
  return ret;
}
template <> std::vector<std::vector<float>>
gather(DVID<std::vector<float>>& d) {
  std::vector<std::vector<float>> ret(get_nodesize());
  d.mapv(dvid_gather_helper2<float>(reinterpret_cast<intptr_t>(&ret)));
  return ret;
}
template <> std::vector<std::vector<int>> 
gather(DVID<std::vector<int>>& d) {
  std::vector<std::vector<int>> ret(get_nodesize());
  d.mapv(dvid_gather_helper2<int>(reinterpret_cast<intptr_t>(&ret)));
  return ret;
}
template <> std::vector<std::vector<size_t>>
gather(DVID<std::vector<size_t>>& d) {
  std::vector<std::vector<size_t>> ret(get_nodesize());
  d.mapv(dvid_gather_helper2<size_t>(reinterpret_cast<intptr_t>(&ret)));
  return ret;
}

}
