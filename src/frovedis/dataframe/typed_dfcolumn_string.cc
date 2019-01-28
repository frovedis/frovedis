#include "dfcolumn_impl.hpp"
#include <regex>

namespace frovedis {

using namespace std;

template class typed_dfcolumn<std::string>;

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

void typed_dfcolumn<string>::init(dvector<string>& dv) {
  nulls = make_node_local_allocate<vector<size_t>>();
  auto nl = dv.viewas_node_local();
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
}

vector<size_t> extract_idx(vector<pair<string,size_t>>& v) {
  size_t size = v.size();
  vector<size_t> ret(size);
  for(size_t i = 0; i < size; i++) ret[i] = v[i].second;
  return ret;
}

vector<size_t> convert_sorted_idx(vector<size_t>& val,
                                  vector<size_t>& idx_dic) {
  size_t dicsize = idx_dic.size();
  vector<size_t> newidx(dicsize);
  size_t* newidxp = &newidx[0];
  for(size_t i = 0; i < dicsize; i++) newidxp[i] = i;
  auto ht = unique_hashtable<size_t, size_t>(idx_dic, newidx);
  return ht.lookup(val);
}

typed_dfcolumn<size_t>
typed_dfcolumn<string>::sort_prepare() {
  typed_dfcolumn<size_t> rescol;
  auto with_idx = dic_idx->map(append_idx);
  auto with_idx_dv = with_idx.moveto_dvector<pair<string,size_t>>();
  auto with_idx_sorted = with_idx_dv.sort().moveto_node_local();
  auto idx_dic = broadcast(with_idx_sorted.map(extract_idx).
                           viewas_dvector<size_t>().gather());
  rescol.val = val.map(convert_sorted_idx, idx_dic);
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
  return tmpcol.sort_with_idx_desc(idx,res_idx);
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
  auto filtered_idx = val.map(filter_eq_helper<size_t>, rightval);
  if(contain_nulls)
    return filtered_idx.map(set_difference<size_t>, nulls);
  else return filtered_idx;
}

node_local<std::vector<size_t>>
typed_dfcolumn<string>::filter_neq(std::shared_ptr<dfcolumn>& right) {
  auto right2 = std::dynamic_pointer_cast<typed_dfcolumn<string>>(right);
  if(!right2) throw std::runtime_error("filter_eq: column types are different");
  auto rightval = equal_prepare(right2);
  auto filtered_idx = val.map(filter_neq_helper<size_t>, rightval);
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
typed_dfcolumn<string>::filter_eq_immed(const std::string& right) {
  bool found;
  size_t right_val = dic->get(right, found);
  if(found) {
    auto filtered_idx = val.map(filter_eq_immed_helper<size_t>,
                                broadcast(right_val));
    if(contain_nulls)
      return filtered_idx.map(set_difference<size_t>, nulls);
    else return filtered_idx;
  } else {
    return make_node_local_allocate<std::vector<size_t>>();
  }
}

node_local<std::vector<size_t>>
typed_dfcolumn<string>::filter_neq_immed(const std::string& right) {
  bool found;
  size_t right_val = dic->get(right, found);
  if(found) {
    auto filtered_idx = val.map(filter_neq_immed_helper<size_t>,
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
#pragma cdir nodep
#pragma _NEC ivdep
      for(size_t i = 0 ; i < size; i++) {
        retp[i] = valp[idxp[i]];
      }
      return ret;
    }, to_store_idx);
  if(contain_nulls) {
    auto exnulls = nulls.map(global_extract_null_helper, exchanged_idx);
    auto exchanged_back_nulls = alltoall_exchange(exnulls);
    auto null_exists = make_node_local_allocate<int>();
    auto nullhashes = exchanged_back_nulls.map(create_null_hash_from_partition,
                                               null_exists);
    ret->nulls = nullhashes.map(global_extract_null_helper2, global_idx,
                                null_exists);
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
  return count_impl(nulls, local_grouped_idx, local_idx_split,
                    hash_divide, merge_map, row_sizes);
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
    ret->contain_nulls = true;
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
    ret->contain_nulls = true;
  } else {
    ret->nulls = make_node_local_allocate<std::vector<size_t>>();    
    ret->contain_nulls = true;
  }
  ret->dic = dic;
  ret->dic_idx = dic_idx;
  return ret;
}

}
