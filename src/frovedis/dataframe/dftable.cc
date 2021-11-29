#include <unordered_set>
#include "dftable.hpp"
#include "dfoperator.hpp"
#include "dftable_to_words.hpp"
#include "make_dftable_loadtext.hpp"
#include "../text/char_int_conv.hpp"
#include "../text/load_text.hpp"
#include "../text/datetime_to_words.hpp"
#include "../core/prefix_sum.hpp"
#include "dffunction.hpp"

#define GROUPED_DFTABLE_VLEN 256

namespace frovedis {

std::vector<std::string> dftable_base::columns() const {
  return col_order;
}

std::vector<std::pair<std::string, std::string>>
dftable_base::dtypes() {
  std::vector<std::pair<std::string,std::string>> ret;
  auto cols = columns();
  for(size_t i = 0; i < cols.size(); i++) {
    ret.push_back(std::make_pair(cols[i],
                                 column(cols[i])->dtype()));
  }
  return ret;
}

dftable dftable_base::fselect
(const std::vector<std::shared_ptr<dffunction>>& cols) {
  dftable ret;
  ret.row_size = row_size;
  ret.row_sizes = row_sizes;
  for(size_t i = 0; i < cols.size(); i++) {
    auto as = cols[i]->get_as();
    if(ret.col.find(as) != ret.col.end())
      throw std::runtime_error("select: same column name already exists");
    if(cols[i]->is_id()) { // no need to restore
      ret.col[as] = cols[i]->execute(*this);
    } else {
      use_dfcolumn use(cols[i]->columns_to_use(*this));    
      ret.col[as] = cols[i]->execute(*this);
      ret.col[as]->spill();
    }
    ret.col_order.push_back(as);
  }
  return ret;
}

dftable dftable_base::select(const std::vector<std::string>& cols) {
  dftable ret;
  ret.row_size = row_size;
  ret.row_sizes = row_sizes;
  for(size_t i = 0; i < cols.size(); i++) {
    ret.col[cols[i]] = column(cols[i]);
  }
  ret.col_order = cols;
  return ret;
}

dftable dftable_base::materialize(){return select(columns());}

std::vector<int> 
construct_isnull_column(size_t size, 
                        const std::vector<size_t>& nullpos) {
  auto ret = vector_zeros<int>(size);
  auto rptr = ret.data();
  auto nptr = nullpos.data();
  auto nullsz = nullpos.size();
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
  for(size_t i = 0; i < nullsz; ++i) rptr[nptr[i]] = 1;
  return ret;
}

dftable dftable_base::isnull(const std::vector<std::string>& cols) {
  dftable ret;
  auto mysz = make_node_local_scatter(num_rows());
  for (auto& c: cols) {
    use_dfcolumn use(raw_column(c));
    auto dfcol = column(c);
    auto nullpos = dfcol->get_nulls();
    auto vals = mysz.map(construct_isnull_column, nullpos);
    auto nulls = make_node_local_allocate<std::vector<size_t>>();  
    auto rescol = std::make_shared<typed_dfcolumn<int>>(std::move(vals), 
                                                        std::move(nulls));
    ret.append_column(c, rescol);
  }
  return ret;
}

sorted_dftable dftable_base::sort(const std::string& name) {
  use_dfcolumn use(raw_column(name));
  node_local<std::vector<size_t>> idx;
  auto to_sort_column = column(name);
  auto sorted_column = to_sort_column->sort(idx);
  if(to_sort_column->if_contain_nulls())
    return sorted_dftable(*this, std::move(idx));
  else {
    sorted_column->spill();
    return sorted_dftable(*this, std::move(idx), name,
                          std::move(sorted_column));
  }
}

sorted_dftable dftable_base::sort_desc(const std::string& name) {
  use_dfcolumn use(raw_column(name));
  node_local<std::vector<size_t>> idx;
  auto to_sort_column = column(name);
  auto sorted_column = to_sort_column->sort_desc(idx);
  if(to_sort_column->if_contain_nulls())
    return sorted_dftable(*this, std::move(idx));
  else {
    sorted_column->spill();
    return sorted_dftable(*this, std::move(idx), name,
                          std::move(sorted_column));
  }
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

hash_joined_dftable
dftable_base::hash_join(dftable_base& right_,
                        const std::shared_ptr<dfoperator>& op) {
  if(right_.is_right_joinable()) {
    auto& right = right_;
    use_dfcolumn use(op->columns_to_use(*this, right));
    join_column_name_check(col, right.col);
    auto left_idx = get_local_index();
    auto right_idx = right.get_local_index();
    auto idxs = op->hash_join(*this, right, left_idx, right_idx);
    return hash_joined_dftable(*this, right, std::move(idxs.first),
                               std::move(idxs.second));
  }
  else {
    auto right = right_.materialize();
    use_dfcolumn use(op->columns_to_use(*this, right));
    join_column_name_check(col, right.col);
    auto left_idx = get_local_index();
    auto right_idx = right.get_local_index();
    auto idxs = op->hash_join(*this, right, left_idx, right_idx);
    return hash_joined_dftable(*this, right, std::move(idxs.first),
                               std::move(idxs.second));
  }
}

hash_joined_dftable 
dftable_base::outer_hash_join(dftable_base& right_,
                              const std::shared_ptr<dfoperator>& op) {
  if(right_.is_right_joinable()) {
    auto& right = right_;
    use_dfcolumn use(op->columns_to_use(*this, right));
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
    use_dfcolumn use(op->columns_to_use(*this, right));
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
dftable_base::bcast_join(dftable_base& right_,
                         const std::shared_ptr<dfoperator>& op,
                         bool allow_exchange_lr) {
  if(allow_exchange_lr && num_row() < right_.num_row()) {
    return right_.bcast_join(*this, op->exchange_lr(), false);
  } else {
    if(right_.is_right_joinable()) {
      auto& right = right_;
      use_dfcolumn use(op->columns_to_use(*this, right));
      join_column_name_check(col, right.col);
      auto left_idx = get_local_index();
      auto right_idx = right.get_local_index();
      auto idxs = op->bcast_join(*this, right, left_idx, right_idx);
      return bcast_joined_dftable(*this, right, std::move(idxs.first),
                                  std::move(idxs.second));
    }
    else {
      auto right = right_.materialize();
      use_dfcolumn use(op->columns_to_use(*this, right));
      join_column_name_check(col, right.col);
      auto left_idx = get_local_index();
      auto right_idx = right.get_local_index();
      auto idxs = op->bcast_join(*this, right, left_idx, right_idx);
      return bcast_joined_dftable(*this, right, std::move(idxs.first),
                                  std::move(idxs.second));
    }
  }
}

bcast_joined_dftable 
dftable_base::outer_bcast_join(dftable_base& right_,
                               const std::shared_ptr<dfoperator>& op) {
  if(right_.is_right_joinable()) {
    auto& right = right_;
    use_dfcolumn use(op->columns_to_use(*this, right));
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
    use_dfcolumn use(op->columns_to_use(*this, right));
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
dftable_base::star_join(const std::vector<dftable_base*>& dftables, 
                        const std::vector<std::shared_ptr<dfoperator>>& ops) {
  size_t dftablessize = dftables.size();
  if(dftablessize == 0)
    throw std::runtime_error("star_join: size of table is zero");
  if(dftablessize != ops.size())
    throw std::runtime_error
      ("star_join: size of table is different from size of ops");

  std::vector<dftable_base> rights(dftablessize);
  node_local<std::vector<size_t>> ret_left_idx = get_local_index();
  std::vector<node_local<std::vector<size_t>>> ret_right_idxs(dftablessize);
  for(size_t i = 0; i < dftablessize; i++) {
    dftable_base* rightp;
    dftable_base right_tmp;
    if(dftables[i]->is_right_joinable()) {
      rights[i] = *dftables[i];
      rightp = dftables[i];
    } else {
      rights[i] = dftables[i]->materialize();
      rightp = &rights[i];
    }
    use_dfcolumn use(ops[i]->columns_to_use(*this, *rightp));
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

void create_merge_map(std::vector<size_t>& nodeid,
                      std::vector<size_t>& split,
                      std::vector<std::vector<size_t>>& merge_map);
                                   
grouped_dftable
dftable_base::group_by(const std::vector<std::string>& cols) {
  use_dfcolumn use(cols, *this);
  size_t size = cols.size();
  if(size == 0) {
    throw std::runtime_error("column is not specified for group by");
  } else if (size == 1) {
    auto local_idx = get_local_index(); // might be filtered
    auto split_idx = make_node_local_allocate<std::vector<size_t>>();
    auto hash_divide =
      make_node_local_allocate<std::vector<std::vector<size_t>>>();
    auto merge_map =
      make_node_local_allocate<std::vector<std::vector<size_t>>>();
    auto grouped_column = raw_column(cols[0])->group_by
      (local_idx, split_idx, hash_divide, merge_map);
    auto num_row_ = grouped_column->size();
    auto num_rows_ = grouped_column->sizes();
    grouped_column->spill();
    return grouped_dftable(*this, std::move(local_idx), std::move(split_idx),
                           std::move(hash_divide), std::move(merge_map),
                           {grouped_column}, cols, num_row_, num_rows_);
  } else {
    std::vector<std::shared_ptr<dfcolumn>> pcols(size);
    for(size_t i = 0; i < size; i++) pcols[i] = raw_column(cols[i]);
    auto local_idx = get_local_index(); // might be filtered
    // sort reverse order to show intuitive result
    for(size_t i = 0; i < size-1; i++) {
      pcols[size - i - 1]->multi_group_by_sort(local_idx);
    }
    auto split_idx = pcols[0]->multi_group_by_sort_split(local_idx);
    for(size_t i = 1; i < size; i++) {
      auto tmp = pcols[i]->multi_group_by_split(local_idx);
      split_idx.mapv(merge_split, tmp);
    }
    std::vector<std::shared_ptr<dfcolumn>> pcols2(size);
    for(size_t i = 0; i < size; i++) {
      pcols2[i] = pcols[i]->multi_group_by_extract(local_idx, split_idx,
                                                   false);
    }
    auto hash_base = pcols2[0]->calc_hash_base();
    // 52 is fraction of double, size_t might be 32bit...
    int bit_len = std::min(sizeof(size_t) * 8, size_t(52));
    int shift = bit_len / size;
    // from memory access point of view, not efficient though...
    for(size_t i = 1; i < size; i++) {
      pcols2[i]->calc_hash_base(hash_base, shift);
    }
    auto hash_divide =
      make_node_local_allocate<std::vector<std::vector<size_t>>>();
    hash_base.mapv
      (+[](std::vector<size_t>& hash_base,
           std::vector<std::vector<size_t>>& hash_divide) {
        auto size = hash_base.size();
        std::vector<size_t> iota(size);
        auto iotap = iota.data();
        for(size_t i = 0; i < size; i++) iotap[i] = i;
        split_by_hash_no_outval(hash_base, iota, hash_divide);
      }, hash_divide);
    std::vector<std::shared_ptr<dfcolumn>> pcols3(size);
    for(size_t i = 0; i < size; i++) {
      pcols3[i] = pcols2[i]->multi_group_by_exchange(hash_divide);
    }
    auto sizes_tmp = hash_divide.map
      (+[](std::vector<std::vector<size_t>>& hash_divide) {
        std::vector<size_t> ret(hash_divide.size());
        for(size_t i = 0; i < hash_divide.size(); i++) {
          ret[i] = hash_divide[i].size();
        }
        return ret;
      });
    auto sizes = alltoall_exchange(sizes_tmp);
    auto nodeid = sizes.map
      (+[](std::vector<size_t>& sizes) {
        size_t total = 0;
        auto sizesp = sizes.data();
        for(size_t i = 0; i < sizes.size(); i++) total += sizesp[i];
        std::vector<size_t> ret(total);
        auto retp = ret.data();
        for(size_t i = 0; i < sizes.size(); i++) {
          auto size = sizes[i];
          for(size_t j = 0; j < size; j++) {
            retp[j] = i;
          }
          retp += size;
        }
        return ret;
      });
    auto local_idx2 = nodeid.map
      (+[](std::vector<size_t>& nodeid) {
        size_t size = nodeid.size();
        std::vector<size_t> ret(size);
        auto retp = ret.data();
        for(size_t i = 0; i < size; i++) retp[i] = i;
        return ret;
      });
    for(size_t i = 0; i < size-1; i++) {
      pcols3[size - i - 1]->multi_group_by_sort(local_idx2);
    }
    auto split_idx2 = pcols3[0]->multi_group_by_sort_split(local_idx2);
    for(size_t i = 1; i < size; i++) {
      auto tmp = pcols3[i]->multi_group_by_split(local_idx2);
      split_idx2.mapv(merge_split, tmp);
    }
    auto nodeid2 = nodeid.map
      (+[](std::vector<size_t>& nodeid, std::vector<size_t>& local_idx) {
        auto size = nodeid.size();
        auto nodeidp = nodeid.data();
        auto local_idxp = local_idx.data();
        std::vector<size_t> ret(size);
        auto retp = ret.data();
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
        for(size_t i = 0; i < size; i++) {
          retp[i] = nodeidp[local_idxp[i]];
        }
        return ret;
      }, local_idx2);
    auto merge_map =
      make_node_local_allocate<std::vector<std::vector<size_t>>>();
    merge_map.mapv
      (+[](std::vector<std::vector<size_t>>& merge_map,
           std::vector<size_t>& sizes) {
        merge_map.resize(sizes.size());
        for(size_t i = 0; i < sizes.size(); i++) {
          merge_map[i].resize(sizes[i]);
        }
      }, sizes);
    nodeid2.mapv(create_merge_map, split_idx2, merge_map);
    std::vector<std::shared_ptr<dfcolumn>> pcols4(size);    
    for(size_t i = 0; i < size; i++) {
      pcols4[i] = pcols3[i]->
        multi_group_by_extract(local_idx2, split_idx2, true);
    }
    auto num_row_ = pcols4[0]->size();
    auto num_rows_ = pcols4[0]->sizes();
    for(size_t i = 0; i < size; i++) {pcols4[i]->spill();}
    return grouped_dftable(*this, std::move(local_idx), std::move(split_idx),
                           std::move(hash_divide), std::move(merge_map),
                           std::move(pcols4), cols, num_row_, num_rows_);
  }
}

node_local<words>
dftable_base::as_words(const std::string name,
                       size_t precision,
                       const std::string& datetime_fmt,
                       bool quote_escape,
                       const std::string& nullstr) {
  use_dfcolumn use(raw_column(name));
  return column(name)->as_words(precision, datetime_fmt, quote_escape, nullstr);
}

dftable dftable_base::head(size_t limit) {
  dftable limit_table;
  auto cols = columns();
  for(size_t i = 0; i < cols.size(); i++) {
    use_dfcolumn use(this->raw_column(cols[i]));
    auto c = this->column(cols[i]);
    limit_table.append_column(cols[i], c->head(limit));
  }
  return limit_table;
}

dftable dftable_base::tail(size_t limit) {
  dftable limit_table;
  auto cols = columns();
  for(size_t i = 0; i < cols.size(); i++) {
    use_dfcolumn use(this->raw_column(cols[i]));
    auto c = this->column(cols[i]);
    limit_table.append_column(cols[i],c->tail(limit));
  }
  return limit_table;
}

void to_stream_impl(bool with_index, size_t start_idx, bool with_column_name,
                    words& table_words, std::vector<std::string>& cols,
                    std::ostream& stream) {
  auto num_col = cols.size();
  if (num_col == 0) return;
  auto num_words = table_words.starts.size();
  auto num_row = num_words / num_col;
  if(num_words % num_col != 0)
    throw std::runtime_error("show_all: incorrect number of col or row");
  if(with_column_name) {
    if(with_index) stream << "\t";
    for(size_t i = 0; i < num_col-1; i++) {
      stream << cols[i] << "\t";
    }
    stream << cols[num_col-1] << std::endl;
  }
  auto charsp = table_words.chars.data();
  for(size_t i = 0; i < num_row; i++) {
    if(with_index) stream << i + start_idx << "\t";
    for(size_t j = 0; j < num_col-1; j++) {
      auto crnt_start = table_words.starts[i * num_col + j];
      auto crnt_len = table_words.lens[i * num_col + j];
      for(size_t k = 0; k < crnt_len; k++) {
        stream << static_cast<char>(charsp[crnt_start + k]);
      }
      stream << "\t";
    }
    auto crnt_start = table_words.starts[i * num_col + num_col - 1];
    auto crnt_len = table_words.lens[i * num_col + num_col - 1];
    for(size_t k = 0; k < crnt_len; k++) {
      stream << static_cast<char>(charsp[crnt_start + k]);
    }
    stream << "\n";
  }
}

void dftable_base::show_all(bool with_index) {
  auto table_words = dftable_to_words(*this).reduce(merge_words);
  auto cols = columns();
  to_stream_impl(with_index, 0, true, table_words, cols, std::cout);
}

void dftable_base::show(size_t limit) {
  head(limit).show_all();
}

void dftable_base::show() {
  show(20);
  if(num_row() > 20) std::cout << "..." << std::endl;
}

void dftable_base::print() {
  auto nr = num_row();
  if(nr < 61) show_all(true);
  else {
    head(30).show_all(true);
    std::cout << "..." << std::endl;
    auto t = tail(30);
    auto table_words = dftable_to_words(t).reduce(merge_words);
    auto cols = columns(); // not used 
    to_stream_impl(true, nr-30, false, table_words, cols, std::cout);
    std::cout << "\n[" << num_row() << " rows x " << num_col() << " columns]\n";
  }
}

void to_string_all(dftable_base& t, bool with_index, std::ostream& os) {
  auto table_words = dftable_to_words(t).reduce(merge_words);
  auto cols = t.columns();
  to_stream_impl(with_index, 0, true, table_words, cols, os);
}

std::string dftable_base::to_string(bool with_index) {
  std::ostringstream os;
  auto nr = num_row();
  if(nr < 61) to_string_all(*this, with_index, os);
  else {
    auto h = head(30);
    to_string_all(h, with_index, os);
    os << "..." << std::endl;
    auto t = tail(30);
    auto table_words = dftable_to_words(t).reduce(merge_words);
    auto cols = columns(); // not used
    to_stream_impl(with_index, nr-30, false, table_words, cols, os);
    os << "\n[" << num_row() << " rows x " << num_col() << " columns]\n";
  }
  std::string ret = os.str();
  return ret;
}

size_t dftable_base::count(const std::string& name) {
  use_dfcolumn use(raw_column(name));
  return column(name)->count();
}

double dftable_base::avg(const std::string& name) {
  use_dfcolumn use(raw_column(name));
  return column(name)->avg();
}

double dftable_base::std(const std::string& name, double ddof) {
  use_dfcolumn use(raw_column(name));
  return column(name)->std(ddof);
}

double dftable_base::sem(const std::string& name, double ddof) {
  use_dfcolumn use(raw_column(name));
  return column(name)->sem(ddof);
}

double dftable_base::mad(const std::string& name) {
  use_dfcolumn use(raw_column(name));
  return column(name)->mad();
}

double dftable_base::var(const std::string& name, double ddof) {
  use_dfcolumn use(raw_column(name));
  return column(name)->var(ddof);
}

std::shared_ptr<dfcolumn> dftable_base::raw_column(const std::string& name) {
  return dftable_base::column(name);
}

void dftable_base::save(const std::string& dir) {
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
    use_dfcolumn use(raw_column(dt[i].first));
    colstr << dt[i].first << std::endl;
    typestr << dt[i].second << std::endl;
    column(dt[i].first)->save(dir + "/" + dt[i].first);
  }
}

std::vector<char> savetext_helper(const words& ws, size_t num_col,
                                  const std::string& sep) {
  if(ws.starts.size() == 0) return std::vector<char>();
  std::vector<size_t> new_starts;
  auto vecint = concat_words(ws, sep, new_starts);
  auto new_starts_size = new_starts.size();
  auto new_startsp = new_starts.data();
  auto vecintp = vecint.data();
  auto num_row = new_starts_size / num_col;
  if(new_starts_size % num_col != 0)
    throw std::runtime_error("savetext: incorrect number of col or row");
  for(size_t i = 1; i < num_row; i++) {
    vecintp[new_startsp[i * num_col] - 1] = '\n';
  }
  auto vecint_size = vecint.size();
  vecintp[vecint_size-1] = '\n';
  std::vector<char> ret(vecint_size);
  int_to_char(vecintp, vecint_size, ret.data());
  return ret;
}

std::vector<std::pair<std::string, std::string>>
  dftable_base::savetext(const std::string& file,
                         size_t precision,
                         const std::string& datetime_fmt,
                         const std::string& sep,
                         bool quote_and_escape, 
                         const std::string& nullstr) {
  auto table_words = dftable_to_words(*this, precision, datetime_fmt,
                                      quote_and_escape, nullstr);
  table_words.map(savetext_helper, broadcast(num_col()), broadcast(sep))
    .moveto_dvector<char>().savebinary(file);
  return dtypes();
}

colmajor_matrix<float>
dftable_base::to_colmajor_matrix_float(const std::vector<std::string>& cols) {
  auto materialized = select(cols);
  colmajor_matrix<float> ret;
  ret.data = make_node_local_allocate<colmajor_matrix_local<float>>();
  ret.num_col = cols.size();
  ret.num_row = row_size;
  auto bcols = broadcast(cols.size());
  for(size_t i = 0; i < cols.size(); i++) {
    auto c = materialized.column(cols[i]);
    use_dfcolumn use(c);
    auto nl = c->as_dvector_float().moveto_node_local();
    ret.data.mapv(append_column_to_colmajor_matrix<float>, broadcast(i),
                  bcols, nl);
  }
  return ret;
}

colmajor_matrix<double>
dftable_base::to_colmajor_matrix_double(const std::vector<std::string>& cols) {
  auto materialized = select(cols);
  colmajor_matrix<double> ret;
  ret.data = make_node_local_allocate<colmajor_matrix_local<double>>();
  ret.num_col = cols.size();
  ret.num_row = row_size;
  auto bcols = broadcast(cols.size());
  for(size_t i = 0; i < cols.size(); i++) {
    auto c = materialized.column(cols[i]);
    use_dfcolumn use(c);
    auto nl = c->as_dvector_double().moveto_node_local();
    ret.data.mapv(append_column_to_colmajor_matrix<double>, broadcast(i),
                  bcols, nl);
  }
  return ret;
}

rowmajor_matrix<float>
dftable_base::to_rowmajor_matrix_float(const std::vector<std::string>& cols) {
  return to_colmajor_matrix_float(cols).to_rowmajor();
}

rowmajor_matrix<double>
dftable_base::to_rowmajor_matrix_double(const std::vector<std::string>& cols) {
  return to_colmajor_matrix_double(cols).to_rowmajor();
}

void dftable_to_sparse_info::save(const std::string& dir) {
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
  std::string catfile = dir + "/categories";
  std::string numsfile = dir + "/nums";
  std::ofstream colstr;
  std::ofstream catstr;
  std::ofstream numstr;
  colstr.exceptions(std::ofstream::failbit | std::ofstream::badbit);
  catstr.exceptions(std::ofstream::failbit | std::ofstream::badbit);
  numstr.exceptions(std::ofstream::failbit | std::ofstream::badbit);
  colstr.open(colfile.c_str());
  catstr.open(catfile.c_str());
  numstr.open(numsfile.c_str());
  for(size_t i = 0; i < columns.size(); i++) {
    colstr << columns[i] << std::endl;
  }
  for(size_t i = 0; i < categories.size(); i++) {
    catstr << categories[i] << std::endl;
  }
  numstr << num_row << "\n" << num_col << std::endl;
  for(size_t i = 0; i < mapping_tables.size(); i++) {
    mapping_tables[i].save(dir + "/" + categories[i]);
  }
}

void dftable_to_sparse_info::load(const std::string& input) {
  std::string colfile = input + "/columns";
  std::string catfile = input + "/categories";
  std::string numsfile = input + "/nums";
  std::ifstream colstr;
  std::ifstream catstr;
  std::ifstream numstr;
  colstr.open(colfile.c_str());
  catstr.open(catfile.c_str());
  numstr.open(numsfile.c_str());
  std::string tmp;
  columns.clear();
  categories.clear();
  while(std::getline(colstr, tmp)) {
    columns.push_back(tmp);
  }
  while(std::getline(catstr, tmp)) {
    categories.push_back(tmp);
  }
  numstr >> num_row >> num_col;
  mapping_tables.clear();
  size_t cat_size = categories.size();
  mapping_tables.resize(cat_size);
  for(size_t i = 0; i < cat_size; i++) {
    mapping_tables[i].load(input + "/" + categories[i]);
  }
}

ell_matrix<float>
dftable_base::to_ell_matrix_float(dftable_to_sparse_info& info) {
  auto& cols = info.columns;
  auto& cats = info.categories;
  if(cols.size() == 0)
    throw std::runtime_error("to_ell_matrix: no colums to convert");
  auto materialized = select(cols); // cause exception if not applicable
  std::unordered_set<std::string> cats_set(cats.begin(), cats.end());

  ell_matrix<float> ret;
  ret.data = make_node_local_allocate<ell_matrix_local<float>>();
  ret.num_row = info.num_row;
  ret.num_col = info.num_col;
  auto sizes = materialized.num_rows();
  ret.data.mapv(to_ell_matrix_init<float>, broadcast(cols.size()),
                broadcast(sizes), broadcast(ret.num_col));
  size_t current_category = 0;
  size_t current_logical_col = 0;
  for(size_t i = 0; i < cols.size(); i++) {
    auto col = cols[i];
    if(cats_set.find(col) != cats_set.end()) {
      auto tmp1 = materialized.select({col});
      if(tmp1.column(col)->if_contain_nulls())
        throw std::runtime_error
          ("to_ell_matrix: column with NULL is not supported");
      auto tmp2 = info.mapping_tables[current_category++];
      auto col_idx = tmp1.bcast_join(tmp2, frovedis::eq(col, "key")).
        column("col_idx")->as_dvector<size_t>().moveto_node_local();
      ret.data.mapv(to_ell_matrix_addcategory<float>,
                    col_idx, broadcast(i));
      current_logical_col += tmp2.num_row();
    } else {
      auto c = materialized.column(col);
      use_dfcolumn use(c);
      auto val = c->as_dvector_float().moveto_node_local();
      ret.data.mapv(to_ell_matrix_addvalue<float>,
                    val, broadcast(current_logical_col), broadcast(i));
      current_logical_col++;
    }
  }
  return ret;
}

ell_matrix<float>
dftable_base::to_ell_matrix_float(const std::vector<std::string>& cols,
                                  const std::vector<std::string>& cats,
                                  dftable_to_sparse_info& info) {
  if(cols.size() == 0)
    throw std::runtime_error("to_ell_matrix: no colums to convert");
  auto materialized = select(cols); // cause exception if not applicable
  info.columns = cols;
  info.categories = cats;
  info.mapping_tables.clear();
  size_t current_logical_col = 0;
  std::unordered_set<std::string> cats_set(cats.begin(), cats.end());
  // first collect information like size
  for(size_t i = 0; i < cols.size(); i++) {
    if(cats_set.find(cols[i]) != cats_set.end()) {
      auto col = cols[i];
      auto tmp1 = materialized.select({col});
      if(tmp1.column(col)->if_contain_nulls())
        throw std::runtime_error
          ("to_ell_matrix: column with NULL is not supported");
      auto tmp2 = tmp1.group_by({col}).select({col}).sort(col).
        materialize().rename(col, "key").
        append_rowid("col_idx", current_logical_col);
      info.mapping_tables.push_back(tmp2);
      current_logical_col += tmp2.num_row();
    } else {
      current_logical_col++;
    }
  }
  info.num_row = materialized.num_row();
  info.num_col = current_logical_col;
  // next, actually copies the data
  ell_matrix<float> ret;
  ret.data = make_node_local_allocate<ell_matrix_local<float>>();
  ret.num_row = info.num_row;
  ret.num_col = info.num_col;
  auto sizes = materialized.num_rows();
  ret.data.mapv(to_ell_matrix_init<float>, broadcast(cols.size()),
                broadcast(sizes), broadcast(ret.num_col));
  size_t current_category = 0;
  current_logical_col = 0;
  for(size_t i = 0; i < cols.size(); i++) {
    auto col = cols[i];
    if(cats_set.find(col) != cats_set.end()) {
      auto tmp1 = materialized.select({col});
      auto tmp2 = info.mapping_tables[current_category++];
      auto col_idx = tmp1.bcast_join(tmp2, frovedis::eq(col, "key")).
        column("col_idx")->as_dvector<size_t>().moveto_node_local();
      ret.data.mapv(to_ell_matrix_addcategory<float>,
                    col_idx, broadcast(i));
      current_logical_col += tmp2.num_row();
    } else {
      auto c = materialized.column(col);
      use_dfcolumn use(c);
      auto val = c->as_dvector_float().moveto_node_local();
      ret.data.mapv(to_ell_matrix_addvalue<float>,
                    val, broadcast(current_logical_col), broadcast(i));
      current_logical_col++;
    }
  }
  return ret;
}

ell_matrix<double>
dftable_base::to_ell_matrix_double(dftable_to_sparse_info& info) {
  auto& cols = info.columns;
  auto& cats = info.categories;
  if(cols.size() == 0)
    throw std::runtime_error("to_ell_matrix: no colums to convert");
  auto materialized = select(cols); // cause exception if not applicable
  std::unordered_set<std::string> cats_set(cats.begin(), cats.end());

  ell_matrix<double> ret;
  ret.data = make_node_local_allocate<ell_matrix_local<double>>();
  ret.num_row = info.num_row;
  ret.num_col = info.num_col;
  auto sizes = materialized.num_rows();
  ret.data.mapv(to_ell_matrix_init<double>, broadcast(cols.size()),
                broadcast(sizes), broadcast(ret.num_col));
  size_t current_category = 0;
  size_t current_logical_col = 0;
  for(size_t i = 0; i < cols.size(); i++) {
    auto col = cols[i];
    if(cats_set.find(col) != cats_set.end()) {
      auto tmp1 = materialized.select({col});
      if(tmp1.column(col)->if_contain_nulls())
        throw std::runtime_error
          ("to_ell_matrix: column with NULL is not supported");
      auto tmp2 = info.mapping_tables[current_category++];
      auto col_idx = tmp1.bcast_join(tmp2, frovedis::eq(col, "key")).
        column("col_idx")->as_dvector<size_t>().moveto_node_local();
      ret.data.mapv(to_ell_matrix_addcategory<double>,
                    col_idx, broadcast(i));
      current_logical_col += tmp2.num_row();
    } else {
      auto c = materialized.column(col);
      use_dfcolumn use(c);
      auto val = c->as_dvector_double().moveto_node_local();
      ret.data.mapv(to_ell_matrix_addvalue<double>,
                    val, broadcast(current_logical_col), broadcast(i));
      current_logical_col++;
    }
  }
  return ret;
}

ell_matrix<double>
dftable_base::to_ell_matrix_double(const std::vector<std::string>& cols,
                                   const std::vector<std::string>& cats,
                                   dftable_to_sparse_info& info) {
  if(cols.size() == 0)
    throw std::runtime_error("to_ell_matrix: no colums to convert");
  auto materialized = select(cols); // cause exception if not applicable
  info.columns = cols;
  info.categories = cats;
  info.mapping_tables.clear();
  size_t current_logical_col = 0;
  std::unordered_set<std::string> cats_set(cats.begin(), cats.end());
  // first collect information like size
  for(size_t i = 0; i < cols.size(); i++) {
    if(cats_set.find(cols[i]) != cats_set.end()) {
      auto col = cols[i];
      auto tmp1 = materialized.select({col});
      if(tmp1.column(col)->if_contain_nulls())
        throw std::runtime_error
          ("to_ell_matrix: column with NULL is not supported");
      auto tmp2 = tmp1.group_by({col}).select({col}).sort(col).
        materialize().rename(col, "key").
        append_rowid("col_idx", current_logical_col);
      info.mapping_tables.push_back(tmp2);
      current_logical_col += tmp2.num_row();
    } else {
      current_logical_col++;
    }
  }
  info.num_row = materialized.num_row();
  info.num_col = current_logical_col;
  // next, actually copies the data
  ell_matrix<double> ret;
  ret.data = make_node_local_allocate<ell_matrix_local<double>>();
  ret.num_row = info.num_row;
  ret.num_col = info.num_col;
  auto sizes = materialized.num_rows();
  ret.data.mapv(to_ell_matrix_init<double>, broadcast(cols.size()),
                broadcast(sizes), broadcast(ret.num_col));
  size_t current_category = 0;
  current_logical_col = 0;
  for(size_t i = 0; i < cols.size(); i++) {
    auto col = cols[i];
    if(cats_set.find(col) != cats_set.end()) {
      auto tmp1 = materialized.select({col});
      auto tmp2 = info.mapping_tables[current_category++];
      auto col_idx = tmp1.bcast_join(tmp2, frovedis::eq(col, "key")).
        column("col_idx")->as_dvector<size_t>().moveto_node_local();
      ret.data.mapv(to_ell_matrix_addcategory<double>,
                    col_idx, broadcast(i));
      current_logical_col += tmp2.num_row();
    } else {
      auto c = materialized.column(col);
      use_dfcolumn use(c);
      auto val = c->as_dvector_double().moveto_node_local();
      ret.data.mapv(to_ell_matrix_addvalue<double>,
                    val, broadcast(current_logical_col), broadcast(i));
      current_logical_col++;
    }
  }
  return ret;
}

crs_matrix<float>
dftable_base::to_crs_matrix_float(const std::vector<std::string>& cols,
                                  const std::vector<std::string>& cat,
                                  dftable_to_sparse_info& info) {
  return to_ell_matrix_float(cols, cat, info).to_crs_allow_zero();
}

crs_matrix<float>
dftable_base::to_crs_matrix_float(dftable_to_sparse_info& info) {
  return to_ell_matrix_float(info).to_crs_allow_zero();
}

crs_matrix<double>
dftable_base::to_crs_matrix_double(const std::vector<std::string>& cols,
                                   const std::vector<std::string>& cat,
                                   dftable_to_sparse_info& info) {
  return to_ell_matrix_double(cols, cat, info).to_crs_allow_zero();
}

crs_matrix<double>
dftable_base::to_crs_matrix_double(dftable_to_sparse_info& info) {
  return to_ell_matrix_double(info).to_crs_allow_zero();
}

// in typed_dfcolumn_string.cc
std::vector<size_t>
to_contiguous_idx(std::vector<size_t>& idx, std::vector<size_t>& sizes);

// TODO: shrink unused dic
dvector<size_t>
dftable_base::to_dictionary_index(const std::string& col,
                                  std::vector<std::string>& dic) {
  auto c = column(col);
  use_dfcolumn use(c);
  if(c->dtype() == "string") {
    auto string_column =
      std::dynamic_pointer_cast<typed_dfcolumn<std::string>>(c);
    auto dicsizes =
      string_column->dic_idx->viewas_dvector<std::string>().sizes();
    dic = string_column->dic_idx->viewas_dvector<std::string>().gather();
    return string_column->val.map(to_contiguous_idx, broadcast(dicsizes)).
      moveto_dvector<size_t>();
  } else if (c->dtype() == "dic_string") {
    auto dic_string_column =
      std::dynamic_pointer_cast<typed_dfcolumn<dic_string>>(c);
    auto dict_words = dic_string_column->dic->decompress();
    dic = words_to_vector_string(dict_words);
    return dic_string_column->val.as_dvector<size_t>();
  } else {
    throw std::runtime_error
      ("to_dictionary_index can be used only for string or dic_string column");
  } 
}

dvector<size_t>
dftable_base::to_dictionary_index(const std::string& col,
                                  words& dic) {
  auto c = column(col);
  use_dfcolumn use(c);
  if(c->dtype() == "string") {
    auto string_column =
      std::dynamic_pointer_cast<typed_dfcolumn<std::string>>(c);
    auto dicsizes =
      string_column->dic_idx->viewas_dvector<std::string>().sizes();
    auto sdic = string_column->dic_idx->viewas_dvector<std::string>().gather();
    dic = vector_string_to_words(sdic);
    return string_column->val.map(to_contiguous_idx, broadcast(dicsizes)).
      moveto_dvector<size_t>();
  } else if (c->dtype() == "dic_string") {
    auto dic_string_column =
      std::dynamic_pointer_cast<typed_dfcolumn<dic_string>>(c);
    dic = dic_string_column->dic->decompress();
    return dic_string_column->val.as_dvector<size_t>();
  } else {
    throw std::runtime_error
      ("to_dictionary_index can be used only for string or dic_string column");
  } 
}

std::shared_ptr<dfcolumn> dftable_base::column(const std::string& name) {
  auto ret = col.find(name);
  if(ret == col.end()) throw std::runtime_error("no such column: " + name);
  else return ret->second;
}
 
node_local<std::vector<size_t>> dftable_base::get_local_index() {
  if(col.size() == 0)
    throw std::runtime_error("get_local_index(): no columns");
  else {
    auto nlsizes = make_node_local_scatter(row_sizes);
    return nlsizes.map(+[](size_t size){
        std::vector<size_t> ret(size);
        auto retp = ret.data();
        for(size_t i = 0; i < size; i++) retp[i] = i;
        return ret;
      });
  }
}

void dftable_base::debug_print() {
  for(auto& cs: dftable_base::columns()) {
    std::cout << "column: " << cs << std::endl;
    dftable_base::column(cs)->debug_print();
  }
  std::cout << "row_size: " << row_size << std::endl;
  std::cout << "row_sizes: ";
  for(auto i: row_sizes) std::cout << i << " ";
  std::cout << std::endl;
}

std::vector<size_t>
extract_sliced_idx(size_t my_st, size_t my_end,
                   size_t g_st, size_t g_end, size_t step) {
  size_t lb = 0, ub = 0;
  auto imax = std::numeric_limits<size_t>::max();
  if (my_st > g_st && my_st > g_end)         { lb = ub = imax; } // slice not possible
  else if (my_st < g_st && my_end < g_st)    { lb = ub = imax; } // slice not possible
  else if (my_st <= g_st && my_end >= g_end) { lb = g_st; ub = g_end; }
  else if (my_st <= g_st && my_end < g_end)  { lb = g_st; ub = my_end; }
  else if (my_st > g_st && my_end <= g_end)  { lb = my_st; ub = my_end; }
  else if (my_st > g_st && my_end >= g_end)  { lb = my_st; ub = g_end; }
  else { // should never occur
    std::string msg = "[" + STR(g_st) + " : " + STR(g_end) + "] => ";
    msg += "[" + STR(my_st) + " : " + STR(my_end) + "]";
    REPORT_ERROR(INTERNAL_ERROR,
    "extract_sliced_idx failed to perform bound check: " + msg + "! Report Bug!\n");
  }
  std::vector<size_t> ret;
  if (lb != imax) { // if slice is possible
    auto traversed = lb - g_st;
    if (traversed % step) lb += step - (traversed % step); // adjust lb as per 'step'
    ret = vector_arrange<size_t>(lb - my_st, ub - my_st, step);
  }
  /*
  auto myrank = get_selfid();
  std::cout << "[" << myrank << "] my_st: " << my_st << "; my_end " << my_end << std::endl;
  std::cout << "[" << myrank << "] lb: " << lb << "; ub: " << ub << std::endl;
  show("ret: ", ret);
  */
  return ret;
}

dftable make_sliced_dftable(dftable_base& t, size_t st, size_t end, size_t step) {
  auto nrow = t.num_row();
  end = std::min(end, nrow); // allowed end to be larger than nrow
  auto idx = make_node_local_allocate<std::vector<size_t>>();
  if (nrow && st < end) {
    auto sizes = t.num_rows();  //show("sizes: ", sizes);
    auto nproc = sizes.size();
    std::vector<size_t> myst(nproc), myend(nproc);
    myst[0] = 0;
    for(size_t i = 1; i < nproc; ++i) myst[i] = myst[i - 1] + sizes[i - 1];
    for(size_t i = 0; i < nproc; ++i) myend[i] = myst[i] + sizes[i];
    auto mst = make_node_local_scatter(myst);
    auto mend = make_node_local_scatter(myend);
    idx = mst.map(extract_sliced_idx, mend, broadcast(st), broadcast(end), broadcast(step));
  }
  return filtered_dftable(t, std::move(idx)).materialize();
}

node_local<std::vector<size_t>>
get_rows_with_nulls_ge_threshold(node_local<std::vector<std::vector<size_t>>>& null_vec_local,
                                 size_t threshold) {
  return null_vec_local.map(+[](std::vector<std::vector<size_t>>& null_vec,
                                size_t thr) {
       auto nsz = null_vec.size();
       if (nsz == 0) return std::vector<size_t>();
       else if (nsz == 1) return null_vec[0];
       else {
         // vectors in null_vec must be sorted
         auto merged = set_merge(null_vec[0], null_vec[1]);
         for(size_t i = 2; i < nsz; ++i) merged = set_merge(merged, null_vec[i]);
         auto sep = set_separate(merged);
         auto n_unique = sep.size() - 1;
         std::vector<size_t> countvec(n_unique);
         auto sptr = sep.data();
         auto cptr = countvec.data();
         for(size_t i = 0; i < n_unique; ++i) cptr[i] = sptr[i + 1] - sptr[i];
         auto targets = vector_find_ge(countvec, thr);
         auto tsz = targets.size();
         std::vector<size_t> ret(tsz);
         auto rptr = ret.data();
         auto tptr = targets.data();
         auto mptr = merged.data();
#pragma _NEC ivdep
//#pragma _NEC vovertake
//#pragma _NEC vob
         for(size_t i = 0; i < tsz; ++i) rptr[i] = mptr[sptr[tptr[i]]];
         return ret;
       }
    }, broadcast(threshold));
}

node_local<std::vector<size_t>>
count_nulls_by_rows_impl(node_local<std::vector<std::vector<size_t>>>& null_vec_local,
                         std::vector<size_t> sizes) {
  auto mysz = make_node_local_scatter(sizes);
  return null_vec_local.map(+[](std::vector<std::vector<size_t>>& null_vec,
                                size_t mysz) {
       auto nvecsz = null_vec.size();
       std::vector<size_t> nullpos, countvec;
       if (nvecsz == 0); // no-nulls, do-nothing
       else if (nvecsz == 1) {
         nullpos.swap(null_vec[0]);
         countvec = vector_ones<size_t>(nullpos.size());
       }
       else {
         // vectors in null_vec must be sorted
         auto merged = set_merge(null_vec[0], null_vec[1]);
         for(size_t i = 2; i < nvecsz; ++i) merged = set_merge(merged, null_vec[i]);
         auto sep = set_separate(merged);
         auto n_unique = sep.size() - 1;
         countvec.resize(n_unique);
         nullpos.resize(n_unique);
         auto sptr = sep.data();
         auto cptr = countvec.data();
         auto nptr = nullpos.data();
         auto mptr = merged.data();
         for(size_t i = 0; i < n_unique; ++i) {
           cptr[i] = sptr[i + 1] - sptr[i];
           nptr[i] = mptr[sptr[i]];
         }
       }
       auto ret = vector_zeros<size_t>(mysz);
       auto rptr = ret.data();
       auto cptr = countvec.data();
       auto nptr = nullpos.data();
       auto nsz = nullpos.size();
#pragma _NEC ivdep
//#pragma _NEC vovertake
//#pragma _NEC vob
       for(size_t i = 0; i < nsz; ++i) rptr[nptr[i]] = cptr[i];
       return ret;
    }, mysz);
}

node_local<std::vector<std::vector<size_t>>>
get_nulls_by_cols(dftable_base& df, const std::vector<std::string>& targets) {
  auto ncol = targets.size();
  auto null_vec_local = make_node_local_allocate<std::vector<std::vector<size_t>>>();
  for(size_t i = 0; i < ncol; ++i) {
    auto dfcol = df.column(targets[i]);
    use_dfcolumn use(dfcol);
    if (dfcol->if_contain_nulls()) {
      null_vec_local.mapv(+[](std::vector<std::vector<size_t>>& null_vec,
                              std::vector<size_t>& nulls) {
                                null_vec.push_back(std::move(nulls));
                              }, dfcol-> get_nulls());
    }
  }
  return null_vec_local;
}

dftable dftable_base::count_nulls(int axis, bool with_index) {
  dftable ret;
  auto cols = columns();
  std::string index = "index";
  if (with_index) {
    index = cols[0];
    cols.erase(cols.begin(), cols.begin() + 1);
  }
  if (axis == 1) { 
    auto null_vec_local = get_nulls_by_cols(*this, cols);
    auto count = count_nulls_by_rows_impl(null_vec_local, num_rows())
                            .moveto_dvector<size_t>();
    ret.append_column("count", std::move(count));
    if (with_index) { 
      auto icol = column(index);
      use_dfcolumn use(icol);
      ret.append_column(index, icol).change_col_position(index, 0); // set_index()
    }
    else ret.prepend_rowid<long>(index);
  }
  else if (axis == 0) {
    auto ncol = cols.size();
    auto nrow = num_row();
    std::vector<size_t> count(ncol);
    auto cptr = count.data();
    for(size_t i = 0; i < ncol; ++i) {
      auto col = column(cols[i]);
      use_dfcolumn use(col);
      cptr[i] = nrow - col->count();
    } 
    ret.append_column("index", make_dvector_scatter(cols));
    ret.append_column("count", make_dvector_scatter(count));
  }
  else REPORT_ERROR(USER_ERROR, "count_nulls: axis can be either 0 or 1!\n");
  return ret;
}

dftable drop_nulls_by_rows_with_threshold_impl(dftable_base& df,
                                   size_t threshold,
                                   const std::vector<std::string>& targets) {
  dftable ret;
  auto null_vec_local = get_nulls_by_cols(df, targets);
  if (null_vec_local.viewas_dvector<std::vector<size_t>>().size() >= threshold) {
    auto drop_targets = get_rows_with_nulls_ge_threshold(null_vec_local, threshold);
    auto ret_idx = df.get_local_index().map(set_difference<size_t>, 
                                            drop_targets);
    ret = filtered_dftable(df, std::move(ret_idx)).materialize();
  } else {
    ret = df.need_materialize() ? df.materialize() : df;
  }
  return ret;
}

dftable drop_nulls_by_rows_any_impl(dftable_base& df,
                                    const std::vector<std::string>& targets) {
  auto tsz = targets.size();
  if (tsz == 0) return df.materialize(); // empty dftable case
  auto fdf = df.filter(is_not_null(targets[0]));
  for(size_t i = 1; i < tsz; ++i) fdf = fdf.filter(is_not_null(targets[i]));
  return fdf.materialize();
}

dftable drop_nulls_by_rows_all_impl(dftable_base& df,
                                    const std::vector<std::string>& targets) {
  auto tsz = targets.size();
  if (tsz == 0) return df.materialize(); // empty dftable case
  auto opt = is_null(targets[0]);
  for(size_t i = 1; i < tsz; ++i) opt = and_op(opt, is_null(targets[i]));
  return df.filter(not_op(opt)).materialize();
}

dftable dftable_base::drop_nulls_by_rows(const std::string& how,
                                         const std::vector<std::string>& targets) {
  require(how == "any" || how == "all", "drop nulls using how: '" + how + "' is not supported!\n");
  dftable ret;
  std::vector<std::string> cols = targets;
  if (targets.empty()) cols = columns();
  if (how == "any") ret = drop_nulls_by_rows_any_impl(*this, cols);
  else ret = drop_nulls_by_rows_all_impl(*this, cols);
  return ret;
}

dftable dftable_base::drop_nulls_by_rows(size_t threshold,
                                         const std::vector<std::string>& targets) {
  std::vector<std::string> cols = targets;
  if (targets.empty()) cols = columns();
  return drop_nulls_by_rows_with_threshold_impl(*this, threshold, cols);
}

dftable drop_nulls_by_cols_impl(dftable_base& df,
                                dftable_base& sliced_df, // drop would be in-place
                                const std::string& how,
                                size_t threshold) {
  auto cols = sliced_df.columns();
  auto nrow = sliced_df.num_row();
  auto ncol = cols.size();
  dftable ret;
  for(size_t i = 0; i < ncol; ++i) {
    auto cname = cols[i];
    auto c = sliced_df.column(cname);
    use_dfcolumn use(c);
    auto nullsz = nrow - c->count(); 
    bool to_drop = false;
    if (threshold == std::numeric_limits<size_t>::max()) { // threshold undefined
      auto is_all = (how == "all");
      to_drop = (!is_all && nullsz > 0) || (is_all && nullsz == nrow);
    } else {
      to_drop = (nullsz >= threshold);
    }
    if (!to_drop) {
      auto actual_c = df.column(cname);
      use_dfcolumn use(actual_c);
      ret.append_column(cname, actual_c); // ret would have same distribution as in 'df'
    }
  }
  return ret;
}

dftable dftable_base::drop_duplicates(const std::vector<std::string>& cols,
                                      const std::string& keep) {
  if(keep != "first" and keep != "last")
    REPORT_ERROR(USER_ERROR,
       "drop_duplicates: either first or last value can be kept!\n");

  // assuming these names can not be in this->columns()
  std::string tmpid = "tmp_index_", r_key = "t_join_r_key_";
  std::shared_ptr<dfaggregator> agg;
  if (keep == "first") agg = min_as(tmpid, r_key);
  else                 agg = max_as(tmpid, r_key);
  dftable left = need_materialize() ? materialize() : *this;
  left.append_rowid(tmpid); 
  auto right = left.group_by(cols).select(cols, {agg}).select({r_key});
  auto ret = left.bcast_join(right, eq(tmpid, r_key));
  return ret.select(columns());
}

sorted_dftable 
multiple_sort(dftable_base& df,
              const std::vector<std::string>& targets,
              bool is_desc) {
  auto size = targets.size();
  checkAssumption(size > 0);
  auto cname = targets[size - 1];
  auto ret = is_desc ? df.sort_desc(cname) : df.sort(cname);
  for (int i = size - 2; i >= 0; --i) {
    auto cname = targets[i];
    ret = is_desc ? ret.sort_desc(cname) : ret.sort(cname);
  }
  return ret;
} 

dftable ksort_impl(dftable_base& df, 
                   int k, const std::vector<std::string>& targets,
                   const std::string& keep, bool is_desc) {
  if (targets.empty()) return df.materialize();
  auto tmp = df.drop_nulls_by_rows("any", {targets[0]}); 
  dftable sorted;
  if (keep == "last") {
    std::string tid = "__tmp_id__";
    auto last_ordered = tmp.append_rowid(tid).sort_desc(tid);
    sorted = multiple_sort(last_ordered, targets, is_desc).drop(tid).materialize();
  }
  else sorted = multiple_sort(tmp, targets, is_desc).materialize();

  int actual_k = k;
  if (keep == "all") { 
    std::string cnt = "__tmp_count__";
    // TODO: confirm if goupby internally sorts table in ascending order
    auto grouped = tmp.group_by(targets) 
                      .select(targets, {count_as(targets[0], cnt)});
    if (grouped.num_row() != tmp.num_row()) { // if there are duplicates
      auto target_group = is_desc ? grouped.tail(k) : grouped.head(k);
      if (target_group.max<size_t>(cnt) != 1) { // duplicates in target group
        auto countvec = target_group.as_dvector<size_t>(cnt).gather();
        if (is_desc) countvec = vector_reverse(countvec);
        auto psum = prefix_sum(countvec);
        auto ub = std::lower_bound(psum.begin(), psum.end(), k);
        if (ub == psum.end()) actual_k = df.num_row();
        else                  actual_k = *ub;
      }
    }
  }
  return sorted.head(actual_k);
}

dftable dftable_base::nlargest(int n, const std::vector<std::string>& targets,
                               const std::string& keep) {
  require(n >= 0, "nlargest: expected a positive integer for 'n'!\n");
  require(keep == "first" || keep == "last" || keep == "all", 
  "nlargest: supported 'keep' can be all, first or last!\n");
  return ksort_impl(*this, n, targets, keep, true);
}

dftable dftable_base::nsmallest(int n, const std::vector<std::string>& targets,
                                const std::string& keep) {
  require(n >= 0, "nsmallest: expected a positive integer for 'n'!\n");
  require(keep == "first" || keep == "last" || keep == "all", 
  "nsmallest: supported 'keep' can be all, first or last!\n");
  return ksort_impl(*this, n, targets, keep, false);
}

// ---------- for dftable ----------

// reused for other type of tables
void drop_impl(const std::string& name,
               std::map<std::string, std::shared_ptr<dfcolumn>>& col,
               std::vector<std::string>& col_order) {
  col.erase(name);
  col_order.erase(std::remove(col_order.begin(), col_order.end(), name),
                  col_order.end());
}

dftable_base* dftable_base::drop_cols(const std::vector<std::string>& cols) { 
  for(auto& c: cols) drop_impl(c, col, col_order);
  return this;
}

dftable& dftable::drop(const std::string& name) {
  drop_impl(name, col, col_order);
  return *this;
}

dftable_base* dftable::drop_cols(const std::vector<std::string>& cols) { 
  for(auto& c: cols) drop(c);
  return this;
}

void rename_impl(const std::string& name, const std::string& name2,
                 std::map<std::string, std::shared_ptr<dfcolumn>>& col,
                 std::vector<std::string>& col_order) {
  auto ret = col.find(name);
  if(ret == col.end()) throw std::runtime_error("no such column: " + name);
  auto name_col = ret->second;
  if(col.find(name2) != col.end())
    throw std::runtime_error("column already exists: " + name2);
  col.erase(name);
  col[name2] = name_col;
  for(size_t i = 0; i < col_order.size(); i++) {
    if(col_order[i] == name) {
      col_order[i] = name2;
      break;
    }
  }
}

dftable& dftable::rename(const std::string& name, const std::string& name2) {
  rename_impl(name, name2, col, col_order);
  return *this;
}

template <>
dftable& dftable::append_column(const std::string& name,
                                dvector<std::string>& d,
                                bool check_null_like) {
  if(col.find(name) != col.end())
    throw std::runtime_error(std::string("append_column: column '") 
                             + name + "' already exists");
  std::shared_ptr<dfcolumn> c;
  if(col.size() == 0) {
    row_size = d.size();
    d.align_block();
    row_sizes = d.sizes();
  } else {
    if(d.size() != row_size)
      throw std::runtime_error(name + ": different size of columns");
    d.align_as(row_sizes);
  }
  auto d_nloc = d.as_node_local(); // d: lvalue
  auto nulls = make_node_local_allocate<std::vector<size_t>>();
  if(check_null_like) nulls = d_nloc.map(get_null_like_positions<std::string>);
  std::string nullstr = "NULL";
  auto words = d_nloc.map(dfcolumn_string_as_words_helper,nulls,
                          broadcast(nullstr));
  c = std::make_shared<typed_dfcolumn<dic_string>>(std::move(words),
                                                   std::move(nulls));
  c->spill();
  col.insert(std::make_pair(name, c));
  col_order.push_back(name);
  return *this;
}

template <>
dftable& dftable::append_column(const std::string& name,
                                dvector<std::string>&& d,
                                bool check_null_like) {
  if(col.find(name) != col.end())
    throw std::runtime_error(std::string("append_column: column '") 
                             + name + "' already exists");
  std::shared_ptr<dfcolumn> c;
  if(col.size() == 0) {
    row_size = d.size();
    d.align_block();
    row_sizes = d.sizes();
  } else {
    if(d.size() != row_size)
      throw std::runtime_error(name + ": different size of columns");
    d.align_as(row_sizes);
  }
  auto d_nloc = d.moveto_node_local(); // d: rvalue
  auto nulls = make_node_local_allocate<std::vector<size_t>>();
  if(check_null_like) nulls = d_nloc.map(get_null_like_positions<std::string>);
  std::string nullstr = "NULL";
  auto words = d_nloc.map(dfcolumn_string_as_words_helper,nulls,
                          broadcast(nullstr));
  c = std::make_shared<typed_dfcolumn<dic_string>>(std::move(words),
                                                   std::move(nulls));
  c->spill();
  col.insert(std::make_pair(name, c));
  col_order.push_back(name);
  return *this;
}

dftable& dftable::append_datetime_column(const std::string& name, 
                                         dvector<datetime_t>& d,
                                         bool check_null_like) {
  if(col.find(name) != col.end())
    throw std::runtime_error(std::string("append_column: column '") 
                             + name + "' already exists");
  std::shared_ptr<dfcolumn> c;
  if(col.size() == 0) {
    row_size = d.size();
    d.align_block();
    row_sizes = d.sizes();
  } else {
    if(d.size() != row_size)
      throw std::runtime_error(name + ": different size of columns");
    d.align_as(row_sizes);
  }
  if(check_null_like) {
    auto d_nloc = d.as_node_local(); // d: lvalue
    auto nulls = d_nloc.map(get_null_like_positions<datetime_t>);
    c = std::make_shared<typed_dfcolumn<datetime>>(std::move(d_nloc), 
                                                   std::move(nulls));
  }
  else {
    c = std::make_shared<typed_dfcolumn<datetime>>(d);
  }
  c->spill();
  col.insert(std::make_pair(name, c));
  col_order.push_back(name);
  return *this;
}

dftable& dftable::append_datetime_column(const std::string& name, 
                                         dvector<datetime_t>&& d,
                                         bool check_null_like) {
  if(col.find(name) != col.end())
    throw std::runtime_error(std::string("append_column: column '") 
                             + name + "' already exists");
  std::shared_ptr<dfcolumn> c;
  if(col.size() == 0) {
    row_size = d.size();
    d.align_block();
    row_sizes = d.sizes();
  } else {
    if(d.size() != row_size)
      throw std::runtime_error(name + ": different size of columns");
    d.align_as(row_sizes);
  }
  if(check_null_like) {
    auto d_nloc = d.moveto_node_local(); // d: rvalue
    auto nulls = d_nloc.map(get_null_like_positions<datetime_t>);
    c = std::make_shared<typed_dfcolumn<datetime>>(std::move(d_nloc), 
                                                   std::move(nulls));
  }
  else {
    c = std::make_shared<typed_dfcolumn<datetime>>(std::move(d));
  }
  c->spill();
  col.insert(std::make_pair(name, c));
  col_order.push_back(name);
  return *this;
}

dftable& dftable::append_dic_string_column(const std::string& name,
                                           dvector<std::string>& d,
                                           bool check_null_like) {
  return append_column(name, d, check_null_like);
}

dftable& dftable::append_dic_string_column(const std::string& name,
                                           dvector<std::string>&& d,
                                           bool check_null_like) {
  return append_column(name, std::move(d), check_null_like);
}

dftable& dftable::append_raw_string_column(const std::string& name,
                                           dvector<std::string>& d,
                                           bool check_null_like) {
  if(col.find(name) != col.end())
    throw std::runtime_error(std::string("append_column: column '") 
                             + name + "' already exists");
  std::shared_ptr<dfcolumn> c;
  if(col.size() == 0) {
    row_size = d.size();
    d.align_block();
    row_sizes = d.sizes();
 } else {
    if(d.size() != row_size)
      throw std::runtime_error(name + ": different size of columns");
    d.align_as(row_sizes);
  }
  auto d_nloc = d.as_node_local(); // d: lvalue
  auto nulls = make_node_local_allocate<std::vector<size_t>>();
  if(check_null_like) nulls = d_nloc.map(get_null_like_positions<std::string>);
  std::string nullstr = "NULL";
  auto words = d_nloc.map(dfcolumn_string_as_words_helper,nulls,
                          broadcast(nullstr));
  c = std::make_shared<typed_dfcolumn<raw_string>>(std::move(words),
                                                   std::move(nulls));
  c->spill();
  col.insert(std::make_pair(name, c));
  col_order.push_back(name);
  return *this;
}

dftable& dftable::append_raw_string_column(const std::string& name,
                                           dvector<std::string>&& d,
                                           bool check_null_like) {
  if(col.find(name) != col.end())
    throw std::runtime_error(std::string("append_column: column '") 
                             + name + "' already exists");
  std::shared_ptr<dfcolumn> c;
  if(col.size() == 0) {
    row_size = d.size();
    d.align_block();
    row_sizes = d.sizes();
  } else {
    if(d.size() != row_size)
      throw std::runtime_error(name + ": different size of columns");
    d.align_as(row_sizes);
  }
  auto d_nloc = d.moveto_node_local(); // d: rvalue
  auto nulls = make_node_local_allocate<std::vector<size_t>>();
  if(check_null_like) nulls = d_nloc.map(get_null_like_positions<std::string>);
  std::string nullstr = "NULL";
  auto words = d_nloc.map(dfcolumn_string_as_words_helper,nulls,
                          broadcast(nullstr));
  c = std::make_shared<typed_dfcolumn<raw_string>>(std::move(words),
                                                   std::move(nulls));
  c->spill();
  col.insert(std::make_pair(name, c));
  col_order.push_back(name);
  return *this;
}

dftable& dftable::append_string_column(const std::string& name, 
                                       dvector<std::string>& d,
                                       bool check_null_like) {
  if(col.find(name) != col.end())
    throw std::runtime_error(std::string("append_column: column '") 
                             + name + "' already exists");
  std::shared_ptr<dfcolumn> c;
  if(col.size() == 0) {
    row_size = d.size();
    d.align_block();
    row_sizes = d.sizes();
  } else {
    if(d.size() != row_size)
      throw std::runtime_error(name + ": different size of columns");
    d.align_as(row_sizes);
  }
  if(check_null_like) {
    auto d_nloc = d.as_node_local(); // d: lvalue
    auto nulls = d_nloc.map(get_null_like_positions<std::string>);
    c = std::make_shared<typed_dfcolumn<std::string>>(std::move(d_nloc), 
                                                      std::move(nulls));
  }
  else {
    c = std::make_shared<typed_dfcolumn<std::string>>(d);
  }
  c->spill();
  col.insert(std::make_pair(name, c));
  col_order.push_back(name);
  return *this;
}

dftable& dftable::append_string_column(const std::string& name, 
                                       dvector<std::string>&& d,
                                       bool check_null_like) {
  if(col.find(name) != col.end())
    throw std::runtime_error(std::string("append_column: column '") 
                             + name + "' already exists");
  std::shared_ptr<dfcolumn> c;
  if(col.size() == 0) {
    row_size = d.size();
    d.align_block();
    row_sizes = d.sizes();
  } else {
    if(d.size() != row_size)
      throw std::runtime_error(name + ": different size of columns");
    d.align_as(row_sizes);
  }
  if(check_null_like) {
    auto d_nloc = d.moveto_node_local(); // d: rvalue
    auto nulls = d_nloc.map(get_null_like_positions<std::string>);
    c = std::make_shared<typed_dfcolumn<std::string>>(std::move(d_nloc), 
                                                      std::move(nulls));
  }
  else {
    c = std::make_shared<typed_dfcolumn<std::string>>(std::move(d));
  }
  c->spill();
  col.insert(std::make_pair(name, c));
  col_order.push_back(name);
  return *this;
}

// defined in typed_dfcolumn_raw_string.cc
void compressed_words_align_as(node_local<compressed_words>& comp_words,
                               const std::vector<size_t>& mysizes,
                               const std::vector<size_t>& dst);

struct is_same_value {
  is_same_value(size_t v) : v(v) {}
  int operator()(size_t a) const {return a == v;}
  size_t v;
  SERIALIZE(v)
};

dftable& dftable::append_dic_string_column(const std::string& name,
                                           node_local<words>& w, 
                                           bool check_null_like) {
  if(col.find(name) != col.end())
    throw std::runtime_error(std::string("append_column: column '") 
                             + name + "' already exists");
  auto cw = w.map(make_compressed_words);
  auto sizes = w.map(+[](words& ws){return ws.starts.size();}).gather();
  size_t total_size = 0;
  auto sizesp = sizes.data();
  auto sizes_size = sizes.size();
  for(size_t i = 0; i < sizes_size; i++) total_size += sizesp[i];
  std::vector<size_t> dst_sizes;
  if(col.size() == 0) { 
    row_size = total_size;
    // align_block
    dst_sizes.resize(sizes_size);
    auto dst_sizesp = dst_sizes.data();
    auto sz = total_size;
    size_t each = ceil_div(sz, sizes_size);
    for(size_t i = 0; i < sizes_size; i++) {
      if(sz > each) {
        dst_sizesp[i] = each;
        sz -= each;
      } else {
        dst_sizesp[i] = sz;
        break;
      }
    }
    compressed_words_align_as(cw, sizes, dst_sizes);
    row_sizes = dst_sizes;
  } else {
    if(total_size != row_size)
      throw std::runtime_error(name + ": different size of columns");
    compressed_words_align_as(cw, sizes, row_sizes);
  }
  auto dic_string_column = 
    std::make_shared<typed_dfcolumn<dic_string>>(std::move(cw));
  if(check_null_like) {
    words nullw;
    nullw.chars = char_to_int(std::string("NULL"));
    nullw.starts = {0};
    nullw.lens = {4};
    auto nullcw = make_compressed_words(nullw);
    auto nullval = dic_string_column->dic->lookup(nullcw);
    if(nullval[0] != std::numeric_limits<size_t>::max()) {
      dic_string_column->nulls = dic_string_column->val.map
        (+[](std::vector<size_t>& val, size_t nullval) {
          return find_condition(val, is_same_value(nullval));
        }, broadcast(nullval[0]));
      dic_string_column->nulls.mapv(reset_null_val<size_t>,
                                    dic_string_column->val);
      dic_string_column->contain_nulls_check();
    }
  }
  std::shared_ptr<dfcolumn> c = dic_string_column;
  c->spill();
  col.insert(std::make_pair(name, c));
  col_order.push_back(name);
  return *this;
}

struct is_same_comp_word {
  is_same_comp_word(uint64_t v) : v(v) {}
  int operator()(uint64_t a) const {return a == v;}
  uint64_t v;
  SERIALIZE(v)
};

dftable& dftable::append_raw_string_column(const std::string& name,
                                           node_local<words>& w, 
                                           bool check_null_like) {
  if(col.find(name) != col.end())
    throw std::runtime_error(std::string("append_column: column '") 
                             + name + "' already exists");
  auto cw = w.map(make_compressed_words);
  auto sizes = w.map(+[](words& ws){return ws.starts.size();}).gather();
  size_t total_size = 0;
  auto sizesp = sizes.data();
  auto sizes_size = sizes.size();
  for(size_t i = 0; i < sizes_size; i++) total_size += sizesp[i];
  std::vector<size_t> dst_sizes;
  if(col.size() == 0) { 
    row_size = total_size;
    // align_block
    dst_sizes.resize(sizes_size);
    auto dst_sizesp = dst_sizes.data();
    auto sz = total_size;
    size_t each = ceil_div(sz, sizes_size);
    for(size_t i = 0; i < sizes_size; i++) {
      if(sz > each) {
        dst_sizesp[i] = each;
        sz -= each;
      } else {
        dst_sizesp[i] = sz;
        break;
      }
    }
    compressed_words_align_as(cw, sizes, dst_sizes);
    row_sizes = dst_sizes;
  } else {
    if(total_size != row_size)
      throw std::runtime_error(name + ": different size of columns");
    compressed_words_align_as(cw, sizes, row_sizes);
  }
  auto raw_string_column = 
    std::make_shared<typed_dfcolumn<raw_string>>(std::move(cw));
  if(check_null_like) {
    raw_string_column->nulls = raw_string_column->comp_words.map
      (+[](compressed_words& comp_words){
        words nullw;
        nullw.chars = char_to_int(std::string("NULL"));
        nullw.starts = {0};
        nullw.lens = {4};
        auto nullcw = make_compressed_words(nullw);
        auto nullcw0 = nullcw.cwords[0];
        if(comp_words.lens[0] != 1) return std::vector<size_t>();
        else {
          auto len = comp_words.lens_num[0];
          auto pos =  find_condition(comp_words.cwords.data(), len,
                                     is_same_comp_word(nullcw0));
          auto pos_size = pos.size();
          std::vector<size_t> ret(pos_size);
          auto posp = pos.data();
          auto retp = ret.data();
          auto orderp = comp_words.order.data();
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
          for(size_t i = 0; i < pos_size; i++) {
            retp[i] = orderp[posp[i]];
          }
          return ret;
        }
      });
    raw_string_column->contain_nulls_check();
  }
  std::shared_ptr<dfcolumn> c = raw_string_column;
  c->spill();
  col.insert(std::make_pair(name, c));
  col_order.push_back(name);
  return *this;
}

dftable& dftable::append_column(const std::string& name,
                                const std::shared_ptr<dfcolumn>& c) {
  if(col.find(name) != col.end())
    throw std::runtime_error(std::string("append_column: column '") 
                             + name + "' already exists");
  if(col.size() == 0) {
    use_dfcolumn use(c);
    row_size = c->size();
    row_sizes = c->sizes();
  } else {
    use_dfcolumn use(c);
    if(c->size() != row_size)
      throw std::runtime_error(name + ": different size of columns");
  }
  c->spill();
  col.insert(std::make_pair(name, c));
  col_order.push_back(name);
  return *this;
}

dftable& dftable::datetime_extract(datetime_type type,
                                   const std::string& src_column,
                                   const std::string& to_append_column) {
  auto c = column(src_column);
  if(c->dtype() != "datetime") {
    throw std::runtime_error
      ("datetime_extract can be used only for datetime column");
  } else {
    use_dfcolumn use(c);
    auto retc = c->datetime_extract(type);
    append_column(to_append_column, retc);
    return *this;
  }
}

dftable& dftable::append_dictionary_index(const std::string& src_column,
                                          const std::string& to_append_column,
                                          std::vector<std::string>& dic) {
  auto dv = to_dictionary_index(src_column, dic);
  append_column(to_append_column, dv, true);
  return *this;
}

dftable& dftable::append_dictionary_index(const std::string& src_column,
                                          const std::string& to_append_column,
                                          words& dic) {
  auto dv = to_dictionary_index(src_column, dic);
  append_column(to_append_column, dv, true);
  return *this;
}

std::vector<std::pair<std::string, size_t>>
add_contiguous_idx(std::vector<std::string>& vs, std::vector<size_t>& sizes) {
  int self = get_selfid();
  size_t toadd = 0;
  size_t* sizesp = sizes.data();
  for(size_t i = 0; i < self; i++) toadd += sizesp[i];
  size_t size = vs.size();
  std::vector<std::pair<std::string, size_t>> ret(size);
  std::pair<std::string, size_t>* retp = ret.data();
  std::string* vsp = vs.data();
  for(size_t i = 0; i < size; i++) {
    retp[i].first = vsp[i];
    retp[i].second = i + toadd;
  }
  return ret;
}

dftable& dftable::call_function(const std::shared_ptr<dffunction>& func) {
  auto as = func->get_as();
  if(col.find(as) != col.end())
    throw std::runtime_error("call_function: same column name already exists");
  use_dfcolumn use(func->columns_to_use(*this));  
  auto c = func->execute(*this);
  append_column(as, c);
  return *this;
}

void prepare_string_load(my_map<std::string, std::vector<size_t>>& group,
                         my_map<std::string, size_t>& dic,
                         std::vector<std::string>& dic_idx,
                         std::vector<size_t>& cont_idx,
                         std::vector<size_t>& global_idx) {
  size_t selfid = static_cast<size_t>(get_selfid());
  size_t nodeinfo = selfid << DFNODESHIFT;
  auto size = group.size();
  dic_idx.resize(size);
  if(selfid == 0) {
    cont_idx.resize(size+1);
    global_idx.resize(size+1);
  } else {
    cont_idx.resize(size);
    global_idx.resize(size);
  }
  size_t i = 0;
  for(auto it = group.begin(); it != group.end(); ++it, ++i) {
    auto str = it->first;
    dic.insert(std::make_pair(str, i + nodeinfo));
    dic_idx[i] = str;
    cont_idx[i] = (it->second)[0];
  }
  size_t* global_idxp = global_idx.data();
  for(i = 0; i < size; i++) {
    global_idxp[i] = i + nodeinfo;
  }
  if(selfid == 0) {
    // for null
    cont_idx[size] = std::numeric_limits<size_t>::max();
    global_idx[size] = std::numeric_limits<size_t>::max();
  }
}

void dftable_offset_nulls(std::vector<size_t>& nulls,
                          const std::vector<size_t>& pxsizes) {
  auto selfid = get_selfid();
  auto lower = selfid == 0 ? 0 : pxsizes[selfid - 1];
  auto upper = pxsizes[selfid];
  auto lowerpos = std::lower_bound(nulls.begin(), nulls.end(), lower)
    - nulls.begin();
  auto upperpos = std::lower_bound(nulls.begin(), nulls.end(), upper)
    - nulls.begin();
  auto new_nulls_size = upperpos - lowerpos;
  std::vector<size_t> new_nulls(new_nulls_size);
  auto nullsp = nulls.data();
  auto new_nullsp = new_nulls.data();
  for(size_t i = 0; i < new_nulls_size; i++) {
    new_nullsp[i] = nullsp[lowerpos + i] - lower;
  }
  nulls.swap(new_nulls);
}

void dftable::load(const std::string& input) {
  col.clear();
  col_order.clear();
  row_size = 0;
  row_sizes.clear();
  std::string colfile = input + "/columns";
  std::string typefile = input + "/types";
  std::ifstream colstr;
  std::ifstream typestr;
  colstr.open(colfile.c_str());
  typestr.open(typefile.c_str());
  if(!colstr || !typestr) throw std::runtime_error("no such table: " + input);
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
      // loaded binary has same alignment; no need to call align
      auto vec = make_dvector_loadbinary<int>(valfile);
      auto pxsizes = broadcast(prefix_sum(vec.sizes()));
      auto nulls = broadcast(make_dvector_loadbinary<size_t>(nullsfile)
                             .gather()).mapv(dftable_offset_nulls, pxsizes);
      auto val = vec.moveto_node_local();
      std::shared_ptr<dfcolumn> c =
        std::make_shared<typed_dfcolumn<int>>(std::move(val),
                                              std::move(nulls));
      append_column(cols[i], c);
    } else if(types[i] == "unsigned int") {
      auto vec = make_dvector_loadbinary<unsigned int>(valfile);
      auto pxsizes = broadcast(prefix_sum(vec.sizes()));
      auto nulls = broadcast(make_dvector_loadbinary<size_t>(nullsfile)
                             .gather()).mapv(dftable_offset_nulls, pxsizes);
      auto val = vec.moveto_node_local();
      std::shared_ptr<dfcolumn> c =
        std::make_shared<typed_dfcolumn<unsigned int>>(std::move(val),
                                                       std::move(nulls));
      append_column(cols[i], c);
    } else if(types[i] == "long") {
      auto vec = make_dvector_loadbinary<long>(valfile);
      auto pxsizes = broadcast(prefix_sum(vec.sizes()));
      auto nulls = broadcast(make_dvector_loadbinary<size_t>(nullsfile)
                             .gather()).mapv(dftable_offset_nulls, pxsizes);
      auto val = vec.moveto_node_local();
      std::shared_ptr<dfcolumn> c =
        std::make_shared<typed_dfcolumn<long>>(std::move(val),
                                               std::move(nulls));
      append_column(cols[i], c);
    } else if(types[i] == "unsigned long") {
      auto vec = make_dvector_loadbinary<unsigned long>(valfile);
      auto pxsizes = broadcast(prefix_sum(vec.sizes()));
      auto nulls = broadcast(make_dvector_loadbinary<size_t>(nullsfile)
                             .gather()).mapv(dftable_offset_nulls, pxsizes);
      auto val = vec.moveto_node_local();
      std::shared_ptr<dfcolumn> c =
        std::make_shared<typed_dfcolumn<unsigned long>>(std::move(val),
                                                        std::move(nulls));
      append_column(cols[i], c);
    } else if(types[i] == "float") {
      auto vec = make_dvector_loadbinary<float>(valfile);
      auto pxsizes = broadcast(prefix_sum(vec.sizes()));
      auto nulls = broadcast(make_dvector_loadbinary<size_t>(nullsfile)
                             .gather()).mapv(dftable_offset_nulls, pxsizes);
      auto val = vec.moveto_node_local();
      std::shared_ptr<dfcolumn> c =
        std::make_shared<typed_dfcolumn<float>>(std::move(val),
                                                std::move(nulls));
      append_column(cols[i], c);
    } else if(types[i] == "double") {
      auto vec = make_dvector_loadbinary<double>(valfile);
      auto pxsizes = broadcast(prefix_sum(vec.sizes()));
      auto nulls = broadcast(make_dvector_loadbinary<size_t>(nullsfile)
                             .gather()).mapv(dftable_offset_nulls, pxsizes);
      auto val = vec.moveto_node_local();
      std::shared_ptr<dfcolumn> c =
        std::make_shared<typed_dfcolumn<double>>(std::move(val),
                                                 std::move(nulls));
      append_column(cols[i], c);
    } else if(types[i] == "string") {
      auto toappend = std::make_shared<typed_dfcolumn<std::string>>();
      auto loaddic = make_dvector_loadline<std::string>(valfile + "_dic");
      auto dicsizes = loaddic.sizes();
      auto withcontidx = loaddic.map_partitions(add_contiguous_idx,
                                                broadcast(dicsizes));
      auto group = withcontidx.group_by_key<std::string,size_t>();
      auto groupnl = group.viewas_node_local();
      toappend->dic = std::make_shared<dunordered_map<std::string,size_t>>
        (make_dunordered_map_allocate<std::string,size_t>());
      toappend->dic_idx = std::make_shared<node_local<std::vector<std::string>>>
        (make_node_local_allocate<std::vector<std::string>>());
      auto retdicnl = toappend->dic->viewas_node_local();
      auto cont_idx = make_node_local_allocate<std::vector<size_t>>();
      auto global_idx = make_node_local_allocate<std::vector<size_t>>();
      groupnl.mapv(prepare_string_load, retdicnl, *toappend->dic_idx, cont_idx,
                   global_idx);

      auto vec = make_dvector_loadbinary<size_t>(valfile + "_idx");
      auto pxsizes = broadcast(prefix_sum(vec.sizes()));
      auto nulls = broadcast(make_dvector_loadbinary<size_t>(nullsfile)
                             .gather()).mapv(dftable_offset_nulls, pxsizes);
      toappend->nulls = std::move(nulls);
      toappend->contain_nulls_check();
      dftable t1, t2;
      t1.append_column("original", std::move(vec));
      t2.append_column("contiguous", cont_idx.moveto_dvector<size_t>());
      t2.append_column("global_idx", global_idx.moveto_dvector<size_t>());
      auto conved = t1.bcast_join(t2, frovedis::eq("original", "contiguous"))
        .as_dvector<size_t>("global_idx");
      toappend->val = conved.moveto_node_local();
      append_column(cols[i], std::move(toappend));
    } else if(types[i] == "dic_string") {
      auto toappend = std::make_shared<typed_dfcolumn<dic_string>>();
      auto loaddic = loadbinary_local<char>(valfile + "_dic");
      auto vintdic = vchar_to_int(loaddic);
      auto dicwords = split_to_words(vintdic, "\n");
      auto dic_to_use = make_dict_from_words(dicwords);
      toappend->dic = std::make_shared<dict>(std::move(dic_to_use));
      auto vec = make_dvector_loadbinary<size_t>(valfile + "_idx");
      auto pxsizes = broadcast(prefix_sum(vec.sizes()));
      auto nulls = broadcast(make_dvector_loadbinary<size_t>(nullsfile)
                             .gather()).mapv(dftable_offset_nulls, pxsizes);
      toappend->val = std::move(vec.moveto_node_local());
      toappend->nulls = std::move(nulls);
      toappend->nulls.mapv(reset_null_val<size_t>, toappend->val);
      toappend->contain_nulls_check();
      append_column(cols[i], std::move(toappend));
    } else if(types[i] == "raw_string") {
      auto toappend = std::make_shared<typed_dfcolumn<raw_string>>();
      auto sep = make_node_local_allocate<std::vector<size_t>>();
      auto len = make_node_local_allocate<std::vector<size_t>>();
      auto loaded_text = load_text(valfile, "\n", sep, len);
      toappend->comp_words =
        loaded_text.map(+[](std::vector<int>& t,
                            std::vector<size_t>& s,
                            std::vector<size_t>& l) {
                          words w;
                          w.chars.swap(t);
                          w.starts.swap(s);
                          w.lens.swap(l);
                          return make_compressed_words(w);
                        }, sep, len);
      std::vector<size_t> sizes;
      if(i != 0) {
        sizes = row_sizes;
        toappend->align_as(sizes);
      } else {
        sizes = toappend->sizes();
      }
      auto pxsizes = broadcast(prefix_sum(sizes));
      auto nulls = broadcast(make_dvector_loadbinary<size_t>(nullsfile)
                             .gather()).mapv(dftable_offset_nulls, pxsizes);
      toappend->nulls = std::move(nulls);
      toappend->contain_nulls_check();
      append_column(cols[i], std::move(toappend));
    } else if(types[i] == "datetime") {
      auto vec = make_dvector_loadbinary<datetime_t>(valfile);
      auto pxsizes = broadcast(prefix_sum(vec.sizes()));
      auto nulls = broadcast(make_dvector_loadbinary<size_t>(nullsfile)
                             .gather()).mapv(dftable_offset_nulls, pxsizes);
      auto val = vec.moveto_node_local();
      std::shared_ptr<dfcolumn> c =
        std::make_shared<typed_dfcolumn<datetime>>(std::move(val),
                                                   std::move(nulls));
      append_column(cols[i], c);
    }
  }
}

dftable make_dftable_load(const std::string& input) {
  dftable t;
  t.load(input);
  return t;
}

void dftable::loadtext(const std::string& filename,
                       const std::vector<std::string>& types,
                       int separator,
                       const std::vector<std::string>& nullstr,
                       bool is_crlf) {
  auto ret = make_dftable_loadtext(filename, types, separator, nullstr,
                                   is_crlf);
  *this = std::move(ret);
}

void dftable::loadtext(const std::string& filename,
                       const std::vector<std::string>& types,
                       const std::vector<std::string>& names,
                       int separator,
                       const std::vector<std::string>& nullstr,
                       bool is_crlf) {
  auto ret = make_dftable_loadtext(filename, types, names, separator, nullstr,
                                   is_crlf);
  *this = std::move(ret);
}

dftable& dftable::type_cast(const std::string& from_name,
                            const std::string& to_name,
                            const std::string& to_type,
                            bool check_bool_like) {
  auto c = column(from_name);
  use_dfcolumn use(c);
  auto new_column = c->type_cast(to_type, check_bool_like);
  append_column(to_name, new_column);
  return *this;
}

void
union_tables_create_global_idx_helper(std::vector<size_t>& ret,
                                      size_t start_idx,
                                      size_t end_idx,
                                      size_t start_pos_in_idx,
                                      size_t end_pos_in_idx,
                                      std::vector<size_t>& flat_num_rows,
                                      size_t each) {
  size_t node_size = get_nodesize();
  auto flat_num_rows_size = flat_num_rows.size();
  auto flat_num_rowsp = flat_num_rows.data();
  size_t table_size = flat_num_rows_size / node_size;

  std::vector<size_t> flat_num_rows_trans(flat_num_rows_size);
  auto flat_num_rows_transp = flat_num_rows_trans.data();
  for(size_t i = 0; i < node_size; i++) {
    for(size_t j = 0; j < table_size; j++) {
      flat_num_rows_transp[i * table_size + j] =
        flat_num_rows[j * node_size + i];
    }
  }

  std::vector<size_t> colpx_flat_num_rows_trans(flat_num_rows_size);
  auto colpx_flat_num_rows_transp = colpx_flat_num_rows_trans.data();
  for(size_t i = 0; i < node_size; i++) {
    prefix_sum(flat_num_rows_transp + i * table_size,
               colpx_flat_num_rows_transp + i * table_size + 1,
               table_size - 1); // exclusive scan
  }
  
  size_t ret_size;
  if(end_idx < flat_num_rows_size) ret_size = each;
  else { // even start_idx == flat_num_rows_size, same
    ret_size = 0;
    for(size_t i = start_idx; i < end_idx; i++) {
      ret_size += flat_num_rowsp[i];
    }
    ret_size -= start_pos_in_idx;
  }
  ret.resize(ret_size);
  auto crnt_retp = ret.data();
  if(start_idx < flat_num_rows_size) {
    size_t crnt_rank = start_idx % node_size;
    size_t crnt_table = start_idx / node_size;
    size_t crnt_size =
      std::min(flat_num_rowsp[start_idx] - start_pos_in_idx, ret_size);
    size_t nodeinfo = crnt_rank << DFNODESHIFT;
    size_t col_pos =
      colpx_flat_num_rows_transp[crnt_rank * table_size + crnt_table];
    auto to_add = col_pos + start_pos_in_idx + nodeinfo;
    for(size_t i = 0; i < crnt_size; i++) {
      crnt_retp[i] = i + to_add;
    }
    crnt_retp += crnt_size;
    for(size_t i = start_idx + 1; i < end_idx; i++) {
      crnt_rank = i % node_size;
      crnt_table = i / node_size;
      crnt_size = flat_num_rowsp[i];
      nodeinfo = crnt_rank << DFNODESHIFT;
      col_pos = colpx_flat_num_rows_transp[crnt_rank * table_size + crnt_table];
      to_add = col_pos + nodeinfo;
      for(size_t j = 0; j < crnt_size; j++) {
        crnt_retp[j] = j + to_add;
      }
      crnt_retp += crnt_size;
    }
    if(start_idx != end_idx && end_idx < flat_num_rows_size) {
      crnt_rank = end_idx % node_size;
      crnt_table = end_idx / node_size;
      crnt_size = end_pos_in_idx;
      nodeinfo = crnt_rank << DFNODESHIFT;
      col_pos = colpx_flat_num_rows_transp[crnt_rank * table_size + crnt_table];
      to_add = col_pos + nodeinfo;
      for(size_t i = 0; i < crnt_size; i++) {
        crnt_retp[i] = i + to_add;
      }
    }
  }
}

node_local<std::vector<size_t>>
union_tables_create_global_idx
(const std::vector<std::vector<size_t>>& num_rows, size_t table_size) {
  size_t node_size = get_nodesize();
  std::vector<size_t> flat_num_rows(table_size * node_size);
  auto crnt_flat_num_rowsp = flat_num_rows.data();
  for(size_t i = 0; i < table_size; i++) {
    auto crnt_num_rowsp = num_rows[i].data();
    for(size_t j = 0; j < node_size; j++) {
      crnt_flat_num_rowsp[j] = crnt_num_rowsp[j];
    }
    crnt_flat_num_rowsp += node_size;
  }
  auto px_flat_num_rows = prefix_sum(flat_num_rows);
  auto total = px_flat_num_rows[px_flat_num_rows.size() - 1];
  auto each = ceil_div(total, node_size);
  std::vector<size_t> to_find(node_size-1);
  auto to_findp = to_find.data();
  for(size_t i = 1; i < node_size; i++) to_findp[i-1] = each * i;
  auto found = lower_bound(px_flat_num_rows, to_find);
  auto foundp = found.data();
  std::vector<size_t> start_idx(node_size), end_idx(node_size);
  std::vector<size_t> start_pos_in_idx(node_size), end_pos_in_idx(node_size);
  auto start_idxp = start_idx.data();
  auto end_idxp = end_idx.data();
  auto start_pos_in_idxp = start_pos_in_idx.data();
  auto end_pos_in_idxp = end_pos_in_idx.data();
  auto px_flat_num_rowsp = px_flat_num_rows.data();
  auto px_flat_num_rows_size = px_flat_num_rows.size();
  auto flat_num_rowsp = flat_num_rows.data();
  for(size_t i = 0; i < node_size-1; i++) {
    end_idx[i] = foundp[i]; // might be end()
    if(foundp[i] < px_flat_num_rows_size) {
      auto diff = px_flat_num_rowsp[foundp[i]] - to_findp[i]; // always >= 0
      end_pos_in_idxp[i] = flat_num_rowsp[foundp[i]] - diff;
    } 
  }
  end_idx[node_size-1] = px_flat_num_rows_size;
  for(size_t i = 1; i < node_size; i++) {
    start_idxp[i] = end_idxp[i-1];
    start_pos_in_idxp[i] = end_pos_in_idxp[i-1];
  }
  auto nl_start_idx = make_node_local_scatter(start_idx);
  auto nl_end_idx = make_node_local_scatter(end_idx);
  auto nl_start_pos_in_idx = make_node_local_scatter(start_pos_in_idx);
  auto nl_end_pos_in_idx = make_node_local_scatter(end_pos_in_idx);
  auto b_flat_num_rows = broadcast(flat_num_rows);
  auto r = make_node_local_allocate<std::vector<size_t>>();
  r.mapv(union_tables_create_global_idx_helper,
         nl_start_idx, nl_end_idx, nl_start_pos_in_idx, nl_end_pos_in_idx,
         b_flat_num_rows, broadcast(each));
  return r;
}

dftable dftable::union_tables(std::vector<dftable *>& ts, bool keep_order,
                              bool keep_dftable) {
  time_spent t(DEBUG);
  auto table_size = ts.size();
  if(table_size == 0) return *this;
  dftable r;
  std::vector<std::vector<size_t>> all_num_rows;
  if(keep_order) { // keep the num_row information before dropping columns
    if(col_order.size() != 0) {
      all_num_rows.push_back(num_rows());
    }
    for(size_t i = 0; i < table_size; i++) {
      if(ts[i]->col_order.size() != 0) {
        all_num_rows.push_back(ts[i]->num_rows());
      }
    }
  }
  auto colnames = col_order; // to prepare for drop
  auto colnames_size = colnames.size();
  for(size_t i = 0; i < colnames_size; i++) {
    auto colname = colnames[i];
    auto col = column(colname);
    use_dfcolumn use(col);
    std::vector<std::shared_ptr<dfcolumn>> to_union;
    for(size_t j = 0; j < table_size; j++) {
      to_union.push_back(ts[j]->column(colname));
    }
    use_dfcolumn use2(to_union);
    auto newcolumn = col->union_columns(to_union);
    if(!keep_dftable) drop(colname);
    for(size_t j = 0; j < table_size; j++) {
      if(!keep_dftable) ts[j]->drop(colname);
    }
    r.append_column(colname, newcolumn);
    t.show("union_tables: union columns: " + colname + ": ");
  }
  if(keep_order) {
    dftable r2;
    // + 1 for this
    auto global_idx = union_tables_create_global_idx(all_num_rows,
                                                     table_size + 1);
    sorted_dftable sorted_df(r, std::move(global_idx));
    r2.row_size = sorted_df.num_row();
    r2.row_sizes = sorted_df.num_rows();
    {dftable tmp; std::swap(r, tmp);} // clear r
    t.show("union_tables: create sorted_dftable: ");
    for(size_t i = 0; i < colnames_size; i++) {
      auto colname = colnames[i];
      auto sorted_col = sorted_df.column(colname);
      r2.append_column(colname, sorted_col);
      sorted_df.drop(colname);
    }
    t.show("union_tables: materialize: ");
    return r2;
  } else {
    return r;
  }
}

dftable dftable::distinct() {
  return group_by(columns()).select(columns());
}

std::vector<size_t>
align_as_create_global_idx_helper(const std::vector<size_t>& dstsizes,
                                  const std::vector<size_t>& srcsizes) {
  size_t self = get_selfid();
  size_t node_size = get_nodesize();
  auto srcsizesp = srcsizes.data();
  auto dstsizesp = dstsizes.data();
  std::vector<size_t> pfx_srcsizes(node_size+1);
  std::vector<size_t> pfx_dstsizes(node_size+1);
  auto pfx_srcsizesp = pfx_srcsizes.data();
  auto pfx_dstsizesp = pfx_dstsizes.data();
  prefix_sum(srcsizesp, pfx_srcsizesp+1, node_size);
  prefix_sum(dstsizesp, pfx_dstsizesp+1, node_size);
  size_t dst_global_start = pfx_dstsizesp[self];
  size_t dst_global_end = pfx_dstsizesp[self+1];
  std::vector<size_t> ret(dstsizes[self]);
  auto retp = ret.data();
  auto crnt_retp = retp;
  for(size_t i = 0; i < node_size; i++) {
    size_t src_global_start = pfx_srcsizesp[i];
    size_t src_global_end = pfx_srcsizesp[i+1];
    size_t nodeinfo = i << DFNODESHIFT;
    if(dst_global_start < src_global_start &&
       dst_global_end < src_global_start) {
      ;
    } else if(dst_global_start < src_global_start &&
              dst_global_end >= src_global_start &&
              dst_global_end < src_global_end) {
      size_t size = dst_global_end - src_global_start;
      for(size_t j = 0; j < size; j++) {
        crnt_retp[j] = nodeinfo + j;
      }
      crnt_retp += size;
    } else if(dst_global_start < src_global_start &&
              dst_global_end >= src_global_end) {
      size_t size = src_global_end - src_global_start;
      for(size_t j = 0; j < size; j++) {
        crnt_retp[j] = nodeinfo + j;
      }
      crnt_retp += size;
    } else if(dst_global_start >= src_global_start &&
              dst_global_start < src_global_end &&
              dst_global_end >= src_global_start &&
              dst_global_end < src_global_end) {
      size_t shift = dst_global_start - src_global_start;
      size_t  size = dst_global_end - dst_global_start;
      for(size_t j = 0; j < size; j++) {
        crnt_retp[j] = nodeinfo + j + shift;
      }
      crnt_retp += size;
    } else if(dst_global_start >= src_global_start &&
              dst_global_start < src_global_end &&
              dst_global_end >= src_global_end) {
      size_t shift = dst_global_start - src_global_start;
      size_t size = src_global_end - dst_global_start;
      for(size_t j = 0; j < size; j++) {
        crnt_retp[j] = nodeinfo + j + shift;
      }
      crnt_retp += size;
    } else if(dst_global_start >= src_global_end &&
              dst_global_end >= src_global_end) {
      ;
    } else {
      throw
        std::runtime_error
        ("internal error in align_as_create_global_idx_helper");
    }
  }
  return ret;
}

node_local<std::vector<size_t>>
align_as_create_global_idx(const std::vector<size_t>& sizes,
                           const std::vector<size_t>& org_sizes) {
  auto bsizes = broadcast(sizes);
  auto borg_sizes = broadcast(org_sizes);
  return bsizes.map(align_as_create_global_idx_helper, borg_sizes);
}

dftable& dftable::align_as(const std::vector<size_t>& sizes) {
  if(col.size() == 0) {
    return *this;
  } else {
    dftable ret;
    auto sorted_df = sorted_dftable
      (*this,align_as_create_global_idx(sizes, row_sizes));
    auto colnames = col_order;
    auto colnames_size = colnames.size();
    for(size_t i = 0; i < colnames_size; i++) {
      auto colname = colnames[i];
      auto sorted_col = sorted_df.column(colname);
      ret.append_column(colname, sorted_col);
      sorted_df.drop(colname);
      drop(colname);
    }
    col.swap(ret.col);
    row_sizes.swap(ret.row_sizes);
    col_order.swap(ret.col_order);
    return *this;
  }
}

dftable& dftable::align_block() {
  size_t sz = num_row();
  size_t nodesize = get_nodesize();
  std::vector<size_t> block_size(nodesize);
  size_t each = ceil_div(sz, nodesize);
  for(size_t i = 0; i < nodesize; i++) {
    if(sz > each) {
      block_size[i] = each;
      sz -= each;
    } else {
      block_size[i] = sz;
      break;
    }
  }
  return align_as(block_size);
}

// df.change_col_position("x", 0); => similar to df.set_index("x") in pandas...
dftable& dftable::change_col_position(const std::string& name, size_t pos) {
  auto cols = columns();
  require(pos < cols.size(), "change_col_position: given position is out-of-bound!\n");
  auto find_it = std::find (cols.begin(), cols.end(), name);
  require(find_it != cols.end(), "change_col_position: given column doesn't exist!\n");
  auto find_index = find_it - cols.begin();
  // do nothing if requested for same position
  if (pos != find_index) vector_shift_inplace(col_order, find_index, pos);
  return *this;
}

dftable& dftable::set_col_order(std::vector<std::string>& new_col_order){
  assert(col_order.size() == new_col_order.size());
  this->col_order.swap(new_col_order);
  return *this;
}

// ---------- for sorted_dftable ----------

dftable sorted_dftable::select(const std::vector<std::string>& cols) {
  dftable ret;
  for(size_t i = 0; i < cols.size(); i++) {
    use_dfcolumn use(raw_column(cols[i]));
    ret.col[cols[i]] = column(cols[i]);
    ret.col[cols[i]]->spill();
  }
  ret.row_size = global_idx.viewas_dvector<size_t>().size();
  ret.row_sizes = global_idx.viewas_dvector<size_t>().sizes();
  ret.col_order = cols;
  return ret;
}

dftable sorted_dftable::fselect
(const std::vector<std::shared_ptr<dffunction>>& cols) {
  dftable ret;
  for(size_t i = 0; i < cols.size(); i++) {
    auto as = cols[i]->get_as();
    if(ret.col.find(as) != ret.col.end())
      throw std::runtime_error("select: same column name already exists");
    use_dfcolumn use(cols[i]->columns_to_use(*this));    
    ret.col[as] = cols[i]->execute(*this);
    ret.col[as]->spill();
    ret.col_order.push_back(as);
  }
  ret.row_size = global_idx.viewas_dvector<size_t>().size();
  ret.row_sizes = global_idx.viewas_dvector<size_t>().sizes();
  return ret;
}

sorted_dftable sorted_dftable::sort(const std::string& name) {
  node_local<std::vector<size_t>> idx;
  auto to_sort_column = column(name);
  use_dfcolumn use(to_sort_column);
  auto sorted_column = to_sort_column->sort_with_idx(global_idx, idx);
  if(to_sort_column->if_contain_nulls())
    return sorted_dftable(*this, std::move(idx));
  else
    return sorted_dftable(*this, std::move(idx), name,
                          std::move(sorted_column));
}

sorted_dftable sorted_dftable::sort_desc(const std::string& name) {
  node_local<std::vector<size_t>> idx;
  auto to_sort_column = column(name);
  use_dfcolumn use(to_sort_column);
  auto sorted_column = to_sort_column->sort_with_idx_desc(global_idx, idx);
  if(to_sort_column->if_contain_nulls())
    return sorted_dftable(*this, std::move(idx));
  else
    return sorted_dftable(*this, std::move(idx), name,
                          std::move(sorted_column));
}

hash_joined_dftable
sorted_dftable::hash_join(dftable_base& right,
                          const std::shared_ptr<dfoperator>& op) {
  RLOG(DEBUG) << "calling hash_join after sort" << std::endl;
  return materialize().hash_join(right, op);
}

hash_joined_dftable
sorted_dftable::outer_hash_join(dftable_base& right,
                                const std::shared_ptr<dfoperator>& op) {
  RLOG(DEBUG) << "calling outer_hash_join after sort" << std::endl;
  return materialize().outer_hash_join(right, op);
}

bcast_joined_dftable
sorted_dftable::bcast_join(dftable_base& right,
                           const std::shared_ptr<dfoperator>& op,
                           bool allow_exchange_lr) {
  RLOG(DEBUG) << "calling bcast_join after sort" << std::endl;
  return materialize().bcast_join(right, op, allow_exchange_lr);
}

bcast_joined_dftable
sorted_dftable::outer_bcast_join(dftable_base& right,
                                 const std::shared_ptr<dfoperator>& op) {
  RLOG(DEBUG) << "calling outer_bcast_join after sort" << std::endl;
  return materialize().outer_bcast_join(right, op);
}

star_joined_dftable
sorted_dftable::star_join
(const std::vector<dftable_base*>& dftables, 
 const std::vector<std::shared_ptr<dfoperator>>& op){
  return materialize().star_join(dftables, op);
}

grouped_dftable
sorted_dftable::group_by(const std::vector<std::string>& cols) {
  RLOG(DEBUG) << "calling group_by after sort" << std::endl;
  return materialize().group_by(cols);
}

std::shared_ptr<dfcolumn> sorted_dftable::column(const std::string& name) {
  if(name == column_name && is_cachable) return sorted_column;
  else {
    auto ret = col.find(name);
    if(ret == col.end()) throw std::runtime_error("no such column: " + name);
    else {
      auto col = (*ret).second;
      use_dfcolumn use(col); // column may be called directly w/o outside use
      return col->global_extract(global_idx, to_store_idx, exchanged_idx);
    }
  }
}

// returning sorted_column is a bit strange for "raw" column,
// but this is required for spill-restore functionality,
// since use_dfcolumn is called for raw_column basically
std::shared_ptr<dfcolumn> sorted_dftable::raw_column(const std::string& name) {
  if(name == column_name && is_cachable) return sorted_column;
  return dftable_base::raw_column(name);
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
  std::cout << "to_store_idx: " << std::endl;
  for(auto& i: to_store_idx.gather()) {
    for(auto j: i) {
      std::cout << j << " ";
    }
    std::cout << ": ";
  }
  std::cout << std::endl;
  std::cout << "exchanged_idx: " << std::endl;
  for(auto& i: exchanged_idx.gather()) {
    for(auto j: i) {
      for(auto k: j) {
        std::cout << k << " ";
      }
      std::cout << "| ";
    }
    std::cout << ": ";
  }
  std::cout << std::endl;
  std::cout << "sorted_column: " << column_name << std::endl;
  dftable_base::debug_print();
}

size_t sorted_dftable::num_row() {
  return global_idx.viewas_dvector<size_t>().size();
}

std::vector<size_t> sorted_dftable::num_rows() {
  return global_idx.viewas_dvector<size_t>().sizes();
}

// ---------- hash_joined_dftable ----------

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

size_t hash_joined_dftable::num_col() const {
    return dftable_base::num_col() + right.num_col();
}

size_t hash_joined_dftable::num_row() {
  return left_idx.viewas_dvector<size_t>().size();
}

std::vector<size_t> hash_joined_dftable::num_rows() {
  return left_idx.viewas_dvector<size_t>().sizes();
}

std::vector<std::string> hash_joined_dftable::columns() const {
  std::vector<std::string> ret = dftable_base::columns();
  auto right_cols = right.columns();
  ret.insert(ret.end(), right_cols.begin(), right_cols.end());
  return ret;
}

dftable hash_joined_dftable::select(const std::vector<std::string>& cols) {    
  dftable ret;
  for(size_t i = 0; i < cols.size(); i++) {
    use_dfcolumn use(raw_column(cols[i]));
    ret.col[cols[i]] = column(cols[i]);
    ret.col[cols[i]]->spill();
  }
  ret.row_size = left_idx.viewas_dvector<size_t>().size();
  ret.row_sizes = left_idx.viewas_dvector<size_t>().sizes();
  ret.col_order = cols;
  return ret;
}

dftable hash_joined_dftable::fselect
(const std::vector<std::shared_ptr<dffunction>>& cols) {
  dftable ret;
  for(size_t i = 0; i < cols.size(); i++) {
    auto as = cols[i]->get_as();
    if(ret.col.find(as) != ret.col.end())
      throw std::runtime_error("select: same column name already exists");
    use_dfcolumn use(cols[i]->columns_to_use(*this));    
    ret.col[as] = cols[i]->execute(*this);
    ret.col[as]->spill();
    ret.col_order.push_back(as);
  }
  ret.row_size = left_idx.viewas_dvector<size_t>().size();
  ret.row_sizes = left_idx.viewas_dvector<size_t>().sizes();
  return ret;
}

// TODO: sort only on the specified column to create left/right idx
// it is possible but return type becomes hash_joined_dftable...
sorted_dftable hash_joined_dftable::sort(const std::string& name) {
  return materialize().sort(name);
}

sorted_dftable hash_joined_dftable::sort_desc(const std::string& name) {
  return materialize().sort_desc(name);
}

hash_joined_dftable
hash_joined_dftable::hash_join(dftable_base& right,
                               const std::shared_ptr<dfoperator>& op) {
  return materialize().hash_join(right, op);
}

hash_joined_dftable
hash_joined_dftable::outer_hash_join(dftable_base& right,
                                     const std::shared_ptr<dfoperator>& op) {
  return materialize().outer_hash_join(right, op);
}

bcast_joined_dftable
hash_joined_dftable::bcast_join(dftable_base& right,
                                const std::shared_ptr<dfoperator>& op,
                                bool allow_exchange_lr) {
  return materialize().bcast_join(right, op, allow_exchange_lr);
}

bcast_joined_dftable
hash_joined_dftable::outer_bcast_join(dftable_base& right,
                                     const std::shared_ptr<dfoperator>& op) {
  return materialize().outer_bcast_join(right, op);
}

star_joined_dftable
hash_joined_dftable::star_join
(const std::vector<dftable_base*>& dftables, 
 const std::vector<std::shared_ptr<dfoperator>>& op){
  return materialize().star_join(dftables, op);
}

grouped_dftable
hash_joined_dftable::group_by(const std::vector<std::string>& cols) {
  return materialize().group_by(cols);
}

std::shared_ptr<dfcolumn>
hash_joined_dftable::column(const std::string& name) {
  auto left_ret = col.find(name);
  if(left_ret != col.end()) {
    auto c = (*left_ret).second;
    use_dfcolumn use(c);
    // if outer, left_idx is changed to contain nulls in ctor
    return c->global_extract(left_idx, left_to_store_idx,
                             left_exchanged_idx);
  } else {
    auto right_ret = right.col.find(name);
    if(right_ret != right.col.end()) {
      auto c = (*right_ret).second;
      use_dfcolumn use(c);
      if(is_outer) {
        auto retcol = c->global_extract(right_idx, right_to_store_idx,
                                        right_exchanged_idx);
        retcol->append_nulls(right_nulls);
        return retcol;
      } else {
        return c->global_extract(right_idx, right_to_store_idx,
                                 right_exchanged_idx);
      }
    } else {
      throw std::runtime_error("no such column: " + name);
    }
  }
}

std::shared_ptr<dfcolumn>
hash_joined_dftable::raw_column(const std::string& name) {
  auto left_ret = col.find(name);
  if(left_ret != col.end()) {
    return dftable_base::raw_column(name);
  } else {
    auto right_ret = right.col.find(name);
    if(right_ret != right.col.end()) {
      return right.raw_column(name);
    } else {
      throw std::runtime_error("no such column: " + name);
    }
  }
}

void hash_joined_dftable::debug_print() {
  std::cout << "left: " << std::endl;
  dftable_base::debug_print();
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
  std::cout << "left_to_store_idx: " << std::endl;
  for(auto& i: left_to_store_idx.gather()) {
    for(auto j: i) {
      std::cout << j << " ";
    }
    std::cout << ": ";
  }
  std::cout << std::endl;
  std::cout << "right_to_store_idx: " << std::endl;
  for(auto& i: right_to_store_idx.gather()) {
    for(auto j: i) {
      std::cout << j << " ";
    }
    std::cout << ": ";
  }
  std::cout << std::endl;
  std::cout << "left_exchanged_idx: " << std::endl;
  for(auto& i: left_exchanged_idx.gather()) {
    for(auto j: i) {
      for(auto k: j) {
        std::cout << k << " ";
      }
      std::cout << "| ";
    }
    std::cout << ": ";
  }
  std::cout << std::endl;
  std::cout << "right_exchanged_idx: " << std::endl;
  for(auto& i: right_exchanged_idx.gather()) {
    for(auto j: i) {
      for(auto k: j) {
        std::cout << k << " ";
      }
      std::cout << "| ";
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
  
// ---------- bcast_joined_dftable ----------

size_t bcast_joined_dftable::num_col() const {
  return dftable_base::num_col() + right.num_col();
}

size_t bcast_joined_dftable::num_row() {
  return left_idx.viewas_dvector<size_t>().size();
}

std::vector<size_t> bcast_joined_dftable::num_rows() {
  return left_idx.viewas_dvector<size_t>().sizes();
}

std::vector<std::string> bcast_joined_dftable::columns() const {
  std::vector<std::string> ret = dftable_base::columns();
  auto right_cols = right.columns();
  ret.insert(ret.end(), right_cols.begin(), right_cols.end());
  return ret;
}

dftable bcast_joined_dftable::select(const std::vector<std::string>& cols) {
  dftable ret;
  for(size_t i = 0; i < cols.size(); i++) {
    use_dfcolumn use(raw_column(cols[i]));
    ret.col[cols[i]] = column(cols[i]);
    ret.col[cols[i]]->spill();
  }
  ret.row_size = left_idx.viewas_dvector<size_t>().size();
  ret.row_sizes = left_idx.viewas_dvector<size_t>().sizes();
  ret.col_order = cols;
  return ret;
}

dftable bcast_joined_dftable::fselect
(const std::vector<std::shared_ptr<dffunction>>& cols) {
  dftable ret;
  for(size_t i = 0; i < cols.size(); i++) {
    auto as = cols[i]->get_as();
    if(ret.col.find(as) != ret.col.end())
      throw std::runtime_error("select: same column name already exists");
    use_dfcolumn use(cols[i]->columns_to_use(*this));    
    ret.col[as] = cols[i]->execute(*this);
    ret.col[as]->spill();
    ret.col_order.push_back(as);
  }
  ret.row_size = left_idx.viewas_dvector<size_t>().size();
  ret.row_sizes = left_idx.viewas_dvector<size_t>().sizes();
  return ret;
}

// TODO: sort only on the specified column to create left/right idx
// it is possible but return type becomes hash_joined_dftable...
sorted_dftable bcast_joined_dftable::sort(const std::string& name) {
  return materialize().sort(name);
}

sorted_dftable bcast_joined_dftable::sort_desc(const std::string& name) {
  return materialize().sort_desc(name);
}

hash_joined_dftable
bcast_joined_dftable::hash_join(dftable_base& right,
                                const std::shared_ptr<dfoperator>& op) {
  return materialize().hash_join(right, op);
}

hash_joined_dftable
bcast_joined_dftable::outer_hash_join(dftable_base& right,
                                      const std::shared_ptr<dfoperator>& op) {
  return materialize().outer_hash_join(right, op);
}

bcast_joined_dftable
bcast_joined_dftable::bcast_join(dftable_base& right,
                                 const std::shared_ptr<dfoperator>& op,
                                 bool allow_exchange_lr) {
  return materialize().bcast_join(right, op, allow_exchange_lr);
}

bcast_joined_dftable
bcast_joined_dftable::outer_bcast_join(dftable_base& right,
                                       const std::shared_ptr<dfoperator>& op) {
  return materialize().outer_bcast_join(right, op);
}

star_joined_dftable
bcast_joined_dftable::star_join
(const std::vector<dftable_base*>& dftables, 
 const std::vector<std::shared_ptr<dfoperator>>& op){
  return materialize().star_join(dftables, op);
}

grouped_dftable
bcast_joined_dftable::group_by(const std::vector<std::string>& cols) {
  return materialize().group_by(cols);
}

std::shared_ptr<dfcolumn>
bcast_joined_dftable::column(const std::string& name) {
  auto left_ret = col.find(name);
  if(left_ret != col.end()) {
    auto c = (*left_ret).second;
    use_dfcolumn use(c);
    return c->extract(left_idx);
  } else {
    auto right_ret = right.col.find(name);
    if(right_ret != right.col.end()) {
      auto c = (*right_ret).second;
      use_dfcolumn use(c);
      if(is_outer) {
        auto retcol = c->global_extract(right_idx, right_to_store_idx,
                                        right_exchanged_idx);
        retcol->append_nulls(right_nulls);
        return retcol;
      } else {
        return c->global_extract(right_idx, right_to_store_idx,
                                 right_exchanged_idx);
      }
    } else {
      throw std::runtime_error("no such column: " + name);
    }
  }
}

std::shared_ptr<dfcolumn>
bcast_joined_dftable::raw_column(const std::string& name) {
  auto left_ret = col.find(name);
  if(left_ret != col.end()) {
    return dftable_base::raw_column(name);
  } else {
    auto right_ret = right.col.find(name);
    if(right_ret != right.col.end()) {
      return right.raw_column(name);
    } else {
      throw std::runtime_error("no such column: " + name);
    }
  }
}

void bcast_joined_dftable::debug_print() {
  std::cout << "left: " << std::endl;
  dftable_base::debug_print();
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

void bcast_joined_dftable::update_to_store_idx_and_exchanged_idx() {
  auto unique_right_idx = right_idx.map(get_unique_idx);
  auto right_partitioned_idx = partition_global_index_bynode(unique_right_idx);
  right_to_store_idx = make_to_store_idx(right_partitioned_idx, right_idx);
  right_exchanged_idx = exchange_partitioned_index(right_partitioned_idx);
}

void
bcast_joined_dftable::inplace_filter_pre(const std::shared_ptr<dfoperator>& op) {
  if(is_outer) throw std::runtime_error
                 ("inplace_filter cannot be used for outer joined table");
  use_dfcolumn use(op->columns_to_use(*this));    
  auto filtered_idx = op->filter(*this);
  left_idx.mapv(filter_idx, filtered_idx);
  right_idx.mapv(filter_idx, filtered_idx);
}

bcast_joined_dftable&
bcast_joined_dftable::inplace_filter(const std::shared_ptr<dfoperator>& op) {
  inplace_filter_pre(op);
  update_to_store_idx_and_exchanged_idx();
  return *this;
}

// ---------- star_joined_dftable ----------

size_t star_joined_dftable::num_col() const {
  size_t total = dftable_base::num_col();
  for(size_t i = 0; i < rights.size(); i++) total += rights[i].num_col();
  return total;
}

size_t star_joined_dftable::num_row() {
  return left_idx.viewas_dvector<size_t>().size();
}

std::vector<size_t> star_joined_dftable::num_rows() {
  return left_idx.viewas_dvector<size_t>().sizes();
}

std::vector<std::string> star_joined_dftable::columns() const {
  std::vector<std::string> ret = dftable_base::columns();
  for(size_t i = 0; i < rights.size(); i++) {
    auto right_cols = rights[i].columns();
    ret.insert(ret.end(), right_cols.begin(), right_cols.end());
  }
  return ret;
}

dftable star_joined_dftable::select(const std::vector<std::string>& cols) {    
  dftable ret;
  for(size_t i = 0; i < cols.size(); i++) {
    use_dfcolumn use(raw_column(cols[i]));
    ret.col[cols[i]] = column(cols[i]);
    ret.col[cols[i]]->spill();
  }
  ret.row_size = left_idx.viewas_dvector<size_t>().size();
  ret.row_sizes = left_idx.viewas_dvector<size_t>().sizes();
  ret.col_order = cols;
  return ret;
}

dftable star_joined_dftable::fselect
(const std::vector<std::shared_ptr<dffunction>>& cols) {
  dftable ret;
  for(size_t i = 0; i < cols.size(); i++) {
    auto as = cols[i]->get_as();
    if(ret.col.find(as) != ret.col.end())
      throw std::runtime_error("select: same column name already exists");
    use_dfcolumn use(cols[i]->columns_to_use(*this));    
    ret.col[as] = cols[i]->execute(*this);
    ret.col[as]->spill();
    ret.col_order.push_back(as);
  }
  ret.row_size = left_idx.viewas_dvector<size_t>().size();
  ret.row_sizes = left_idx.viewas_dvector<size_t>().sizes();
  return ret;
}

sorted_dftable star_joined_dftable::sort(const std::string& name) {
  return materialize().sort(name);
}

sorted_dftable star_joined_dftable::sort_desc(const std::string& name) {
  return materialize().sort_desc(name);
}

hash_joined_dftable
star_joined_dftable::hash_join(dftable_base& right,
                               const std::shared_ptr<dfoperator>& op) {
  return materialize().hash_join(right, op);
}

hash_joined_dftable
star_joined_dftable::outer_hash_join(dftable_base& right,
                                     const std::shared_ptr<dfoperator>& op) {
  return materialize().outer_hash_join(right, op);
}

bcast_joined_dftable
star_joined_dftable::bcast_join(dftable_base& right,
                                const std::shared_ptr<dfoperator>& op,
                                bool allow_exchange_lr) {
  return materialize().bcast_join(right, op, allow_exchange_lr);
}

star_joined_dftable
star_joined_dftable::star_join
(const std::vector<dftable_base*>& dftables, 
 const std::vector<std::shared_ptr<dfoperator>>& op){
  return materialize().star_join(dftables, op);
}

bcast_joined_dftable
star_joined_dftable::outer_bcast_join(dftable_base& right,
                                      const std::shared_ptr<dfoperator>& op) {
  return materialize().outer_bcast_join(right, op);
}

grouped_dftable
star_joined_dftable::group_by(const std::vector<std::string>& cols) {
  return materialize().group_by(cols);
}

std::shared_ptr<dfcolumn>
star_joined_dftable::column(const std::string& name) {
  auto left_ret = col.find(name);
  if(left_ret != col.end()) {
    auto c = (*left_ret).second;
    use_dfcolumn use(c);
    return c->extract(left_idx);
  } else {
    for(size_t i = 0; i < rights.size(); i++) {
      auto right_ret = rights[i].col.find(name);
      if(right_ret != rights[i].col.end()) {
        auto c = (*right_ret).second;
        use_dfcolumn use(c);
        return c->global_extract(right_idxs[i], right_to_store_idxs[i],
                                 right_exchanged_idxs[i]);
      }
    }
    throw std::runtime_error("no such column: " + name);
  }
}

std::shared_ptr<dfcolumn>
star_joined_dftable::raw_column(const std::string& name) {
  auto left_ret = col.find(name);
  if(left_ret != col.end()) {
    return dftable_base::raw_column(name);
  } else {
    for(size_t i = 0; i < rights.size(); i++) {
      auto right_ret = rights[i].col.find(name);
      if(right_ret != rights[i].col.end()) {
        return rights[i].raw_column(name);
      }
    }
    throw std::runtime_error("no such column: " + name);
  }
}

void star_joined_dftable::debug_print() {
  std::cout << "left: " << std::endl;
  dftable_base::debug_print();
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

// ---------- grouped_dftable ----------

dftable grouped_dftable::select(const std::vector<std::string>& cols) {
  dftable ret_table;
  size_t colssize = cols.size();
  for(size_t i = 0; i < colssize; i++) {
    bool in_grouped_cols = false;
    for(size_t j = 0; j < grouped_col_names.size(); j++) {
      if(cols[i] == grouped_col_names[j]) {
        in_grouped_cols = true; break;
      }
    }
    if(!in_grouped_cols)
      throw std::runtime_error("select not grouped column");
    ret_table.col[cols[i]] = grouped_cols[i];
  }
  ret_table.row_size = num_row();
  ret_table.row_sizes = num_rows();
  ret_table.col_order = cols;
  return ret_table;
}

dftable
grouped_dftable::select(const std::vector<std::string>& cols,
                        const std::vector<std::shared_ptr<dfaggregator>>& aggs) {
  dftable ret_table;
  size_t colssize = cols.size();
  if(colssize != 0) {
    for(size_t i = 0; i < colssize; i++) {
      bool in_grouped_cols = false;
      for(size_t j = 0; j < grouped_col_names.size(); j++) {
        if(cols[i] == grouped_col_names[j]) {
          in_grouped_cols = true; break;
        }
      }
      if(!in_grouped_cols)
        throw std::runtime_error("select not grouped column");
      ret_table.col[cols[i]] = grouped_cols[i];
    }
  }
  ret_table.col_order = cols;
  size_t aggssize = aggs.size();
  auto nl_row_sizes = make_node_local_scatter(num_rows());
  for(size_t i = 0; i < aggssize; i++) {
    use_dfcolumn use(org_table.raw_column(aggs[i]->col));
    auto newcol = aggs[i]->aggregate(org_table,
                                     local_grouped_idx,
                                     local_idx_split,
                                     hash_divide,
                                     merge_map,
                                     nl_row_sizes);
    newcol->spill();
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
  ret_table.row_size = num_row();
  ret_table.row_sizes = num_rows();
  return ret_table;
}

void grouped_dftable::debug_print() {
  std::cout << "table: " << std::endl;
  org_table.debug_print();
  std::cout << "local_grouped_idx: " << std::endl;
  for(auto& i: local_grouped_idx.gather()) {
    for(auto j: i) {
      std::cout << j << " ";
    }
    std::cout << ": ";
  }
  std::cout << std::endl;
  std::cout << "local_idx_split: " << std::endl;
  for(auto& i: local_idx_split.gather()) {
    for(auto j: i) {
        std::cout << j << " "; 
    }
    std::cout << ": ";
  }
  std::cout << std::endl;
  std::cout << "hash_divide: " << std::endl;
  for(auto& i: hash_divide.gather()) {
    for(auto& j: i) {
      for(auto k: j) {
        std::cout << k << " "; 
      }
      std::cout << ": ";
    }
    std::cout << "| ";
  }
  std::cout << std::endl;
  std::cout << "merge_map: " << std::endl;
  for(auto& i: merge_map.gather()) {
    for(auto& j: i) {
      for(auto k: j) {
        std::cout << k << " "; 
      }
      std::cout << ": ";
    }
    std::cout << "| ";
  }
  std::cout << std::endl;
  std::cout << "grouped_cols: " << std::endl;
  for(auto& i: grouped_cols) i->debug_print();
  std::cout << "grouped_col_names: " << std::endl;
  for(auto& i: grouped_col_names) std::cout << i << " ";
  std::cout << std::endl;
}

// ---------- other functions ----------

dftable_base* dftable_base::clone() {
  return new dftable_base(*this);
}

dftable_base* dftable::clone() {
  return new dftable(*this);
}

dftable_base* sorted_dftable::clone() {
  return new sorted_dftable(*this);
}

dftable_base* hash_joined_dftable::clone() {
  return new hash_joined_dftable(*this);
}

dftable_base* bcast_joined_dftable::clone() {
  return new bcast_joined_dftable(*this);
}

dftable_base* star_joined_dftable::clone() {
  return new star_joined_dftable(*this);
}

dftable_base* filtered_dftable::clone() {
  return new filtered_dftable(*this);
}

dftable_base* dftable_base::rename_cols(const std::string& name,
                                        const std::string& name2){
  rename_impl(name, name2, col, col_order);
  return this;
}

dftable_base* dftable::rename_cols(const std::string& name,
                                   const std::string& name2){
  rename_impl(name, name2, col, col_order);
  return this;
}

dftable_base* sorted_dftable::rename_cols(const std::string& name,
                                          const std::string& name2){
  rename_impl(name, name2, col, col_order);
  if(name == column_name) column_name = name2;
  return this;
}

sorted_dftable& sorted_dftable::rename(const std::string& name,
                                       const std::string& name2) {
  rename_cols(name, name2);
  return *this;
}

dftable_base* hash_joined_dftable::rename_cols(const std::string& name,
                                               const std::string& name2){
  bool in_left = col.find(name) != col.end();
  bool in_right = right.col.find(name) != right.col.end();

  if (in_left) rename_impl(name, name2, col, col_order);
  else if (in_right) rename_impl(name, name2, right.col, right.col_order);
  else throw std::runtime_error("no such column: " + name);

  return this;
}

hash_joined_dftable& hash_joined_dftable::rename(const std::string& name,
                                                 const std::string& name2){
  rename_cols(name, name2);
  return *this;
}

dftable_base* bcast_joined_dftable::rename_cols(const std::string& name,
                                                const std::string& name2){
  bool in_left = col.find(name) != col.end();
  bool in_right = right.col.find(name) != right.col.end();

  if (in_left) rename_impl(name, name2, col, col_order);
  else if (in_right) rename_impl(name, name2, right.col, right.col_order);
  else throw std::runtime_error("no such column: " + name);
  return this;
}

bcast_joined_dftable& bcast_joined_dftable::rename(const std::string& name,
                                                   const std::string& name2){
  rename_cols(name, name2);
  return *this;
}

dftable_base* star_joined_dftable::rename_cols(const std::string& name,
                                               const std::string& name2){
  bool found = col.find(name) != col.end();
  if (found) rename_impl(name, name2, col, col_order);
  else {
    for(size_t i = 0; i < rights.size(); i++) {
      found = rights[i].col.find(name) != rights[i].col.end();
      if(found) {
        rename_impl(name, name2, rights[i].col, rights[i].col_order);
        break;
      } 
    }
    if (!found) throw std::runtime_error("no such column: " + name);
  }
  return this;
}

star_joined_dftable& star_joined_dftable::rename(const std::string& name,
                                                 const std::string& name2){
  rename_cols(name, name2);
  return *this;
}

dftable_base* filtered_dftable::rename_cols(const std::string& name,
                                            const std::string& name2){
  rename_impl(name, name2, col, col_order);
  return this;
}

filtered_dftable& filtered_dftable::rename(const std::string& name,
                                           const std::string& name2){
  rename_cols(name, name2);
  return *this;
}

sorted_dftable& sorted_dftable::drop(const std::string& name) {
  if(name == column_name) {
    column_name = "";
    sorted_column = std::shared_ptr<dfcolumn>();
  }
  drop_impl(name, col, col_order);
  return *this;
}

dftable_base* sorted_dftable::drop_cols(const std::vector<std::string>& cols) {
  for(auto& c: cols) drop(c);
  return this;
}

hash_joined_dftable& hash_joined_dftable::drop(const std::string& name) {
  bool in_left = col.find(name) != col.end();
  bool in_right = right.col.find(name) != right.col.end();

  if (in_left) drop_impl(name, col, col_order);
  else if (in_right) drop_impl(name, right.col, right.col_order);
  else throw std::runtime_error("no such column: " + name);
  return *this;
}

dftable_base* hash_joined_dftable::drop_cols(const std::vector<std::string>& cols) {
  for(auto& c: cols) drop(c);
  return this;
}

bcast_joined_dftable& bcast_joined_dftable::drop(const std::string& name) {
  bool in_left = col.find(name) != col.end();
  bool in_right = right.col.find(name) != right.col.end();

  if (in_left) drop_impl(name, col, col_order);
  else if (in_right) drop_impl(name, right.col, right.col_order);
  else throw std::runtime_error("no such column: " + name);
  return *this;
}

dftable_base* bcast_joined_dftable::drop_cols(const std::vector<std::string>& cols) {
  for(auto& c: cols) drop(c);
  return this;
}

star_joined_dftable& star_joined_dftable::drop(const std::string& name) {
  bool found = col.find(name) != col.end();
  if (found) drop_impl(name, col, col_order);
  else {
    for(size_t i = 0; i < rights.size(); i++) {
      found = rights[i].col.find(name) != rights[i].col.end();
      if(found) {
        drop_impl(name, rights[i].col, rights[i].col_order);
        break;
      } 
    }
    if (!found) throw std::runtime_error("no such column: " + name);
  }
  return *this;
}

dftable_base* star_joined_dftable::drop_cols(const std::vector<std::string>& cols) {
  for(auto& c: cols) drop(c);
  return this;
}

// for covariance
double dftable::covariance(const std::string& c1, 
                           const std::string& c2,
                           int min_periods, 
                           double ddof) {
  auto tmp = select({c1, c2});
  auto na_count = 2 * tmp.num_row() - (tmp.count(c1) + tmp.count(c2));
  if (na_count) {
    tmp = tmp.drop_nulls_by_rows("any");
    ddof = 1.0;
  }
  auto n_observations = tmp.num_row();
  if (n_observations == 0 ||
      n_observations < min_periods ||
      n_observations - ddof < 1) return std::numeric_limits<double>::max();
  auto mean1 = tmp.avg(c1);
  auto mean2 = tmp.avg(c2);
  tmp.call_function(sub_im_as(c1, mean1, "_t1"));
  tmp.call_function(sub_im_as(c2, mean2, "_t2"));
  tmp.call_function(mul_col_as("_t1", "_t2", "_t3"));
  return tmp.sum<double>("_t3") / (n_observations - ddof);
}

double dftable::covariance(const std::string& c1,
                           int min_periods, 
                           double ddof) {
  auto tmp = select({c1});
  auto na_count = tmp.num_row() - tmp.count(c1);
  if (na_count) {
    tmp = tmp.drop_nulls_by_rows("any");
    ddof = 1.0;
  }
  auto n_observations = tmp.num_row();
  if (n_observations == 0 ||
      n_observations < min_periods ||
      n_observations - ddof < 1) return std::numeric_limits<double>::max();
  auto mean1 = tmp.avg(c1);
  tmp.call_function(sub_im_as(c1, mean1, "_t1"));
  tmp.call_function(mul_col_as("_t1", "_t1", "_t3"));
  return tmp.sum<double>("_t3") / (n_observations - ddof);
}

dftable dftable::covariance(int min_periods,
                            double ddof, 
                            bool low_memory) {
  auto cols = columns();
  auto ncol = cols.size();
  rowmajor_matrix<double> ret;
  auto na_count = count_nulls().sum<size_t>("count");

  if(low_memory) { // column-by-column computation
    rowmajor_matrix_local<double> covmat(ncol, ncol);
    auto retp = covmat.val.data();
    if (na_count) {
      auto nulls = isnull(cols);
      for(size_t i = 0; i < ncol; ++i) {
        auto x = cols[i];
        use_dfcolumn use(raw_column(x));
        auto valX = column(x)->as_dvector_double().moveto_node_local();
        auto nullX = nulls.as_dvector<int>(x).moveto_node_local();
        retp[i * ncol + i] = cov_impl(valX, valX, nullX, nullX, min_periods);
        for(size_t j = i + 1; j < ncol; ++j) {
          auto y = cols[j];
          use_dfcolumn use2(raw_column(y));
          auto valY = column(y)->as_dvector_double().moveto_node_local();
          auto nullY = nulls.as_dvector<int>(y).moveto_node_local();
          retp[i * ncol + j] = retp[j * ncol + i] = \
            cov_impl(valX, valY, nullX, nullY, min_periods);
        }
      }
    } else {
      for(size_t i = 0; i < ncol; ++i) {
        auto x = cols[i];
        use_dfcolumn use(raw_column(x));
        auto valX = column(x)->as_dvector_double().moveto_node_local();
        retp[i * ncol + i] = cov_impl(valX, valX, min_periods, ddof);
        for(size_t j = i + 1; j < ncol; ++j) {
          auto y = cols[j];
          use_dfcolumn use2(raw_column(y));
          auto valY = column(y)->as_dvector_double().moveto_node_local();
          retp[i * ncol + j] = retp[j * ncol + i] = \
            cov_impl(valX, valY, min_periods, ddof);
        }
      }
    }
    ret = make_rowmajor_matrix_scatter(covmat);
  } else { // use matrix version
    auto mat = to_rowmajor_matrix_double(cols);
    if (na_count) {
      auto nulls = isnull(cols).to_rowmajor_matrix_double(cols);
      ret = matrix_covariance(mat, nulls, min_periods);
    } else {
      ret = matrix_covariance(mat, min_periods, ddof, false);
    }
  }
  return ret.to_dataframe(cols);
}

dftable
mode_helper(dftable_base& df, const std::string& name, bool dropna) {
  auto grp_df = df.group_by({name});
  dftable res;

  if (dropna) res = grp_df.select({name} , {frovedis::count_as(name, "count_elements")});
  else res = grp_df.select({name} , {frovedis::size_as(name, "count_elements")});
  auto max_count = res.max<size_t>("count_elements");

  auto res2 = res.filter(eq_im("count_elements", max_count));
  res2.drop_cols({"count_elements"});

  return res2.materialize();
}

}
