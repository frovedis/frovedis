#include "dftable.hpp"
#include "dfaggregator.hpp"

namespace frovedis {

std::shared_ptr<dfcolumn> 
dfaggregator_sum::
aggregate(dftable_base& table,
          node_local<std::vector<size_t>>& local_grouped_idx,
          node_local<std::vector<size_t>>& local_idx_split,
          node_local<std::vector<std::vector<size_t>>>& hash_divide,
          node_local<std::vector<std::vector<size_t>>>& merge_map,
          node_local<size_t>& row_sizes,
          dftable& grouped_table) {
  // slice to get raw_column from execute
  dftable_base sliced_table = table;
  auto colp = col->execute(sliced_table);
  return colp->sum(local_grouped_idx, local_idx_split, hash_divide,
                   merge_map, row_sizes);
}

std::shared_ptr<dfcolumn> 
dfaggregator_count::
aggregate(dftable_base& table,
          node_local<std::vector<size_t>>& local_grouped_idx,
          node_local<std::vector<size_t>>& local_idx_split,
          node_local<std::vector<std::vector<size_t>>>& hash_divide,
          node_local<std::vector<std::vector<size_t>>>& merge_map,
          node_local<size_t>& row_sizes,
          dftable& grouped_table) {
  dftable_base sliced_table = table;
  auto colp = col->execute(sliced_table);
  return colp->count(local_grouped_idx, local_idx_split, hash_divide,
                     merge_map, row_sizes);
}

std::shared_ptr<dfcolumn> 
dfaggregator_size::
aggregate(dftable_base& table,
          node_local<std::vector<size_t>>& local_grouped_idx,
          node_local<std::vector<size_t>>& local_idx_split,
          node_local<std::vector<std::vector<size_t>>>& hash_divide,
          node_local<std::vector<std::vector<size_t>>>& merge_map,
          node_local<size_t>& row_sizes,
          dftable& grouped_table) {
  dftable_base sliced_table = table;
  auto colp = col->execute(sliced_table);
  return colp->size(local_grouped_idx, local_idx_split, hash_divide,
                     merge_map, row_sizes);
}

std::shared_ptr<dfcolumn> 
dfaggregator_avg::
aggregate(dftable_base& table,
          node_local<std::vector<size_t>>& local_grouped_idx,
          node_local<std::vector<size_t>>& local_idx_split,
          node_local<std::vector<std::vector<size_t>>>& hash_divide,
          node_local<std::vector<std::vector<size_t>>>& merge_map,
          node_local<size_t>& row_sizes,
          dftable& grouped_table) {
  dftable_base sliced_table = table;
  auto colp = col->execute(sliced_table);
  return colp->avg(local_grouped_idx, local_idx_split, hash_divide,
                   merge_map, row_sizes);
}

std::shared_ptr<dfcolumn> 
dfaggregator_var::
aggregate(dftable_base& table,
          node_local<std::vector<size_t>>& local_grouped_idx,
          node_local<std::vector<size_t>>& local_idx_split,
          node_local<std::vector<std::vector<size_t>>>& hash_divide,
          node_local<std::vector<std::vector<size_t>>>& merge_map,
          node_local<size_t>& row_sizes,
          dftable& grouped_table) {
  dftable_base sliced_table = table;
  auto colp = col->execute(sliced_table);
  return colp->var(local_grouped_idx, local_idx_split, hash_divide,
                   merge_map, row_sizes, ddof);
}

std::shared_ptr<dfcolumn> 
dfaggregator_sem::
aggregate(dftable_base& table,
          node_local<std::vector<size_t>>& local_grouped_idx,
          node_local<std::vector<size_t>>& local_idx_split,
          node_local<std::vector<std::vector<size_t>>>& hash_divide,
          node_local<std::vector<std::vector<size_t>>>& merge_map,
          node_local<size_t>& row_sizes,
          dftable& grouped_table) {
  dftable_base sliced_table = table;
  auto colp = col->execute(sliced_table);
  return colp->sem(local_grouped_idx, local_idx_split, hash_divide,
                   merge_map, row_sizes, ddof);
}

std::shared_ptr<dfcolumn>
dfaggregator_std::
aggregate(dftable_base& table,
          node_local<std::vector<size_t>>& local_grouped_idx,
          node_local<std::vector<size_t>>& local_idx_split,
          node_local<std::vector<std::vector<size_t>>>& hash_divide,
          node_local<std::vector<std::vector<size_t>>>& merge_map,
          node_local<size_t>& row_sizes,
          dftable& grouped_table) {
  dftable_base sliced_table = table;
  auto colp = col->execute(sliced_table);
  return colp->std(local_grouped_idx, local_idx_split, hash_divide,
                   merge_map, row_sizes, ddof);
}

std::shared_ptr<dfcolumn>
dfaggregator_mad::
aggregate(dftable_base& table,
          node_local<std::vector<size_t>>& local_grouped_idx,
          node_local<std::vector<size_t>>& local_idx_split,
          node_local<std::vector<std::vector<size_t>>>& hash_divide,
          node_local<std::vector<std::vector<size_t>>>& merge_map,
          node_local<size_t>& row_sizes,
          dftable& grouped_table) {
  auto avgcol = "avg_col";
  auto tcol = col->get_as();    // target column name
  auto grouped_cols = grouped_table.columns();
  auto renamed_grouped_cols = grouped_table.columns() + "_";

  dftable_base sliced_table = table;
  auto colp = col->execute(sliced_table);
  auto avg_colp = colp->avg(local_grouped_idx, local_idx_split, hash_divide,
                            merge_map, row_sizes);

  dftable t1 = table.select(grouped_cols);
  if (!vector_contains(grouped_cols, tcol)) t1.append_column(tcol, colp);

  dftable t2 = grouped_table.select(grouped_cols);
  for (auto& c: grouped_cols) t2.rename(c, c + "_");
  t2.append_column(avgcol, avg_colp);

  dftable joined;
  if (grouped_cols.size() == 1) { // single-key-group-by
    joined = t1.bcast_join(t2, eq(grouped_cols[0], renamed_grouped_cols[0]))
               .select({tcol, avgcol});
  } else {
    joined = t1.bcast_join(t2, multi_eq(grouped_cols, renamed_grouped_cols))
               .select({tcol, avgcol});
  }
  joined.call_function(abs_col_as(sub_col(tcol, avgcol),"abs_diff"));

  use_dfcolumn use(joined.raw_column("abs_diff"));
  auto diff_colp = joined.column("abs_diff");
  return diff_colp->avg(local_grouped_idx, local_idx_split, hash_divide,
                        merge_map, row_sizes);
}

std::shared_ptr<dfcolumn> 
dfaggregator_max::
aggregate(dftable_base& table,
          node_local<std::vector<size_t>>& local_grouped_idx,
          node_local<std::vector<size_t>>& local_idx_split,
          node_local<std::vector<std::vector<size_t>>>& hash_divide,
          node_local<std::vector<std::vector<size_t>>>& merge_map,
          node_local<size_t>& row_sizes,
          dftable& grouped_table) {
  dftable_base sliced_table = table;
  auto colp = col->execute(sliced_table);
  return colp->max(local_grouped_idx, local_idx_split, hash_divide,
                   merge_map, row_sizes);
}

std::shared_ptr<dfcolumn> 
dfaggregator_min::
aggregate(dftable_base& table,
          node_local<std::vector<size_t>>& local_grouped_idx,
          node_local<std::vector<size_t>>& local_idx_split,
          node_local<std::vector<std::vector<size_t>>>& hash_divide,
          node_local<std::vector<std::vector<size_t>>>& merge_map,
          node_local<size_t>& row_sizes,
          dftable& grouped_table) {
  dftable_base sliced_table = table;
  auto colp = col->execute(sliced_table);
  return colp->min(local_grouped_idx, local_idx_split, hash_divide,
                   merge_map, row_sizes);
}

std::shared_ptr<dfcolumn> 
dfaggregator_count_distinct::
aggregate(dftable_base& table,
          node_local<std::vector<size_t>>& local_grouped_idx,
          node_local<std::vector<size_t>>& local_idx_split,
          node_local<std::vector<std::vector<size_t>>>& hash_divide,
          node_local<std::vector<std::vector<size_t>>>& merge_map,
          node_local<size_t>& row_sizes,
          dftable& grouped_table) {
  dftable_base sliced_table = table;
  auto colp = col->execute(sliced_table);
  return colp->count_distinct(local_grouped_idx, local_idx_split, hash_divide,
                              merge_map, row_sizes);
}

std::shared_ptr<dfcolumn> 
dfaggregator_sum_distinct::
aggregate(dftable_base& table,
          node_local<std::vector<size_t>>& local_grouped_idx,
          node_local<std::vector<size_t>>& local_idx_split,
          node_local<std::vector<std::vector<size_t>>>& hash_divide,
          node_local<std::vector<std::vector<size_t>>>& merge_map,
          node_local<size_t>& row_sizes,
          dftable& grouped_table) {
  dftable_base sliced_table = table;
  auto colp = col->execute(sliced_table);
  return colp->sum_distinct(local_grouped_idx, local_idx_split, hash_divide,
                            merge_map, row_sizes);
}

std::shared_ptr<dfcolumn> 
dfaggregator_first::
aggregate(dftable_base& table,
          node_local<std::vector<size_t>>& local_grouped_idx,
          node_local<std::vector<size_t>>& local_idx_split,
          node_local<std::vector<std::vector<size_t>>>& hash_divide,
          node_local<std::vector<std::vector<size_t>>>& merge_map,
          node_local<size_t>& row_sizes,
          dftable& grouped_table) {
  dftable_base sliced_table = table;
  auto colp = col->execute(sliced_table);
  return colp->first(local_grouped_idx, local_idx_split, hash_divide,
                     merge_map, row_sizes, ignore_nulls);
}

std::shared_ptr<dfcolumn> 
dfaggregator_last::
aggregate(dftable_base& table,
          node_local<std::vector<size_t>>& local_grouped_idx,
          node_local<std::vector<size_t>>& local_idx_split,
          node_local<std::vector<std::vector<size_t>>>& hash_divide,
          node_local<std::vector<std::vector<size_t>>>& merge_map,
          node_local<size_t>& row_sizes,
          dftable& grouped_table) {
  dftable_base sliced_table = table;
  auto colp = col->execute(sliced_table);
  return colp->last(local_grouped_idx, local_idx_split, hash_divide,
                    merge_map, row_sizes, ignore_nulls);
}

std::shared_ptr<dfcolumn> 
dfaggregator_sum::
whole_column_aggregate(dftable_base& table) {
  auto colp = col->execute(table);
  if(colp->dtype() == "double") {
    auto colp2 = std::dynamic_pointer_cast<typed_dfcolumn<double>>(colp);
    if(!static_cast<bool>(colp2))
      throw std::runtime_error("internal type error");
    if(colp2->is_all_null()) return one_null_column<double>();
    double r = colp2->sum();
    std::vector<double> v = {r};
    return std::make_shared<typed_dfcolumn<double>>(make_dvector_scatter(v));
  } else if(colp->dtype() == "float") {
    auto colp2 = std::dynamic_pointer_cast<typed_dfcolumn<float>>(colp);
    if(!static_cast<bool>(colp2))
      throw std::runtime_error("internal type error");
    if(colp2->is_all_null()) return one_null_column<float>();
    float r = colp2->sum();
    std::vector<float> v = {r};
    return std::make_shared<typed_dfcolumn<float>>(make_dvector_scatter(v));
  } else if(colp->dtype() == "long") {
    auto colp2 = std::dynamic_pointer_cast<typed_dfcolumn<long>>(colp);
    if(!static_cast<bool>(colp2))
      throw std::runtime_error("internal type error");
    if(colp2->is_all_null()) return one_null_column<long>();
    long r = colp2->sum();
    std::vector<long> v = {r};
    return std::make_shared<typed_dfcolumn<long>>(make_dvector_scatter(v));
  } else if(colp->dtype() == "unsigned long") {
    auto colp2 = std::dynamic_pointer_cast<typed_dfcolumn<unsigned long>>(colp);
    if(!static_cast<bool>(colp2))
      throw std::runtime_error("internal type error");
    if(colp2->is_all_null()) return one_null_column<unsigned long>();
    unsigned long r = colp2->sum();
    std::vector<unsigned long> v = {r};
    return
      std::make_shared<typed_dfcolumn<unsigned long>>(make_dvector_scatter(v));
  } else if(colp->dtype() == "int") {
    auto colp2 = std::dynamic_pointer_cast<typed_dfcolumn<int>>(colp);
    if(!static_cast<bool>(colp2))
      throw std::runtime_error("internal type error");
    if(colp2->is_all_null()) return one_null_column<int>();
    int r = colp2->sum();
    std::vector<int> v = {r};
    return std::make_shared<typed_dfcolumn<int>>(make_dvector_scatter(v));
  } else if(colp->dtype() == "unsigned int") {
    auto colp2 = std::dynamic_pointer_cast<typed_dfcolumn<unsigned int>>(colp);
    if(!static_cast<bool>(colp2))
      throw std::runtime_error("internal type error");
    if(colp2->is_all_null()) return one_null_column<unsigned int>();
    unsigned int r = colp2->sum();
    std::vector<unsigned int> v = {r};
    return
      std::make_shared<typed_dfcolumn<unsigned int>>(make_dvector_scatter(v));
  } else throw std::runtime_error("unsupported type: " + colp->dtype());
}

std::shared_ptr<dfcolumn> 
dfaggregator_count::
whole_column_aggregate(dftable_base& table) {
  auto colp = col->execute(table);
  size_t r = colp->count();
  std::vector<size_t> v = {r};
  return std::make_shared<typed_dfcolumn<size_t>>(make_dvector_scatter(v));
}

std::shared_ptr<dfcolumn> 
dfaggregator_size::
whole_column_aggregate(dftable_base& table) {
  auto colp = col->execute(table);
  size_t r = colp->size();
  std::vector<size_t> v = {r};
  return std::make_shared<typed_dfcolumn<size_t>>(make_dvector_scatter(v));
}

std::shared_ptr<dfcolumn> 
dfaggregator_avg::
whole_column_aggregate(dftable_base& table) {
  auto colp = col->execute(table);
  if(colp->is_all_null()) return one_null_column<double>(); 
  double r = colp->avg();
  std::vector<double> v = {r};
  return std::make_shared<typed_dfcolumn<double>>(make_dvector_scatter(v));
}

std::shared_ptr<dfcolumn>
dfaggregator_var::
whole_column_aggregate(dftable_base& table) {
  auto colp = col->execute(table);
  if(colp->is_all_null()) return one_null_column<double>(); 
  double r = colp->var(ddof);
  std::vector<double> v = {r};
  return std::make_shared<typed_dfcolumn<double>>(make_dvector_scatter(v));
}

std::shared_ptr<dfcolumn>
dfaggregator_sem::
whole_column_aggregate(dftable_base& table) {
  auto colp = col->execute(table);
  if(colp->is_all_null()) return one_null_column<double>(); 
  double r = colp->sem(ddof);
  std::vector<double> v = {r};
  return std::make_shared<typed_dfcolumn<double>>(make_dvector_scatter(v));
}

std::shared_ptr<dfcolumn>
dfaggregator_std::
whole_column_aggregate(dftable_base& table) {
  auto colp = col->execute(table);
  if(colp->is_all_null()) return one_null_column<double>(); 
  double r = colp->std(ddof);
  std::vector<double> v = {r};
  return std::make_shared<typed_dfcolumn<double>>(make_dvector_scatter(v));
}

std::shared_ptr<dfcolumn>
dfaggregator_mad::
whole_column_aggregate(dftable_base& table) {
  auto colp = col->execute(table);
  if(colp->is_all_null()) return one_null_column<double>(); 
  double r = colp->mad();
  std::vector<double> v = {r};
  return std::make_shared<typed_dfcolumn<double>>(make_dvector_scatter(v));
}

template <>
std::shared_ptr<typed_dfcolumn<datetime>>
one_null_column() {
  auto max = std::numeric_limits<datetime_t>::max();
  std::vector<datetime_t> v = {max};
  std::vector<size_t> nullsv = {0};
  auto val = make_dvector_scatter(v).moveto_node_local();
  auto nulls = make_dvector_scatter(nullsv).moveto_node_local();
  return std::make_shared<typed_dfcolumn<datetime>>(std::move(val),std::move(nulls));
}

std::shared_ptr<dfcolumn>
dfaggregator_max::
whole_column_aggregate(dftable_base& table) {
  auto colp = col->execute(table);
  if(colp->dtype() == "double") {
    auto colp2 = std::dynamic_pointer_cast<typed_dfcolumn<double>>(colp);
    if(!static_cast<bool>(colp2))
      throw std::runtime_error("internal type error");
    if(colp2->is_all_null()) return one_null_column<double>();
    double r = colp2->max();
    std::vector<double> v = {r};
    return std::make_shared<typed_dfcolumn<double>>(make_dvector_scatter(v));
  } else if(colp->dtype() == "float") {
    auto colp2 = std::dynamic_pointer_cast<typed_dfcolumn<float>>(colp);
    if(!static_cast<bool>(colp2))
      throw std::runtime_error("internal type error");
    if(colp2->is_all_null()) return one_null_column<float>();
    float r = colp2->max();
    std::vector<float> v = {r};
    return std::make_shared<typed_dfcolumn<float>>(make_dvector_scatter(v));
  } else if(colp->dtype() == "long") {
    auto colp2 = std::dynamic_pointer_cast<typed_dfcolumn<long>>(colp);
    if(!static_cast<bool>(colp2))
      throw std::runtime_error("internal type error");
    if(colp2->is_all_null()) return one_null_column<long>();
    long r = colp2->max();
    std::vector<long> v = {r};
    return std::make_shared<typed_dfcolumn<long>>(make_dvector_scatter(v));
  } else if(colp->dtype() == "unsigned long") {
    auto colp2 = std::dynamic_pointer_cast<typed_dfcolumn<unsigned long>>(colp);
    if(!static_cast<bool>(colp2))
      throw std::runtime_error("internal type error");
    if(colp2->is_all_null()) return one_null_column<unsigned long>();
    unsigned long r = colp2->max();
    std::vector<unsigned long> v = {r};
    return
      std::make_shared<typed_dfcolumn<unsigned long>>(make_dvector_scatter(v));
  } else if(colp->dtype() == "int") {
    auto colp2 = std::dynamic_pointer_cast<typed_dfcolumn<int>>(colp);
    if(!static_cast<bool>(colp2))
      throw std::runtime_error("internal type error");
    if(colp2->is_all_null()) return one_null_column<int>();
    int r = colp2->max();
    std::vector<int> v = {r};
    return std::make_shared<typed_dfcolumn<int>>(make_dvector_scatter(v));
  } else if(colp->dtype() == "unsigned int") {
    auto colp2 = std::dynamic_pointer_cast<typed_dfcolumn<unsigned int>>(colp);
    if(!static_cast<bool>(colp2))
      throw std::runtime_error("internal type error");
    if(colp2->is_all_null()) return one_null_column<unsigned int>();
    unsigned int r = colp2->max();
    std::vector<unsigned int> v = {r};
    return
      std::make_shared<typed_dfcolumn<unsigned int>>(make_dvector_scatter(v));
  } else if(colp->dtype() == "datetime") {
    auto colp2 = std::dynamic_pointer_cast<typed_dfcolumn<datetime>>(colp);
    if(!static_cast<bool>(colp2))
      throw std::runtime_error("internal type error");
    if(colp2->is_all_null()) return one_null_column<datetime>();
    datetime_t r = colp2->max();
    std::vector<datetime_t> v = {r};
    return
      std::make_shared<typed_dfcolumn<datetime>>(make_dvector_scatter(v));
  } else throw std::runtime_error("unsupported type: " + colp->dtype());
}

std::shared_ptr<dfcolumn>
dfaggregator_min::
whole_column_aggregate(dftable_base& table) {
  auto colp = col->execute(table);
  if(colp->dtype() == "double") {
    auto colp2 = std::dynamic_pointer_cast<typed_dfcolumn<double>>(colp);
    if(!static_cast<bool>(colp2))
      throw std::runtime_error("internal type error");
    if(colp2->is_all_null()) return one_null_column<double>();
    double r = colp2->min();
    std::vector<double> v = {r};
    return std::make_shared<typed_dfcolumn<double>>(make_dvector_scatter(v));
  } else if(colp->dtype() == "float") {
    auto colp2 = std::dynamic_pointer_cast<typed_dfcolumn<float>>(colp);
    if(!static_cast<bool>(colp2))
      throw std::runtime_error("internal type error");
    if(colp2->is_all_null()) return one_null_column<float>();
    float r = colp2->min();
    std::vector<float> v = {r};
    return std::make_shared<typed_dfcolumn<float>>(make_dvector_scatter(v));
  } else if(colp->dtype() == "long") {
    auto colp2 = std::dynamic_pointer_cast<typed_dfcolumn<long>>(colp);
    if(!static_cast<bool>(colp2))
      throw std::runtime_error("internal type error");
    if(colp2->is_all_null()) return one_null_column<long>();
    long r = colp2->min();
    std::vector<long> v = {r};
    return std::make_shared<typed_dfcolumn<long>>(make_dvector_scatter(v));
  } else if(colp->dtype() == "unsigned long") {
    auto colp2 = std::dynamic_pointer_cast<typed_dfcolumn<unsigned long>>(colp);
    if(!static_cast<bool>(colp2))
      throw std::runtime_error("internal type error");
    if(colp2->is_all_null()) return one_null_column<unsigned long>();
    unsigned long r = colp2->min();
    std::vector<unsigned long> v = {r};
    return
      std::make_shared<typed_dfcolumn<unsigned long>>(make_dvector_scatter(v));
  } else if(colp->dtype() == "int") {
    auto colp2 = std::dynamic_pointer_cast<typed_dfcolumn<int>>(colp);
    if(!static_cast<bool>(colp2))
      throw std::runtime_error("internal type error");
    if(colp2->is_all_null()) return one_null_column<int>();
    int r = colp2->min();
    std::vector<int> v = {r};
    return std::make_shared<typed_dfcolumn<int>>(make_dvector_scatter(v));
  } else if(colp->dtype() == "unsigned int") {
    auto colp2 = std::dynamic_pointer_cast<typed_dfcolumn<unsigned int>>(colp);
    if(!static_cast<bool>(colp2))
      throw std::runtime_error("internal type error");
    if(colp2->is_all_null()) return one_null_column<unsigned int>();
    unsigned int r = colp2->min();
    std::vector<unsigned int> v = {r};
    return
      std::make_shared<typed_dfcolumn<unsigned int>>(make_dvector_scatter(v));
  } else if(colp->dtype() == "datetime") {
    auto colp2 = std::dynamic_pointer_cast<typed_dfcolumn<datetime>>(colp);
    if(!static_cast<bool>(colp2))
      throw std::runtime_error("internal type error");
    if(colp2->is_all_null()) return one_null_column<datetime>();
    datetime_t r = colp2->min();
    std::vector<datetime_t> v = {r};
    return
      std::make_shared<typed_dfcolumn<datetime>>(make_dvector_scatter(v));
  } else throw std::runtime_error("unsupported type: " + colp->dtype());
}

std::shared_ptr<dfcolumn>
dfaggregator_count_distinct::
whole_column_aggregate(dftable_base& table) {
  auto colp = col->execute(table);
  dftable tmp;
  std::string c("tmp");
  tmp.append_column(c, colp);
  auto tmp2 = tmp.group_by({c}).select({c}).aggregate({count_as(c,c)});
  return tmp2.column(c);
}

std::shared_ptr<dfcolumn>
dfaggregator_sum_distinct::
whole_column_aggregate(dftable_base& table) {
  auto colp = col->execute(table);
  dftable tmp;
  std::string c("tmp");
  tmp.append_column(c, colp);
  auto tmp2 = tmp.group_by({c}).select({c}).aggregate({sum_as(c,c)});
  return tmp2.column(c);
}

std::shared_ptr<dfcolumn>
dfaggregator_first::
whole_column_aggregate(dftable_base& table) {
  auto colp = col->execute(table);
  if(colp->dtype() == "double") {
    auto colp2 = std::dynamic_pointer_cast<typed_dfcolumn<double>>(colp);
    if(!static_cast<bool>(colp2))
      throw std::runtime_error("internal type error");
    double r = colp2->first(ignore_nulls);
    if(r == std::numeric_limits<double>::max()) {
      return one_null_column<double>();
    } else {
      std::vector<double> v = {r};
      return std::make_shared<typed_dfcolumn<double>>(make_dvector_scatter(v));
    }
  } else if(colp->dtype() == "float") {
    auto colp2 = std::dynamic_pointer_cast<typed_dfcolumn<float>>(colp);
    if(!static_cast<bool>(colp2))
      throw std::runtime_error("internal type error");
    float r = colp2->first(ignore_nulls);
    if(r == std::numeric_limits<float>::max()) {
      return one_null_column<float>();
    } else {
      std::vector<float> v = {r};
      return std::make_shared<typed_dfcolumn<float>>(make_dvector_scatter(v));
    }
  } else if(colp->dtype() == "long") {
    auto colp2 = std::dynamic_pointer_cast<typed_dfcolumn<long>>(colp);
    if(!static_cast<bool>(colp2))
      throw std::runtime_error("internal type error");
    long r = colp2->first(ignore_nulls);
    if(r == std::numeric_limits<long>::max()) {
      return one_null_column<long>();
    } else {
      std::vector<long> v = {r};
      return std::make_shared<typed_dfcolumn<long>>(make_dvector_scatter(v));
    }
  } else if(colp->dtype() == "unsigned long") {
    auto colp2 = std::dynamic_pointer_cast<typed_dfcolumn<unsigned long>>(colp);
    if(!static_cast<bool>(colp2))
      throw std::runtime_error("internal type error");
    unsigned long r = colp2->first(ignore_nulls);
    if(r == std::numeric_limits<unsigned long>::max()) {
      return one_null_column<unsigned long>();
    } else {
      std::vector<unsigned long> v = {r};
      return std::make_shared<typed_dfcolumn<unsigned long>>
        (make_dvector_scatter(v));
    }
  } else if(colp->dtype() == "int") {
    auto colp2 = std::dynamic_pointer_cast<typed_dfcolumn<int>>(colp);
    if(!static_cast<bool>(colp2))
      throw std::runtime_error("internal type error");
    int r = colp2->first(ignore_nulls);
    if(r == std::numeric_limits<int>::max()) {
      return one_null_column<int>();
    } else {
      std::vector<int> v = {r};
      return std::make_shared<typed_dfcolumn<int>>(make_dvector_scatter(v));
    }
  } else if(colp->dtype() == "unsigned int") {
    auto colp2 = std::dynamic_pointer_cast<typed_dfcolumn<unsigned int>>(colp);
    if(!static_cast<bool>(colp2))
      throw std::runtime_error("internal type error");
    unsigned int r = colp2->first(ignore_nulls);
    if(r == std::numeric_limits<unsigned int>::max()) {
      return one_null_column<unsigned int>();
    } else {
      std::vector<unsigned int> v = {r};
      return std::make_shared<typed_dfcolumn<unsigned int>>
        (make_dvector_scatter(v));
    }
  } else if(colp->dtype() == "string") {
    auto colp2 = std::dynamic_pointer_cast<typed_dfcolumn<std::string>>(colp);
    if(!static_cast<bool>(colp2))
      throw std::runtime_error("internal type error");
    std::string r = colp2->first(ignore_nulls);
    std::vector<std::string> rv = {r};
    dftable tmp;
    tmp.append_string_column("tmp", make_dvector_scatter(rv),true);
    return tmp.column("tmp");
  } else if(colp->dtype() == "dic_string") {
    auto colp2 = std::dynamic_pointer_cast<typed_dfcolumn<dic_string>>(colp);
    if(!static_cast<bool>(colp2))
      throw std::runtime_error("internal type error");
    std::string r = colp2->first(ignore_nulls);
    std::vector<std::string> rv = {r};
    dftable tmp;
    tmp.append_dic_string_column("tmp", make_dvector_scatter(rv), true);
    return tmp.column("tmp");
  } else if(colp->dtype() == "datetime") {
    auto colp2 = std::dynamic_pointer_cast<typed_dfcolumn<datetime>>(colp);
    if(!static_cast<bool>(colp2))
      throw std::runtime_error("internal type error");
    datetime_t r = colp2->first(ignore_nulls);
    std::vector<datetime_t> rv = {r};
    dftable tmp;
    tmp.append_datetime_column("tmp", make_dvector_scatter(rv), true);
    return tmp.column("tmp");
  } else throw std::runtime_error("unsupported type: " + colp->dtype());
}

std::shared_ptr<dfcolumn>
dfaggregator_last::
whole_column_aggregate(dftable_base& table) {
  auto colp = col->execute(table);
  if(colp->dtype() == "double") {
    auto colp2 = std::dynamic_pointer_cast<typed_dfcolumn<double>>(colp);
    if(!static_cast<bool>(colp2))
      throw std::runtime_error("internal type error");
    double r = colp2->last(ignore_nulls);
    if(r == std::numeric_limits<double>::max()) {
      return one_null_column<double>();
    } else {
      std::vector<double> v = {r};
      return std::make_shared<typed_dfcolumn<double>>(make_dvector_scatter(v));
    }
  } else if(colp->dtype() == "float") {
    auto colp2 = std::dynamic_pointer_cast<typed_dfcolumn<float>>(colp);
    if(!static_cast<bool>(colp2))
      throw std::runtime_error("internal type error");
    float r = colp2->last(ignore_nulls);
    if(r == std::numeric_limits<float>::max()) {
      return one_null_column<float>();
    } else {
      std::vector<float> v = {r};
      return std::make_shared<typed_dfcolumn<float>>(make_dvector_scatter(v));
    }
  } else if(colp->dtype() == "long") {
    auto colp2 = std::dynamic_pointer_cast<typed_dfcolumn<long>>(colp);
    if(!static_cast<bool>(colp2))
      throw std::runtime_error("internal type error");
    long r = colp2->last(ignore_nulls);
    if(r == std::numeric_limits<long>::max()) {
      return one_null_column<long>();
    } else {
      std::vector<long> v = {r};
      return std::make_shared<typed_dfcolumn<long>>(make_dvector_scatter(v));
    }
  } else if(colp->dtype() == "unsigned long") {
    auto colp2 = std::dynamic_pointer_cast<typed_dfcolumn<unsigned long>>(colp);
    if(!static_cast<bool>(colp2))
      throw std::runtime_error("internal type error");
    unsigned long r = colp2->last(ignore_nulls);
    if(r == std::numeric_limits<unsigned long>::max()) {
      return one_null_column<unsigned long>();
    } else {
      std::vector<unsigned long> v = {r};
      return std::make_shared<typed_dfcolumn<unsigned long>>
        (make_dvector_scatter(v));
    }
  } else if(colp->dtype() == "int") {
    auto colp2 = std::dynamic_pointer_cast<typed_dfcolumn<int>>(colp);
    if(!static_cast<bool>(colp2))
      throw std::runtime_error("internal type error");
    int r = colp2->last(ignore_nulls);
    if(r == std::numeric_limits<int>::max()) {
      return one_null_column<int>();
    } else {
      std::vector<int> v = {r};
      return std::make_shared<typed_dfcolumn<int>>(make_dvector_scatter(v));
    }
  } else if(colp->dtype() == "unsigned int") {
    auto colp2 = std::dynamic_pointer_cast<typed_dfcolumn<unsigned int>>(colp);
    if(!static_cast<bool>(colp2))
      throw std::runtime_error("internal type error");
    unsigned int r = colp2->last(ignore_nulls);
    if(r == std::numeric_limits<unsigned int>::max()) {
      return one_null_column<unsigned int>();
    } else {
      std::vector<unsigned int> v = {r};
      return std::make_shared<typed_dfcolumn<unsigned int>>
        (make_dvector_scatter(v));
    }
  } else if(colp->dtype() == "string") {
    auto colp2 = std::dynamic_pointer_cast<typed_dfcolumn<std::string>>(colp);
    if(!static_cast<bool>(colp2))
      throw std::runtime_error("internal type error");
    std::string r = colp2->last(ignore_nulls);
    std::vector<std::string> rv = {r};
    dftable tmp;
    tmp.append_string_column("tmp", make_dvector_scatter(rv),true);
    return tmp.column("tmp");
  } else if(colp->dtype() == "dic_string") {
    auto colp2 = std::dynamic_pointer_cast<typed_dfcolumn<dic_string>>(colp);
    if(!static_cast<bool>(colp2))
      throw std::runtime_error("internal type error");
    std::string r = colp2->last(ignore_nulls);
    std::vector<std::string> rv = {r};
    dftable tmp;
    tmp.append_dic_string_column("tmp", make_dvector_scatter(rv), true);
    return tmp.column("tmp");
  } else if(colp->dtype() == "datetime") {
    auto colp2 = std::dynamic_pointer_cast<typed_dfcolumn<datetime>>(colp);
    if(!static_cast<bool>(colp2))
      throw std::runtime_error("internal type error");
    datetime_t r = colp2->last(ignore_nulls);
    std::vector<datetime_t> rv = {r};
    dftable tmp;
    tmp.append_datetime_column("tmp", make_dvector_scatter(rv), true);
    return tmp.column("tmp");
  } else throw std::runtime_error("unsupported type: " + colp->dtype());
}

std::shared_ptr<dfaggregator> sum(const std::string& col) {
  return std::make_shared<dfaggregator_sum>(id_col(col));
}

std::shared_ptr<dfaggregator> sum(const std::shared_ptr<dffunction>& col) {
  return std::make_shared<dfaggregator_sum>(col);
}

std::shared_ptr<dfaggregator> sum_as(const std::string& col,
                                     const std::string& as) {
  return std::make_shared<dfaggregator_sum>(id_col(col),as);
}

std::shared_ptr<dfaggregator> sum_as(const std::shared_ptr<dffunction>& col,
                                     const std::string& as) {
  return std::make_shared<dfaggregator_sum>(col,as);
}

std::shared_ptr<dfaggregator> count(const std::string& col) {
  return std::make_shared<dfaggregator_count>(id_col(col));
}

std::shared_ptr<dfaggregator> count(const std::shared_ptr<dffunction>& col) {
  return std::make_shared<dfaggregator_count>(col);
}

std::shared_ptr<dfaggregator> count_as(const std::string& col,
                                       const std::string& as) {
  return std::make_shared<dfaggregator_count>(id_col(col),as);
}

std::shared_ptr<dfaggregator> count_as(const std::shared_ptr<dffunction>& col,
                                       const std::string& as) {
  return std::make_shared<dfaggregator_count>(col,as);
}

std::shared_ptr<dfaggregator> size(const std::string& col) {
  return std::make_shared<dfaggregator_size>(id_col(col));
}

std::shared_ptr<dfaggregator> size(const std::shared_ptr<dffunction>& col) {
  return std::make_shared<dfaggregator_size>(col);
}

std::shared_ptr<dfaggregator> size_as(const std::string& col,
                                      const std::string& as) {
  return std::make_shared<dfaggregator_size>(id_col(col),as);
}

std::shared_ptr<dfaggregator> size_as(const std::shared_ptr<dffunction>& col,
                                      const std::string& as) {
  return std::make_shared<dfaggregator_size>(col,as);
}

// size (including nulls) of any column
std::shared_ptr<dfaggregator> count_all() {
  return std::make_shared<dfaggregator_size>(id_col("*"));
}

std::shared_ptr<dfaggregator> count_all_as(const std::string& as) {
  return std::make_shared<dfaggregator_size>(id_col("*"),as);
}

std::shared_ptr<dfaggregator> avg(const std::string& col) {
  return std::make_shared<dfaggregator_avg>(id_col(col));
}

std::shared_ptr<dfaggregator> avg(const std::shared_ptr<dffunction>& col) {
  return std::make_shared<dfaggregator_avg>(col);
}

std::shared_ptr<dfaggregator> avg_as(const std::string& col,
                                     const std::string& as) {
  return std::make_shared<dfaggregator_avg>(id_col(col),as);
}

std::shared_ptr<dfaggregator> avg_as(const std::shared_ptr<dffunction>& col,
                                     const std::string& as) {
  return std::make_shared<dfaggregator_avg>(col,as);
}

std::shared_ptr<dfaggregator> var(const std::string& col, const double& ddof) {
  return std::make_shared<dfaggregator_var>(id_col(col), ddof);
}

std::shared_ptr<dfaggregator> var(const std::shared_ptr<dffunction>& col,
                                  const double& ddof) {
  return std::make_shared<dfaggregator_var>(col, ddof);
}

std::shared_ptr<dfaggregator> var_as(const std::string& col,
                                     const std::string& as,
                                     const double& ddof) {
  return std::make_shared<dfaggregator_var>(id_col(col),as, ddof);
}

std::shared_ptr<dfaggregator> var_as(const std::shared_ptr<dffunction>& col,
                                     const std::string& as,
                                     const double& ddof) {
  return std::make_shared<dfaggregator_var>(col,as, ddof);
}

std::shared_ptr<dfaggregator> sem(const std::string& col, const double& ddof) {
  return std::make_shared<dfaggregator_sem>(id_col(col), ddof);
}

std::shared_ptr<dfaggregator> sem(const std::shared_ptr<dffunction>& col,
                                  const double& ddof) {
  return std::make_shared<dfaggregator_sem>(col, ddof);
}

std::shared_ptr<dfaggregator> sem_as(const std::string& col,
                                     const std::string& as,
                                     const double& ddof) {
  return std::make_shared<dfaggregator_sem>(id_col(col),as, ddof);
}

std::shared_ptr<dfaggregator> sem_as(const std::shared_ptr<dffunction>& col,
                                     const std::string& as,
                                     const double& ddof) {
  return std::make_shared<dfaggregator_sem>(col,as, ddof);
}

std::shared_ptr<dfaggregator> std(const std::string& col, const double& ddof) {
  return std::make_shared<dfaggregator_std>(id_col(col), ddof);
}

std::shared_ptr<dfaggregator> std(const std::shared_ptr<dffunction>& col,
                                  const double& ddof) {
  return std::make_shared<dfaggregator_std>(col, ddof);
}

std::shared_ptr<dfaggregator> std_as(const std::string& col,
                                     const std::string& as,
                                     const double& ddof) {
  return std::make_shared<dfaggregator_std>(id_col(col),as, ddof);
}

std::shared_ptr<dfaggregator> std_as(const std::shared_ptr<dffunction>& col,
                                     const std::string& as,
                                     const double& ddof) {
  return std::make_shared<dfaggregator_std>(col,as, ddof);
}

std::shared_ptr<dfaggregator> mad(const std::string& col) {
  return std::make_shared<dfaggregator_mad>(id_col(col));
}

std::shared_ptr<dfaggregator> mad(const std::shared_ptr<dffunction>& col) {
  return std::make_shared<dfaggregator_mad>(col);
}

std::shared_ptr<dfaggregator> mad_as(const std::string& col,
                                     const std::string& as) {
  return std::make_shared<dfaggregator_mad>(id_col(col),as);
}

std::shared_ptr<dfaggregator> mad_as(const std::shared_ptr<dffunction>& col,
                                     const std::string& as) {
  return std::make_shared<dfaggregator_mad>(col,as);
}

std::shared_ptr<dfaggregator> max(const std::string& col) {
  return std::make_shared<dfaggregator_max>(id_col(col));
}

std::shared_ptr<dfaggregator> max(const std::shared_ptr<dffunction>& col) {
  return std::make_shared<dfaggregator_max>(col);
}

std::shared_ptr<dfaggregator> max_as(const std::string& col,
                                     const std::string& as) {
  return std::make_shared<dfaggregator_max>(id_col(col),as);
}

std::shared_ptr<dfaggregator> max_as(const std::shared_ptr<dffunction>& col,
                                     const std::string& as) {
  return std::make_shared<dfaggregator_max>(col,as);
}

std::shared_ptr<dfaggregator> min(const std::string& col) {
  return std::make_shared<dfaggregator_min>(id_col(col));
}

std::shared_ptr<dfaggregator> min(const std::shared_ptr<dffunction>& col) {
  return std::make_shared<dfaggregator_min>(col);
}

std::shared_ptr<dfaggregator> min_as(const std::string& col,
                                     const std::string& as) {
  return std::make_shared<dfaggregator_min>(id_col(col),as);
}

std::shared_ptr<dfaggregator> min_as(const std::shared_ptr<dffunction>& col,
                                     const std::string& as) {
  return std::make_shared<dfaggregator_min>(col,as);
}

std::shared_ptr<dfaggregator> count_distinct(const std::string& col) {
  return std::make_shared<dfaggregator_count_distinct>(id_col(col));
}

std::shared_ptr<dfaggregator>
count_distinct(const std::shared_ptr<dffunction>& col) {
  return std::make_shared<dfaggregator_count_distinct>(col);
}

std::shared_ptr<dfaggregator> count_distinct_as(const std::string& col,
                                                const std::string& as) {
  return std::make_shared<dfaggregator_count_distinct>(id_col(col),as);
}

std::shared_ptr<dfaggregator>
count_distinct_as(const std::shared_ptr<dffunction>& col,
                  const std::string& as) {
  return std::make_shared<dfaggregator_count_distinct>(col,as);
}

std::shared_ptr<dfaggregator> sum_distinct(const std::string& col) {
  return std::make_shared<dfaggregator_sum_distinct>(id_col(col));
}

std::shared_ptr<dfaggregator>
sum_distinct(const std::shared_ptr<dffunction>& col) {
  return std::make_shared<dfaggregator_sum_distinct>(col);
}

std::shared_ptr<dfaggregator> sum_distinct_as(const std::string& col,
                                              const std::string& as) {
  return std::make_shared<dfaggregator_sum_distinct>(id_col(col),as);
}

std::shared_ptr<dfaggregator>
sum_distinct_as(const std::shared_ptr<dffunction>& col,
                const std::string& as) {
  return std::make_shared<dfaggregator_sum_distinct>(col,as);
}

std::shared_ptr<dfaggregator> first(const std::string& col, bool ignore_nulls) {
  return std::make_shared<dfaggregator_first>(id_col(col),ignore_nulls);
}

std::shared_ptr<dfaggregator>
first(const std::shared_ptr<dffunction>& col, bool ignore_nulls) {
  return std::make_shared<dfaggregator_first>(col,ignore_nulls);
}

std::shared_ptr<dfaggregator> first_as(const std::string& col,
                                       const std::string& as,
                                       bool ignore_nulls) {
  return std::make_shared<dfaggregator_first>(id_col(col),as,ignore_nulls);
}

std::shared_ptr<dfaggregator>
first_as(const std::shared_ptr<dffunction>& col,
         const std::string& as,
         bool ignore_nulls) {
  return std::make_shared<dfaggregator_first>(col,as,ignore_nulls);
}

std::shared_ptr<dfaggregator> last(const std::string& col, bool ignore_nulls) {
  return std::make_shared<dfaggregator_last>(id_col(col),ignore_nulls);
}

std::shared_ptr<dfaggregator>
last(const std::shared_ptr<dffunction>& col, bool ignore_nulls) {
  return std::make_shared<dfaggregator_last>(col,ignore_nulls);
}

std::shared_ptr<dfaggregator> last_as(const std::string& col,
                                      const std::string& as,
                                      bool ignore_nulls) {
  return std::make_shared<dfaggregator_last>(id_col(col),as,ignore_nulls);
}

std::shared_ptr<dfaggregator>
last_as(const std::shared_ptr<dffunction>& col,
        const std::string& as,
        bool ignore_nulls) {
  return std::make_shared<dfaggregator_last>(col,as,ignore_nulls);
}

// ----- operators -----
std::shared_ptr<dffunction> operator+(const std::shared_ptr<dffunction>& a,
                                      const std::shared_ptr<dffunction>& b) {
  return add_col(a,b);
}

std::shared_ptr<dffunction> operator+(const std::shared_ptr<dffunction>& a,
                                      const std::shared_ptr<dfaggregator>& b) {
  return add_col(a,b);
}

std::shared_ptr<dffunction> operator+(const std::shared_ptr<dffunction>& a,
                                      const std::shared_ptr<dfoperator>& b) {
  return add_col(a,b);
}

std::shared_ptr<dffunction> operator+(const std::shared_ptr<dfaggregator>& a,
                                      const std::shared_ptr<dffunction>& b) {
  return add_col(a,b);
}

std::shared_ptr<dffunction> operator+(const std::shared_ptr<dfaggregator>& a,
                                      const std::shared_ptr<dfaggregator>& b) {
  return add_col(a,b);
}

std::shared_ptr<dffunction> operator+(const std::shared_ptr<dfaggregator>& a,
                                      const std::shared_ptr<dfoperator>& b) {
  return add_col(a,b);
}

std::shared_ptr<dffunction> operator+(const std::shared_ptr<dfoperator>& a,
                                      const std::shared_ptr<dffunction>& b) {
  return add_col(a,b);
}

std::shared_ptr<dffunction> operator+(const std::shared_ptr<dfoperator>& a,
                                      const std::shared_ptr<dfaggregator>& b) {
  return add_col(a,b);
}

std::shared_ptr<dffunction> operator+(const std::shared_ptr<dfoperator>& a,
                                      const std::shared_ptr<dfoperator>& b) {
  return add_col(a,b);
}

std::shared_ptr<dffunction> operator*(const std::shared_ptr<dffunction>& a,
                                      const std::shared_ptr<dffunction>& b) {
  return mul_col(a,b);
}

std::shared_ptr<dffunction> operator*(const std::shared_ptr<dffunction>& a,
                                      const std::shared_ptr<dfaggregator>& b) {
  return mul_col(a,b);
}

std::shared_ptr<dffunction> operator*(const std::shared_ptr<dffunction>& a,
                                      const std::shared_ptr<dfoperator>& b) {
  return mul_col(a,b);
}

std::shared_ptr<dffunction> operator*(const std::shared_ptr<dfaggregator>& a,
                                      const std::shared_ptr<dffunction>& b) {
  return mul_col(a,b);
}

std::shared_ptr<dffunction> operator*(const std::shared_ptr<dfaggregator>& a,
                                      const std::shared_ptr<dfoperator>& b) {
  return mul_col(a,b);
}

std::shared_ptr<dffunction> operator*(const std::shared_ptr<dfaggregator>& a,
                                      const std::shared_ptr<dfaggregator>& b) {
  return mul_col(a,b);
}

std::shared_ptr<dffunction> operator*(const std::shared_ptr<dfoperator>& a,
                                      const std::shared_ptr<dffunction>& b) {
  return mul_col(a,b);
}

std::shared_ptr<dffunction> operator*(const std::shared_ptr<dfoperator>& a,
                                      const std::shared_ptr<dfaggregator>& b) {
  return mul_col(a,b);
}

std::shared_ptr<dffunction> operator*(const std::shared_ptr<dfoperator>& a,
                                      const std::shared_ptr<dfoperator>& b) {
  return mul_col(a,b);
}

std::shared_ptr<dffunction> operator-(const std::shared_ptr<dffunction>& a,
                                      const std::shared_ptr<dffunction>& b) {
  return sub_col(a,b);
}

std::shared_ptr<dffunction> operator-(const std::shared_ptr<dffunction>& a,
                                      const std::shared_ptr<dfaggregator>& b) {
  return sub_col(a,b);
}

std::shared_ptr<dffunction> operator-(const std::shared_ptr<dffunction>& a,
                                      const std::shared_ptr<dfoperator>& b) {
  return sub_col(a,b);
}

std::shared_ptr<dffunction> operator-(const std::shared_ptr<dfaggregator>& a,
                                      const std::shared_ptr<dffunction>& b) {
  return sub_col(a,b);
}

std::shared_ptr<dffunction> operator-(const std::shared_ptr<dfaggregator>& a,
                                      const std::shared_ptr<dfoperator>& b) {
  return sub_col(a,b);
}

std::shared_ptr<dffunction> operator-(const std::shared_ptr<dfaggregator>& a,
                                      const std::shared_ptr<dfaggregator>& b) {
  return sub_col(a,b);
}

std::shared_ptr<dffunction> operator-(const std::shared_ptr<dfoperator>& a,
                                      const std::shared_ptr<dffunction>& b) {
  return sub_col(a,b);
}

std::shared_ptr<dffunction> operator-(const std::shared_ptr<dfoperator>& a,
                                      const std::shared_ptr<dfaggregator>& b) {
  return sub_col(a,b);
}

std::shared_ptr<dffunction> operator-(const std::shared_ptr<dfoperator>& a,
                                      const std::shared_ptr<dfoperator>& b) {
  return sub_col(a,b);
}

std::shared_ptr<dffunction> operator/(const std::shared_ptr<dffunction>& a,
                                      const std::shared_ptr<dffunction>& b) {
  return fdiv_col(a,b);
}

std::shared_ptr<dffunction> operator/(const std::shared_ptr<dffunction>& a,
                                      const std::shared_ptr<dfaggregator>& b) {
  return fdiv_col(a,b);
}

std::shared_ptr<dffunction> operator/(const std::shared_ptr<dffunction>& a,
                                      const std::shared_ptr<dfoperator>& b) {
  return fdiv_col(a,b);
}

std::shared_ptr<dffunction> operator/(const std::shared_ptr<dfaggregator>& a,
                                      const std::shared_ptr<dffunction>& b) {
  return fdiv_col(a,b);
}

std::shared_ptr<dffunction> operator/(const std::shared_ptr<dfaggregator>& a,
                                      const std::shared_ptr<dfoperator>& b) {
  return fdiv_col(a,b);
}

std::shared_ptr<dffunction> operator/(const std::shared_ptr<dfaggregator>& a,
                                      const std::shared_ptr<dfaggregator>& b) {
  return fdiv_col(a,b);
}

std::shared_ptr<dffunction> operator/(const std::shared_ptr<dfoperator>& a,
                                      const std::shared_ptr<dffunction>& b) {
  return fdiv_col(a,b);
}

std::shared_ptr<dffunction> operator/(const std::shared_ptr<dfoperator>& a,
                                      const std::shared_ptr<dfaggregator>& b) {
  return fdiv_col(a,b);
}

std::shared_ptr<dffunction> operator/(const std::shared_ptr<dfoperator>& a,
                                      const std::shared_ptr<dfoperator>& b) {
  return fdiv_col(a,b);
}

std::shared_ptr<dffunction> operator%(const std::shared_ptr<dffunction>& a,
                                      const std::shared_ptr<dffunction>& b) {
  return mod_col(a,b);
}

std::shared_ptr<dffunction> operator%(const std::shared_ptr<dffunction>& a,
                                      const std::shared_ptr<dfaggregator>& b) {
  return mod_col(a,b);
}

std::shared_ptr<dffunction> operator%(const std::shared_ptr<dffunction>& a,
                                      const std::shared_ptr<dfoperator>& b) {
  return mod_col(a,b);
}

std::shared_ptr<dffunction> operator%(const std::shared_ptr<dfaggregator>& a,
                                      const std::shared_ptr<dffunction>& b) {
  return mod_col(a,b);
}

std::shared_ptr<dffunction> operator%(const std::shared_ptr<dfaggregator>& a,
                                      const std::shared_ptr<dfoperator>& b) {
  return mod_col(a,b);
}

std::shared_ptr<dffunction> operator%(const std::shared_ptr<dfaggregator>& a,
                                      const std::shared_ptr<dfaggregator>& b) {
  return mod_col(a,b);
}

std::shared_ptr<dffunction> operator%(const std::shared_ptr<dfoperator>& a,
                                      const std::shared_ptr<dffunction>& b) {
  return mod_col(a,b);
}

std::shared_ptr<dffunction> operator%(const std::shared_ptr<dfoperator>& a,
                                      const std::shared_ptr<dfaggregator>& b) {
  return mod_col(a,b);
}

std::shared_ptr<dffunction> operator%(const std::shared_ptr<dfoperator>& a,
                                      const std::shared_ptr<dfoperator>& b) {
  return mod_col(a,b);
}

// ----- operators for dfoperator -----
std::shared_ptr<dfoperator> operator==(const std::shared_ptr<dffunction>& a,
                                       const std::shared_ptr<dffunction>& b) {
  return eq(a,b);
}

std::shared_ptr<dfoperator> operator==(const std::shared_ptr<dffunction>& a,
                                       const std::shared_ptr<dfaggregator>& b) {
  return eq(a,b);
}

std::shared_ptr<dfoperator> operator==(const std::shared_ptr<dfaggregator>& a,
                                       const std::shared_ptr<dffunction>& b) {
  return eq(a,b);
}

std::shared_ptr<dfoperator> operator==(const std::shared_ptr<dfaggregator>& a,
                                       const std::shared_ptr<dfaggregator>& b) {
  return eq(a,b);
}

std::shared_ptr<dfoperator> operator!=(const std::shared_ptr<dffunction>& a,
                                       const std::shared_ptr<dffunction>& b) {
  return neq(a,b);
}

std::shared_ptr<dfoperator> operator!=(const std::shared_ptr<dffunction>& a,
                                       const std::shared_ptr<dfaggregator>& b) {
  return neq(a,b);
}

std::shared_ptr<dfoperator> operator!=(const std::shared_ptr<dfaggregator>& a,
                                       const std::shared_ptr<dffunction>& b) {
  return neq(a,b);
}

std::shared_ptr<dfoperator> operator!=(const std::shared_ptr<dfaggregator>& a,
                                       const std::shared_ptr<dfaggregator>& b) {
  return neq(a,b);
}

std::shared_ptr<dfoperator> operator<(const std::shared_ptr<dffunction>& a,
                                      const std::shared_ptr<dffunction>& b) {
  return lt(a,b);
}

std::shared_ptr<dfoperator> operator<(const std::shared_ptr<dffunction>& a,
                                      const std::shared_ptr<dfaggregator>& b) {
  return lt(a,b);
}

std::shared_ptr<dfoperator> operator<(const std::shared_ptr<dfaggregator>& a,
                                      const std::shared_ptr<dffunction>& b) {
  return lt(a,b);
}

std::shared_ptr<dfoperator> operator<(const std::shared_ptr<dfaggregator>& a,
                                      const std::shared_ptr<dfaggregator>& b) {
  return lt(a,b);
}

std::shared_ptr<dfoperator> operator>=(const std::shared_ptr<dffunction>& a,
                                       const std::shared_ptr<dffunction>& b) {
  return ge(a,b);
}

std::shared_ptr<dfoperator> operator>=(const std::shared_ptr<dffunction>& a,
                                       const std::shared_ptr<dfaggregator>& b) {
  return ge(a,b);
}

std::shared_ptr<dfoperator> operator>=(const std::shared_ptr<dfaggregator>& a,
                                       const std::shared_ptr<dffunction>& b) {
  return ge(a,b);
}

std::shared_ptr<dfoperator> operator>=(const std::shared_ptr<dfaggregator>& a,
                                       const std::shared_ptr<dfaggregator>& b) {
  return ge(a,b);
}

std::shared_ptr<dfoperator> operator<=(const std::shared_ptr<dffunction>& a,
                                       const std::shared_ptr<dffunction>& b) {
  return le(a,b);
}

std::shared_ptr<dfoperator> operator<=(const std::shared_ptr<dffunction>& a,
                                       const std::shared_ptr<dfaggregator>& b) {
  return le(a,b);
}

std::shared_ptr<dfoperator> operator<=(const std::shared_ptr<dfaggregator>& a,
                                       const std::shared_ptr<dffunction>& b) {
  return le(a,b);
}

std::shared_ptr<dfoperator> operator<=(const std::shared_ptr<dfaggregator>& a,
                                       const std::shared_ptr<dfaggregator>& b) {
  return le(a,b);
}

std::shared_ptr<dfoperator> operator>(const std::shared_ptr<dffunction>& a,
                                      const std::shared_ptr<dffunction>& b) {
  return gt(a,b);
}

std::shared_ptr<dfoperator> operator>(const std::shared_ptr<dffunction>& a,
                                      const std::shared_ptr<dfaggregator>& b) {
  return gt(a,b);
}

std::shared_ptr<dfoperator> operator>(const std::shared_ptr<dfaggregator>& a,
                                      const std::shared_ptr<dffunction>& b) {
  return gt(a,b);
}

std::shared_ptr<dfoperator> operator>(const std::shared_ptr<dfaggregator>& a,
                                      const std::shared_ptr<dfaggregator>& b) {
  return gt(a,b);
}

// use dfoperator for argument to avoid ambiguous operator error
// cannot use dffunction
std::shared_ptr<dfoperator> operator&&(const std::shared_ptr<dfoperator>& a,
                                       const std::shared_ptr<dfoperator>& b) {
  return and_op(a,b);
}

std::shared_ptr<dfoperator> operator||(const std::shared_ptr<dfoperator>& a,
                                       const std::shared_ptr<dfoperator>& b) {
  return or_op(a,b);
}

std::shared_ptr<dfoperator> operator!(const std::shared_ptr<dfoperator>& a) {
  return not_op(a);
}

}
