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
is_like(const std::string& col, const std::string& pattern) {
  return std::make_shared<dfoperator_like>(col, pattern);
}

std::shared_ptr<dfoperator>
is_not_like(const std::string& col, const std::string& pattern) {
  return std::make_shared<dfoperator_not_like>(col, pattern);
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

std::shared_ptr<dfoperator>
not_op(const std::shared_ptr<dfoperator>& op) {
  return std::make_shared<dfoperator_not>(op);
}

// ---------- filter of all kinds of tables ----------

filtered_dftable dftable_base::filter(const std::shared_ptr<dfoperator>& op) {
  use_dfcolumn use(op->columns_to_use(*this));
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
bcast_joined_dftable::filter(const std::shared_ptr<dfoperator>& op) {
  RLOG(DEBUG) << "calling filter after bcast_join" << std::endl;
  return materialize().filter(op);
}

filtered_dftable
star_joined_dftable::filter(const std::shared_ptr<dfoperator>& op) {
  RLOG(DEBUG) << "calling filter after bcast_join" << std::endl;
  return materialize().filter(op);
}

// ---------- for filtered_dftable ----------

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
  auto to_sort_column = column(name);
  use_dfcolumn use(to_sort_column);
  auto sorted_column = column(name)->sort_with_idx(global_idx, idx);
  if(to_sort_column->if_contain_nulls())
    return sorted_dftable(*this, std::move(idx));
  else
    return sorted_dftable(*this, std::move(idx), name,
                          std::move(sorted_column));
}

sorted_dftable filtered_dftable::sort_desc(const std::string& name) {
  auto global_idx = local_to_global_idx(filtered_idx);
  node_local<std::vector<size_t>> idx;
  auto to_sort_column = column(name);
  use_dfcolumn use(to_sort_column);
  auto sorted_column = column(name)->sort_with_idx_desc(global_idx, idx);
  if(to_sort_column->if_contain_nulls())
    return sorted_dftable(*this, std::move(idx));
  else
    return sorted_dftable(*this, std::move(idx), name,
                          std::move(sorted_column));
}

size_t filtered_dftable::num_row() {
  return filtered_idx.viewas_dvector<size_t>().size();
}

std::vector<size_t> filtered_dftable::num_rows() {
  return filtered_idx.viewas_dvector<size_t>().sizes();
}

dftable filtered_dftable::select(const std::vector<std::string>& cols) {
  dftable ret;
  for(size_t i = 0; i < cols.size(); i++) {
    ret.col[cols[i]] = column(cols[i]);
    ret.col[cols[i]]->spill();
  }
  ret.row_size = filtered_idx.viewas_dvector<size_t>().size();
  ret.row_sizes = filtered_idx.viewas_dvector<size_t>().sizes();
  ret.col_order = cols;
  return ret;
}

filtered_dftable
filtered_dftable::filter(const std::shared_ptr<dfoperator>& op) {
  use_dfcolumn use(op->columns_to_use(*this));  
  auto new_filtered_idx = op->filter(*this);
  return filtered_dftable(*this, filtered_idx.map(convert_filtered_idx,
                                                  new_filtered_idx));
}

std::shared_ptr<dfcolumn> filtered_dftable::column(const std::string& name) {
  auto ret = col.find(name);
  if(ret == col.end()) throw std::runtime_error("no such column: " + name);
  else {
    auto c = (*ret).second;
    use_dfcolumn use(c);
    return c->extract(filtered_idx);
  }
}

void filtered_dftable::debug_print() {
  dftable_base::debug_print();
  std::cout << "filtered_idx: " << std::endl;
  for(auto& i: filtered_idx.gather()) {
    for(auto j: i) {
      std::cout << j << " ";
    }
    std::cout << ": ";
  }
  std::cout << std::endl;
}

// ---------- for multi_eq ----------

std::shared_ptr<dfoperator>
multi_eq(const std::vector<std::string>& left,
         const std::vector<std::string>& right) {
  return std::make_shared<dfoperator_multi_eq>(left,right);
}

void filter_idx(std::vector<size_t>& idx,
                const std::vector<size_t>& filter) {
  auto idxp = idx.data();
  auto filter_size = filter.size();
  auto filterp = filter.data();
  std::vector<size_t> r(filter_size);
  auto rp = r.data();
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
  for(size_t i = 0; i < filter_size; i++) {
    rp[i] = idxp[filterp[i]];
  }
  swap(idx, r);
}

std::pair<node_local<std::vector<size_t>>,
          node_local<std::vector<size_t>>>
dfoperator_multi_eq::hash_join
               (dftable_base& left_t, dftable_base& right_t,
                node_local<std::vector<size_t>>& left_idx,
                node_local<std::vector<size_t>>& right_idx) const {
  auto size = leftv.size();
  if(size == 0) {
    throw std::runtime_error("column is not specified for hash_join");    
  } if(size == 1) {
    auto left_column = left_t.raw_column(leftv[0]);
    auto right_column = right_t.raw_column(rightv[0]);
    return left_column->hash_join_eq(right_column, left_idx, right_idx);
  } else {
    std::vector<std::shared_ptr<dfcolumn>> left_pcols(size), right_pcols(size);
    for(size_t i = 0; i < size; i++) {
      left_pcols[i] = left_t.raw_column(leftv[i]);
      right_pcols[i] = right_t.raw_column(rightv[i]);
    }
    // TODO: even in the case of filtered_dftable, calculate all hash values,
    // which can be avoided...
    auto left_hash_base = left_pcols[0]->calc_hash_base();
    // to convert index for string and dic_string
    auto right_hash_base =
      right_pcols[0]->calc_hash_base_multi_join(left_pcols[0]);
    // 52 is fraction of double, size_t might be 32bit...
    int bit_len = std::min(sizeof(size_t) * 8, size_t(52));
    int shift = bit_len / size;
    for(size_t i = 1; i < size; i++) {
      left_pcols[i]->calc_hash_base(left_hash_base, shift);
      right_pcols[i]->
        calc_hash_base_multi_join(right_hash_base, shift, left_pcols[i]);
    }
    auto left_nulls = left_pcols[0]->get_nulls();
    auto right_nulls = right_pcols[0]->get_nulls();
    for(size_t i = 1; i < size; i++) {
      left_nulls.mapv(+[](std::vector<size_t>& nulls,
                          std::vector<size_t>& to_merge) {
                        auto r = set_union(nulls, to_merge);
                        nulls = r;
                      }, left_pcols[i]->get_nulls());
      right_nulls.mapv(+[](std::vector<size_t>& nulls,
                           std::vector<size_t>& to_merge) {
                         auto r = set_union(nulls, to_merge);
                         nulls = r;
                       }, right_pcols[i]->get_nulls());
    }
    shared_ptr<dfcolumn> left_hash =
      std::make_shared<typed_dfcolumn<size_t>>(left_hash_base, left_nulls);
    shared_ptr<dfcolumn> right_hash =
      std::make_shared<typed_dfcolumn<size_t>>(right_hash_base, right_nulls);
    auto idx_pair = left_hash->hash_join_eq(right_hash, left_idx, right_idx);
    auto& left_joined_idx = idx_pair.first;
    auto& right_joined_idx = idx_pair.second;
    auto unique_left_idx = left_joined_idx.map(get_unique_idx);
    auto left_partitioned_idx = partition_global_index_bynode(unique_left_idx);
    auto left_to_store_idx =
      make_to_store_idx(left_partitioned_idx, left_joined_idx);
    auto left_exchanged_idx = exchange_partitioned_index(left_partitioned_idx);
    auto unique_right_idx = right_joined_idx.map(get_unique_idx);
    auto right_partitioned_idx =
      partition_global_index_bynode(unique_right_idx);
    auto right_to_store_idx =
      make_to_store_idx(right_partitioned_idx, right_joined_idx);
    auto right_exchanged_idx =
      exchange_partitioned_index(right_partitioned_idx);
    auto left_extracted = left_pcols[0]->global_extract
      (left_joined_idx, left_to_store_idx, left_exchanged_idx);
    auto right_extracted = right_pcols[0]->global_extract
      (right_joined_idx, right_to_store_idx, right_exchanged_idx);
    auto filtered_idx = left_extracted->filter_eq(right_extracted);
    for(size_t i = 1; i < size; i++) {
      left_extracted = left_pcols[i]->global_extract
        (left_joined_idx, left_to_store_idx, left_exchanged_idx);
      right_extracted = right_pcols[i]->global_extract
        (right_joined_idx, right_to_store_idx, right_exchanged_idx);
      auto filtered_idx_each = left_extracted->filter_eq(right_extracted);
      filtered_idx.mapv(+[](std::vector<size_t>& idx1,
                            std::vector<size_t>& idx2) {
                          auto r = set_intersection(idx1, idx2);
                          swap(idx1, r);
                        }, filtered_idx_each);
    }
    left_joined_idx.mapv(filter_idx, filtered_idx);
    right_joined_idx.mapv(filter_idx, filtered_idx);
    return idx_pair;
  }
}

std::pair<node_local<std::vector<size_t>>,
          node_local<std::vector<size_t>>>
dfoperator_multi_eq::bcast_join
               (dftable_base& left_t, dftable_base& right_t,
                node_local<std::vector<size_t>>& left_idx,
                node_local<std::vector<size_t>>& right_idx) const {
  auto size = leftv.size();
  if(size == 0) {
    throw std::runtime_error("column is not specified for bcast_join");    
  } if(size == 1) {
    auto left_column = left_t.raw_column(leftv[0]);
    auto right_column = right_t.raw_column(rightv[0]);
    return left_column->bcast_join_eq(right_column, left_idx, right_idx);
  } else {
    std::vector<std::shared_ptr<dfcolumn>> left_pcols(size), right_pcols(size);
    for(size_t i = 0; i < size; i++) {
      left_pcols[i] = left_t.raw_column(leftv[i]);
      right_pcols[i] = right_t.raw_column(rightv[i]);
    }
    // TODO: even in the case of filtered_dftable, calculate all hash values,
    // which can be avoided...
    auto left_hash_base = left_pcols[0]->calc_hash_base();
    // to convert index for string and dic_string
    auto right_hash_base =
      right_pcols[0]->calc_hash_base_multi_join(left_pcols[0]);
    // 52 is fraction of double, size_t might be 32bit...
    int bit_len = std::min(sizeof(size_t) * 8, size_t(52));
    int shift = bit_len / size;
    for(size_t i = 1; i < size; i++) {
      left_pcols[i]->calc_hash_base(left_hash_base, shift);
      right_pcols[i]->
        calc_hash_base_multi_join(right_hash_base, shift, left_pcols[i]);
    }
    auto left_nulls = left_pcols[0]->get_nulls();
    auto right_nulls = right_pcols[0]->get_nulls();
    for(size_t i = 1; i < size; i++) {
      left_nulls.mapv(+[](std::vector<size_t>& nulls,
                          std::vector<size_t>& to_merge) {
                        auto r = set_union(nulls, to_merge);
                        nulls = r;
                      }, left_pcols[i]->get_nulls());
      right_nulls.mapv(+[](std::vector<size_t>& nulls,
                           std::vector<size_t>& to_merge) {
                         auto r = set_union(nulls, to_merge);
                         nulls = r;
                       }, right_pcols[i]->get_nulls());
    }
    shared_ptr<dfcolumn> left_hash =
      std::make_shared<typed_dfcolumn<size_t>>(left_hash_base, left_nulls);
    shared_ptr<dfcolumn> right_hash =
      std::make_shared<typed_dfcolumn<size_t>>(right_hash_base, right_nulls);
    auto idx_pair = left_hash->bcast_join_eq(right_hash, left_idx, right_idx);
    auto& left_joined_idx = idx_pair.first;
    auto& right_joined_idx = idx_pair.second;
    auto unique_right_idx = right_joined_idx.map(get_unique_idx);
    auto right_partitioned_idx =
      partition_global_index_bynode(unique_right_idx);
    auto right_to_store_idx =
      make_to_store_idx(right_partitioned_idx, right_joined_idx);
    auto right_exchanged_idx =
      exchange_partitioned_index(right_partitioned_idx);
    auto left_extracted = left_pcols[0]->extract(left_joined_idx);
    auto right_extracted = right_pcols[0]->global_extract
      (right_joined_idx, right_to_store_idx, right_exchanged_idx);
    auto filtered_idx = left_extracted->filter_eq(right_extracted);
    for(size_t i = 1; i < size; i++) {
      left_extracted = left_pcols[i]->extract(left_joined_idx);
      right_extracted = right_pcols[i]->global_extract
        (right_joined_idx, right_to_store_idx, right_exchanged_idx);
      auto filtered_idx_each = left_extracted->filter_eq(right_extracted);
      filtered_idx.mapv(+[](std::vector<size_t>& idx1,
                            std::vector<size_t>& idx2) {
                          auto r = set_intersection(idx1, idx2);
                          swap(idx1, r);
                        }, filtered_idx_each);
    }
    left_joined_idx.mapv(filter_idx, filtered_idx);
    right_joined_idx.mapv(filter_idx, filtered_idx);
    return idx_pair;
  }
}

// ---------- for cross ----------

void make_cross_idx(const std::vector<size_t>& left_idx,
                    const std::vector<size_t>& right_idx,
                    std::vector<size_t>& left_idx_ret,
                    std::vector<size_t>& right_idx_ret) {
  auto left_idx_size = left_idx.size();
  auto right_idx_size = right_idx.size();
  auto total_size = left_idx_size * right_idx_size;
  left_idx_ret.resize(total_size);
  right_idx_ret.resize(total_size);
  auto left_idx_retp = left_idx_ret.data();
  auto right_idx_retp = right_idx_ret.data();
  auto left_idxp = left_idx.data();
  auto right_idxp = right_idx.data();
  for(size_t i = 0; i < left_idx_size; i++) {
    for(size_t j = 0; j < right_idx_size; j++) {
      right_idx_retp[i * right_idx_size + j] = right_idxp[j];
      left_idx_retp[i * right_idx_size + j] = left_idxp[i];
    }
  }
}

std::pair<node_local<std::vector<size_t>>,
          node_local<std::vector<size_t>>>
dfoperator_cross::bcast_join
               (dftable_base& left_t, dftable_base& right_t,
                node_local<std::vector<size_t>>& left_idx,
                node_local<std::vector<size_t>>& right_idx) const {
  auto right_global_idx = local_to_global_idx(right_idx);
  auto right_global_idx_bcast =
    broadcast(right_global_idx.template viewas_dvector<size_t>().gather());
  auto left_idx_ret = make_node_local_allocate<std::vector<size_t>>();
  auto right_idx_ret = make_node_local_allocate<std::vector<size_t>>();
  left_idx.mapv(make_cross_idx, right_global_idx_bcast, left_idx_ret,
                right_idx_ret);
  return std::make_pair(std::move(left_idx_ret), std::move(right_idx_ret));
}

std::shared_ptr<dfoperator> cross() {
  return std::make_shared<dfoperator_cross>();
}

// ---------- for dfoperator_and bcast_join ----------

void flatten_and_op_helper(const shared_ptr<dfoperator>& op,
                           std::vector<shared_ptr<dfoperator>>& ret) {
  auto and_op_cand = dynamic_pointer_cast<dfoperator_and>(op);
  if(and_op_cand) {
    flatten_and_op_helper(and_op_cand->left, ret);
    flatten_and_op_helper(and_op_cand->right, ret);
  } else {
    if(dynamic_pointer_cast<dfoperator_or>(op)) {
      throw std::runtime_error("only and_op can be used for bcast_join");
    }
    ret.push_back(op);
  }
}

std::vector<shared_ptr<dfoperator>>
flatten_and_op(const dfoperator_and& op) {
  std::vector<shared_ptr<dfoperator>> ret;
  flatten_and_op_helper(op.left, ret);
  flatten_and_op_helper(op.right, ret);
  return ret;
}

std::pair<node_local<std::vector<size_t>>,
          node_local<std::vector<size_t>>>
dfoperator_and::bcast_join(dftable_base& left, dftable_base& right,
                           node_local<std::vector<size_t>>& left_idx,
                           node_local<std::vector<size_t>>& right_idx) const {

  auto flattend_op = flatten_and_op(*this);
  std::vector<shared_ptr<dfoperator>> filter_op;
  std::pair<node_local<std::vector<size_t>>,
            node_local<std::vector<size_t>>> idx_pair;
  if(dynamic_pointer_cast<dfoperator_eq>(flattend_op[0])) {
    auto multi_eq_op = dfoperator_multi_eq({}, {});
    size_t i = 0;
    for(; i < flattend_op.size(); i++) {
      if(auto eq_op = dynamic_pointer_cast<dfoperator_eq>(flattend_op[i])) {
        multi_eq_op.leftv.push_back(eq_op->left);
        multi_eq_op.rightv.push_back(eq_op->right);
      } else break;
    }
    for(; i < flattend_op.size(); i++) {
      filter_op.push_back(flattend_op[i]);
    }
    idx_pair = multi_eq_op.bcast_join(left, right, left_idx, right_idx);
  } else {
    idx_pair = flattend_op[0]->bcast_join(left, right, left_idx, right_idx);
    for(size_t i = 1; i < flattend_op.size(); i++) {
      filter_op.push_back(flattend_op[i]);
    }
  }
  bcast_joined_dftable joined_table(left, right,
                                    std::move(idx_pair.first),
                                    std::move(idx_pair.second));
  for(size_t i = 0; i < filter_op.size(); i++) {
    joined_table.inplace_filter_pre(filter_op[i]);
    if(i != filter_op.size() - 1) {
      joined_table.update_to_store_idx_and_exchanged_idx();
    }
  }
  idx_pair.first = std::move(joined_table.get_left_idx());
  idx_pair.second = std::move(joined_table.get_right_idx());
  return idx_pair;
}

filtered_dftable& filtered_dftable::drop(const std::string& name) {
  col.erase(name);
  col_order.erase(std::remove(col_order.begin(), col_order.end(), name),
                  col_order.end());
  return *this;
}

dftable_base* filtered_dftable::drop_cols(const std::vector<std::string>& cols) {
  for(auto& c: cols) drop(c);
  return this;
}


}
