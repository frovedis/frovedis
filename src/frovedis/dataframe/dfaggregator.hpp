#ifndef DFAGGREGATOR_HPP
#define DFAGGREGATOR_HPP

#include "dfcolumn.hpp"
#include "dffunction.hpp"

namespace frovedis {

class dftable_base;

struct dfaggregator {
  dfaggregator(const std::shared_ptr<dffunction>& col,
               const std::string& as_name) :
    col(col), as_name(as_name) {}
  virtual std::shared_ptr<dfcolumn>
  aggregate(dftable_base& table,
            node_local<std::vector<size_t>>& local_grouped_idx,
            node_local<std::vector<size_t>>& local_idx_split,
            node_local<std::vector<std::vector<size_t>>>& hash_divide,
            node_local<std::vector<std::vector<size_t>>>& merge_map,
            node_local<size_t>& row_sizes) = 0;
  std::string get_as() {return as_name;}
  virtual std::shared_ptr<dfaggregator> as(const std::string& cname) = 0;
  virtual std::shared_ptr<dfcolumn>
  whole_column_aggregate(dftable_base& table) {
    throw std::runtime_error("aggregate_whole of this type is not implemented");
  }
  std::shared_ptr<dffunction> col;
  std::string as_name;
};

struct dfaggregator_sum : public dfaggregator {
  dfaggregator_sum(const std::shared_ptr<dffunction>& col,
                   const std::string& as_name) : dfaggregator(col,as_name) {}
  dfaggregator_sum(const std::shared_ptr<dffunction>& col) :
    dfaggregator(col, "sum(" + col->get_as() + ")") {}
  virtual std::shared_ptr<dfaggregator> as(const std::string& cname) {
    as_name = cname;
    return std::make_shared<dfaggregator_sum>(*this);
  }
  virtual std::shared_ptr<dfcolumn>
  aggregate(dftable_base& table,
            node_local<std::vector<size_t>>& local_grouped_idx,
            node_local<std::vector<size_t>>& local_idx_split,
            node_local<std::vector<std::vector<size_t>>>& hash_divide,
            node_local<std::vector<std::vector<size_t>>>& merge_map,
            node_local<size_t>& row_sizes);
  virtual std::shared_ptr<dfcolumn> whole_column_aggregate(dftable_base& table);
};

struct dfaggregator_count : public dfaggregator {
  dfaggregator_count(const std::shared_ptr<dffunction>& col,
                     const std::string& as_name) : dfaggregator(col,as_name) {}
  dfaggregator_count(const std::shared_ptr<dffunction>& col) :
    dfaggregator(col, "count(" + col->get_as() + ")") {}
  virtual std::shared_ptr<dfaggregator> as(const std::string& cname) {
    as_name = cname;
    return std::make_shared<dfaggregator_count>(*this);
  }
  virtual std::shared_ptr<dfcolumn>
  aggregate(dftable_base& table,
            node_local<std::vector<size_t>>& local_grouped_idx,
            node_local<std::vector<size_t>>& local_idx_split,
            node_local<std::vector<std::vector<size_t>>>& hash_divide,
            node_local<std::vector<std::vector<size_t>>>& merge_map,
            node_local<size_t>& row_sizes);
  virtual std::shared_ptr<dfcolumn> whole_column_aggregate(dftable_base& table);
};

struct dfaggregator_size : public dfaggregator {
  dfaggregator_size(const std::shared_ptr<dffunction>& col,
                    const std::string& as_name) : dfaggregator(col,as_name) {}
  dfaggregator_size(const std::shared_ptr<dffunction>& col) :
    dfaggregator(col, "size(" + col->get_as() + ")") {}
  virtual std::shared_ptr<dfaggregator> as(const std::string& cname) {
    as_name = cname;
    return std::make_shared<dfaggregator_size>(*this);
  }
  virtual std::shared_ptr<dfcolumn>
  aggregate(dftable_base& table,
            node_local<std::vector<size_t>>& local_grouped_idx,
            node_local<std::vector<size_t>>& local_idx_split,
            node_local<std::vector<std::vector<size_t>>>& hash_divide,
            node_local<std::vector<std::vector<size_t>>>& merge_map,
            node_local<size_t>& row_sizes);
  virtual std::shared_ptr<dfcolumn> whole_column_aggregate(dftable_base& table);
};

struct dfaggregator_avg : public dfaggregator {
  dfaggregator_avg(const std::shared_ptr<dffunction>& col,
                   const std::string& as_name) : dfaggregator(col,as_name) {}
  dfaggregator_avg(const std::shared_ptr<dffunction>& col) :
    dfaggregator(col, "avg(" + col->get_as() + ")") {}
  virtual std::shared_ptr<dfaggregator> as(const std::string& cname) {
    as_name = cname;
    return std::make_shared<dfaggregator_avg>(*this);
  }
  virtual std::shared_ptr<dfcolumn>
  aggregate(dftable_base& table,
            node_local<std::vector<size_t>>& local_grouped_idx,
            node_local<std::vector<size_t>>& local_idx_split,
            node_local<std::vector<std::vector<size_t>>>& hash_divide,
            node_local<std::vector<std::vector<size_t>>>& merge_map,
            node_local<size_t>& row_sizes);
  virtual std::shared_ptr<dfcolumn> whole_column_aggregate(dftable_base& table);
};

struct dfaggregator_var : public dfaggregator {
  dfaggregator_var(const std::shared_ptr<dffunction>& col,
                   const std::string& as_name,
                   const double& ddof) :
    dfaggregator(col,as_name), ddof(ddof) {}
  dfaggregator_var(const std::shared_ptr<dffunction>& col,
                   const double& ddof) :
    dfaggregator(col, "var(" + col->get_as() + ")"), ddof(ddof) {}
  virtual std::shared_ptr<dfaggregator> as(const std::string& cname) {
    as_name = cname;
    return std::make_shared<dfaggregator_var>(*this);
  }
  virtual std::shared_ptr<dfcolumn>
  aggregate(dftable_base& table,
            node_local<std::vector<size_t>>& local_grouped_idx,
            node_local<std::vector<size_t>>& local_idx_split,
            node_local<std::vector<std::vector<size_t>>>& hash_divide,
            node_local<std::vector<std::vector<size_t>>>& merge_map,
            node_local<size_t>& row_sizes);
  virtual std::shared_ptr<dfcolumn> whole_column_aggregate(dftable_base& table);
  double ddof = 1.0;
};

struct dfaggregator_sem : public dfaggregator {
  dfaggregator_sem(const std::shared_ptr<dffunction>& col,
                   const std::string& as_name,
                   const double& ddof) :
    dfaggregator(col,as_name), ddof(ddof) {}
  dfaggregator_sem(const std::shared_ptr<dffunction>& col,
                   const double& ddof) :
    dfaggregator(col, "sem(" + col->get_as() + ")"), ddof(ddof) {}
  virtual std::shared_ptr<dfaggregator> as(const std::string& cname) {
    as_name = cname;
    return std::make_shared<dfaggregator_sem>(*this);
  }
  virtual std::shared_ptr<dfcolumn>
  aggregate(dftable_base& table,
            node_local<std::vector<size_t>>& local_grouped_idx,
            node_local<std::vector<size_t>>& local_idx_split,
            node_local<std::vector<std::vector<size_t>>>& hash_divide,
            node_local<std::vector<std::vector<size_t>>>& merge_map,
            node_local<size_t>& row_sizes);
  virtual std::shared_ptr<dfcolumn> whole_column_aggregate(dftable_base& table);
  double ddof = 1.0;
};

struct dfaggregator_std : public dfaggregator {
  dfaggregator_std(const std::shared_ptr<dffunction>& col,
                   const std::string& as_name,
                   const double& ddof) :
    dfaggregator(col,as_name), ddof(ddof) {}
  dfaggregator_std(const std::shared_ptr<dffunction>& col,
                   const double& ddof) :
    dfaggregator(col, "stddev(" + col->get_as() + ")"), ddof(ddof) {}
  virtual std::shared_ptr<dfaggregator> as(const std::string& cname) {
    as_name = cname;
    return std::make_shared<dfaggregator_std>(*this);
  }
  virtual std::shared_ptr<dfcolumn>
  aggregate(dftable_base& table,
            node_local<std::vector<size_t>>& local_grouped_idx,
            node_local<std::vector<size_t>>& local_idx_split,
            node_local<std::vector<std::vector<size_t>>>& hash_divide,
            node_local<std::vector<std::vector<size_t>>>& merge_map,
            node_local<size_t>& row_sizes);
  virtual std::shared_ptr<dfcolumn> whole_column_aggregate(dftable_base& table);
  double ddof = 1.0;
};

struct dfaggregator_mad : public dfaggregator {
  dfaggregator_mad(const std::shared_ptr<dffunction>& col,
                   const std::string& as_name): dfaggregator(col,as_name) {}
  dfaggregator_mad(const std::shared_ptr<dffunction>& col) :
    dfaggregator(col, "mad(" + col->get_as() + ")") {}
  virtual std::shared_ptr<dfaggregator> as(const std::string& cname) {
    as_name = cname;
    return std::make_shared<dfaggregator_mad>(*this);
  }
  virtual std::shared_ptr<dfcolumn>
  aggregate(dftable_base& table,
            node_local<std::vector<size_t>>& local_grouped_idx,
            node_local<std::vector<size_t>>& local_idx_split,
            node_local<std::vector<std::vector<size_t>>>& hash_divide,
            node_local<std::vector<std::vector<size_t>>>& merge_map,
            node_local<size_t>& row_sizes);
  virtual std::shared_ptr<dfcolumn> whole_column_aggregate(dftable_base& table);
};

struct dfaggregator_max : public dfaggregator {
  dfaggregator_max(const std::shared_ptr<dffunction>& col,
                   const std::string& as_name) : dfaggregator(col,as_name) {}
  dfaggregator_max(const std::shared_ptr<dffunction>& col) :
    dfaggregator(col, "max(" + col->get_as() + ")") {}
  virtual std::shared_ptr<dfaggregator> as(const std::string& cname) {
    as_name = cname;
    return std::make_shared<dfaggregator_max>(*this);
  }
  virtual std::shared_ptr<dfcolumn>
  aggregate(dftable_base& table,
            node_local<std::vector<size_t>>& local_grouped_idx,
            node_local<std::vector<size_t>>& local_idx_split,
            node_local<std::vector<std::vector<size_t>>>& hash_divide,
            node_local<std::vector<std::vector<size_t>>>& merge_map,
            node_local<size_t>& row_sizes);
  virtual std::shared_ptr<dfcolumn> whole_column_aggregate(dftable_base& table);
};

struct dfaggregator_min : public dfaggregator {
  dfaggregator_min(const std::shared_ptr<dffunction>& col,
                   const std::string& as_name) : dfaggregator(col,as_name) {}
  dfaggregator_min(const std::shared_ptr<dffunction>& col) :
    dfaggregator(col, "min(" + col->get_as() + ")") {}
  virtual std::shared_ptr<dfaggregator> as(const std::string& cname) {
    as_name = cname;
    return std::make_shared<dfaggregator_min>(*this);
  }
  virtual std::shared_ptr<dfcolumn>
  aggregate(dftable_base& table,
            node_local<std::vector<size_t>>& local_grouped_idx,
            node_local<std::vector<size_t>>& local_idx_split,
            node_local<std::vector<std::vector<size_t>>>& hash_divide,
            node_local<std::vector<std::vector<size_t>>>& merge_map,
            node_local<size_t>& row_sizes);
  virtual std::shared_ptr<dfcolumn> whole_column_aggregate(dftable_base& table);
};

struct dfaggregator_count_distinct : public dfaggregator {
  dfaggregator_count_distinct(const std::shared_ptr<dffunction>& col,
                              const std::string& as_name) :
    dfaggregator(col,as_name) {}
  dfaggregator_count_distinct(const std::shared_ptr<dffunction>& col) :
    dfaggregator(col, "count_distinct(" + col->get_as() + ")") {}
  virtual std::shared_ptr<dfaggregator> as(const std::string& cname) {
    as_name = cname;
    return std::make_shared<dfaggregator_count_distinct>(*this);
  }
  virtual std::shared_ptr<dfcolumn>
  aggregate(dftable_base& table,
            node_local<std::vector<size_t>>& local_grouped_idx,
            node_local<std::vector<size_t>>& local_idx_split,
            node_local<std::vector<std::vector<size_t>>>& hash_divide,
            node_local<std::vector<std::vector<size_t>>>& merge_map,
            node_local<size_t>& row_sizes);
};

struct dfaggregator_sum_distinct : public dfaggregator {
  dfaggregator_sum_distinct(const std::shared_ptr<dffunction>& col,
                            const std::string& as_name) :
    dfaggregator(col,as_name) {}
  dfaggregator_sum_distinct(const std::shared_ptr<dffunction>& col) :
    dfaggregator(col, "sum_distinct(" + col->get_as() + ")") {}
  virtual std::shared_ptr<dfaggregator> as(const std::string& cname) {
    as_name = cname;
    return std::make_shared<dfaggregator_sum_distinct>(*this);
  }
  virtual std::shared_ptr<dfcolumn>
  aggregate(dftable_base& table,
            node_local<std::vector<size_t>>& local_grouped_idx,
            node_local<std::vector<size_t>>& local_idx_split,
            node_local<std::vector<std::vector<size_t>>>& hash_divide,
            node_local<std::vector<std::vector<size_t>>>& merge_map,
            node_local<size_t>& row_sizes);
};

std::shared_ptr<dfaggregator>
sum(const std::string& col);

std::shared_ptr<dfaggregator>
sum_as(const std::string& col, const std::string& as);

std::shared_ptr<dfaggregator>
sum(const std::shared_ptr<dffunction>& col);

std::shared_ptr<dfaggregator>
sum_as(const std::shared_ptr<dffunction>& col, const std::string& as);

std::shared_ptr<dfaggregator>
count(const std::string& col);

std::shared_ptr<dfaggregator>
count_as(const std::string& col, const std::string& as);

std::shared_ptr<dfaggregator>
count(const std::shared_ptr<dffunction>& col);

std::shared_ptr<dfaggregator>
count_as(const std::shared_ptr<dffunction>& col, const std::string& as);
  
std::shared_ptr<dfaggregator>
size(const std::string& col);

std::shared_ptr<dfaggregator>
size_as(const std::string& col, const std::string& as);

std::shared_ptr<dfaggregator>
size(const std::shared_ptr<dffunction>& col);

std::shared_ptr<dfaggregator>
size_as(const std::shared_ptr<dffunction>& col, const std::string& as);
        
// size (including nulls) of any column
std::shared_ptr<dfaggregator> count_all();

std::shared_ptr<dfaggregator> count_all_as(const std::string& as);
       
std::shared_ptr<dfaggregator>
avg(const std::string& col);

std::shared_ptr<dfaggregator>
avg_as(const std::string& col, const std::string& as);

std::shared_ptr<dfaggregator>
avg(const std::shared_ptr<dffunction>& col);

std::shared_ptr<dfaggregator>
avg_as(const std::shared_ptr<dffunction>& col, const std::string& as);
       
std::shared_ptr<dfaggregator>
var(const std::string& col,
    const double& ddof = 1.0);

std::shared_ptr<dfaggregator>
var_as(const std::string& col, const std::string& as,
       const double& ddof = 1.0);

std::shared_ptr<dfaggregator>
var(const std::shared_ptr<dffunction>& col, const double& ddof = 1.0);

std::shared_ptr<dfaggregator>
var_as(const std::shared_ptr<dffunction>& col,
       const std::string& as,
       const double& ddof = 1.0);

std::shared_ptr<dfaggregator>
sem(const std::string& col,
    const double& ddof = 1.0);

std::shared_ptr<dfaggregator>
sem_as(const std::string& col, const std::string& as,
       const double& ddof = 1.0);

std::shared_ptr<dfaggregator>
sem(const std::shared_ptr<dffunction>& col, const double& ddof = 1.0);

std::shared_ptr<dfaggregator>
sem_as(const std::shared_ptr<dffunction>& col,
       const std::string& as,
       const double& ddof = 1.0);

std::shared_ptr<dfaggregator>
std(const std::string& col,
    const double& ddof = 1.0);

std::shared_ptr<dfaggregator>
std_as(const std::string& col, const std::string& as,
       const double& ddof = 1.0);

std::shared_ptr<dfaggregator>
std(const std::shared_ptr<dffunction>& col, const double& ddof = 1.0);

std::shared_ptr<dfaggregator>
std_as(const std::shared_ptr<dffunction>& col,
       const std::string& as,
       const double& ddof = 1.0);

std::shared_ptr<dfaggregator>
mad(const std::string& col);

std::shared_ptr<dfaggregator>
mad_as(const std::string& col, const std::string& as);

std::shared_ptr<dfaggregator>
mad(const std::shared_ptr<dffunction>& col);

std::shared_ptr<dfaggregator>
mad_as(const std::shared_ptr<dffunction>& col,
       const std::string& as);

std::shared_ptr<dfaggregator>
max(const std::string& col);

std::shared_ptr<dfaggregator>
max_as(const std::string& col, const std::string& as);

std::shared_ptr<dfaggregator>
max(const std::shared_ptr<dffunction>& col);

std::shared_ptr<dfaggregator>
max_as(const std::shared_ptr<dffunction>& col,
       const std::string& as);

std::shared_ptr<dfaggregator>
min(const std::string& col);

std::shared_ptr<dfaggregator>
min(const std::shared_ptr<dffunction>& col);

std::shared_ptr<dfaggregator>
min_as(const std::string& col, const std::string& as);

std::shared_ptr<dfaggregator>
min_as(const std::shared_ptr<dffunction>& col, const std::string& as);

std::shared_ptr<dfaggregator>
count_distinct(const std::string& col);

std::shared_ptr<dfaggregator>
count_distinct(const std::shared_ptr<dffunction>& col);

std::shared_ptr<dfaggregator>
count_distinct_as(const std::string& col, const std::string& as);

std::shared_ptr<dfaggregator>
count_distinct_as(const std::shared_ptr<dffunction>& col, const std::string& as);

std::shared_ptr<dfaggregator>
sum_distinct(const std::string& col);

std::shared_ptr<dfaggregator>
sum_distinct(const std::shared_ptr<dffunction>& col);

std::shared_ptr<dfaggregator>
sum_distinct_as(const std::string& col, const std::string& as);

std::shared_ptr<dfaggregator>
sum_distinct_as(const std::shared_ptr<dffunction>& col, const std::string& as);
       
}
#endif
