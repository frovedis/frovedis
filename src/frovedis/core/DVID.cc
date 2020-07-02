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

// stacks for split_context_execution
std::vector<std::map<dvid_t,intptr_t>> dvid_table_stack;
std::vector<dvid_t> current_dvid_stack;

/*
  for vector of PoD, serialization is skipped
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
DVID<std::vector<long>> make_dvid_broadcast(const std::vector<long>& src) {
  std::vector<long>* newsrc = new std::vector<long>();
  *newsrc = src;
  auto d = get_new_dvid();
  intptr_t root_buf_ptr = reinterpret_cast<intptr_t>(&(*newsrc)[0]);
  size_t size = src.size() * sizeof(long);
  bcast_rpc_oneway(dvid_broadcast_helper2<long>, d, size, root_buf_ptr);
  set_data<std::vector<long>>(d, newsrc);
  return DVID<std::vector<long>>(d);
}

template <>
DVID<std::vector<long long>>
make_dvid_broadcast(const std::vector<long long>& src) {
  std::vector<long long>* newsrc = new std::vector<long long>();
  *newsrc = src;
  auto d = get_new_dvid();
  intptr_t root_buf_ptr = reinterpret_cast<intptr_t>(&(*newsrc)[0]);
  size_t size = src.size() * sizeof(long long);
  bcast_rpc_oneway(dvid_broadcast_helper2<long long>, d, size, root_buf_ptr);
  set_data<std::vector<long long>>(d, newsrc);
  return DVID<std::vector<long long>>(d);
}

template <>
DVID<std::vector<unsigned int>>
make_dvid_broadcast(const std::vector<unsigned int>& src) {
  std::vector<unsigned int>* newsrc = new std::vector<unsigned int>();
  *newsrc = src;
  auto d = get_new_dvid();
  intptr_t root_buf_ptr = reinterpret_cast<intptr_t>(&(*newsrc)[0]);
  size_t size = src.size() * sizeof(unsigned int);
  bcast_rpc_oneway(dvid_broadcast_helper2<unsigned int>, d, size, root_buf_ptr);
  set_data<std::vector<unsigned int>>(d, newsrc);
  return DVID<std::vector<unsigned int>>(d);
}

template <>
DVID<std::vector<unsigned long>>
make_dvid_broadcast(const std::vector<unsigned long>& src) {
  std::vector<unsigned long>* newsrc = new std::vector<unsigned long>();
  *newsrc = src;
  auto d = get_new_dvid();
  intptr_t root_buf_ptr = reinterpret_cast<intptr_t>(&(*newsrc)[0]);
  size_t size = src.size() * sizeof(unsigned long);
  bcast_rpc_oneway(dvid_broadcast_helper2<unsigned long>, d, size,
                   root_buf_ptr);
  set_data<std::vector<unsigned long>>(d, newsrc);
  return DVID<std::vector<unsigned long>>(d);
}

template <>
DVID<std::vector<unsigned long long>>
make_dvid_broadcast(const std::vector<unsigned long long>& src) {
  std::vector<unsigned long long>* newsrc =
    new std::vector<unsigned long long>();
  *newsrc = src;
  auto d = get_new_dvid();
  intptr_t root_buf_ptr = reinterpret_cast<intptr_t>(&(*newsrc)[0]);
  size_t size = src.size() * sizeof(unsigned long long);
  bcast_rpc_oneway(dvid_broadcast_helper2<unsigned long long>, d, size,
                   root_buf_ptr);
  set_data<std::vector<unsigned long long>>(d, newsrc);
  return DVID<std::vector<unsigned long long>>(d);
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
template <> std::vector<std::vector<long>> 
gather(DVID<std::vector<long>>& d) {
  std::vector<std::vector<long>> ret(get_nodesize());
  d.mapv(dvid_gather_helper2<long>(reinterpret_cast<intptr_t>(&ret)));
  return ret;
}
template <> std::vector<std::vector<long long>> 
gather(DVID<std::vector<long long>>& d) {
  std::vector<std::vector<long long>> ret(get_nodesize());
  d.mapv(dvid_gather_helper2<long long>(reinterpret_cast<intptr_t>(&ret)));
  return ret;
}
template <> std::vector<std::vector<unsigned int>> 
gather(DVID<std::vector<unsigned int>>& d) {
  std::vector<std::vector<unsigned int>> ret(get_nodesize());
  d.mapv(dvid_gather_helper2<unsigned int>(reinterpret_cast<intptr_t>(&ret)));
  return ret;
}
template <> std::vector<std::vector<unsigned long>> 
gather(DVID<std::vector<unsigned long>>& d) {
  std::vector<std::vector<unsigned long>> ret(get_nodesize());
  d.mapv(dvid_gather_helper2<unsigned long>(reinterpret_cast<intptr_t>(&ret)));
  return ret;
}
template <> std::vector<std::vector<unsigned long long>> 
gather(DVID<std::vector<unsigned long long>>& d) {
  std::vector<std::vector<unsigned long long>> ret(get_nodesize());
  d.mapv(dvid_gather_helper2<unsigned long long>
         (reinterpret_cast<intptr_t>(&ret)));
  return ret;
}
}
