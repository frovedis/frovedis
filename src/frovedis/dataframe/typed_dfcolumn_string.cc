#include "dfcolumn_impl.hpp"
#include <regex>
#if !(defined(_SX) || defined(__ve__))
#include <unordered_set>
#endif

namespace frovedis {

using namespace std;

template class typed_dfcolumn<std::string>;

struct is_4 {
  int operator()(size_t a) const {return a == 4;}
};

struct is_nullstr {
  is_nullstr(int n) : n(n) {}
  int operator()(int a) const {return a == n;}
  int n;
};

template <>
std::vector<size_t>
get_null_like_positions (std::vector<std::string>& col) {
  auto size = col.size();
  auto colp = col.data();
  std::vector<size_t> string_size(size);
  auto string_sizep = string_size.data();
  for(size_t i = 0; i < size; i++) {
    string_sizep[i] = colp[i].size();
  }
  auto cand = find_condition(string_size, is_4());
  auto candp = cand.data();
  auto cand_size = cand.size();
  std::vector<int*> candptr(cand_size);
  auto candptrp = candptr.data();
  for(size_t i = 0; i < cand_size; i++) {
    candptrp[i] = 
      reinterpret_cast<int*>(const_cast<char*>(colp[candp[i]].data()));
  }
  // this is possible because "NULL" happend to be four characters...
  const char* nullstr = "NULL";
  int intnullstr = *reinterpret_cast<const int*>(nullstr);
  auto hit = find_condition_pointer(candptrp, cand_size,
                                    is_nullstr(intnullstr));
  auto hit_size = hit.size();
  auto hitp = hit.data();
  std::vector<size_t> ret(hit_size);
  auto retp = ret.data();
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
  for(size_t i = 0; i < hit_size; i++) {
    retp[i] = candp[hitp[i]];
  }
  return ret;
}

vector<string>
convert_val(vector<size_t>& val, vector<vector<string>>& dic_idx) {
  vector<string> ret(val.size());
  size_t nodemask = (size_t(1) << DFNODESHIFT) - 1;
  int nodesize = get_nodesize();
  vector<vector<size_t>> part_idx(nodesize);
  size_t size = val.size();
  for(size_t i = 0; i < size; i++) {
    if(val[i] == numeric_limits<size_t>::max()) ret[i] = "NULL";
    else {
      auto node = val[i] >> DFNODESHIFT;
      auto idx = val[i] & nodemask;
      ret[i] = dic_idx[node][idx];
    }
  }
  return ret;
}

vector<pair<string, size_t>> append_idx(vector<string>& vs) {
  size_t selfid = static_cast<size_t>(get_selfid());
  size_t nodeinfo = selfid << DFNODESHIFT;
  vector<pair<string, size_t>> ret(vs.size());
  for(size_t i = 0; i < vs.size(); i++) {
    ret[i].first = vs[i];
    ret[i].second = i + nodeinfo;
  }
  return ret;
}

void convert_idx(my_map<string,vector<size_t>>& group,
                 my_map<string,size_t>& dic,
                 vector<string>& dic_idx,
                 vector<vector<size_t>>& local_idx,
                 vector<vector<size_t>>& global_idx) {
  int nodesize = get_nodesize();
  size_t selfid = static_cast<size_t>(get_selfid());
  size_t nodeinfo = selfid << DFNODESHIFT;
  size_t nodemask = (size_t(1) << DFNODESHIFT) - 1;
  local_idx.resize(nodesize);
  global_idx.resize(nodesize);
  dic_idx.resize(group.size());
  size_t i = 0;
  for(auto it = group.begin(); it != group.end(); ++it, ++i) {
    dic_idx[i] = it->first;
    dic[it->first] = i + nodeinfo;
    auto& samestrvec = it->second;
    for(size_t j = 0; j < samestrvec.size(); j++) {
      size_t nodeid = samestrvec[j] >> DFNODESHIFT;
      local_idx[nodeid].push_back(samestrvec[j] & nodemask);
      global_idx[nodeid].push_back(i + nodeinfo);
    }
  }
}

void set_idx(vector<size_t>& val,
             vector<vector<size_t>>& local_idx,
             vector<vector<size_t>>& global_idx) {
  size_t total = 0;
  for(size_t i = 0; i < local_idx.size(); i++) total += local_idx[i].size();
  val.resize(total);
  for(size_t i = 0; i < local_idx.size(); i++) {
#pragma cdir nodep
#pragma _NEC ivdep
    for(size_t j = 0; j < local_idx[i].size(); j++) {
      val[local_idx[i][j]] = global_idx[i][j];
    }
  }
}

node_local<vector<string>> typed_dfcolumn<string>::get_val() {
  auto bcast_dic_idx = broadcast(dic_idx->gather()); // TODO: expensive!
  return val.map(convert_val, bcast_dic_idx);
}
void typed_dfcolumn<string>::init(node_local<std::vector<std::string>>& nl) {
  nulls = make_node_local_allocate<vector<size_t>>();
  auto withidx = nl.map(append_idx);
  auto withidxdv = withidx.viewas_dvector<pair<string,size_t>>();
  auto group = withidxdv.group_by_key<string,size_t>();
  auto groupnl = group.viewas_node_local();
  dic = make_shared<dunordered_map<string,size_t>>
    (make_dunordered_map_allocate<string,size_t>());
  dic_idx = make_shared<node_local<vector<string>>>
    (make_node_local_allocate<vector<string>>());
  auto dicnl = dic->viewas_node_local();
  auto local_idx = make_node_local_allocate<vector<vector<size_t>>>();
  auto global_idx = make_node_local_allocate<vector<vector<size_t>>>();
  groupnl.mapv(convert_idx, dicnl, *dic_idx, local_idx, global_idx);
  auto exchanged_local_idx = alltoall_exchange(local_idx);
  auto exchanged_global_idx = alltoall_exchange(global_idx);
  val = make_node_local_allocate<vector<size_t>>();
  val.mapv(set_idx, exchanged_local_idx, exchanged_global_idx);
}

void typed_dfcolumn<string>::debug_print() {
  std::cout << "dtype: " << dtype() << std::endl;
  size_t nodemask = (size_t(1) << DFNODESHIFT) - 1;
  std::cout << "dic: " << std::endl;
  for(auto& i: dic->viewas_node_local().gather()) {
    for(auto j: i) {
      cout << j.first << "|" 
           << (j.second >> DFNODESHIFT) << "-"
           << (j.second & nodemask) << " ";
    }
    std::cout << ": ";
  }
  cout << endl;
  std::cout << "dic_idx: " << std::endl;
  for(auto& i: dic_idx->gather()) {
    for(auto j: i) {
      cout << j << " ";
    }
    std::cout << ": ";
  }
  cout << endl;
  std::cout << "val: " << std::endl;
  for(auto& i: val.gather()) {
    for(auto j: i) {
      std::cout << (j >> DFNODESHIFT) << "-" << (j & nodemask) << " ";
    }
    std::cout << ": ";
  }
  std::cout << std::endl;
  std::cout << "nulls: ";
  for(auto& i: nulls.gather()) {
    for(auto j: i) std::cout << j << " ";
    std::cout << ": ";
  }
  std::cout << std::endl;
  std::cout << "contain_nulls: " << contain_nulls << std::endl;
  if(spillable) {
    std::cout << "spill_initialized: " << spill_initialized << std::endl;
    std::cout << "already_spilled_to_disk: " << already_spilled_to_disk
              << std::endl;
    std::cout << "cleared: " << cleared << std::endl;
    std::cout << "spill_state: " << spill_state << std::endl;
    std::cout << "spill_size_cache: " << spill_size_cache << std::endl;
    if(spill_initialized) {
      std::cout << "spill_path: ";
      auto spill_paths = spill_path.gather();
      for(auto& p: spill_paths) std::cout << p << ", ";
      std::cout << std::endl;
    }
  }
}

vector<size_t> extract_idx(vector<pair<string,size_t>>& v) {
  size_t size = v.size();
  vector<size_t> ret(size);
  for(size_t i = 0; i < size; i++) ret[i] = v[i].second;
  return ret;
}

vector<size_t> convert_sorted_idx(vector<size_t>& val,
                                  vector<size_t>& idx_dic,
                                  vector<size_t>& nulls) {
  size_t dicsize = idx_dic.size();
  vector<size_t> newidx(dicsize);
  size_t* newidxp = &newidx[0];
  for(size_t i = 0; i < dicsize; i++) newidxp[i] = i;
  auto ht = unique_hashtable<size_t, size_t>(idx_dic, newidx);
  std::vector<size_t> missed; // not used; should be same as nulls
  auto ret = ht.lookup(val, missed);
  auto retp = ret.data();
  auto nullsp = nulls.data();
  auto nulls_size = nulls.size();
  auto null_value = std::numeric_limits<size_t>::max();
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
  for(size_t i = 0; i < nulls_size; i++) {
    retp[nullsp[i]] = null_value;
  }
  return ret;
}

typed_dfcolumn<size_t>
typed_dfcolumn<string>::sort_prepare() {
  typed_dfcolumn<size_t> rescol;
  auto with_idx = dic_idx->map(append_idx);
  auto with_idx_dv = with_idx.moveto_dvector<pair<string,size_t>>();
  auto with_idx_sorted = with_idx_dv.sort().moveto_node_local();
  auto idx_dic = broadcast(with_idx_sorted.map(extract_idx).
                           viewas_dvector<size_t>().gather());
  rescol.val = val.map(convert_sorted_idx, idx_dic, nulls);
  rescol.nulls = nulls; // copy
  return rescol;
}

shared_ptr<dfcolumn>
typed_dfcolumn<string>::sort(node_local<vector<size_t>>& idx) {
  auto tmpcol = sort_prepare();
  return tmpcol.sort(idx);
}

shared_ptr<dfcolumn>
typed_dfcolumn<string>::sort_desc(node_local<vector<size_t>>& idx) {
  auto tmpcol = sort_prepare();
  return tmpcol.sort_desc(idx);
}

shared_ptr<dfcolumn>
typed_dfcolumn<string>::sort_with_idx(node_local<vector<size_t>>& idx,
                                      node_local<vector<size_t>>& res_idx) {
  auto tmpcol = sort_prepare();
  return tmpcol.sort_with_idx(idx, res_idx);
}

shared_ptr<dfcolumn>
typed_dfcolumn<string>::
sort_with_idx_desc(node_local<vector<size_t>>& idx,
                   node_local<vector<size_t>>& res_idx) {
  auto tmpcol = sort_prepare();
  return tmpcol.sort_with_idx_desc(idx, res_idx);
}

void create_string_trans_table(my_map<string,size_t>& leftdic,
                               my_map<string,size_t>& rightdic,
                               vector<size_t>& from,
                               vector<size_t>& to) {
  auto maxsize = rightdic.size();
  from.reserve(maxsize);
  to.reserve(maxsize);
  for(auto it = rightdic.begin(); it != rightdic.end(); ++it) {
    auto f = leftdic.find(it->first);
    if(f != leftdic.end()) {
      from.push_back(it->second);
      to.push_back(f->second);
    }
  }
}

vector<size_t> equal_prepare_helper(vector<size_t>& val,
                                    vector<size_t>& from,
                                    vector<size_t>& to) {
  auto ht = unique_hashtable<size_t, size_t>(from, to);
  vector<size_t> missed;
  auto ret = ht.lookup(val, missed);
  size_t* retp = &ret[0];
  size_t* missedp = &missed[0];
  size_t missedsize = missed.size();
  size_t misseddummy = numeric_limits<size_t>::max() - 1;
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
  for(size_t i = 0; i < missedsize; i++) {
    retp[missedp[i]] = misseddummy - i;
  }
  return ret;
}

// assumes implementation of dunordered_map
node_local<vector<size_t>>
typed_dfcolumn<string>::
equal_prepare(shared_ptr<typed_dfcolumn<string>>& right) {
  auto& rightdic = *(right->dic);
  auto rightdicnl = rightdic.viewas_node_local();
  auto nlfrom = make_node_local_allocate<std::vector<size_t>>();
  auto nlto = make_node_local_allocate<std::vector<size_t>>();
  dic->viewas_node_local().mapv(create_string_trans_table, rightdicnl,
                                nlfrom, nlto);
  auto bcastfrom = broadcast(nlfrom.moveto_dvector<size_t>().gather());
  auto bcastto = broadcast(nlto.moveto_dvector<size_t>().gather());
  return right->val.map(equal_prepare_helper, bcastfrom, bcastto);
}

node_local<std::vector<size_t>>
typed_dfcolumn<string>::filter_eq(std::shared_ptr<dfcolumn>& right) {
  auto right2 = std::dynamic_pointer_cast<typed_dfcolumn<string>>(right);
  if(!right2) throw std::runtime_error("filter_eq: column types are different");
  auto rightval = equal_prepare(right2);
  auto filtered_idx = val.map(filter_eq_helper<size_t,size_t>, rightval);
  if(contain_nulls)
    return filtered_idx.map(set_difference<size_t>, nulls);
  else return filtered_idx;
}

node_local<std::vector<size_t>>
typed_dfcolumn<string>::filter_neq(std::shared_ptr<dfcolumn>& right) {
  auto right2 = std::dynamic_pointer_cast<typed_dfcolumn<string>>(right);
  if(!right2) throw std::runtime_error("filter_eq: column types are different");
  auto rightval = equal_prepare(right2);
  auto filtered_idx = val.map(filter_neq_helper<size_t,size_t>, rightval);
  if(right2->contain_nulls)
    filtered_idx = filtered_idx.map(set_difference<size_t>, right2->nulls);
  if(contain_nulls)
    return filtered_idx.map(set_difference<size_t>, nulls);
  else return filtered_idx;
}

std::pair<node_local<std::vector<size_t>>, node_local<std::vector<size_t>>>
typed_dfcolumn<string>::hash_join_eq
(std::shared_ptr<dfcolumn>& right,
 node_local<std::vector<size_t>>& left_full_local_idx, 
 node_local<std::vector<size_t>>& right_full_local_idx) {
  auto right2 = std::dynamic_pointer_cast<typed_dfcolumn<string>>(right);
  if(!right2) 
    throw std::runtime_error("hash_join_eq: column types are different");

  auto left_split_val =
    make_node_local_allocate<std::vector<std::vector<size_t>>>();
  auto left_split_idx =
    make_node_local_allocate<std::vector<std::vector<size_t>>>();
  auto right_non_null_idx = make_node_local_allocate<std::vector<size_t>>();
  auto right_split_val =
    make_node_local_allocate<std::vector<std::vector<size_t>>>();
  auto right_split_idx =
    make_node_local_allocate<std::vector<std::vector<size_t>>>();
  if(contain_nulls) {
    auto left_non_null_idx = make_node_local_allocate<std::vector<size_t>>();
    auto left_non_null_val = val.map(extract_non_null<size_t>,
                                     left_full_local_idx, nulls,
                                     left_non_null_idx);
    auto left_global_idx = local_to_global_idx(left_non_null_idx);
    left_non_null_val.mapv(split_by_hash<size_t>, left_split_val,
                           left_global_idx, left_split_idx);
  } else {
    auto left_non_null_val = val.map(extract_helper2<size_t>,
                                     left_full_local_idx);
    auto left_global_idx = local_to_global_idx(left_full_local_idx);
    left_non_null_val.mapv(split_by_hash<size_t>, left_split_val,
                           left_global_idx,left_split_idx);
  }
  auto left_exchanged_val = alltoall_exchange(left_split_val);
  auto left_exchanged_idx = alltoall_exchange(left_split_idx);

  auto right_val = equal_prepare(right2);
  auto& right_nulls = right2->nulls;
  if(right2->contain_nulls) {
    auto right_non_null_val =
      right_val.map(extract_non_null<size_t>, right_full_local_idx,
                    right_nulls, right_non_null_idx);
    auto right_global_idx = local_to_global_idx(right_non_null_idx);
    right_non_null_val.mapv(split_by_hash<size_t>, right_split_val,
                            right_global_idx, right_split_idx);
  } else {
    auto right_non_null_val =
      right_val.map(extract_helper2<size_t>, right_full_local_idx);
    auto right_global_idx = local_to_global_idx(right_full_local_idx);
    right_non_null_val.mapv(split_by_hash<size_t>, right_split_val,
                            right_global_idx, right_split_idx);
  }
  auto right_exchanged_val = alltoall_exchange(right_split_val);
  auto right_exchanged_idx = alltoall_exchange(right_split_idx);
  auto left_idx_ret = make_node_local_allocate<std::vector<size_t>>();
  auto right_idx_ret = make_node_local_allocate<std::vector<size_t>>();
  left_exchanged_val.mapv(hash_join_eq_helper<size_t>, left_exchanged_idx,
                          right_exchanged_val, right_exchanged_idx,
                          left_idx_ret, right_idx_ret);
  return std::make_pair(std::move(left_idx_ret), std::move(right_idx_ret));
}

std::tuple<node_local<std::vector<size_t>>,
           node_local<std::vector<size_t>>,
           node_local<std::vector<size_t>>>
typed_dfcolumn<string>::outer_hash_join_eq
(std::shared_ptr<dfcolumn>& right,
 node_local<std::vector<size_t>>& left_full_local_idx, 
 node_local<std::vector<size_t>>& right_full_local_idx) {
  auto right2 = std::dynamic_pointer_cast<typed_dfcolumn<string>>(right);
  if(!right2)
    throw std::runtime_error("outer_hash_join_eq: column types are different");
  auto left_non_null_idx = make_node_local_allocate<std::vector<size_t>>();
  auto left_split_val =
    make_node_local_allocate<std::vector<std::vector<size_t>>>();
  auto left_split_idx =
    make_node_local_allocate<std::vector<std::vector<size_t>>>();
  auto right_non_null_idx = make_node_local_allocate<std::vector<size_t>>();
  auto right_split_val =
    make_node_local_allocate<std::vector<std::vector<size_t>>>();
  auto right_split_idx =
    make_node_local_allocate<std::vector<std::vector<size_t>>>();
  if(contain_nulls) {
    auto left_non_null_val = val.map(extract_non_null<size_t>,
                                     left_full_local_idx, nulls,
                                     left_non_null_idx);
    auto left_global_idx = local_to_global_idx(left_non_null_idx);
    left_non_null_val.mapv(split_by_hash<size_t>, left_split_val,
                           left_global_idx, left_split_idx);
  } else {
    auto left_non_null_val = val.map(extract_helper2<size_t>,
                                     left_full_local_idx);
    auto left_global_idx = local_to_global_idx(left_full_local_idx);
    left_non_null_val.mapv(split_by_hash<size_t>, left_split_val,
                           left_global_idx, left_split_idx);
  }
  auto left_exchanged_val = alltoall_exchange(left_split_val);
  auto left_exchanged_idx = alltoall_exchange(left_split_idx);
  auto right_val = equal_prepare(right2);
  auto& right_nulls = right2->nulls;
  if(right2->contain_nulls) {
    auto right_non_null_val =
      right_val.map(extract_non_null<size_t>, right_full_local_idx,
                    right_nulls, right_non_null_idx);
    auto right_global_idx = local_to_global_idx(right_non_null_idx);
    right_non_null_val.mapv(split_by_hash<size_t>, right_split_val,
                            right_global_idx, right_split_idx);
  } else {
    auto right_non_null_val =
      right_val.map(extract_helper2<size_t>, right_full_local_idx);
    auto right_global_idx = local_to_global_idx(right_full_local_idx);
    right_non_null_val.mapv(split_by_hash<size_t>, right_split_val,
                            right_global_idx, right_split_idx);
  }
  auto right_exchanged_val = alltoall_exchange(right_split_val);
  auto right_exchanged_idx = alltoall_exchange(right_split_idx);
  auto left_idx_ret = make_node_local_allocate<std::vector<size_t>>();
  auto right_idx_ret = make_node_local_allocate<std::vector<size_t>>();
  auto null_idx_ret = left_exchanged_val.map(outer_hash_join_eq_helper<size_t>,
                                             left_exchanged_idx,
                                             right_exchanged_val,
                                             right_exchanged_idx,
                                             left_idx_ret,
                                             right_idx_ret);
  return std::make_tuple(std::move(left_idx_ret), std::move(right_idx_ret),
                         std::move(null_idx_ret));
}

std::pair<node_local<std::vector<size_t>>,
          node_local<std::vector<size_t>>>
typed_dfcolumn<string>::bcast_join_eq
(std::shared_ptr<dfcolumn>& right,
 node_local<std::vector<size_t>>& left_full_local_idx, 
 node_local<std::vector<size_t>>& right_full_local_idx) {
  auto right2 = std::dynamic_pointer_cast<typed_dfcolumn<string>>(right);
  if(!right2)
    throw std::runtime_error("bcast_join_eq: column types are different");
  node_local<std::vector<size_t>> left_non_null_idx;
  node_local<std::vector<size_t>> left_non_null_val;
  if(contain_nulls) {
    left_non_null_idx = make_node_local_allocate<std::vector<size_t>>();
    left_non_null_val = val.map(extract_non_null<size_t>, left_full_local_idx,
                                nulls, left_non_null_idx);
  } else {
    left_non_null_val = val.map(extract_helper2<size_t>, left_full_local_idx);
    left_non_null_idx = std::move(left_full_local_idx);
  }
  auto right_non_null_idx = make_node_local_allocate<std::vector<size_t>>();
  auto right_val = equal_prepare(right2);
  auto& right_nulls = right2->nulls;
  node_local<std::vector<size_t>> right_non_null_val;
  node_local<std::vector<size_t>> right_global_idx;
  if(right2->contain_nulls) {
    right_non_null_val =
      right_val.map(extract_non_null<size_t>, right_full_local_idx,
                    right_nulls, right_non_null_idx);
    right_global_idx = local_to_global_idx(right_non_null_idx);
  } else {
    right_non_null_val = right_val.map(extract_helper2<size_t>,
                                       right_full_local_idx);
    right_global_idx = local_to_global_idx(right_full_local_idx);
  }
  auto right_non_null_val_bcast =
    broadcast(right_non_null_val.template viewas_dvector<size_t>().gather());
  auto right_global_idx_bcast =
    broadcast(right_global_idx.template viewas_dvector<size_t>().gather());
  auto left_idx_ret = make_node_local_allocate<std::vector<size_t>>();
  auto right_idx_ret = make_node_local_allocate<std::vector<size_t>>();
  left_non_null_val.mapv(equi_join<size_t>, left_non_null_idx,
                         right_non_null_val_bcast, right_global_idx_bcast,
                         left_idx_ret, right_idx_ret);
  return std::make_pair(std::move(left_idx_ret), std::move(right_idx_ret));
}

std::tuple<node_local<std::vector<size_t>>,
           node_local<std::vector<size_t>>,
           node_local<std::vector<size_t>>>
typed_dfcolumn<string>::outer_bcast_join_eq
(std::shared_ptr<dfcolumn>& right,
 node_local<std::vector<size_t>>& left_full_local_idx, 
 node_local<std::vector<size_t>>& right_full_local_idx) {
  auto right2 = std::dynamic_pointer_cast<typed_dfcolumn<string>>(right);
  if(!right2)
    throw std::runtime_error("bcast_join_eq: column types are different");
  node_local<std::vector<size_t>> left_non_null_idx;
  node_local<std::vector<size_t>> left_non_null_val;
  if(contain_nulls) {
    left_non_null_idx = make_node_local_allocate<std::vector<size_t>>();
    left_non_null_val = val.map(extract_non_null<size_t>, left_full_local_idx,
                                nulls, left_non_null_idx);
  } else {
    left_non_null_val = val.map(extract_helper2<size_t>, left_full_local_idx);
    left_non_null_idx = std::move(left_full_local_idx);
  }
  auto right_non_null_idx = make_node_local_allocate<std::vector<size_t>>();
  auto right_val = equal_prepare(right2);
  auto& right_nulls = right2->nulls;
  node_local<std::vector<size_t>> right_non_null_val;
  node_local<std::vector<size_t>> right_global_idx;
  if(right2->contain_nulls) {
    right_non_null_val =
      right_val.map(extract_non_null<size_t>, right_full_local_idx,
                    right_nulls, right_non_null_idx);
    right_global_idx = local_to_global_idx(right_non_null_idx);
  } else {
    right_non_null_val = right_val.map(extract_helper2<size_t>,
                                       right_full_local_idx);
    right_global_idx = local_to_global_idx(right_full_local_idx);
  }
  auto right_non_null_val_bcast =
    broadcast(right_non_null_val.template viewas_dvector<size_t>().gather());
  auto right_global_idx_bcast =
    broadcast(right_global_idx.template viewas_dvector<size_t>().gather());
  auto left_idx_ret = make_node_local_allocate<std::vector<size_t>>();
  auto right_idx_ret = make_node_local_allocate<std::vector<size_t>>();
  auto null_idx_ret = left_non_null_val.map(outer_equi_join<size_t>,
                                            left_non_null_idx,
                                            right_non_null_val_bcast,
                                            right_global_idx_bcast,
                                            left_idx_ret, right_idx_ret);
  return std::make_tuple(std::move(left_idx_ret), std::move(right_idx_ret),
                         std::move(null_idx_ret));
}

std::pair<node_local<std::vector<size_t>>,
          node_local<std::vector<size_t>>>
typed_dfcolumn<string>::star_join_eq
(std::shared_ptr<dfcolumn>& right,
 node_local<std::vector<size_t>>& left_full_local_idx, 
 node_local<std::vector<size_t>>& right_full_local_idx) {
  auto right2 = std::dynamic_pointer_cast<typed_dfcolumn<string>>(right);
  if(!right2)
    throw std::runtime_error("star_join_eq: column types are different");
  node_local<std::vector<size_t>> left_non_null_idx;
  node_local<std::vector<size_t>> left_non_null_val;
  if(contain_nulls) {
    left_non_null_idx = make_node_local_allocate<std::vector<size_t>>();
    left_non_null_val = val.map(extract_non_null<size_t>, left_full_local_idx,
                                nulls, left_non_null_idx);
  } else {
    left_non_null_val = val.map(extract_helper2<size_t>, left_full_local_idx);
    left_non_null_idx = left_full_local_idx;
  }
  auto right_non_null_idx = make_node_local_allocate<std::vector<size_t>>();
  auto right_val = equal_prepare(right2);
  auto& right_nulls = right2->nulls;
  node_local<std::vector<size_t>> right_non_null_val;
  node_local<std::vector<size_t>> right_global_idx;
  if(right2->contain_nulls) {
    right_non_null_val =
      right_val.map(extract_non_null<size_t>, right_full_local_idx,
                    right_nulls, right_non_null_idx);
    right_global_idx = local_to_global_idx(right_non_null_idx);
  } else {
    right_non_null_val = right_val.map(extract_helper2<size_t>,
                                       right_full_local_idx);
    right_global_idx = local_to_global_idx(right_full_local_idx);
  }
  auto right_non_null_val_bcast = 
    broadcast(right_non_null_val.template viewas_dvector<size_t>().gather());
  auto right_global_idx_bcast =
    broadcast(right_global_idx.template viewas_dvector<size_t>().gather());
  auto left_idx_ret = make_node_local_allocate<std::vector<size_t>>();
  auto right_idx_ret = make_node_local_allocate<std::vector<size_t>>();
  auto missed = 
    left_non_null_val.map(unique_equi_join2<size_t>, left_non_null_idx,
                          right_non_null_val_bcast, right_global_idx_bcast,
                          right_idx_ret);
  return std::make_pair(std::move(right_idx_ret), std::move(missed));
}

vector<size_t> filter_regex_helper(vector<string>& dic, std::string& pattern) {
  vector<size_t> ret;
  size_t size = dic.size();
  regex re(pattern);
  size_t selfid = static_cast<size_t>(get_selfid());
  size_t nodeinfo = selfid << DFNODESHIFT;
  for(size_t i = 0; i < size; i++) {
    if(regex_match(dic[i], re)) ret.push_back(i+nodeinfo);
  }
  return ret;
}

vector<size_t> filter_not_regex_helper(vector<string>& dic,
                                       std::string& pattern) {
  vector<size_t> ret;
  size_t size = dic.size();
  regex re(pattern);
  size_t selfid = static_cast<size_t>(get_selfid());
  size_t nodeinfo = selfid << DFNODESHIFT;
  for(size_t i = 0; i < size; i++) {
    if(!regex_match(dic[i], re)) ret.push_back(i+nodeinfo);
  }
  return ret;
}

#if !(defined(_SX) || defined(__ve__))
vector<size_t> filter_regex_join(std::vector<size_t>& left,
                                 std::vector<size_t>& left_idx, 
                                 std::vector<size_t>& right) {
  std::unordered_set<size_t> right_set;
  for(size_t i = 0; i < right.size(); i++) {
    right_set.insert(right[i]);
  }
  vector<size_t> ret;
  for(size_t i = 0; i < left.size(); i++) {
    auto it = right_set.find(left[i]);
    if(it != right_set.end()) {
      ret.push_back(left_idx[i]);
    }
  }
  return ret;
}
#else
std::vector<size_t> filter_regex_join(std::vector<size_t>& left,
                                      std::vector<size_t>& left_idx,
                                      std::vector<size_t>& right) {
  vector<size_t> dummy(right.size());
  unique_hashtable<size_t, size_t> ht(right, dummy);
  std::vector<size_t> missed;
  ht.lookup(left, missed); // ret val not used
  return shrink_missed(left_idx, missed);
}
#endif

// implement as join, since matched strings might be many
node_local<std::vector<size_t>>
typed_dfcolumn<string>::
filter_regex(const std::string& pattern) {
  auto right_non_null_val = dic_idx->map(filter_regex_helper, broadcast(pattern));
  auto left_full_local_idx = get_local_index();
  node_local<std::vector<size_t>> left_non_null_idx;
  node_local<std::vector<size_t>> left_non_null_val;
  if(contain_nulls) {
    left_non_null_idx = make_node_local_allocate<std::vector<size_t>>();
    left_non_null_val = val.map(extract_non_null<size_t>, left_full_local_idx,
                                nulls, left_non_null_idx);
  } else {
    left_non_null_val = val.map(extract_helper2<size_t>, left_full_local_idx);
    left_non_null_idx = std::move(left_full_local_idx);
  }
  auto right_non_null_val_bcast =
    broadcast(right_non_null_val.template viewas_dvector<size_t>().gather());
  auto left_idx_ret = make_node_local_allocate<std::vector<size_t>>();
  return left_non_null_val.map(filter_regex_join, left_non_null_idx,
                               right_non_null_val_bcast);
}

node_local<std::vector<size_t>>
typed_dfcolumn<string>::
filter_not_regex(const std::string& pattern) {
  auto right_non_null_val = dic_idx->map(filter_not_regex_helper, broadcast(pattern));
  auto left_full_local_idx = get_local_index();
  node_local<std::vector<size_t>> left_non_null_idx;
  node_local<std::vector<size_t>> left_non_null_val;
  if(contain_nulls) {
    left_non_null_idx = make_node_local_allocate<std::vector<size_t>>();
    left_non_null_val = val.map(extract_non_null<size_t>, left_full_local_idx,
                                nulls, left_non_null_idx);
  } else {
    left_non_null_val = val.map(extract_helper2<size_t>, left_full_local_idx);
    left_non_null_idx = std::move(left_full_local_idx);
  }
  auto right_non_null_val_bcast =
    broadcast(right_non_null_val.template viewas_dvector<size_t>().gather());
  auto left_idx_ret = make_node_local_allocate<std::vector<size_t>>();
  return left_non_null_val.map(filter_regex_join, left_non_null_idx,
                               right_non_null_val_bcast);
}

size_t typed_dfcolumn<string>::size() {
  return val.viewas_dvector<size_t>().size();
}

std::vector<size_t> typed_dfcolumn<string>::sizes() {
  return val.template viewas_dvector<size_t>().sizes();
}

node_local<std::vector<size_t>>
typed_dfcolumn<string>::filter_eq_immed(std::shared_ptr<dfscalar>& right) {
  auto right2 = std::dynamic_pointer_cast<typed_dfscalar<std::string>>(right);
  if(!right2) throw std::runtime_error("filter string column with non string");
  bool found;
  size_t right_val = dic->get(right2->val, found);
  if(found) {
    auto filtered_idx = val.map(filter_eq_immed_helper<size_t,size_t>,
                                broadcast(right_val));
    if(contain_nulls)
      return filtered_idx.map(set_difference<size_t>, nulls);
    else return filtered_idx;
  } else {
    return make_node_local_allocate<std::vector<size_t>>();
  }
}

node_local<std::vector<size_t>>
typed_dfcolumn<string>::filter_neq_immed(std::shared_ptr<dfscalar>& right) {
  auto right2 = std::dynamic_pointer_cast<typed_dfscalar<std::string>>(right);
  if(!right2) throw std::runtime_error("filter string column with non string");
  bool found;
  size_t right_val = dic->get(right2->val, found);
  if(found) {
    auto filtered_idx = val.map(filter_neq_immed_helper<size_t,size_t>,
                                broadcast(right_val));
    if(contain_nulls)
      return filtered_idx.map(set_difference<size_t>, nulls);
    else return filtered_idx;
  } else {
    return val.map(get_local_index_helper<size_t>);
  }
}

node_local<std::vector<size_t>>
typed_dfcolumn<string>::filter_is_null() {return nulls;}

node_local<std::vector<size_t>>
typed_dfcolumn<string>::filter_is_not_null() {
  auto local_idx = get_local_index();
  if(contain_nulls)
    return local_idx.map(set_difference<size_t>, nulls);
  else return local_idx;
}

std::shared_ptr<dfcolumn>
typed_dfcolumn<string>::extract(node_local<std::vector<size_t>>& idx) {
  auto ret = std::make_shared<typed_dfcolumn<std::string>>();
  auto retnulls = make_node_local_allocate<std::vector<size_t>>();
  if(contain_nulls) {
    ret->val = val.map(extract_helper<size_t>, idx, nulls, retnulls);
    ret->nulls = std::move(retnulls);
    ret->contain_nulls_check();
  } else {
    ret->val = val.map(extract_helper2<size_t>, idx);
    ret->nulls = std::move(retnulls);
  }
  ret->dic = dic;
  ret->dic_idx = dic_idx;
  return ret;
}

std::shared_ptr<dfcolumn>
typed_dfcolumn<string>::global_extract
(node_local<std::vector<size_t>>& global_idx,
 node_local<std::vector<size_t>>& to_store_idx,
 node_local<std::vector<std::vector<size_t>>>& exchanged_idx) {
  auto ret = std::make_shared<typed_dfcolumn<std::string>>();
  auto exdata = val.map(global_extract_helper<size_t>, exchanged_idx);
  auto exchanged_back = alltoall_exchange(exdata).map(flatten<size_t>);
  ret->val = exchanged_back.map
    (+[](std::vector<size_t>& val, std::vector<size_t>& idx) {
      auto valp = val.data();
      auto idxp = idx.data();
      auto size = idx.size();
      std::vector<size_t> ret(size);
      auto retp = ret.data();
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
      for(size_t i = 0 ; i < size; i++) {
        retp[i] = valp[idxp[i]];
      }
      return ret;
    }, to_store_idx);
  if(contain_nulls) {
    /*
    auto exnulls = nulls.map(global_extract_null_helper, exchanged_idx);
    auto exchanged_back_nulls = alltoall_exchange(exnulls);
    auto null_exists = make_node_local_allocate<int>();
    auto nullhashes = exchanged_back_nulls.map(create_null_hash_from_partition,
                                               null_exists);
    ret->nulls = nullhashes.map(global_extract_null_helper2, global_idx,
                                null_exists);
    */
    // assume that val always contains max() iff NULL; this is much faster
    // val does not become max() in the case of string
    ret->nulls = ret->val.map(+[](std::vector<size_t>& val) {
        return vector_find_eq(val, std::numeric_limits<size_t>::max());
      });
    ret->contain_nulls_check();
  } else {
    ret->nulls = make_node_local_allocate<std::vector<size_t>>();    
  }
  ret->dic = dic;
  ret->dic_idx = dic_idx;
  return ret;
}

node_local<std::vector<size_t>> 
typed_dfcolumn<string>::get_nulls(){return nulls;}

dvector<std::string> typed_dfcolumn<string>::as_string() {
  return get_val().template moveto_dvector<std::string>();
}

words dfcolumn_string_as_words_helper(const std::vector<string>& str,
                                      const std::vector<size_t>& nulls,
                                      const std::string& nullstr) {
  auto ws = vector_string_to_words(str);
  auto nulls_size = nulls.size();
  auto nullsp = nulls.data();
  auto nullstrvec = char_to_int(nullstr);
  auto nullstr_size = nullstr.size();
  auto chars_size = ws.chars.size();
  auto newchars_size = chars_size + nullstr_size;
  auto charsp = ws.chars.data();
  std::vector<int> newchars(newchars_size);
  auto newcharsp = newchars.data();
  for(size_t i = 0; i < chars_size; i++) newcharsp[i] = charsp[i];
  auto nullstrvecp = nullstrvec.data();
  for(size_t i = 0; i < nullstr_size; i++) {
    newcharsp[chars_size + i] = nullstrvecp[i];
  }
  ws.chars.swap(newchars);
  size_t nullstart = chars_size;
  size_t nulllens = nullstr_size;
  auto startsp = ws.starts.data();
  auto lensp = ws.lens.data();
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob  
  for(size_t i = 0; i < nulls_size; i++) {
    startsp[nullsp[i]] = nullstart;
    lensp[nullsp[i]] = nulllens;
  }
  return ws;
}

node_local<words>
typed_dfcolumn<string>::as_words(size_t precision, // not used
                                 const std::string& datetime_fmt, // not used
                                 bool quote_escape,
                                 const std::string& nullstr) {
  if(contain_nulls) {
    auto nl_words = get_val().map(dfcolumn_string_as_words_helper, nulls,
                                  broadcast(nullstr));
    if(quote_escape) nl_words.mapv(quote_and_escape);
    return nl_words;
  } else {
    auto nl_words = get_val().map(vector_string_to_words);
    if(quote_escape) nl_words.mapv(quote_and_escape);
    return nl_words;
  }
}

node_local<std::vector<size_t>> typed_dfcolumn<string>::get_local_index() {
  return val.map(get_local_index_helper<size_t>);
}

void typed_dfcolumn<string>::append_nulls
(node_local<std::vector<size_t>>& to_append) {
  val.mapv(append_nulls_helper<size_t>, to_append, nulls);
  contain_nulls = true;
}

std::shared_ptr<dfcolumn>
typed_dfcolumn<string>::group_by
(node_local<std::vector<size_t>>& local_idx,
 node_local<std::vector<size_t>>& split_idx,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map) {
  auto ret = std::make_shared<typed_dfcolumn<string>>();
  ret->nulls = make_node_local_allocate<std::vector<size_t>>();
  group_by_impl(val, nulls, local_idx, split_idx, hash_divide, merge_map,
                ret->val, ret->nulls);
  ret->dic = dic;
  ret->dic_idx = dic_idx;
  ret->contain_nulls_check();
  return ret;
}

void typed_dfcolumn<string>::multi_group_by_sort
(node_local<std::vector<size_t>>& local_idx) {
  val.mapv(multi_group_by_sort_helper<size_t>, local_idx);
}

node_local<std::vector<size_t>>
typed_dfcolumn<string>::multi_group_by_sort_split
(node_local<std::vector<size_t>>& local_idx) {
  return val.map(multi_group_by_sort_split_helper<size_t>, local_idx);
}

node_local<std::vector<size_t>>
typed_dfcolumn<string>::multi_group_by_split
(node_local<std::vector<size_t>>& local_idx) {
  return val.map(multi_group_by_split_helper<size_t>, local_idx);
}

std::shared_ptr<dfcolumn>
typed_dfcolumn<string>::multi_group_by_extract
(node_local<std::vector<size_t>>& local_idx,
 node_local<std::vector<size_t>>& split_idx,
 bool check_nulls) {
  auto ret = std::make_shared<typed_dfcolumn<string>>();
  ret->val = val.map(multi_group_by_extract_helper<size_t>, local_idx, split_idx);
  ret->contain_nulls = contain_nulls;
  if(contain_nulls && check_nulls) {
    ret->nulls = ret->val.map(+[](std::vector<size_t>& val) {
        return find_value(val, std::numeric_limits<size_t>::max());        
      });
  } else {
    ret->nulls = make_node_local_allocate<std::vector<size_t>>();
  }
  ret->dic = dic;
  ret->dic_idx = dic_idx;
  return ret;
}

node_local<std::vector<size_t>>
typed_dfcolumn<string>::calc_hash_base() {
  return val.map(calc_hash_base_helper<size_t>);
}

void typed_dfcolumn<string>::calc_hash_base
(node_local<std::vector<size_t>>& hash_base, int shift) {
  val.mapv(calc_hash_base_helper2<size_t>(shift), hash_base);
}

std::shared_ptr<dfcolumn> 
typed_dfcolumn<string>::multi_group_by_exchange
(node_local<std::vector<std::vector<size_t>>>& hash_divide) {
  auto ret = std::make_shared<typed_dfcolumn<string>>();
  ret->val = multi_group_by_exchange_helper(val, hash_divide);
  ret->nulls = make_node_local_allocate<std::vector<size_t>>();
  ret->contain_nulls = contain_nulls;
  ret->dic = dic;
  ret->dic_idx = dic_idx;
  return ret;
}

std::shared_ptr<dfcolumn>
typed_dfcolumn<string>::count
(node_local<std::vector<size_t>>& local_grouped_idx,
 node_local<std::vector<size_t>>& local_idx_split,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map,
 node_local<size_t>& row_sizes) {
  return count_impl(val, nulls, local_grouped_idx, local_idx_split,
                    hash_divide, merge_map, row_sizes);
}

std::shared_ptr<dfcolumn>
typed_dfcolumn<string>::size
(node_local<std::vector<size_t>>& local_grouped_idx,
 node_local<std::vector<size_t>>& local_idx_split,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map,
 node_local<size_t>& row_sizes) {
  return count_impl(val, nulls, local_grouped_idx, local_idx_split,
                    hash_divide, merge_map, row_sizes, true);
}

std::shared_ptr<dfcolumn>
typed_dfcolumn<string>::count_distinct
(node_local<std::vector<size_t>>& local_grouped_idx,
 node_local<std::vector<size_t>>& local_idx_split,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map,
 node_local<size_t>& row_sizes) {
  return count_distinct_impl(val, local_grouped_idx, local_idx_split,
                             hash_divide, merge_map, row_sizes);
}

std::shared_ptr<dfcolumn>
typed_dfcolumn<string>::first
(node_local<std::vector<size_t>>& local_grouped_idx,
 node_local<std::vector<size_t>>& local_idx_split,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map,
 node_local<size_t>& row_sizes,
 bool ignore_nulls) {
#ifndef DONOT_ALLOW_MAX_AS_VALUE
  throw std::runtime_error
    ("define DONOT_ALLOW_MAX_AS_VALUE to use first");
#endif
  auto bignore_nulls = broadcast(ignore_nulls);
  auto ret = std::make_shared<typed_dfcolumn<string>>();
  ret->dic = dic;
  ret->dic_idx = dic_idx;
  ret->nulls = make_node_local_allocate<std::vector<size_t>>();
  auto local_agg = val.map(first_helper<size_t>, local_grouped_idx,
                           local_idx_split, hash_divide,
                           bignore_nulls);
  auto exchanged = alltoall_exchange(local_agg);
  auto newval = exchanged.map
    (+[](std::vector<std::vector<size_t>>& exchanged,
         std::vector<std::vector<size_t>>& merge_map,
         std::vector<size_t>& nulls,
         bool ignore_nulls,
         size_t row_size) {
      std::vector<size_t> newval(row_size);
      auto newvalp = newval.data();
      auto max = std::numeric_limits<size_t>::max();
      if(ignore_nulls) {
        for(size_t i = 0; i < row_size; i++) newvalp[i] = max;
      }
      auto exchanged_size = exchanged.size();
      for(size_t i = 0; i < exchanged_size; i++) {
        auto currentp = exchanged[exchanged_size - 1 - i].data();
        auto current_size = exchanged[exchanged_size - 1 - i].size();
        auto merge_mapp = merge_map[exchanged_size - 1 - i].data();
        if(!ignore_nulls) {
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
          for(size_t j = 0; j < current_size; j++) {
            newvalp[merge_mapp[j]] = currentp[j];
          }
        } else {
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
          for(size_t j = 0; j < current_size; j++) {
            if(currentp[j] != max)
              newvalp[merge_mapp[j]] = currentp[j];
          }
        }
      }
      nulls = vector_find_tmax(newval); 
      return newval;
    }, merge_map, ret->nulls, bignore_nulls, row_sizes);
  ret->val = std::move(newval);
  ret->contain_nulls_check();
  return ret;
}

std::shared_ptr<dfcolumn>
typed_dfcolumn<string>::last
(node_local<std::vector<size_t>>& local_grouped_idx,
 node_local<std::vector<size_t>>& local_idx_split,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map,
 node_local<size_t>& row_sizes,
 bool ignore_nulls) {
#ifndef DONOT_ALLOW_MAX_AS_VALUE
  throw std::runtime_error
    ("define DONOT_ALLOW_MAX_AS_VALUE to use last");
#endif
  auto bignore_nulls = broadcast(ignore_nulls);
  auto ret = std::make_shared<typed_dfcolumn<string>>();
  ret->dic = dic;
  ret->dic_idx = dic_idx;
  ret->nulls = make_node_local_allocate<std::vector<size_t>>();
  auto local_agg = val.map(last_helper<size_t>, local_grouped_idx,
                           local_idx_split, hash_divide,
                           bignore_nulls);
  auto exchanged = alltoall_exchange(local_agg);
  auto newval = exchanged.map
    (+[](std::vector<std::vector<size_t>>& exchanged,
         std::vector<std::vector<size_t>>& merge_map,
         std::vector<size_t>& nulls,
         bool ignore_nulls,
         size_t row_size) {
      std::vector<size_t> newval(row_size);
      auto newvalp = newval.data();
      auto max = std::numeric_limits<size_t>::max();
      if(ignore_nulls) {
        for(size_t i = 0; i < row_size; i++) newvalp[i] = max;
      }
      auto exchanged_size = exchanged.size();
      for(size_t i = 0; i < exchanged_size; i++) {
        auto currentp = exchanged[i].data();
        auto current_size = exchanged[i].size();
        auto merge_mapp = merge_map[i].data();
        if(!ignore_nulls) {
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
          for(size_t j = 0; j < current_size; j++) {
            newvalp[merge_mapp[j]] = currentp[j];
          }
        } else {
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
          for(size_t j = 0; j < current_size; j++) {
            if(currentp[j] != max)
              newvalp[merge_mapp[j]] = currentp[j];
          }
        }
      }
      nulls = vector_find_tmax(newval); 
      return newval;
    }, merge_map, ret->nulls, bignore_nulls, row_sizes);
  ret->val = std::move(newval);
  ret->contain_nulls_check();
  return ret;
}

std::string
typed_dfcolumn<string>::first(bool ignore_nulls) {
#ifndef DONOT_ALLOW_MAX_AS_VALUE
  throw std::runtime_error
    ("define DONOT_ALLOW_MAX_AS_VALUE to use first");
#endif
  auto firsts = val.map(first_helper2<size_t>,broadcast(ignore_nulls)).gather();
  auto firsts_size = firsts.size();
  size_t max = std::numeric_limits<size_t>::max();
  size_t ret = max;
  for(size_t i = 0; i < firsts_size; i++) {
    auto crnt = firsts[firsts_size - 1 - i];
    if(!ignore_nulls) {
      if(crnt.size() != 0) ret = crnt[0];
    } else {
      if(crnt.size() != 0 && crnt[0] != max) ret = crnt[0];
    }
  }
  auto gdic_idx = dic_idx->gather();    
  if(ret == numeric_limits<size_t>::max()) return "NULL";
  else {
    size_t nodemask = (size_t(1) << DFNODESHIFT) - 1;
    auto node = ret >> DFNODESHIFT;
    auto idx = ret & nodemask;
    return gdic_idx[node][idx];
  }
}

std::string
typed_dfcolumn<string>::last(bool ignore_nulls) {
#ifndef DONOT_ALLOW_MAX_AS_VALUE
  throw std::runtime_error
    ("define DONOT_ALLOW_MAX_AS_VALUE to use last");
#endif
  auto lasts = val.map(last_helper2<size_t>,broadcast(ignore_nulls)).gather();
  auto lasts_size = lasts.size();
  size_t max = std::numeric_limits<size_t>::max();
  size_t ret = max;
  for(size_t i = 0; i < lasts_size; i++) {
    auto crnt = lasts[i];
    if(!ignore_nulls) {
      if(crnt.size() != 0) ret = crnt[0];
    } else {
      if(crnt.size() != 0 && crnt[0] != max) ret = crnt[0];
    }
  }
  auto gdic_idx = dic_idx->gather();    
  if(ret == numeric_limits<size_t>::max()) return "NULL";
  else {
    size_t nodemask = (size_t(1) << DFNODESHIFT) - 1;
    auto node = ret >> DFNODESHIFT;
    auto idx = ret & nodemask;
    return gdic_idx[node][idx];
  }
}

size_t typed_dfcolumn<string>::count() {
  size_t size = val.viewas_dvector<size_t>().size();
  if(contain_nulls) {
    size_t nullsize = nulls.template viewas_dvector<size_t>().size();
    return size - nullsize;
  } else return size;
}

vector<size_t> to_contiguous_idx(vector<size_t>& idx, vector<size_t>& sizes) {
  size_t nodemask = (size_t(1) << DFNODESHIFT) - 1;
  int nodesize = get_nodesize();
  vector<size_t> toadd(nodesize);
  size_t* toaddp = toadd.data();
  size_t* sizesp = sizes.data();
  for(size_t i = 1; i < nodesize; i++) {
    toaddp[i] = toaddp[i-1] + sizesp[i-1];
  }
  size_t size = idx.size();
  vector<size_t> ret(size);
  size_t* idxp = idx.data();
  size_t* retp = ret.data();
  size_t max = std::numeric_limits<size_t>::max();
#pragma cdir nodep
#pragma _NEC ivdep
  for(size_t i = 0; i < size; i++) {
    if(idxp[i] != max) {
      int node = idxp[i] >> DFNODESHIFT;
      retp[i] = (idxp[i] & nodemask) + toaddp[node];
    }
    else retp[i] = idxp[i];
  }
  return ret;
}

vector<vector<size_t>> split_by_node(vector<size_t>& idx) {
  size_t nodemask = (size_t(1) << DFNODESHIFT) - 1;
  int nodesize = get_nodesize();
  size_t max = std::numeric_limits<size_t>::max();
  vector<vector<size_t>> split(nodesize);
  vector<size_t> each_size(nodesize);
  size_t* each_sizep = each_size.data();
  size_t size = idx.size();
  size_t* idxp = idx.data();
#pragma cdir nodep
#pragma _NEC ivdep
  for(size_t i = 0; i < size; i++) {
    if(idxp[i] != max) {
      int node = idxp[i] >> DFNODESHIFT;
      each_sizep[node]++;
    }
  }
  for(size_t i = 0; i < nodesize; i++) {
    split[i].resize(each_size[i]);
  }
  vector<size_t*> each_split(nodesize);
  size_t** each_splitp = each_split.data();
  vector<size_t> each_split_current(nodesize);
  size_t* each_split_currentp = each_split_current.data();
  for(size_t i = 0; i < nodesize; i++) each_splitp[i] = split[i].data();
#pragma cdir nodep
#pragma _NEC ivdep
  for(size_t i = 0; i < size; i++) {
    if(idxp[i] != max) {
      int node = idxp[i] >> DFNODESHIFT;
      each_splitp[node][each_split_currentp[node]] = idxp[i] & nodemask;
      each_split_currentp[node]++;
    }
  }
  return split;
}

void sort_idx(vector<size_t>& idx) {
#if defined(_SX) || defined(__ve__)
  std::vector<size_t> dummy(idx.size());
  radix_sort(idx, dummy);
#else
  std::sort(idx.begin(), idx.end());
#endif
}

vector<size_t> create_exist_map(vector<size_t>& exist_idx,
                                vector<size_t>& table_val) {
  size_t selfid = static_cast<size_t>(get_selfid());
  size_t nodeinfo = selfid << DFNODESHIFT;
  size_t* exist_idxp = exist_idx.data();
  size_t size = exist_idx.size();
  size_t tbsize;
  if(selfid == 0) tbsize = size + 1;
  else tbsize = size;
  vector<size_t> table_key(tbsize);
  size_t* table_keyp = table_key.data();
  table_val.resize(tbsize);
  size_t* table_valp = table_val.data();
  for(size_t i = 0; i < size; i++) {
    table_keyp[i] = exist_idxp[i] + nodeinfo;
    table_valp[i] = i + nodeinfo;
  }
  if(selfid == 0) {
    size_t max = std::numeric_limits<size_t>::max();
    table_keyp[size] = max;
    table_valp[size] = max;
  }
  return table_key;
}

vector<size_t> shrink_string_idx(vector<size_t>& val,
                                 vector<size_t>& table_key,
                                 vector<size_t>& table_val) {
  unique_hashtable<size_t, size_t> ht(table_key, table_val);
  return ht.lookup(val);
}


vector<string> shrink_string_dic(vector<string>& dic_idx,
                                 vector<size_t>& exist_idx) {
  size_t size = exist_idx.size();
  vector<string> ret(size);
  for(size_t i = 0; i < size; i++) {
    ret[i] = dic_idx[exist_idx[i]];
  }
  return ret;
}

void typed_dfcolumn<string>::save(const std::string& file) {
  auto split = val.map(split_by_node);
  auto exchanged = alltoall_exchange(split);
  auto exist_idx = exchanged.map(flatten<size_t>).map(get_unique_idx);
  if(exist_idx.viewas_dvector<size_t>().size() !=
     dic_idx->viewas_dvector<string>().size()) {
    exist_idx.mapv(sort_idx);
    auto table_val = make_node_local_allocate<vector<size_t>>();
    auto table_key = exist_idx.map(create_exist_map, table_val);
    auto bcast_table_key =
      broadcast(table_key.viewas_dvector<size_t>().gather());
    auto bcast_table_val =
      broadcast(table_val.viewas_dvector<size_t>().gather());
    auto new_val = val.map(shrink_string_idx, bcast_table_key,
                           bcast_table_val);
    auto new_dic_idx = dic_idx->map(shrink_string_dic,
                                    exist_idx);
    auto dicsizes = new_dic_idx.viewas_dvector<string>().sizes();
    new_val.map(to_contiguous_idx, broadcast(dicsizes)).
      moveto_dvector<size_t>().savebinary(file + "_idx");
    new_dic_idx.viewas_dvector<string>().saveline(file + "_dic");
  } else {
    auto dicsizes = dic_idx->viewas_dvector<string>().sizes();
    val.map(to_contiguous_idx, broadcast(dicsizes)).
      moveto_dvector<size_t>().savebinary(file + "_idx");
    dic_idx->viewas_dvector<string>().saveline(file + "_dic");
  }
  auto dv_nulls = nulls.template viewas_dvector<size_t>();
  auto sizes = val.template viewas_dvector<size_t>().sizes();
  std::vector<size_t> pxsizes(sizes.size());
  for(size_t i = 1; i < pxsizes.size(); i++) {
    pxsizes[i] += pxsizes[i-1] + sizes[i-1];
  }
  auto nl_sizes = make_node_local_scatter(pxsizes);
  dv_nulls.map<size_t>(shift_local_index(), nl_sizes).
    savebinary(file+"_nulls");
}

void typed_dfcolumn<string>::contain_nulls_check() {
  if(nulls.template viewas_dvector<size_t>().size() == 0)
    contain_nulls = false;
  else contain_nulls = true;
}

std::shared_ptr<dfcolumn>
typed_dfcolumn<string>::head(size_t limit) {
  auto ret = std::make_shared<typed_dfcolumn<std::string>>();
  ret->val = val.viewas_dvector<size_t>().head(limit).moveto_node_local();
  if(contain_nulls) {
    ret->nulls = limit_nulls_head(nulls, sizes(), limit);
    ret->contain_nulls_check();
  } else {
    ret->nulls = make_node_local_allocate<std::vector<size_t>>();    
  }
  ret->dic = dic;
  ret->dic_idx = dic_idx;
  return ret;
}

std::shared_ptr<dfcolumn>
typed_dfcolumn<string>::tail(size_t limit) {
  auto ret = std::make_shared<typed_dfcolumn<std::string>>();
  ret->val = val.viewas_dvector<size_t>().tail(limit).moveto_node_local();
  auto new_sizes = ret->val.template viewas_dvector<size_t>().sizes();
  if(contain_nulls) {
    ret->nulls = limit_nulls_tail(nulls, sizes(), new_sizes, limit);
    ret->contain_nulls_check();
  } else {
    ret->nulls = make_node_local_allocate<std::vector<size_t>>();    
  }
  ret->dic = dic;
  ret->dic_idx = dic_idx;
  return ret;
}

void union_columns_string_create_dic
(my_map<std::string,size_t>& newdic,
 std::vector<std::string>& newdic_idx,
 std::vector<my_map<std::string,size_t>*>& dic_colsp,
 std::vector<std::vector<std::string>*>& dic_idx_colsp) {
  auto cols_size = dic_colsp.size();
  size_t selfid = static_cast<size_t>(get_selfid());
  size_t nodeinfo = selfid << DFNODESHIFT;
  for(size_t i = 0; i < cols_size; i++) {
    auto& crnt_dic_idx = *dic_idx_colsp[i];
    auto crnt_dic_idx_size = crnt_dic_idx.size();
    for(size_t j = 0; j < crnt_dic_idx_size; j++) {
      newdic[crnt_dic_idx[j]] = 0;
    }
  }
  size_t i = 0;
  for(auto it = newdic.begin(); it != newdic.end(); ++it, ++i) {
    newdic_idx.push_back(it->first);
    it->second = i + nodeinfo;
  }
}

void union_columns_string_resize_newval
(std::vector<size_t>& newval, std::vector<std::vector<size_t>*>& val_colsp) {
  auto cols_size = val_colsp.size();
  std::vector<size_t> val_sizes(cols_size);
  auto val_colspp = val_colsp.data();
  auto val_sizesp = val_sizes.data();
  for(size_t i = 0; i < cols_size; i++) {
    val_sizesp[i] = val_colspp[i]->size();
  }
  size_t total_val_size = 0;
  for(size_t i = 0; i < cols_size; i++) {
    total_val_size += val_sizesp[i];
  }
  newval.resize(total_val_size);
}

node_local<vector<size_t>>
union_columns_string_prepare(node_local<my_map<std::string, size_t>>& newdic,
                             node_local<my_map<std::string, size_t>>& dic,
                             node_local<std::vector<size_t>>& val) {
  auto nlfrom = make_node_local_allocate<std::vector<size_t>>();
  auto nlto = make_node_local_allocate<std::vector<size_t>>();
  newdic.mapv(create_string_trans_table, dic, nlfrom, nlto);
  auto bcastfrom = broadcast(nlfrom.moveto_dvector<size_t>().gather());
  auto bcastto = broadcast(nlto.moveto_dvector<size_t>().gather());
  return val.map(equal_prepare_helper, bcastfrom, bcastto);
}

void union_columns_string_update_nulls
(std::vector<size_t>& newnulls,
 std::vector<std::vector<size_t>*>& val_colsp,
 std::vector<std::vector<size_t>*>& nulls_colsp) {

  auto cols_size = val_colsp.size();
  std::vector<size_t> val_sizes(cols_size);
  auto val_colspp = val_colsp.data();
  auto val_sizesp = val_sizes.data();
  for(size_t i = 0; i < cols_size; i++) {
    val_sizesp[i] = val_colspp[i]->size();
  }
  std::vector<size_t> nulls_sizes(cols_size);
  auto nulls_colspp = nulls_colsp.data();
  auto nulls_sizesp = nulls_sizes.data();
  for(size_t i = 0; i < cols_size; i++) {
    nulls_sizesp[i] = nulls_colspp[i]->size();
  }
  size_t total_nulls_size = 0;
  for(size_t i = 0; i < cols_size; i++) {
    total_nulls_size += nulls_sizesp[i];
  }
  newnulls.resize(total_nulls_size);
  auto crnt_newnullsp = newnulls.data();
  auto crnt_shift = 0;
  for(size_t i = 0; i < cols_size; i++) {
    auto nulls_size = nulls_sizesp[i];
    auto nullsp = nulls_colspp[i]->data();
    for(size_t j = 0; j < nulls_size; j++) {
      crnt_newnullsp[j] = nullsp[j] + crnt_shift;
    }
    crnt_shift += val_sizesp[i];
    crnt_newnullsp += nulls_size;
  }
}

std::shared_ptr<dfcolumn>
typed_dfcolumn<std::string>::union_columns
(const std::vector<std::shared_ptr<dfcolumn>>& cols) {
  auto cols_size = cols.size();
  if(cols_size == 0) {
    return std::make_shared<typed_dfcolumn<std::string>>
      (val, nulls, dic, dic_idx);
  }
  std::vector<std::shared_ptr<typed_dfcolumn<std::string>>> rights(cols_size);
  for(size_t i = 0; i < cols_size; i++) {
    rights[i] = std::dynamic_pointer_cast<typed_dfcolumn<std::string>>(cols[i]);
    if(!rights[i]) throw std::runtime_error("union_columns: different type");
  }
  auto val_colsp =
    make_node_local_allocate<std::vector<std::vector<size_t>*>>();
  auto nulls_colsp =
    make_node_local_allocate<std::vector<std::vector<size_t>*>>();
  auto dic_colsp =
    make_node_local_allocate<std::vector<my_map<std::string,size_t>*>>();
  auto dic_idx_colsp =
    make_node_local_allocate<std::vector<std::vector<std::string>*>>();
  val.mapv(+[](std::vector<size_t>& val,
               std::vector<std::vector<size_t>*>& val_colsp)
           {val_colsp.push_back(&val);}, val_colsp);
  nulls.mapv(+[](std::vector<size_t>& nulls,
                 std::vector<std::vector<size_t>*>& nulls_colsp)
             {nulls_colsp.push_back(&nulls);}, nulls_colsp);
  dic->viewas_node_local().
    mapv(+[](my_map<std::string,size_t>& dic,
             std::vector<my_map<std::string,size_t>*>& dic_colsp)
         {dic_colsp.push_back(&dic);}, dic_colsp);
  dic_idx->mapv(+[](std::vector<std::string>& dic_idx,
                    std::vector<std::vector<std::string>*>& dic_idx_colsp)
                {dic_idx_colsp.push_back(&dic_idx);}, dic_idx_colsp);
  for(size_t i = 0; i < cols_size; i++) {
    rights[i]->val.mapv(+[](std::vector<size_t>& val,
                            std::vector<std::vector<size_t>*>& val_colsp)
                        {val_colsp.push_back(&val);}, val_colsp);
    rights[i]->nulls.mapv(+[](std::vector<size_t>& nulls,
                              std::vector<std::vector<size_t>*>& nulls_colsp)
                          {nulls_colsp.push_back(&nulls);}, nulls_colsp);
    rights[i]->dic->viewas_node_local().
      mapv(+[](my_map<std::string,size_t>& dic,
               std::vector<my_map<std::string,size_t>*>& dic_colsp)
           {dic_colsp.push_back(&dic);}, dic_colsp);
    rights[i]->dic_idx->
      mapv(+[](std::vector<std::string>& dic_idx,
               std::vector<std::vector<std::string>*>& dic_idx_colsp)
           {dic_idx_colsp.push_back(&dic_idx);}, dic_idx_colsp);
  }
  auto newval = make_node_local_allocate<std::vector<size_t>>();
  auto newnulls = make_node_local_allocate<std::vector<size_t>>();
  auto newdic = make_shared<dunordered_map<std::string,size_t>>
    (make_dunordered_map_allocate<std::string,size_t>());
  auto newdic_idx = make_shared<node_local<vector<std::string>>>
    (make_node_local_allocate<vector<std::string>>());
  auto newdicnl = newdic->viewas_node_local();
  newdicnl.mapv(union_columns_string_create_dic, *newdic_idx,
                dic_colsp, dic_idx_colsp);
  newval.mapv(union_columns_string_resize_newval, val_colsp);
  auto crnt_pos = broadcast(size_t(0));
  {
    auto newdicnl = newdic->viewas_node_local(); // for fail safe
    auto dicnl = dic->viewas_node_local();
    auto crnt_newval = union_columns_string_prepare(newdicnl, dicnl, val);
    newval.mapv
      (+[](std::vector<size_t>& newval, std::vector<size_t>& val, size_t& pos) {
        auto crnt_newvalp = newval.data() + pos;
        auto valp = val.data();
        auto val_size = val.size();
        for(size_t i = 0; i < val_size; i++) {crnt_newvalp[i] = valp[i];}
        pos += val_size;
      }, crnt_newval, crnt_pos);
  }
  for(size_t i = 0; i < cols_size; i++) {
    auto newdicnl = newdic->viewas_node_local(); // for fail safe
    auto dicnl = rights[i]->dic->viewas_node_local();
    auto crnt_newval =
      union_columns_string_prepare(newdicnl, dicnl, rights[i]->val);
    newval.mapv
      (+[](std::vector<size_t>& newval, std::vector<size_t>& val, size_t& pos) {
        auto crnt_newvalp = newval.data() + pos;
        auto valp = val.data();
        auto val_size = val.size();
        for(size_t i = 0; i < val_size; i++) {crnt_newvalp[i] = valp[i];}
        pos += val_size;
      }, crnt_newval, crnt_pos);
  }
  newnulls.mapv(union_columns_string_update_nulls, val_colsp, nulls_colsp);
  return std::make_shared<typed_dfcolumn<std::string>>
    (std::move(newval), std::move(newnulls),
     std::move(newdic), std::move(newdic_idx));
}

// same as equal_prepare; argument is not shared_ptr
node_local<vector<size_t>>
typed_dfcolumn<string>::
equal_prepare_multi_join(typed_dfcolumn<string>& right) {
  auto& rightdic = *(right.dic);
  auto rightdicnl = rightdic.viewas_node_local();
  auto nlfrom = make_node_local_allocate<std::vector<size_t>>();
  auto nlto = make_node_local_allocate<std::vector<size_t>>();
  dic->viewas_node_local().mapv(create_string_trans_table, rightdicnl,
                                nlfrom, nlto);
  auto bcastfrom = broadcast(nlfrom.moveto_dvector<size_t>().gather());
  auto bcastto = broadcast(nlto.moveto_dvector<size_t>().gather());
  return right.val.map(equal_prepare_helper, bcastfrom, bcastto);
}

node_local<std::vector<size_t>>
typed_dfcolumn<string>::calc_hash_base_multi_join
(std::shared_ptr<dfcolumn>& left) {
  auto left2 = std::dynamic_pointer_cast<typed_dfcolumn<string>>(left);
  if(!left2)
    throw std::runtime_error("multi_join: column types are different");
  auto thisval = left2->equal_prepare_multi_join(*this);
  return thisval.map(calc_hash_base_helper<size_t>);
}

void typed_dfcolumn<string>::calc_hash_base_multi_join
(node_local<std::vector<size_t>>& hash_base, int shift,
 std::shared_ptr<dfcolumn>& left) {
  auto left2 = std::dynamic_pointer_cast<typed_dfcolumn<string>>(left);
  if(!left2)
    throw std::runtime_error("multi_join: column types are different");
  auto thisval = left2->equal_prepare_multi_join(*this);
  thisval.mapv(calc_hash_base_helper2<size_t>(shift), hash_base);
}

bool typed_dfcolumn<string>::is_unique() {
  auto& typed_col = dynamic_cast<typed_dfcolumn<string>&>(*this);
  auto key = typed_col.val.viewas_dvector<size_t>().gather();
  auto nulls_count = typed_col.get_nulls().viewas_dvector<size_t>().size();
  return (nulls_count <= 1) && vector_is_unique(key);
}

// for spill-restore

// TODO: spill dic and dic_idx; currently they are shared_ptr,
// so it is difficult to deallocate them
void typed_dfcolumn<string>::spill_to_disk() {
  if(already_spilled_to_disk) {
    val.mapv(+[](std::vector<size_t>& v){
        std::vector<size_t> tmp;
        tmp.swap(v);
      });
    nulls.mapv(+[](std::vector<size_t>& n){
        std::vector<size_t> tmp;
        tmp.swap(n);
      });
  } else {
    val.mapv(+[](std::vector<size_t>& v, std::string& spill_path){
        savebinary_local(v, spill_path+"/val");
        std::vector<size_t> tmp;
        tmp.swap(v);
      }, spill_path);
    nulls.mapv(+[](std::vector<size_t>& n, std::string& spill_path){
        savebinary_local(n, spill_path+"/nulls");
        std::vector<size_t> tmp;
        tmp.swap(n);
      }, spill_path);
    already_spilled_to_disk = true;
  }
  cleared = true;
}

void typed_dfcolumn<string>::restore_from_disk() {
  val.mapv(+[](std::vector<size_t>& v, std::string& spill_path){
      v = loadbinary_local<size_t>(spill_path+"/val");
    }, spill_path);
  nulls.mapv(+[](std::vector<size_t>& n, std::string& spill_path){
      n = loadbinary_local<size_t>(spill_path+"/nulls");
    }, spill_path);
  cleared = false;
}

size_t typed_dfcolumn<string>::calc_spill_size() {
  auto valsize = size();
  auto nullsize = nulls.map(+[](std::vector<size_t>& n){return n.size();}).
    reduce(+[](const size_t l, const size_t r){return l + r;});
  return valsize * sizeof(size_t) + nullsize * sizeof(size_t);
}

std::shared_ptr<dfcolumn>
typed_dfcolumn<std::string>::type_cast(const std::string& to_type,
                                       bool check_bool_like) {
  std::shared_ptr<dfcolumn> ret;
  if(to_type == "boolean") {
    if (check_bool_like) {
      auto dic_dv = dic_idx->viewas_dvector<std::string>();
      auto dicsizes = dic_dv.sizes();
      auto sdic = dic_dv.gather();
      auto tmp_dic = vector_string_to_words(sdic);
      auto idx = val.map(to_contiguous_idx, broadcast(dicsizes));
      auto b_words_to_bool_map = broadcast(words_to_bool(tmp_dic));
      auto newcol = b_words_to_bool_map.map(vector_take<int,int>, idx);
      // TODO: should treat nulls as true?
      ret = std::make_shared<typed_dfcolumn<int>>(std::move(newcol), nulls);
    } else {
      // True if non-empty string
      auto newcol = get_val().map(+[](const std::vector<std::string>& vec) {
                      auto sz = vec.size();
                      std::vector<int> ret(sz); 
                      for(size_t i = 0; i < sz; ++i) ret[i] = vec[i] != "";
                      return ret;
                    }).template moveto_dvector<int>();
      // nulls would also be treated as true, hence no nulls in casted column
      ret = std::make_shared<typed_dfcolumn<int>>(std::move(newcol));
    }
  } else {
    throw std::runtime_error("string column doesn't support casting to: " + to_type);
  }
  return ret;
}

}
