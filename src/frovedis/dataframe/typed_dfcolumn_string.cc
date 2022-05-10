#include "dfcolumn.hpp"

namespace frovedis {

using namespace std;

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

node_local<vector<string>> typed_dfcolumn<string>::get_val() {
  auto bcast_dic_idx = broadcast(dic_idx->gather()); // TODO: expensive!
  return val.map(convert_val, bcast_dic_idx);
}

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

template <>
std::shared_ptr<dfcolumn>
create_null_column<string>(const std::vector<size_t>& sizes) {
  auto nlsizes = make_node_local_scatter(sizes);
  auto val = make_node_local_allocate<std::vector<size_t>>();
  auto nulls = val.map(+[](std::vector<size_t>& val, size_t size) {
      val.resize(size);
      auto valp = val.data();
      auto max = std::numeric_limits<size_t>::max();
      std::vector<size_t> nulls(size);
      auto nullsp = nulls.data();
      for(size_t i = 0; i < size; i++) {
        valp[i] = max;
        nullsp[i] = i;
      }
      return nulls;
    }, nlsizes);
  auto ret = make_shared<typed_dfcolumn<string>>();
  ret->val = std::move(val);
  ret->nulls = std::move(nulls);
  ret->contain_nulls_check();
  ret->dic = make_shared<dunordered_map<std::string, size_t>>
    (make_dunordered_map_allocate<std::string,size_t>());
  ret->dic_idx = make_shared<node_local<std::vector<std::string>>>
    (make_node_local_allocate<std::vector<std::string>>());
  return ret;
}

}
