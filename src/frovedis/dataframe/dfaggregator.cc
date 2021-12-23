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
          node_local<size_t>& row_sizes) {
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
          node_local<size_t>& row_sizes) {
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
          node_local<size_t>& row_sizes) {
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
          node_local<size_t>& row_sizes) {
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
          node_local<size_t>& row_sizes) {
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
          node_local<size_t>& row_sizes) {
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
          node_local<size_t>& row_sizes) {
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
          node_local<size_t>& row_sizes) {
  dftable_base sliced_table = table;
  auto colp = col->execute(sliced_table);
  return colp->mad(local_grouped_idx, local_idx_split, hash_divide,
                   merge_map, row_sizes);
}

std::shared_ptr<dfcolumn> 
dfaggregator_max::
aggregate(dftable_base& table,
          node_local<std::vector<size_t>>& local_grouped_idx,
          node_local<std::vector<size_t>>& local_idx_split,
          node_local<std::vector<std::vector<size_t>>>& hash_divide,
          node_local<std::vector<std::vector<size_t>>>& merge_map,
          node_local<size_t>& row_sizes) {
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
          node_local<size_t>& row_sizes) {
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
          node_local<size_t>& row_sizes) {
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
          node_local<size_t>& row_sizes) {
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
          node_local<size_t>& row_sizes) {
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
          node_local<size_t>& row_sizes) {
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
    if(!colp2) throw std::runtime_error("internal type error");
    double r = colp2->sum();
    std::vector<double> v = {r};
    return std::make_shared<typed_dfcolumn<double>>(make_dvector_scatter(v));
  } else if(colp->dtype() == "float") {
    auto colp2 = std::dynamic_pointer_cast<typed_dfcolumn<float>>(colp);
    if(!colp2) throw std::runtime_error("internal type error");
    float r = colp2->sum();
    std::vector<float> v = {r};
    return std::make_shared<typed_dfcolumn<float>>(make_dvector_scatter(v));
  } else if(colp->dtype() == "long") {
    auto colp2 = std::dynamic_pointer_cast<typed_dfcolumn<long>>(colp);
    if(!colp2) throw std::runtime_error("internal type error");
    long r = colp2->sum();
    std::vector<long> v = {r};
    return std::make_shared<typed_dfcolumn<long>>(make_dvector_scatter(v));
  } else if(colp->dtype() == "unsigned long") {
    auto colp2 = std::dynamic_pointer_cast<typed_dfcolumn<unsigned long>>(colp);
    if(!colp2) throw std::runtime_error("internal type error");
    unsigned long r = colp2->sum();
    std::vector<unsigned long> v = {r};
    return
      std::make_shared<typed_dfcolumn<unsigned long>>(make_dvector_scatter(v));
  } else if(colp->dtype() == "int") {
    auto colp2 = std::dynamic_pointer_cast<typed_dfcolumn<int>>(colp);
    if(!colp2) throw std::runtime_error("internal type error");
    int r = colp2->sum();
    std::vector<int> v = {r};
    return std::make_shared<typed_dfcolumn<int>>(make_dvector_scatter(v));
  } else if(colp->dtype() == "unsigned int") {
    auto colp2 = std::dynamic_pointer_cast<typed_dfcolumn<unsigned int>>(colp);
    if(!colp2) throw std::runtime_error("internal type error");
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
  double r = colp->avg();
  std::vector<double> v = {r};
  return std::make_shared<typed_dfcolumn<double>>(make_dvector_scatter(v));
}

std::shared_ptr<dfcolumn>
dfaggregator_var::
whole_column_aggregate(dftable_base& table) {
  auto colp = col->execute(table);
  double r = colp->var(ddof);
  std::vector<double> v = {r};
  return std::make_shared<typed_dfcolumn<double>>(make_dvector_scatter(v));
}

std::shared_ptr<dfcolumn>
dfaggregator_sem::
whole_column_aggregate(dftable_base& table) {
  auto colp = col->execute(table);
  double r = colp->sem(ddof);
  std::vector<double> v = {r};
  return std::make_shared<typed_dfcolumn<double>>(make_dvector_scatter(v));
}

std::shared_ptr<dfcolumn>
dfaggregator_std::
whole_column_aggregate(dftable_base& table) {
  auto colp = col->execute(table);
  double r = colp->std(ddof);
  std::vector<double> v = {r};
  return std::make_shared<typed_dfcolumn<double>>(make_dvector_scatter(v));
}

std::shared_ptr<dfcolumn>
dfaggregator_mad::
whole_column_aggregate(dftable_base& table) {
  auto colp = col->execute(table);
  double r = colp->mad();
  std::vector<double> v = {r};
  return std::make_shared<typed_dfcolumn<double>>(make_dvector_scatter(v));
}

std::shared_ptr<dfcolumn>
dfaggregator_max::
whole_column_aggregate(dftable_base& table) {
  auto colp = col->execute(table);
  if(colp->dtype() == "double") {
    auto colp2 = std::dynamic_pointer_cast<typed_dfcolumn<double>>(colp);
    if(!colp2) throw std::runtime_error("internal type error");
    double r = colp2->max();
    std::vector<double> v = {r};
    return std::make_shared<typed_dfcolumn<double>>(make_dvector_scatter(v));
  } else if(colp->dtype() == "float") {
    auto colp2 = std::dynamic_pointer_cast<typed_dfcolumn<float>>(colp);
    if(!colp2) throw std::runtime_error("internal type error");
    float r = colp2->max();
    std::vector<float> v = {r};
    return std::make_shared<typed_dfcolumn<float>>(make_dvector_scatter(v));
  } else if(colp->dtype() == "long") {
    auto colp2 = std::dynamic_pointer_cast<typed_dfcolumn<long>>(colp);
    if(!colp2) throw std::runtime_error("internal type error");
    long r = colp2->max();
    std::vector<long> v = {r};
    return std::make_shared<typed_dfcolumn<long>>(make_dvector_scatter(v));
  } else if(colp->dtype() == "unsigned long") {
    auto colp2 = std::dynamic_pointer_cast<typed_dfcolumn<unsigned long>>(colp);
    if(!colp2) throw std::runtime_error("internal type error");
    unsigned long r = colp2->max();
    std::vector<unsigned long> v = {r};
    return
      std::make_shared<typed_dfcolumn<unsigned long>>(make_dvector_scatter(v));
  } else if(colp->dtype() == "int") {
    auto colp2 = std::dynamic_pointer_cast<typed_dfcolumn<int>>(colp);
    if(!colp2) throw std::runtime_error("internal type error");
    int r = colp2->max();
    std::vector<int> v = {r};
    return std::make_shared<typed_dfcolumn<int>>(make_dvector_scatter(v));
  } else if(colp->dtype() == "unsigned int") {
    auto colp2 = std::dynamic_pointer_cast<typed_dfcolumn<unsigned int>>(colp);
    if(!colp2) throw std::runtime_error("internal type error");
    unsigned int r = colp2->max();
    std::vector<unsigned int> v = {r};
    return
      std::make_shared<typed_dfcolumn<unsigned int>>(make_dvector_scatter(v));
  } else throw std::runtime_error("unsupported type: " + colp->dtype());
}

std::shared_ptr<dfcolumn>
dfaggregator_min::
whole_column_aggregate(dftable_base& table) {
  auto colp = col->execute(table);
  if(colp->dtype() == "double") {
    auto colp2 = std::dynamic_pointer_cast<typed_dfcolumn<double>>(colp);
    if(!colp2) throw std::runtime_error("internal type error");
    double r = colp2->min();
    std::vector<double> v = {r};
    return std::make_shared<typed_dfcolumn<double>>(make_dvector_scatter(v));
  } else if(colp->dtype() == "float") {
    auto colp2 = std::dynamic_pointer_cast<typed_dfcolumn<float>>(colp);
    if(!colp2) throw std::runtime_error("internal type error");
    float r = colp2->min();
    std::vector<float> v = {r};
    return std::make_shared<typed_dfcolumn<float>>(make_dvector_scatter(v));
  } else if(colp->dtype() == "long") {
    auto colp2 = std::dynamic_pointer_cast<typed_dfcolumn<long>>(colp);
    if(!colp2) throw std::runtime_error("internal type error");
    long r = colp2->min();
    std::vector<long> v = {r};
    return std::make_shared<typed_dfcolumn<long>>(make_dvector_scatter(v));
  } else if(colp->dtype() == "unsigned long") {
    auto colp2 = std::dynamic_pointer_cast<typed_dfcolumn<unsigned long>>(colp);
    if(!colp2) throw std::runtime_error("internal type error");
    unsigned long r = colp2->min();
    std::vector<unsigned long> v = {r};
    return
      std::make_shared<typed_dfcolumn<unsigned long>>(make_dvector_scatter(v));
  } else if(colp->dtype() == "int") {
    auto colp2 = std::dynamic_pointer_cast<typed_dfcolumn<int>>(colp);
    if(!colp2) throw std::runtime_error("internal type error");
    int r = colp2->min();
    std::vector<int> v = {r};
    return std::make_shared<typed_dfcolumn<int>>(make_dvector_scatter(v));
  } else if(colp->dtype() == "unsigned int") {
    auto colp2 = std::dynamic_pointer_cast<typed_dfcolumn<unsigned int>>(colp);
    if(!colp2) throw std::runtime_error("internal type error");
    unsigned int r = colp2->min();
    std::vector<unsigned int> v = {r};
    return
      std::make_shared<typed_dfcolumn<unsigned int>>(make_dvector_scatter(v));
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

}
