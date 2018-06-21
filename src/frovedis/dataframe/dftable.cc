#include "dftable.hpp"
#include "dfoperator.hpp"
#include "dftable_to_string.hpp"

namespace frovedis {

dftable& dftable::append_column(const std::string& name,
                                const std::shared_ptr<dfcolumn>& c) {
  check_appendable();
  if(col.size() == 0) row_size = c->size();
  else if(c->size() != row_size)
    throw std::runtime_error("different size of columns");
  col.insert(std::make_pair(name, c));
  col_order.push_back(name);
  return *this;
}

std::vector<std::string> dftable::columns() const {
  return col_order;
}

std::vector<std::pair<std::string, std::string>>
dftable::dtypes() {
  std::vector<std::pair<std::string,std::string>> ret;
  auto cols = columns();
  for(size_t i = 0; i < cols.size(); i++) {
    ret.push_back(std::make_pair(cols[i],
                                 column(cols[i])->dtype()));
  }
  return ret;
}

void dftable::drop(const std::string& name) {
  col.erase(name);
  col_order.erase(std::remove(col_order.begin(), col_order.end(), name),
                  col_order.end());

}

void dftable::rename(const std::string& name, const std::string& name2) {
  auto tmp = column(name);
  if(col.find(name2) != col.end())
    throw std::runtime_error("column already exists: " + name2);
  col.erase(name);
  col[name2] = tmp;
  for(size_t i = 0; i < col_order.size(); i++) {
    if(col_order[i] == name) {
      col_order[i] = name2;
      break;
    }
  }
}

dftable dftable::select(const std::vector<std::string>& cols) {
  dftable ret;
  ret.row_size = row_size;
  for(size_t i = 0; i < cols.size(); i++) {
    ret.col[cols[i]] = column(cols[i]);
  }
  ret.col_order = cols;
  return ret;
}

/* // removed this because of initlializer list ambiguity
dftable dftable::select(const std::string& col) {
  std::vector<std::string> cols = {col};
  return select(cols);
}
*/

dftable dftable::materialize(){return select(columns());}

size_t dftable::count(const std::string& name) {
  return column(name)->count();
}

double dftable::avg(const std::string& name) {
  return column(name)->avg();
}

void dftable::show() {
  auto table_string = dftable_to_string(*this).gather();
  auto cols = columns();
  for(size_t i = 0; i < cols.size()-1; i++) {
    std::cout << cols[i] << "\t";
  }
  std::cout << cols[cols.size()-1] << std::endl;
  for(size_t i = 0; i < table_string.size(); i++) {
    for(size_t j = 0; j < table_string[i].size()-1; j++) {
      std::cout << table_string[i][j] << "\t";
    }
    std::cout << table_string[i][table_string[i].size()-1] << std::endl;
  }
}

void dftable::show(size_t limit) {
  auto cols = columns();
  dftable limit_table;
  for(size_t i = 0; i < cols.size(); i++) {
    limit_table.append_column(cols[i],
                              this->column(cols[i])->head(limit));
  }
  limit_table.show();
}

std::shared_ptr<dfcolumn> dftable::column(const std::string& name) {
  auto ret = col.find(name);
  if(ret == col.end()) throw std::runtime_error("no such column: " + name);
  else return ret->second;
}
 
std::shared_ptr<dfcolumn> dftable::raw_column(const std::string& name) {
  return column(name);
}

node_local<std::vector<size_t>> dftable::get_local_index() {
  if(col.size() == 0)
    throw std::runtime_error("get_local_index(): no columns");
  else return col.begin()->second->get_local_index();
}

void dftable::debug_print() {
  for(auto& cs: dftable::columns()) {
    std::cout << "column: " << cs << std::endl;
    dftable::column(cs)->debug_print();
  }
  std::cout << "row_size: " << row_size << std::endl;
}

void dftable::save(const std::string& dir) {
  struct stat sb;
  if(stat(dir.c_str(), &sb) != 0) { // no file/directory
    mode_t mode = S_IRWXU | S_IRWXG | S_IRWXO; // man 2 stat
    if(mkdir(dir.c_str(), mode) != 0) {
      perror("mkdir failed:");
      throw std::runtime_error("mkdir failed");
    }
  } else if(!S_ISDIR(sb.st_mode)) {
    throw std::runtime_error(dir + " is not a directory");
  }
  std::string colfile = dir + "/columns";
  std::string typefile = dir + "/types";
  std::ofstream colstr;
  std::ofstream typestr;
  colstr.exceptions(std::ofstream::failbit | std::ofstream::badbit);
  colstr.open(colfile.c_str());
  typestr.exceptions(std::ofstream::failbit | std::ofstream::badbit);
  typestr.open(typefile.c_str());
  auto dt = dtypes();
  for(size_t i = 0; i < dt.size(); i++) {
    colstr << dt[i].first << std::endl;
    typestr << dt[i].second << std::endl;
    column(dt[i].first)->save(dir + "/" + dt[i].first);
  }
}

void dftable::load(const std::string& input) {
  std::string colfile = input + "/columns";
  std::string typefile = input + "/types";
  std::ifstream colstr;
  std::ifstream typestr;
  colstr.open(colfile.c_str());
  typestr.open(typefile.c_str());
  std::vector<std::string> cols;
  std::vector<std::string> types;
  std::string tmp;
  while(std::getline(colstr, tmp)) {
    cols.push_back(tmp);
  }
  while(std::getline(typestr, tmp)) {
    types.push_back(tmp);
  }
  for(size_t i = 0; i < cols.size(); i++) {
    std::string valfile = input + "/" + cols[i];
    std::string nullsfile = input + "/" + cols[i]+ "_nulls";
    if(types[i] == "int") {
      auto vec = make_dvector_loadbinary<int>(valfile);
      auto sizes = vec.sizes();
      std::vector<size_t> pxsizes(sizes.size());
      for(size_t i = 1; i < pxsizes.size(); i++) {
        pxsizes[i] += pxsizes[i-1] + sizes[i-1];
      }
      auto nl_sizes = make_node_local_scatter(pxsizes);
      append_column(cols[i], std::move(vec));
      std::dynamic_pointer_cast<typed_dfcolumn<int>>(column(cols[i]))->
        nulls = make_dvector_loadbinary<size_t>(nullsfile).
        map<size_t>(shiftback_local_index(), nl_sizes).moveto_node_local();
      column(cols[i])->contain_nulls_check();
    } else if(types[i] == "unsigned int") {
      auto vec = make_dvector_loadbinary<unsigned int>(valfile);
      auto sizes = vec.sizes();
      std::vector<size_t> pxsizes(sizes.size());
      for(size_t i = 1; i < pxsizes.size(); i++) {
        pxsizes[i] += pxsizes[i-1] + sizes[i-1];
      }
      auto nl_sizes = make_node_local_scatter(pxsizes);
      append_column(cols[i], std::move(vec));
      std::dynamic_pointer_cast<typed_dfcolumn<unsigned int>>
        (column(cols[i]))->
        nulls = make_dvector_loadbinary<size_t>(nullsfile).
        map<size_t>(shiftback_local_index(), nl_sizes).moveto_node_local();
      column(cols[i])->contain_nulls_check();
    } else if(types[i] == "long") {
      auto vec = make_dvector_loadbinary<long>(valfile);
      auto sizes = vec.sizes();
      std::vector<size_t> pxsizes(sizes.size());
      for(size_t i = 1; i < pxsizes.size(); i++) {
        pxsizes[i] += pxsizes[i-1] + sizes[i-1];
      }
      auto nl_sizes = make_node_local_scatter(pxsizes);
      append_column(cols[i], std::move(vec));
      std::dynamic_pointer_cast<typed_dfcolumn<long>>(column(cols[i]))->
        nulls = make_dvector_loadbinary<size_t>(nullsfile).
        map<size_t>(shiftback_local_index(), nl_sizes).moveto_node_local();
      column(cols[i])->contain_nulls_check();
    } else if(types[i] == "unsigned long") {
      auto vec = make_dvector_loadbinary<unsigned long>(valfile);
      auto sizes = vec.sizes();
      std::vector<size_t> pxsizes(sizes.size());
      for(size_t i = 1; i < pxsizes.size(); i++) {
        pxsizes[i] += pxsizes[i-1] + sizes[i-1];
      }
      auto nl_sizes = make_node_local_scatter(pxsizes);
      append_column(cols[i], std::move(vec));
      std::dynamic_pointer_cast<typed_dfcolumn<unsigned long>>
        (column(cols[i]))->
        nulls = make_dvector_loadbinary<size_t>(nullsfile).
        map<size_t>(shiftback_local_index(), nl_sizes).moveto_node_local();
      column(cols[i])->contain_nulls_check();
    } else if(types[i] == "float") {
      auto vec = make_dvector_loadbinary<float>(valfile);
      auto sizes = vec.sizes();
      std::vector<size_t> pxsizes(sizes.size());
      for(size_t i = 1; i < pxsizes.size(); i++) {
        pxsizes[i] += pxsizes[i-1] + sizes[i-1];
      }
      auto nl_sizes = make_node_local_scatter(pxsizes);
      append_column(cols[i], std::move(vec));
      std::dynamic_pointer_cast<typed_dfcolumn<float>>(column(cols[i]))->
        nulls = make_dvector_loadbinary<size_t>(nullsfile).
        map<size_t>(shiftback_local_index(), nl_sizes).moveto_node_local();
      column(cols[i])->contain_nulls_check();
    } else if(types[i] == "double") {
      auto vec = make_dvector_loadbinary<double>(valfile);
      auto sizes = vec.sizes();
      std::vector<size_t> pxsizes(sizes.size());
      for(size_t i = 1; i < pxsizes.size(); i++) {
        pxsizes[i] += pxsizes[i-1] + sizes[i-1];
      }
      auto nl_sizes = make_node_local_scatter(pxsizes);
      append_column(cols[i], std::move(vec));
      std::dynamic_pointer_cast<typed_dfcolumn<double>>(column(cols[i]))->
        nulls = make_dvector_loadbinary<size_t>(nullsfile).
        map<size_t>(shiftback_local_index(), nl_sizes).moveto_node_local();
      column(cols[i])->contain_nulls_check();
    } else if(types[i] == "string") {
      auto vec = make_dvector_loadline(valfile);
      auto sizes = vec.sizes();
      std::vector<size_t> pxsizes(sizes.size());
      for(size_t i = 1; i < pxsizes.size(); i++) {
        pxsizes[i] += pxsizes[i-1] + sizes[i-1];
      }
      auto nl_sizes = make_node_local_scatter(pxsizes);
      append_column(cols[i], std::move(vec));
      std::dynamic_pointer_cast<typed_dfcolumn<std::string>>
        (column(cols[i]))->
        nulls = make_dvector_loadbinary<size_t>(nullsfile).
        map<size_t>(shiftback_local_index(), nl_sizes).moveto_node_local();
      column(cols[i])->contain_nulls_check();
    }
  }
}

dftable make_dftable_load(const std::string& input) {
  dftable t;
  t.load(input);
  return t;
}

struct dftable_concat_string {
  dftable_concat_string(){}
  dftable_concat_string(std::string sep) : sep(sep) {}
  std::string operator()(std::vector<std::string>& vs) {
    std::string ret;
    for(size_t i = 0; i < vs.size() - 1; i++) {
      ret.append(vs[i]);
      ret.append(sep);
    }
    ret.append(vs[vs.size() - 1]);
    return ret;
  }
  std::string sep;
  SERIALIZE(sep)
};

std::vector<std::pair<std::string, std::string>>
dftable::savetext(const std::string& file, const std::string& sep) {
  auto dv = dftable_to_string(*this);
  auto tosave = dv.map(dftable_concat_string(sep));
  tosave.saveline(file);
  return dtypes();
}

std::vector<std::pair<std::string, std::string>>
dftable::savetext(const std::string& file) {
  return savetext(file, ",");
}

sorted_dftable dftable::sort(const std::string& name) {
  node_local<std::vector<size_t>> idx;
  auto sorted_column = column(name)->sort(idx);
  return sorted_dftable(*this, std::move(idx), name, std::move(sorted_column));
}

sorted_dftable dftable::sort_desc(const std::string& name) {
  node_local<std::vector<size_t>> idx;
  auto sorted_column = column(name)->sort_desc(idx);
  return sorted_dftable(*this, std::move(idx), name, std::move(sorted_column));
}

void join_column_name_check(const std::map<std::string,
                            std::shared_ptr<dfcolumn>>& col1,
                            const std::map<std::string,
                            std::shared_ptr<dfcolumn>>& col2) {
  for(auto i = col2.cbegin(); i != col2.end(); ++i) {
    if(col1.find(i->first) != col1.end())
      throw std::runtime_error("joining tables have same column name: " +
        i->first);
  }
}

hash_joined_dftable dftable::hash_join(dftable& right_,
                                       const std::shared_ptr<dfoperator>& op) {
  if(right_.is_right_joinable()) {
    auto& right = right_;
    join_column_name_check(col, right.col);
    auto left_idx = get_local_index();
    auto right_idx = right.get_local_index();
    auto idxs = op->hash_join(*this, right, left_idx, right_idx);
    return hash_joined_dftable(*this, right, std::move(idxs.first),
                               std::move(idxs.second));
  }
  else {
    auto right = right_.materialize();
    join_column_name_check(col, right.col);
    auto left_idx = get_local_index();
    auto right_idx = right.get_local_index();
    auto idxs = op->hash_join(*this, right, left_idx, right_idx);
    return hash_joined_dftable(*this, right, std::move(idxs.first),
                               std::move(idxs.second));
  }
}

hash_joined_dftable 
dftable::outer_hash_join(dftable& right_,
                         const std::shared_ptr<dfoperator>& op) {
  if(right_.is_right_joinable()) {
    auto& right = right_;
    join_column_name_check(col, right.col);
    auto left_idx = get_local_index();
    auto right_idx = right.get_local_index();
    node_local<std::vector<size_t>> left_idx_;
    node_local<std::vector<size_t>> right_idx_;
    node_local<std::vector<size_t>> right_nulls_;
    std::tie(left_idx_, right_idx_, right_nulls_)
      = op->outer_hash_join(*this, right, left_idx, right_idx);
    return hash_joined_dftable(*this, right, std::move(left_idx_),
                               std::move(right_idx_), std::move(right_nulls_));
  } else {
    auto right = right_.materialize();
    join_column_name_check(col, right.col);
    auto left_idx = get_local_index();
    auto right_idx = right.get_local_index();
    node_local<std::vector<size_t>> left_idx_;
    node_local<std::vector<size_t>> right_idx_;
    node_local<std::vector<size_t>> right_nulls_;
    std::tie(left_idx_, right_idx_, right_nulls_)
      = op->outer_hash_join(*this, right, left_idx, right_idx);
    return hash_joined_dftable(*this, right, std::move(left_idx_),
                               std::move(right_idx_), std::move(right_nulls_));
  }
}

bcast_joined_dftable
dftable::bcast_join(dftable& right_, const std::shared_ptr<dfoperator>& op) {
  if(right_.is_right_joinable()) {
    auto& right = right_;
    join_column_name_check(col, right.col);
    auto left_idx = get_local_index();
    auto right_idx = right.get_local_index();
    auto idxs = op->bcast_join(*this, right, left_idx, right_idx);
    return bcast_joined_dftable(*this, right, std::move(idxs.first),
                                std::move(idxs.second));
  }
  else {
    auto right = right_.materialize();
    join_column_name_check(col, right.col);
    auto left_idx = get_local_index();
    auto right_idx = right.get_local_index();
    auto idxs = op->bcast_join(*this, right, left_idx, right_idx);
    return bcast_joined_dftable(*this, right, std::move(idxs.first),
                                std::move(idxs.second));
  }
}

bcast_joined_dftable 
dftable::outer_bcast_join(dftable& right_,
                          const std::shared_ptr<dfoperator>& op) {
  if(right_.is_right_joinable()) {
    auto& right = right_;
    join_column_name_check(col, right.col);
    auto left_idx = get_local_index();
    auto right_idx = right.get_local_index();
    node_local<std::vector<size_t>> left_idx_;
    node_local<std::vector<size_t>> right_idx_;
    node_local<std::vector<size_t>> right_nulls_;
    std::tie(left_idx_, right_idx_, right_nulls_)
      = op->outer_bcast_join(*this, right, left_idx, right_idx);
    return bcast_joined_dftable(*this, right,
                                std::move(left_idx_),
                                std::move(right_idx_),
                                std::move(right_nulls_));
  } else {
    auto right = right_.materialize();
    join_column_name_check(col, right.col);
    auto left_idx = get_local_index();
    auto right_idx = right.get_local_index();
    node_local<std::vector<size_t>> left_idx_;
    node_local<std::vector<size_t>> right_idx_;
    node_local<std::vector<size_t>> right_nulls_;
    std::tie(left_idx_, right_idx_, right_nulls_)
      = op->outer_bcast_join(*this, right, left_idx, right_idx);
    return bcast_joined_dftable(*this, right,
                                std::move(left_idx_),
                                std::move(right_idx_),
                                std::move(right_nulls_));
  }
}

void shrink_missed_inplace(std::vector<size_t>& to_shrink,
                           std::vector<size_t>& missed) {
  auto ret = shrink_missed(to_shrink, missed);
  to_shrink.swap(ret);
}

// use pointers to get derived tables
// vector of reference is not allowed
star_joined_dftable
dftable::star_join(const std::vector<dftable*>& dftables, 
                   const std::vector<std::shared_ptr<dfoperator>>& ops) {
  size_t dftablessize = dftables.size();
  if(dftablessize == 0)
    throw std::runtime_error("star_join: size of table is zero");
  if(dftablessize != ops.size())
    throw std::runtime_error
      ("star_join: size of table is different from size of ops");

  std::vector<dftable> rights(dftablessize);
  node_local<std::vector<size_t>> ret_left_idx = get_local_index();
  std::vector<node_local<std::vector<size_t>>> ret_right_idxs(dftablessize);
  for(size_t i = 0; i < dftablessize; i++) {
    dftable* rightp;
    dftable right_tmp;
    if(dftables[i]->is_right_joinable()) {
      rights[i] = *dftables[i];
      rightp = dftables[i];
    } else {
      rights[i] = dftables[i]->materialize();
      rightp = &rights[i];
    }
    join_column_name_check(col, rights[i].col);
    auto right_idx = rightp->get_local_index();
    node_local<std::vector<size_t>> ret_right_idx;
    node_local<std::vector<size_t>> missed;
    std::tie(ret_right_idx, missed) = 
      ops[i]->star_join(*this, *rightp, ret_left_idx, right_idx);
    ret_left_idx.mapv(shrink_missed_inplace, missed);
    ret_right_idx.mapv(shrink_missed_inplace, missed);
    ret_right_idxs[i] = std::move(ret_right_idx);
    for(size_t j = 0; j < i; j++) 
      ret_right_idxs[j].mapv(shrink_missed_inplace, missed);
  }
  return star_joined_dftable(*this, std::move(rights),
                             std::move(ret_left_idx), 
                             std::move(ret_right_idxs));
}

void merge_split(std::vector<size_t>& split, std::vector<size_t>& to_merge) {
  auto ret = set_union(split, to_merge);
  split.swap(ret);
}

std::vector<size_t> group_by_convert_idx(std::vector<size_t> global_idx,
                                         std::vector<size_t> exchange) {
  size_t size = global_idx.size();
  size_t* global_idxp = &global_idx[0];
  size_t* exchangep = &exchange[0];
  std::vector<size_t> ret(size);
  size_t* retp = &ret[0];
  for(size_t i = 0; i < size; i++) {
    retp[i] = global_idxp[exchangep[i]];
  }
  return ret;
}
                                         
grouped_dftable
dftable::group_by(const std::vector<std::string>& cols) {
  size_t size = cols.size();
  if(size == 0) {
    throw std::runtime_error("column is not specified for group by");
  } else if (size == 1) {
    auto local_idx = get_local_index(); // might be filtered
    auto global_idx = local_to_global_idx(local_idx);
    auto split_idx = column(cols[0])->group_by(global_idx);
    return grouped_dftable(*this, std::move(global_idx),
                           std::move(split_idx), cols);
  } else {
    time_spent t(DEBUG);
    std::vector<std::shared_ptr<dfcolumn>> pcols(size);
    // in the case of filtered_dftable, shrunk column is created
    for(size_t i = 0; i < size; i++) pcols[i] = column(cols[i]);
    auto hash_base = pcols[0]->calc_hash_base();
    // 52 is fraction of double, size_t might be 32bit...
    int bit_len = std::min(sizeof(size_t) * 8, size_t(52));
    int shift = bit_len / size;
    // from memory access point of view, not efficient though...
    for(size_t i = 0; i < size; i++) {
      pcols[i]->calc_hash_base(hash_base, shift);
    }
    t.show("calc_hash_base: ");
    auto local_idx = get_local_index(); // might be filtered
    auto global_idx = local_to_global_idx(local_idx);
    auto split_val =
      make_node_local_allocate<std::vector<std::vector<size_t>>>();
    auto split_idx =
      make_node_local_allocate<std::vector<std::vector<size_t>>>();
    hash_base.mapv(split_by_hash<size_t>, split_val, global_idx, split_idx);
    t.show("split_by_hash: ");
    auto exchanged_split_idx = alltoall_exchange(split_idx);
    t.show("alltoall_exchange: ");
    auto flattened_idx = exchanged_split_idx.map(flatten<size_t>);
    t.show("flatten: ");
    auto partitioned_idx = partition_global_index_bynode(flattened_idx);
    t.show("partition_global_index_bynode: ");
    auto exchanged_idx = exchange_partitioned_index(partitioned_idx);
    t.show("exchanged_idx: ");
    std::vector<std::shared_ptr<dfcolumn>> ex_pcols(size);
    for(size_t i = 0; i < size; i++) {
      ex_pcols[i] = pcols[i]->global_extract(flattened_idx, partitioned_idx,
                                             exchanged_idx);
    }
    t.show("global_extract: ");
    auto ex_local_idx = ex_pcols[0]->get_local_index();
    for(size_t i = 0; i < size; i++) {
      ex_pcols[i]->multi_group_by_sort(ex_local_idx);
    }
    t.show("multi_group_by_sort: ");
    std::vector<node_local<std::vector<size_t>>> splits(size);
    for(size_t i = 0; i < size; i++) {
      splits[i] = ex_pcols[i]->multi_group_by_split(ex_local_idx);
    }
    t.show("multi_group_by_split: ");
    auto& ret_split = splits[0];
    for(size_t i = 1; i < size; i++) {
      ret_split.mapv(merge_split, splits[i]);
    }
    t.show("merge_split: ");
    auto ret_idx = flattened_idx.map(group_by_convert_idx, ex_local_idx);
    t.show("group_by_convert_idx: ");
    /*
    return grouped_dftable(*this, std::move(ret_idx),
                           std::move(ret_split), cols);
    */
    auto ret = grouped_dftable(*this, std::move(ret_idx),
                               std::move(ret_split), cols);
    t.show("create ret: ");
    return ret;
  }
}

/*
grouped_dftable
dftable::group_by(const std::string& col) {
  std::vector<std::string> cols = {col};
  return group_by(cols);
}
*/

void sorted_dftable::drop(const std::string& name) {
  if(name == column_name) {
    column_name = "";
    sorted_column = std::shared_ptr<dfcolumn>();
  }
  dftable::drop(name);
}

void sorted_dftable::rename(const std::string& name, const std::string& name2) {
  dftable::rename(name, name2); // cause exception if name2 already exists
  if(name == column_name) column_name = name2;
}

dftable sorted_dftable::select(const std::vector<std::string>& cols) {
  dftable ret;
  for(size_t i = 0; i < cols.size(); i++) {
    ret.col[cols[i]] = column(cols[i]);
  }
  ret.row_size = global_idx.viewas_dvector<size_t>().size();
  ret.col_order = cols;
  return ret;
}

std::shared_ptr<dfcolumn> sorted_dftable::column(const std::string& name) {
  if(name == column_name && is_cachable) return sorted_column;
  else {
    auto ret = col.find(name);
    if(ret == col.end()) throw std::runtime_error("no such column: " + name);
    else {
      auto col = (*ret).second;
      return col->global_extract(global_idx, partitioned_idx, exchanged_idx);
    }
  }
}

std::shared_ptr<dfcolumn> sorted_dftable::raw_column(const std::string& name) {
  return dftable::column(name);
}

void sorted_dftable::debug_print() {
  std::cout << "global_idx: " << std::endl;
  size_t nodemask = (size_t(1) << DFNODESHIFT) - 1;
  for(auto& i: global_idx.gather()) {
    for(auto j: i) {
      std::cout << (j >> DFNODESHIFT) << "-" << (j & nodemask) << " ";
    }
    std::cout << ": ";
  }
  std::cout << std::endl;
  std::cout << "sorted_column: " << column_name << std::endl;
  dftable::debug_print();
}

sorted_dftable sorted_dftable::sort(const std::string& name) {
  node_local<std::vector<size_t>> idx;
  auto sorted_column = column(name)->sort_with_idx(global_idx, idx);
  return sorted_dftable(*this, std::move(idx), name, std::move(sorted_column));
}

sorted_dftable sorted_dftable::sort_desc(const std::string& name) {
  node_local<std::vector<size_t>> idx;
  auto sorted_column = column(name)->sort_with_idx_desc(global_idx, idx);
  return sorted_dftable(*this, std::move(idx), name, std::move(sorted_column));
}

hash_joined_dftable
sorted_dftable::hash_join(dftable& right,
                          const std::shared_ptr<dfoperator>& op) {
  RLOG(DEBUG) << "calling hash_join after sort" << std::endl;
  return materialize().hash_join(right, op);
}

hash_joined_dftable
sorted_dftable::outer_hash_join(dftable& right,
                                const std::shared_ptr<dfoperator>& op) {
  RLOG(DEBUG) << "calling outer_hash_join after sort" << std::endl;
  return materialize().outer_hash_join(right, op);
}

bcast_joined_dftable
sorted_dftable::bcast_join(dftable& right,
                           const std::shared_ptr<dfoperator>& op) {
  RLOG(DEBUG) << "calling bcast_join after sort" << std::endl;
  return materialize().bcast_join(right, op);
}

bcast_joined_dftable
sorted_dftable::outer_bcast_join(dftable& right,
                                 const std::shared_ptr<dfoperator>& op) {
  RLOG(DEBUG) << "calling outer_bcast_join after sort" << std::endl;
  return materialize().outer_bcast_join(right, op);
}

star_joined_dftable
sorted_dftable::star_join
(const std::vector<dftable*>& dftables, 
 const std::vector<std::shared_ptr<dfoperator>>& op){
  return materialize().star_join(dftables, op);
}

grouped_dftable
sorted_dftable::group_by(const std::vector<std::string>& cols) {
  RLOG(DEBUG) << "calling group_by after sort" << std::endl;
  return materialize().group_by(cols);
}

std::vector<size_t>
concat_idx(std::vector<size_t>& a, std::vector<size_t>& b) {
  size_t asize = a.size();
  size_t bsize = b.size();
  std::vector<size_t> ret(asize+bsize);
  size_t* ap = &a[0];
  size_t* bp = &b[0];
  size_t* retp = &ret[0];
  for(size_t i = 0; i < asize; i++) retp[i] = ap[i];
  for(size_t i = 0; i < bsize; i++) retp[asize+i] = bp[i];
  return ret;
}

hash_joined_dftable
hash_joined_dftable::hash_join(dftable& right,
                               const std::shared_ptr<dfoperator>& op) {
  return materialize().hash_join(right, op);
}

hash_joined_dftable
hash_joined_dftable::outer_hash_join(dftable& right,
                                     const std::shared_ptr<dfoperator>& op) {
  return materialize().outer_hash_join(right, op);
}

bcast_joined_dftable
hash_joined_dftable::bcast_join(dftable& right,
                               const std::shared_ptr<dfoperator>& op) {
  return materialize().bcast_join(right, op);
}

bcast_joined_dftable
hash_joined_dftable::outer_bcast_join(dftable& right,
                                     const std::shared_ptr<dfoperator>& op) {
  return materialize().outer_bcast_join(right, op);
}

star_joined_dftable
hash_joined_dftable::star_join
(const std::vector<dftable*>& dftables, 
 const std::vector<std::shared_ptr<dfoperator>>& op){
  return materialize().star_join(dftables, op);
}

grouped_dftable
hash_joined_dftable::group_by(const std::vector<std::string>& cols) {
  return materialize().group_by(cols);
}

// TODO: sort only on the specified column to create left/right idx
// it is possible but return type becomes hash_joined_dftable...
sorted_dftable hash_joined_dftable::sort(const std::string& name) {
  return materialize().sort(name);
}

sorted_dftable hash_joined_dftable::sort_desc(const std::string& name) {
  return materialize().sort_desc(name);
}

size_t hash_joined_dftable::num_col() const {
    return dftable::num_col() + right.num_col();
}

size_t hash_joined_dftable::num_row() {
  return left_idx.viewas_dvector<size_t>().size();
}

dftable hash_joined_dftable::select(const std::vector<std::string>& cols) {    
  dftable ret;
  for(size_t i = 0; i < cols.size(); i++) {
    ret.col[cols[i]] = column(cols[i]);
  }
  ret.row_size = left_idx.viewas_dvector<size_t>().size();
  ret.col_order = cols;
  return ret;
}

std::vector<std::string> hash_joined_dftable::columns() const {
  std::vector<std::string> ret = dftable::columns();
  auto right_cols = right.columns();
  ret.insert(ret.end(), right_cols.begin(), right_cols.end());
  return ret;
}

std::shared_ptr<dfcolumn>
hash_joined_dftable::column(const std::string& name) {
  auto left_ret = col.find(name);
  if(left_ret != col.end()) {
    auto c = (*left_ret).second;
    // if outer, left_idx is changed to contain nulls in ctor
    return c->global_extract(left_idx, left_partitioned_idx,
                             left_exchanged_idx);
  } else {
    auto right_ret = right.col.find(name);
    if(right_ret != right.col.end()) {
      auto c = (*right_ret).second;
      if(is_outer) {
        auto retcol = c->global_extract(right_idx, right_partitioned_idx,
                                        right_exchanged_idx);
        retcol->append_nulls(right_nulls);
        return retcol;
      } else {
        return c->global_extract(right_idx, right_partitioned_idx,
                                 right_exchanged_idx);
      }
    } else {
      throw std::runtime_error("no such column: " + name);
    }
  }
}

void hash_joined_dftable::debug_print() {
  std::cout << "left: " << std::endl;
  dftable::debug_print();
  std::cout << "right: " << std::endl;
  right.debug_print();
  std::cout << "left_idx: " << std::endl;
  size_t nodemask = (size_t(1) << DFNODESHIFT) - 1;
  for(auto& i: left_idx.gather()) {
    for(auto j: i) {
      std::cout << (j >> DFNODESHIFT) << "-" << (j & nodemask) << " ";
    }
    std::cout << ": ";
  }
  std::cout << std::endl;
  std::cout << "right_idx: " << std::endl;
  for(auto& i: right_idx.gather()) {
    for(auto j: i) {
      std::cout << (j >> DFNODESHIFT) << "-" << (j & nodemask) << " ";
    }
    std::cout << ": ";
  }
  std::cout << std::endl;
  if(is_outer) {
    std::cout << "right_nulls: " << std::endl;
    for(auto& i: right_nulls.gather()) {
      for(auto j: i) {
        std::cout << (j >> DFNODESHIFT) << "-" << (j & nodemask) << " ";
      }
      std::cout << ": ";
    }
    std::cout << std::endl;
  }
}

hash_joined_dftable
bcast_joined_dftable::hash_join(dftable& right,
                                const std::shared_ptr<dfoperator>& op) {
  return materialize().hash_join(right, op);
}

hash_joined_dftable
bcast_joined_dftable::outer_hash_join(dftable& right,
                                      const std::shared_ptr<dfoperator>& op) {
  return materialize().outer_hash_join(right, op);
}

bcast_joined_dftable
bcast_joined_dftable::bcast_join(dftable& right,
                                 const std::shared_ptr<dfoperator>& op) {
  return materialize().bcast_join(right, op);
}

bcast_joined_dftable
bcast_joined_dftable::outer_bcast_join(dftable& right,
                                       const std::shared_ptr<dfoperator>& op) {
  return materialize().outer_bcast_join(right, op);
}

star_joined_dftable
bcast_joined_dftable::star_join
(const std::vector<dftable*>& dftables, 
 const std::vector<std::shared_ptr<dfoperator>>& op){
  return materialize().star_join(dftables, op);
}

grouped_dftable
bcast_joined_dftable::group_by(const std::vector<std::string>& cols) {
  return materialize().group_by(cols);
}

// TODO: sort only on the specified column to create left/right idx
// it is possible but return type becomes hash_joined_dftable...
sorted_dftable bcast_joined_dftable::sort(const std::string& name) {
  return materialize().sort(name);
}

sorted_dftable bcast_joined_dftable::sort_desc(const std::string& name) {
  return materialize().sort_desc(name);
}

size_t bcast_joined_dftable::num_col() const {
  return dftable::num_col() + right.num_col();
}

size_t bcast_joined_dftable::num_row() {
  return left_idx.viewas_dvector<size_t>().size();
}

dftable bcast_joined_dftable::select(const std::vector<std::string>& cols) {
  dftable ret;
  for(size_t i = 0; i < cols.size(); i++) {
    ret.col[cols[i]] = column(cols[i]);
  }
  ret.row_size = left_idx.viewas_dvector<size_t>().size();
  ret.col_order = cols;
  return ret;
}

std::vector<std::string> bcast_joined_dftable::columns() const {
  std::vector<std::string> ret = dftable::columns();
  auto right_cols = right.columns();
  ret.insert(ret.end(), right_cols.begin(), right_cols.end());
  return ret;
}

std::shared_ptr<dfcolumn>
bcast_joined_dftable::column(const std::string& name) {
  auto left_ret = col.find(name);
  if(left_ret != col.end()) {
    auto c = (*left_ret).second;
    return c->extract(left_idx);
  } else {
    auto right_ret = right.col.find(name);
    if(right_ret != right.col.end()) {
      auto c = (*right_ret).second;
      if(is_outer) {
        auto retcol = c->global_extract(right_idx, right_partitioned_idx,
                                        right_exchanged_idx);
        retcol->append_nulls(right_nulls);
        return retcol;
      } else {
        return c->global_extract(right_idx, right_partitioned_idx,
                                 right_exchanged_idx);
      }
    } else {
      throw std::runtime_error("no such column: " + name);
    }
  }
}

void bcast_joined_dftable::debug_print() {
  std::cout << "left: " << std::endl;
  dftable::debug_print();
  std::cout << "right: " << std::endl;
  right.debug_print();
  std::cout << "left_idx: " << std::endl;
  for(auto& i: left_idx.gather()) {
    for(auto j: i) {
      std::cout << j << " ";
    }
    std::cout << ": ";
  }
  std::cout << std::endl;
  size_t nodemask = (size_t(1) << DFNODESHIFT) - 1;
  std::cout << "right_idx: " << std::endl;
  for(auto& i: right_idx.gather()) {
    for(auto j: i) {
      std::cout << (j >> DFNODESHIFT) << "-" << (j & nodemask) << " ";
    }
    std::cout << ": ";
  }
  std::cout << std::endl;
  if(is_outer) {
    std::cout << "right_nulls: " << std::endl;
    for(auto& i: right_nulls.gather()) {
      for(auto j: i) {
        std::cout << (j >> DFNODESHIFT) << "-" << (j & nodemask) << " ";
      }
      std::cout << ": ";
    }
    std::cout << std::endl;
  }
}

hash_joined_dftable
star_joined_dftable::hash_join(dftable& right,
                               const std::shared_ptr<dfoperator>& op) {
  return materialize().hash_join(right, op);
}

hash_joined_dftable
star_joined_dftable::outer_hash_join(dftable& right,
                                     const std::shared_ptr<dfoperator>& op) {
  return materialize().outer_hash_join(right, op);
}

bcast_joined_dftable
star_joined_dftable::bcast_join(dftable& right,
                                const std::shared_ptr<dfoperator>& op) {
  return materialize().bcast_join(right, op);
}

star_joined_dftable
star_joined_dftable::star_join
(const std::vector<dftable*>& dftables, 
 const std::vector<std::shared_ptr<dfoperator>>& op){
  return materialize().star_join(dftables, op);
}

bcast_joined_dftable
star_joined_dftable::outer_bcast_join(dftable& right,
                                      const std::shared_ptr<dfoperator>& op) {
  return materialize().outer_bcast_join(right, op);
}

grouped_dftable
star_joined_dftable::group_by(const std::vector<std::string>& cols) {
  return materialize().group_by(cols);
}

sorted_dftable star_joined_dftable::sort(const std::string& name) {
  return materialize().sort(name);
}

sorted_dftable star_joined_dftable::sort_desc(const std::string& name) {
  return materialize().sort_desc(name);
}

size_t star_joined_dftable::num_col() const {
  size_t total = dftable::num_col();
  for(size_t i = 0; i < rights.size(); i++) total += rights[i].num_col();
  return total;
}

size_t star_joined_dftable::num_row() {
  return left_idx.viewas_dvector<size_t>().size();
}

dftable star_joined_dftable::select(const std::vector<std::string>& cols) {    
  dftable ret;
  for(size_t i = 0; i < cols.size(); i++) {
    ret.col[cols[i]] = column(cols[i]);
  }
  ret.row_size = left_idx.viewas_dvector<size_t>().size();
  ret.col_order = cols;
  return ret;
}

std::vector<std::string> star_joined_dftable::columns() const {
  std::vector<std::string> ret = dftable::columns();
  for(size_t i = 0; i < rights.size(); i++) {
    auto right_cols = rights[i].columns();
    ret.insert(ret.end(), right_cols.begin(), right_cols.end());
  }
  return ret;
}

std::shared_ptr<dfcolumn>
star_joined_dftable::column(const std::string& name) {
  auto left_ret = col.find(name);
  if(left_ret != col.end()) {
    auto c = (*left_ret).second;
    return c->extract(left_idx);
  } else {
    for(size_t i = 0; i < rights.size(); i++) {
      auto right_ret = rights[i].col.find(name);
      if(right_ret != rights[i].col.end()) {
        auto c = (*right_ret).second;
        return c->global_extract(right_idxs[i], right_partitioned_idxs[i],
                                 right_exchanged_idxs[i]);
      }
    }
    throw std::runtime_error("no such column: " + name);
  }
}

void star_joined_dftable::debug_print() {
  std::cout << "left: " << std::endl;
  dftable::debug_print();
  std::cout << "rights: " << std::endl;
  for(size_t i = 0; i < rights.size(); i++) {
    rights[i].debug_print();
  }
  std::cout << "left_idx: " << std::endl;
  for(auto& i: left_idx.gather()) {
    for(auto j: i) {
      std::cout << j << " ";
    }
    std::cout << ": ";
  }
  std::cout << std::endl;
  size_t nodemask = (size_t(1) << DFNODESHIFT) - 1;
  std::cout << "right_idxs: " << std::endl;
  for(size_t k = 0; k < rights.size(); k++) {
    for(auto& i: right_idxs[k].gather()) {
      for(auto j: i) {
        std::cout << (j >> DFNODESHIFT) << "-" << (j & nodemask) << " ";
      }
      std::cout << ": ";
    }
    std::cout << std::endl;
  }
}

std::vector<size_t> get_first_grouped_idx(std::vector<size_t>& grouped_idx,
                                          std::vector<size_t>& idx_split) {
  size_t splitsize = idx_split.size();
  size_t* grouped_idxp = &grouped_idx[0];
  size_t* idx_splitp = &idx_split[0];
  std::vector<size_t> ret(splitsize-1);
  size_t* retp = &ret[0];
  for(size_t i = 0; i < splitsize-1; i++) {
    retp[i] = grouped_idxp[idx_splitp[i]];
  }
  return ret;
}

size_t grouped_dftable::num_row() {
  // each split of node has one extra item
  return idx_split.viewas_dvector<size_t>().size() - get_nodesize();
} 

dftable grouped_dftable::select(const std::vector<std::string>& cols) {
  dftable ret_table;
  size_t colssize = cols.size();
  auto cols_global_idx = grouped_idx.map(get_first_grouped_idx,
                                         idx_split);
  auto cols_partitioned_idx =
    partition_global_index_bynode(cols_global_idx);
  auto cols_exchanged_idx =
    exchange_partitioned_index(cols_partitioned_idx);
  for(size_t i = 0; i < colssize; i++) {
    bool in_grouped_cols = false;
    for(size_t j = 0; j < grouped_cols.size(); j++) {
      if(cols[i] == grouped_cols[j]) {
        in_grouped_cols = true; break;
      }
    }
    if(!in_grouped_cols)
      throw std::runtime_error("select not grouped column");
    auto newcol = column(cols[i])->
      global_extract(cols_global_idx, cols_partitioned_idx,
                     cols_exchanged_idx);
    ret_table.col[cols[i]] = newcol;
  }
  ret_table.row_size = num_row();
  ret_table.col_order = cols;
  return ret_table;
}

dftable
grouped_dftable::select(const std::vector<std::string>& cols,
                        const std::vector<std::shared_ptr<dfaggregator>>& aggs) {
  time_spent t(DEBUG);
  dftable ret_table;
  size_t colssize = cols.size();
  if(colssize != 0) {
    auto cols_global_idx = grouped_idx.map(get_first_grouped_idx,
                                           idx_split);
    auto cols_partitioned_idx =
      partition_global_index_bynode(cols_global_idx);
    auto cols_exchanged_idx =
      exchange_partitioned_index(cols_partitioned_idx);
    for(size_t i = 0; i < colssize; i++) {
      bool in_grouped_cols = false;
      for(size_t j = 0; j < grouped_cols.size(); j++) {
        if(cols[i] == grouped_cols[j]) {
          in_grouped_cols = true; break;
        }
      }
      if(!in_grouped_cols)
        throw std::runtime_error("select not grouped column");
      auto newcol = column(cols[i])->
        global_extract(cols_global_idx, cols_partitioned_idx,
                       cols_exchanged_idx);
      ret_table.col[cols[i]] = newcol;
    }
  }
  ret_table.col_order = cols;
  t.show("grouped_dftable select global_extract: ");
  size_t aggssize = aggs.size();
  for(size_t i = 0; i < aggssize; i++) {
    /* // modified to allow aggregation of grouped column
    bool in_grouped_cols = false;
    for(size_t j = 0; j < grouped_cols.size(); j++) {
      if(aggs[i]->col == grouped_cols[j]) {
        in_grouped_cols = true; break;
      }
    }
    if(in_grouped_cols)
      throw std::runtime_error("aggregate of grouped column");
    */
    auto newcol = aggs[i]->aggregate(*this, grouped_idx, idx_split,
                                     partitioned_idx, exchanged_idx);
    if(aggs[i]->has_as) {
      if(ret_table.col.find(aggs[i]->as) != ret_table.col.end())
        throw std::runtime_error
          ("grouped_dftable::select: same column name already exists");
      ret_table.col[aggs[i]->as] = newcol;
      ret_table.col_order.push_back(aggs[i]->as);
    } else {
      if(ret_table.col.find(aggs[i]->col) != ret_table.col.end())
        throw std::runtime_error
          ("grouped_dftable::select: same column name already exists");
      ret_table.col[aggs[i]->col] = newcol;
      ret_table.col_order.push_back(aggs[i]->col);
    }
  }
  t.show("grouped_dftable select aggregate: ");
  ret_table.row_size = num_row();
  return ret_table;
}

/* // removed this because of initlializer list ambiguity
dftable
grouped_dftable::select(const std::string& col,
                        const std::vector<std::shared_ptr<dfaggregator>>& aggs) {
  std::vector<std::string> cols = {col};
  return select(cols, aggs);
}

dftable
grouped_dftable::select(const std::vector<std::string>& cols,
                        const std::shared_ptr<dfaggregator>& agg) {
  std::vector<std::shared_ptr<dfaggregator>> aggs = {agg};
  return select(cols, aggs);
}

dftable
grouped_dftable::select(const std::string& col,
                        const std::shared_ptr<dfaggregator>& agg) {
  std::vector<std::string> cols = {col};
  std::vector<std::shared_ptr<dfaggregator>> aggs = {agg};
  return select(cols, aggs);
}
*/

void grouped_dftable::debug_print() {
  std::cout << "table: " << std::endl;
  dftable::debug_print();
  std::cout << "grouped_idx: " << std::endl;
  size_t nodemask = (size_t(1) << DFNODESHIFT) - 1;
  for(auto& i: grouped_idx.gather()) {
    for(auto j: i) {
      std::cout << (j >> DFNODESHIFT) << "-" << (j & nodemask) << " ";
    }
    std::cout << ": ";
  }
  std::cout << std::endl;
  std::cout << "idx_split: " << std::endl;
  for(auto& i: idx_split.gather()) {
    for(auto j: i) std::cout << j << " "; 
    std::cout << ": ";
  }
  std::cout << std::endl;
  std::cout << "grouped_cols: " << std::endl;
  for(auto& i: grouped_cols) std::cout << i << " ";
  std::cout << std::endl;
}

}
