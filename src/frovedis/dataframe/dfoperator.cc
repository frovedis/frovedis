#include "dfoperator.hpp"

namespace frovedis {

using namespace std;

std::shared_ptr<dfoperator>
eq(const std::string& left, const std::string& right) {
  return std::make_shared<dfoperator_eq>(left,right);
}

std::shared_ptr<dfoperator>
neq(const std::string& left, const std::string& right) {
  return std::make_shared<dfoperator_neq>(left,right);
}

std::shared_ptr<dfoperator>
lt(const std::string& left, const std::string& right) {
  return std::make_shared<dfoperator_lt>(left,right);
}

std::shared_ptr<dfoperator>
le(const std::string& left, const std::string& right) {
  return std::make_shared<dfoperator_le>(left,right);
}

std::shared_ptr<dfoperator>
gt(const std::string& left, const std::string& right) {
  return std::make_shared<dfoperator_gt>(left,right);
}

std::shared_ptr<dfoperator>
ge(const std::string& left, const std::string& right) {
  return std::make_shared<dfoperator_ge>(left,right);
}

std::shared_ptr<dfoperator>
is_null(const std::string& col) {
  return std::make_shared<dfoperator_is_null>(col);
}

std::shared_ptr<dfoperator>
is_not_null(const std::string& col) {
  return std::make_shared<dfoperator_is_not_null>(col);
}

std::shared_ptr<dfoperator>
is_regex(const std::string& col, const std::string& pattern) {
  return std::make_shared<dfoperator_regex>(col, pattern);
}

std::shared_ptr<dfoperator>
is_not_regex(const std::string& col, const std::string& pattern) {
  return std::make_shared<dfoperator_not_regex>(col, pattern);
}

std::shared_ptr<dfoperator>
and_op(const std::shared_ptr<dfoperator>& left,
       const std::shared_ptr<dfoperator>& right) {
  return std::make_shared<dfoperator_and>(left,right);
}

std::shared_ptr<dfoperator>
or_op(const std::shared_ptr<dfoperator>& left,
      const std::shared_ptr<dfoperator>& right) {
  return std::make_shared<dfoperator_or>(left,right);
}

filtered_dftable dftable::filter(const std::shared_ptr<dfoperator>& op) {
  return filtered_dftable(*this, op->filter(*this));
}

filtered_dftable
sorted_dftable::filter(const std::shared_ptr<dfoperator>& op) {
    RLOG(DEBUG) << "calling filter after sort" << std::endl;
    return materialize().filter(op);
}

filtered_dftable
hash_joined_dftable::filter(const std::shared_ptr<dfoperator>& op) {
    RLOG(DEBUG) << "calling filter after hash_join" << std::endl;
    return materialize().filter(op);
}

filtered_dftable
grouped_dftable::filter(const std::shared_ptr<dfoperator>& op) {
  throw std::runtime_error("grouped_dftable cannot be filtered");
}

filtered_dftable
bcast_joined_dftable::filter(const std::shared_ptr<dfoperator>& op) {
  RLOG(DEBUG) << "calling filter after bcast_join" << std::endl;
  return materialize().filter(op);
}

filtered_dftable
star_joined_dftable::filter(const std::shared_ptr<dfoperator>& op) {
  RLOG(DEBUG) << "calling filter after bcast_join" << std::endl;
  return materialize().filter(op);
}


std::vector<size_t>
convert_filtered_idx(std::vector<size_t>& org_idx,
                     std::vector<size_t>& created_idx) {
  vector<size_t> ret(created_idx.size());
  size_t* orgp = &org_idx[0];
  size_t* createdp = &created_idx[0];
  size_t* retp = &ret[0];
#pragma cdir nodep
#pragma _NEC ivdep
  for(size_t i = 0; i < ret.size(); i++) {
    retp[i] = orgp[createdp[i]];
  }
   return ret;
}

// in dfcolumn.cc
node_local<std::vector<size_t>>
local_to_global_idx(node_local<std::vector<size_t>>& local_idx);

sorted_dftable filtered_dftable::sort(const std::string& name) {
  auto global_idx = local_to_global_idx(filtered_idx);
  node_local<std::vector<size_t>> idx;
  auto sorted_column = column(name)->sort_with_idx(global_idx, idx);
  return sorted_dftable(*this, std::move(idx), name, std::move(sorted_column));
}

sorted_dftable filtered_dftable::sort_desc(const std::string& name) {
  auto global_idx = local_to_global_idx(filtered_idx);
  node_local<std::vector<size_t>> idx;
  auto sorted_column = column(name)->sort_with_idx_desc(global_idx, idx);
  return sorted_dftable(*this, std::move(idx), name, std::move(sorted_column));
}

size_t filtered_dftable::num_row() {
  return filtered_idx.viewas_dvector<size_t>().size();
}

dftable filtered_dftable::select(const std::vector<std::string>& cols) {
  dftable ret;
  for(size_t i = 0; i < cols.size(); i++) {
    ret.col[cols[i]] = column(cols[i]);
  }
  ret.row_size = filtered_idx.viewas_dvector<size_t>().size();
  ret.col_order = cols;
  return ret;
}

filtered_dftable
filtered_dftable::filter(const std::shared_ptr<dfoperator>& op) {
  auto new_filtered_idx = op->filter(*this);
  return filtered_dftable(*this, filtered_idx.map(convert_filtered_idx,
                                                  new_filtered_idx));
}

std::shared_ptr<dfcolumn> filtered_dftable::column(const std::string& name) {
  auto ret = col.find(name);
  if(ret == col.end()) throw std::runtime_error("no such column: " + name);
  else return (*ret).second->extract(filtered_idx);
}

std::shared_ptr<dfcolumn> filtered_dftable::raw_column(const std::string& name) {
  return dftable::column(name);
}

void filtered_dftable::debug_print() {
  dftable::debug_print();
  std::cout << "filtered_idx: " << std::endl;
  for(auto& i: filtered_idx.gather()) {
    for(auto j: i) {
      std::cout << j << " ";
    }
    std::cout << ": ";
  }
  std::cout << std::endl;
}

}
