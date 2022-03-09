#include "dffunction.hpp"
#include "dfaggregator.hpp"
#include "../text/char_int_conv.hpp"
#include "../text/words.hpp"

namespace frovedis {

// ----- misc -----
bool check_distribution(dftable_base& left, dftable_base& right,
                        std::vector<size_t>& left_sizes) {
  checkAssumption(left.num_col() && right.num_col());
  left_sizes = left.num_rows();
  auto right_sizes = right.num_rows();
  return left_sizes == right_sizes;
}

// re-aligns right as per left
std::shared_ptr<dfcolumn> realign_df(dftable_base& left, 
                                     dftable_base& right,
                                     std::shared_ptr<dfcolumn>& rightcol) {
  std::vector<size_t> left_sizes;
  if (!check_distribution(left, right, left_sizes)) {
    dftable tmp;
    tmp.append_column("tmp", rightcol);
    tmp.align_as(left_sizes);
    return tmp.column("tmp");
  } else {
    return rightcol;
  }
}

bool verify_column_identicality(dftable_base& left,
                                const std::string& lcol,
                                dftable_base& right,
                                const std::string& rcol) {
  if (left.num_row() != right.num_row()) return false;
  auto left_col = left.column(lcol);
  auto right_col = right.column(rcol);
  use_dfcolumn use({left_col, right_col});
  auto dtype = left_col->dtype();
  if (dtype != right_col->dtype()) return false;
  int nproc = get_nodesize();
  bool ret = true;

  if (dtype == "int") {
    auto c1 = std::dynamic_pointer_cast<typed_dfcolumn<int>>(left_col);
    auto c2 = std::dynamic_pointer_cast<typed_dfcolumn<int>>(right_col);
    ret = c1->val.map(vector_is_same<int,int>, c2->val)
      .reduce(add<int>) == nproc;
  } else if (dtype == "unsigned int") {
    auto c1 = std::dynamic_pointer_cast<typed_dfcolumn<unsigned int>>(left_col);
    auto c2 = std::dynamic_pointer_cast<typed_dfcolumn<unsigned int>>(right_col);
    ret = c1->val.map(vector_is_same<unsigned int,unsigned int>, c2->val)
      .reduce(add<int>) == nproc;
  } else if (dtype == "long") {
    auto c1 = std::dynamic_pointer_cast<typed_dfcolumn<long>>(left_col);
    auto c2 = std::dynamic_pointer_cast<typed_dfcolumn<long>>(right_col);
    ret = c1->val.map(vector_is_same<long,long>, c2->val)
      .reduce(add<int>) == nproc;
  } else if (dtype == "unsigned long") {
    auto c1 = std::dynamic_pointer_cast<typed_dfcolumn<unsigned long>>(left_col);
    auto c2 = std::dynamic_pointer_cast<typed_dfcolumn<unsigned long>>(right_col);
    ret = c1->val.map(vector_is_same<unsigned long,unsigned long>, c2->val)
      .reduce(add<int>) == nproc;
  } else if (dtype == "float") {
    auto c1 = std::dynamic_pointer_cast<typed_dfcolumn<float>>(left_col);
    auto c2 = std::dynamic_pointer_cast<typed_dfcolumn<float>>(right_col);
    ret = c1->val.map(vector_is_same<float,float>, c2->val)
      .reduce(add<int>) == nproc;
  } else if (dtype == "double") {
    auto c1 = std::dynamic_pointer_cast<typed_dfcolumn<double>>(left_col);
    auto c2 = std::dynamic_pointer_cast<typed_dfcolumn<double>>(right_col);
    ret = c1->val.map(vector_is_same<double,double>, c2->val)
      .reduce(add<int>) == nproc;
  } else if (dtype == "dic_string") {
    auto c1 = std::dynamic_pointer_cast<typed_dfcolumn<dic_string>>(left_col);
    auto c2 = std::dynamic_pointer_cast<typed_dfcolumn<dic_string>>(right_col);
    ret = c1->val.map(vector_is_same<size_t,size_t>, c2->val) // TODO: FIX
      .reduce(add<int>) == nproc;
  } else if (dtype == "string") {
    auto c1 = std::dynamic_pointer_cast<typed_dfcolumn<std::string>>(left_col);
    auto c2 = std::dynamic_pointer_cast<typed_dfcolumn<std::string>>(right_col);
    ret = c1->val.map(vector_is_same<size_t,size_t>, c2->val) // TODO: FIX
      .reduce(add<int>) == nproc;
  } else {
    REPORT_ERROR(USER_ERROR, 
                 "verify_column_identicality: unsupported column type: " + dtype);
  }
  return ret;
}


// ----- id -----
std::shared_ptr<dffunction> id_col(const std::string& left) {
  return std::make_shared<dffunction_id>(left);
}

std::shared_ptr<dffunction> id_col_as(const std::string& left,
                                      const std::string& as) {
  return std::make_shared<dffunction_id>(left, as);
}

// ----- cast -----
std::shared_ptr<dfcolumn> dffunction_cast::execute(dftable_base& t) const {
  auto left_column = left->execute(t);
  return left_column->type_cast(to_type);
}

std::shared_ptr<dfcolumn> dffunction_cast::aggregate(dftable_base& table,
          node_local<std::vector<size_t>>& local_grouped_idx,
          node_local<std::vector<size_t>>& local_idx_split,
          node_local<std::vector<std::vector<size_t>>>& hash_divide,
          node_local<std::vector<std::vector<size_t>>>& merge_map,
          node_local<size_t>& row_sizes,
          dftable& grouped_table) {
  auto left_column = left->aggregate(table, local_grouped_idx,
                                     local_idx_split, hash_divide,
                                     merge_map, row_sizes, grouped_table);
  return left_column->type_cast(to_type);
}

std::shared_ptr<dfcolumn>
dffunction_cast::whole_column_aggregate(dftable_base& t) {
  auto left_column = left->whole_column_aggregate(t);
  return left_column->type_cast(to_type);
}

std::shared_ptr<dffunction> cast_col(const std::string& left,
                                     const std::string& to) {
  return std::make_shared<dffunction_cast>(id_col(left), to);
}

std::shared_ptr<dffunction>
cast_col(const std::shared_ptr<dffunction>& left, const std::string& to){
  return std::make_shared<dffunction_cast>(left, to);
}

std::shared_ptr<dffunction>
cast_col_as(const std::string& left, const std::string& to,
            const std::string& as) {
  return std::make_shared<dffunction_cast>(id_col(left), to, as);
}

std::shared_ptr<dffunction>
cast_col_as(const std::shared_ptr<dffunction>& left, const std::string& to,
            const std::string& as) {
  return std::make_shared<dffunction_cast>(left, to, as);
}


// ----- add -----
std::shared_ptr<dfcolumn> dffunction_add::execute(dftable_base& t) const {
  auto left_column = left->execute(t);
  return left_column->add(right->execute(t));
}

std::shared_ptr<dfcolumn> dffunction_add::execute(dftable_base& t1, 
                                                  dftable_base& t2) const {
  auto left_column = left->execute(t1);
  auto right_column = right->execute(t2);
  auto aligned_right_column = realign_df(t1, t2, right_column);
  return left_column->add(aligned_right_column);
}

std::shared_ptr<dfcolumn> dffunction_add::aggregate
(dftable_base& table,
 node_local<std::vector<size_t>>& local_grouped_idx,
 node_local<std::vector<size_t>>& local_idx_split,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map,
 node_local<size_t>& row_sizes,
 dftable& grouped_table) {
  auto left_column = left->aggregate(table, local_grouped_idx,
                                     local_idx_split, hash_divide,
                                     merge_map, row_sizes, grouped_table);
  auto right_column = right->aggregate(table, local_grouped_idx,
                                       local_idx_split, hash_divide,
                                       merge_map, row_sizes, grouped_table);
  return left_column->add(right_column);
}

std::shared_ptr<dfcolumn>
dffunction_add::whole_column_aggregate(dftable_base& t) {
  auto left_column = left->whole_column_aggregate(t);
  return left_column->add(right->whole_column_aggregate(t));
}

std::shared_ptr<dffunction> add_col(const std::string& left,
                                    const std::string& right) {
  return std::make_shared<dffunction_add>(id_col(left), id_col(right));
}

std::shared_ptr<dffunction>
add_col(const std::shared_ptr<dffunction>& left, const std::string& right){
  return std::make_shared<dffunction_add>(left, id_col(right));
}

std::shared_ptr<dffunction>
add_col(const std::string& left, const std::shared_ptr<dffunction>& right) {
  return std::make_shared<dffunction_add>(id_col(left), right);
}
 
std::shared_ptr<dffunction>
add_col(const std::shared_ptr<dffunction>& left,
        const std::shared_ptr<dffunction>& right) {
  return std::make_shared<dffunction_add>(left, right);
} 

std::shared_ptr<dffunction> add_col_as(const std::string& left,
                                       const std::string& right,
                                       const std::string& as) {
  return std::make_shared<dffunction_add>(id_col(left), id_col(right), as);
}

std::shared_ptr<dffunction>
add_col_as(const std::shared_ptr<dffunction>& left, const std::string& right,
           const std::string& as) {
  return std::make_shared<dffunction_add>(left, id_col(right), as);
}

std::shared_ptr<dffunction>
add_col_as(const std::string& left, const std::shared_ptr<dffunction>& right,
           const std::string& as) {
  return std::make_shared<dffunction_add>(id_col(left), right, as);
}

std::shared_ptr<dffunction>
add_col_as(const std::shared_ptr<dffunction>& left,
           const std::shared_ptr<dffunction>& right,
           const std::string& as) {
  return std::make_shared<dffunction_add>(left, right, as);
}


// ----- sub -----
std::shared_ptr<dfcolumn> dffunction_sub::execute(dftable_base& t) const {
  auto left_column = left->execute(t);
  return left_column->sub(right->execute(t));
}

std::shared_ptr<dfcolumn> dffunction_sub::execute(dftable_base& t1, 
                                                  dftable_base& t2) const {
  auto left_column = left->execute(t1);
  auto right_column = right->execute(t2);
  auto aligned_right_column = realign_df(t1, t2, right_column);
  return left_column->sub(aligned_right_column);
}

std::shared_ptr<dfcolumn> dffunction_sub::aggregate
(dftable_base& table,
 node_local<std::vector<size_t>>& local_grouped_idx,
 node_local<std::vector<size_t>>& local_idx_split,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map,
 node_local<size_t>& row_sizes,
 dftable& grouped_table) {
  auto left_column = left->aggregate(table, local_grouped_idx,
                                     local_idx_split, hash_divide,
                                     merge_map, row_sizes, grouped_table);
  auto right_column = right->aggregate(table, local_grouped_idx,
                                       local_idx_split, hash_divide,
                                       merge_map, row_sizes, grouped_table);
  return left_column->sub(right_column);
}

std::shared_ptr<dfcolumn>
dffunction_sub::whole_column_aggregate(dftable_base& t) {
  auto left_column = left->whole_column_aggregate(t);
  return left_column->sub(right->whole_column_aggregate(t));
}

std::shared_ptr<dffunction> sub_col(const std::string& left,
                                    const std::string& right) {
  return std::make_shared<dffunction_sub>(id_col(left), id_col(right));
}

std::shared_ptr<dffunction>
sub_col(const std::shared_ptr<dffunction>& left, const std::string& right){
  return std::make_shared<dffunction_sub>(left, id_col(right));
}

std::shared_ptr<dffunction>
sub_col(const std::string& left, const std::shared_ptr<dffunction>& right) {
  return std::make_shared<dffunction_sub>(id_col(left), right);
}
 
std::shared_ptr<dffunction>
sub_col(const std::shared_ptr<dffunction>& left,
        const std::shared_ptr<dffunction>& right) {
  return std::make_shared<dffunction_sub>(left, right);
} 

std::shared_ptr<dffunction> sub_col_as(const std::string& left,
                                       const std::string& right,
                                       const std::string& as) {
  return std::make_shared<dffunction_sub>(id_col(left), id_col(right), as);
}

std::shared_ptr<dffunction>
sub_col_as(const std::shared_ptr<dffunction>& left, const std::string& right,
           const std::string& as) {
  return std::make_shared<dffunction_sub>(left, id_col(right), as);
}

std::shared_ptr<dffunction>
sub_col_as(const std::string& left, const std::shared_ptr<dffunction>& right,
           const std::string& as) {
  return std::make_shared<dffunction_sub>(id_col(left), right, as);
}

std::shared_ptr<dffunction>
sub_col_as(const std::shared_ptr<dffunction>& left,
           const std::shared_ptr<dffunction>& right,
           const std::string& as) {
  return std::make_shared<dffunction_sub>(left, right, as);
}


// ----- mul -----
std::shared_ptr<dfcolumn> dffunction_mul::execute(dftable_base& t) const {
  auto left_column = left->execute(t);
  return left_column->mul(right->execute(t));
}

std::shared_ptr<dfcolumn> dffunction_mul::execute(dftable_base& t1, 
                                                  dftable_base& t2) const {
  auto left_column = left->execute(t1);
  auto right_column = right->execute(t2);
  auto aligned_right_column = realign_df(t1, t2, right_column);
  return left_column->mul(aligned_right_column);
}

std::shared_ptr<dfcolumn> dffunction_mul::aggregate
(dftable_base& table,
 node_local<std::vector<size_t>>& local_grouped_idx,
 node_local<std::vector<size_t>>& local_idx_split,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map,
 node_local<size_t>& row_sizes,
 dftable& grouped_table) {
  auto left_column = left->aggregate(table, local_grouped_idx,
                                     local_idx_split, hash_divide,
                                     merge_map, row_sizes, grouped_table);
  auto right_column = right->aggregate(table, local_grouped_idx,
                                       local_idx_split, hash_divide,
                                       merge_map, row_sizes, grouped_table);
  return left_column->mul(right_column);
}

std::shared_ptr<dfcolumn>
dffunction_mul::whole_column_aggregate(dftable_base& t) {
  auto left_column = left->whole_column_aggregate(t);
  return left_column->mul(right->whole_column_aggregate(t));
}

std::shared_ptr<dffunction> mul_col(const std::string& left,
                                    const std::string& right) {
  return std::make_shared<dffunction_mul>(id_col(left), id_col(right));
}

std::shared_ptr<dffunction>
mul_col(const std::shared_ptr<dffunction>& left, const std::string& right){
  return std::make_shared<dffunction_mul>(left, id_col(right));
}

std::shared_ptr<dffunction>
mul_col(const std::string& left, const std::shared_ptr<dffunction>& right) {
  return std::make_shared<dffunction_mul>(id_col(left), right);
}
 
std::shared_ptr<dffunction>
mul_col(const std::shared_ptr<dffunction>& left,
        const std::shared_ptr<dffunction>& right) {
  return std::make_shared<dffunction_mul>(left, right);
} 

std::shared_ptr<dffunction> mul_col_as(const std::string& left,
                                       const std::string& right,
                                       const std::string& as) {
  return std::make_shared<dffunction_mul>(id_col(left), id_col(right), as);
}

std::shared_ptr<dffunction>
mul_col_as(const std::shared_ptr<dffunction>& left, const std::string& right,
           const std::string& as) {
  return std::make_shared<dffunction_mul>(left, id_col(right), as);
}

std::shared_ptr<dffunction>
mul_col_as(const std::string& left, const std::shared_ptr<dffunction>& right,
           const std::string& as) {
  return std::make_shared<dffunction_mul>(id_col(left), right, as);
}

std::shared_ptr<dffunction>
mul_col_as(const std::shared_ptr<dffunction>& left,
           const std::shared_ptr<dffunction>& right,
           const std::string& as) {
  return std::make_shared<dffunction_mul>(left, right, as);
}


// ----- fdiv -----
std::shared_ptr<dfcolumn> dffunction_fdiv::execute(dftable_base& t) const {
  auto left_column = left->execute(t);
  return left_column->fdiv(right->execute(t));
}

std::shared_ptr<dfcolumn> dffunction_fdiv::execute(dftable_base& t1, 
                                                   dftable_base& t2) const {
  auto left_column = left->execute(t1);
  auto right_column = right->execute(t2);
  auto aligned_right_column = realign_df(t1, t2, right_column);
  return left_column->fdiv(aligned_right_column);
}

std::shared_ptr<dfcolumn> dffunction_fdiv::aggregate
(dftable_base& table,
 node_local<std::vector<size_t>>& local_grouped_idx,
 node_local<std::vector<size_t>>& local_idx_split,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map,
 node_local<size_t>& row_sizes,
 dftable& grouped_table) {
  auto left_column = left->aggregate(table, local_grouped_idx,
                                     local_idx_split, hash_divide,
                                     merge_map, row_sizes, grouped_table);
  auto right_column = right->aggregate(table, local_grouped_idx,
                                       local_idx_split, hash_divide,
                                       merge_map, row_sizes, grouped_table);
  return left_column->fdiv(right_column);
}

std::shared_ptr<dfcolumn>
dffunction_fdiv::whole_column_aggregate(dftable_base& t) {
  auto left_column = left->whole_column_aggregate(t);
  return left_column->fdiv(right->whole_column_aggregate(t));
}

std::shared_ptr<dffunction> fdiv_col(const std::string& left,
                                     const std::string& right) {
  return std::make_shared<dffunction_fdiv>(id_col(left), id_col(right));
}

std::shared_ptr<dffunction>
fdiv_col(const std::shared_ptr<dffunction>& left, const std::string& right){
  return std::make_shared<dffunction_fdiv>(left, id_col(right));
}

std::shared_ptr<dffunction>
fdiv_col(const std::string& left, const std::shared_ptr<dffunction>& right) {
  return std::make_shared<dffunction_fdiv>(id_col(left), right);
}
 
std::shared_ptr<dffunction>
fdiv_col(const std::shared_ptr<dffunction>& left,
         const std::shared_ptr<dffunction>& right) {
  return std::make_shared<dffunction_fdiv>(left, right);
} 

std::shared_ptr<dffunction> fdiv_col_as(const std::string& left,
                                        const std::string& right,
                                        const std::string& as) {
  return std::make_shared<dffunction_fdiv>(id_col(left), id_col(right), as);
}

std::shared_ptr<dffunction>
fdiv_col_as(const std::shared_ptr<dffunction>& left, const std::string& right,
            const std::string& as) {
  return std::make_shared<dffunction_fdiv>(left, id_col(right), as);
}

std::shared_ptr<dffunction>
fdiv_col_as(const std::string& left, const std::shared_ptr<dffunction>& right,
            const std::string& as) {
  return std::make_shared<dffunction_fdiv>(id_col(left), right, as);
}

std::shared_ptr<dffunction>
fdiv_col_as(const std::shared_ptr<dffunction>& left,
            const std::shared_ptr<dffunction>& right,
            const std::string& as) {
  return std::make_shared<dffunction_fdiv>(left, right, as);
}


// ----- idiv -----
std::shared_ptr<dfcolumn> dffunction_idiv::execute(dftable_base& t) const {
  auto left_column = left->execute(t);
  return left_column->idiv(right->execute(t));
}

std::shared_ptr<dfcolumn> dffunction_idiv::execute(dftable_base& t1, 
                                                   dftable_base& t2) const {
  auto left_column = left->execute(t1);
  auto right_column = right->execute(t2);
  auto aligned_right_column = realign_df(t1, t2, right_column);
  return left_column->idiv(aligned_right_column);
}

std::shared_ptr<dfcolumn> dffunction_idiv::aggregate
(dftable_base& table,
 node_local<std::vector<size_t>>& local_grouped_idx,
 node_local<std::vector<size_t>>& local_idx_split,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map,
 node_local<size_t>& row_sizes,
 dftable& grouped_table) {
  auto left_column = left->aggregate(table, local_grouped_idx,
                                     local_idx_split, hash_divide,
                                     merge_map, row_sizes, grouped_table);
  auto right_column = right->aggregate(table, local_grouped_idx,
                                       local_idx_split, hash_divide,
                                       merge_map, row_sizes, grouped_table);
  return left_column->idiv(right_column);
}

std::shared_ptr<dfcolumn>
dffunction_idiv::whole_column_aggregate(dftable_base& t) {
  auto left_column = left->whole_column_aggregate(t);
  return left_column->idiv(right->whole_column_aggregate(t));
}

std::shared_ptr<dffunction> idiv_col(const std::string& left,
                                     const std::string& right) {
  return std::make_shared<dffunction_idiv>(id_col(left), id_col(right));
}

std::shared_ptr<dffunction>
idiv_col(const std::shared_ptr<dffunction>& left, const std::string& right){
  return std::make_shared<dffunction_idiv>(left, id_col(right));
}

std::shared_ptr<dffunction>
idiv_col(const std::string& left, const std::shared_ptr<dffunction>& right) {
  return std::make_shared<dffunction_idiv>(id_col(left), right);
}
 
std::shared_ptr<dffunction>
idiv_col(const std::shared_ptr<dffunction>& left,
         const std::shared_ptr<dffunction>& right) {
  return std::make_shared<dffunction_idiv>(left, right);
} 

std::shared_ptr<dffunction> idiv_col_as(const std::string& left,
                                        const std::string& right,
                                        const std::string& as) {
  return std::make_shared<dffunction_idiv>(id_col(left), id_col(right), as);
}

std::shared_ptr<dffunction>
idiv_col_as(const std::shared_ptr<dffunction>& left, const std::string& right,
            const std::string& as) {
  return std::make_shared<dffunction_idiv>(left, id_col(right), as);
}

std::shared_ptr<dffunction>
idiv_col_as(const std::string& left, const std::shared_ptr<dffunction>& right,
            const std::string& as) {
  return std::make_shared<dffunction_idiv>(id_col(left), right, as);
}

std::shared_ptr<dffunction>
idiv_col_as(const std::shared_ptr<dffunction>& left,
            const std::shared_ptr<dffunction>& right,
            const std::string& as) {
  return std::make_shared<dffunction_idiv>(left, right, as);
}


// ----- mod -----
std::shared_ptr<dfcolumn> dffunction_mod::execute(dftable_base& t) const {
  auto left_column = left->execute(t);
  return left_column->mod(right->execute(t));
}

std::shared_ptr<dfcolumn> dffunction_mod::execute(dftable_base& t1, 
                                                  dftable_base& t2) const {
  auto left_column = left->execute(t1);
  auto right_column = right->execute(t2);
  auto aligned_right_column = realign_df(t1, t2, right_column);
  return left_column->mod(aligned_right_column);
}

std::shared_ptr<dfcolumn> dffunction_mod::aggregate
(dftable_base& table,
 node_local<std::vector<size_t>>& local_grouped_idx,
 node_local<std::vector<size_t>>& local_idx_split,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map,
 node_local<size_t>& row_sizes,
 dftable& grouped_table) {
  auto left_column = left->aggregate(table, local_grouped_idx,
                                     local_idx_split, hash_divide,
                                     merge_map, row_sizes, grouped_table);
  auto right_column = right->aggregate(table, local_grouped_idx,
                                       local_idx_split, hash_divide,
                                       merge_map, row_sizes, grouped_table);
  return left_column->mod(right_column);
}

std::shared_ptr<dfcolumn>
dffunction_mod::whole_column_aggregate(dftable_base& t) {
  auto left_column = left->whole_column_aggregate(t);
  return left_column->mod(right->whole_column_aggregate(t));
}

std::shared_ptr<dffunction> mod_col(const std::string& left,
                                    const std::string& right) {
  return std::make_shared<dffunction_mod>(id_col(left), id_col(right));
}

std::shared_ptr<dffunction>
mod_col(const std::shared_ptr<dffunction>& left, const std::string& right){
  return std::make_shared<dffunction_mod>(left, id_col(right));
}

std::shared_ptr<dffunction>
mod_col(const std::string& left, const std::shared_ptr<dffunction>& right) {
  return std::make_shared<dffunction_mod>(id_col(left), right);
}
 
std::shared_ptr<dffunction>
mod_col(const std::shared_ptr<dffunction>& left,
        const std::shared_ptr<dffunction>& right) {
  return std::make_shared<dffunction_mod>(left, right);
} 

std::shared_ptr<dffunction> mod_col_as(const std::string& left,
                                       const std::string& right,
                                       const std::string& as) {
  return std::make_shared<dffunction_mod>(id_col(left), id_col(right), as);
}

std::shared_ptr<dffunction>
mod_col_as(const std::shared_ptr<dffunction>& left, const std::string& right,
           const std::string& as) {
  return std::make_shared<dffunction_mod>(left, id_col(right), as);
}

std::shared_ptr<dffunction>
mod_col_as(const std::string& left, const std::shared_ptr<dffunction>& right,
           const std::string& as) {
  return std::make_shared<dffunction_mod>(id_col(left), right, as);
}

std::shared_ptr<dffunction>
mod_col_as(const std::shared_ptr<dffunction>& left,
           const std::shared_ptr<dffunction>& right,
           const std::string& as) {
  return std::make_shared<dffunction_mod>(left, right, as);
}


// ----- pow -----
std::shared_ptr<dfcolumn> dffunction_pow::execute(dftable_base& t) const {
  auto left_column = left->execute(t);
  return left_column->pow(right->execute(t));
}

std::shared_ptr<dfcolumn> dffunction_pow::execute(dftable_base& t1, 
                                                  dftable_base& t2) const {
  auto left_column = left->execute(t1);
  auto right_column = right->execute(t2);
  auto aligned_right_column = realign_df(t1, t2, right_column);
  return left_column->pow(aligned_right_column);
}

std::shared_ptr<dfcolumn> dffunction_pow::aggregate
(dftable_base& table,
 node_local<std::vector<size_t>>& local_grouped_idx,
 node_local<std::vector<size_t>>& local_idx_split,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map,
 node_local<size_t>& row_sizes,
 dftable& grouped_table) {
  auto left_column = left->aggregate(table, local_grouped_idx,
                                     local_idx_split, hash_divide,
                                     merge_map, row_sizes, grouped_table);
  auto right_column = right->aggregate(table, local_grouped_idx,
                                       local_idx_split, hash_divide,
                                       merge_map, row_sizes, grouped_table);
  return left_column->pow(right_column);
}

std::shared_ptr<dfcolumn>
dffunction_pow::whole_column_aggregate(dftable_base& t) {
  auto left_column = left->whole_column_aggregate(t);
  return left_column->pow(right->whole_column_aggregate(t));
}

std::shared_ptr<dffunction> pow_col(const std::string& left,
                                    const std::string& right) {
  return std::make_shared<dffunction_pow>(id_col(left), id_col(right));
}

std::shared_ptr<dffunction>
pow_col(const std::shared_ptr<dffunction>& left, const std::string& right){
  return std::make_shared<dffunction_pow>(left, id_col(right));
}

std::shared_ptr<dffunction>
pow_col(const std::string& left, const std::shared_ptr<dffunction>& right) {
  return std::make_shared<dffunction_pow>(id_col(left), right);
}
 
std::shared_ptr<dffunction>
pow_col(const std::shared_ptr<dffunction>& left,
        const std::shared_ptr<dffunction>& right) {
  return std::make_shared<dffunction_pow>(left, right);
} 

std::shared_ptr<dffunction> pow_col_as(const std::string& left,
                                       const std::string& right,
                                       const std::string& as) {
  return std::make_shared<dffunction_pow>(id_col(left), id_col(right), as);
}

std::shared_ptr<dffunction>
pow_col_as(const std::shared_ptr<dffunction>& left, const std::string& right,
           const std::string& as) {
  return std::make_shared<dffunction_pow>(left, id_col(right), as);
}

std::shared_ptr<dffunction>
pow_col_as(const std::string& left, const std::shared_ptr<dffunction>& right,
           const std::string& as) {
  return std::make_shared<dffunction_pow>(id_col(left), right, as);
}

std::shared_ptr<dffunction>
pow_col_as(const std::shared_ptr<dffunction>& left,
           const std::shared_ptr<dffunction>& right,
           const std::string& as) {
  return std::make_shared<dffunction_pow>(left, right, as);
}


// ----- abs -----
std::shared_ptr<dfcolumn> dffunction_abs::execute(dftable_base& t) const {
  auto left_column = left->execute(t);
  return left_column->abs();
}

std::shared_ptr<dfcolumn> dffunction_abs::aggregate
(dftable_base& table,
 node_local<std::vector<size_t>>& local_grouped_idx,
 node_local<std::vector<size_t>>& local_idx_split,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map,
 node_local<size_t>& row_sizes,
 dftable& grouped_table) {
  auto left_column = left->aggregate(table, local_grouped_idx,
                                     local_idx_split, hash_divide,
                                     merge_map, row_sizes, grouped_table);
  return left_column->abs();
}

std::shared_ptr<dfcolumn>
dffunction_abs::whole_column_aggregate(dftable_base& t) {
  auto left_column = left->whole_column_aggregate(t);
  return left_column->abs();
}

std::shared_ptr<dffunction>
abs_col(const std::string& left) {
  return std::make_shared<dffunction_abs>(id_col(left));
}

std::shared_ptr<dffunction>
abs_col(const std::shared_ptr<dffunction>& left) {
  return std::make_shared<dffunction_abs>(left);
}

std::shared_ptr<dffunction>
abs_col_as(const std::string& left, const std::string& as) {
  return std::make_shared<dffunction_abs>(id_col(left), as);
}

std::shared_ptr<dffunction>
abs_col_as(const std::shared_ptr<dffunction>& left, const std::string& as) {
  return std::make_shared<dffunction_abs>(left, as);
}


// ----- datetime_extract -----
std::shared_ptr<dfcolumn>
dffunction_datetime_extract::execute(dftable_base& t) const {
  auto left_column = left->execute(t);
  return left_column->datetime_extract(type);
}

std::shared_ptr<dfcolumn> dffunction_datetime_extract::aggregate
(dftable_base& table,
 node_local<std::vector<size_t>>& local_grouped_idx,
 node_local<std::vector<size_t>>& local_idx_split,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map,
 node_local<size_t>& row_sizes,
 dftable& grouped_table) {
  auto left_column = left->aggregate(table, local_grouped_idx,
                                     local_idx_split, hash_divide,
                                     merge_map, row_sizes, grouped_table);
  return left_column->datetime_extract(type);
}

std::shared_ptr<dfcolumn>
dffunction_datetime_extract::whole_column_aggregate(dftable_base& t) {
  auto left_column = left->whole_column_aggregate(t);
  return left_column->datetime_extract(type);
}

std::shared_ptr<dffunction>
datetime_extract_col(const std::string& left, datetime_type type) {
  return std::make_shared<dffunction_datetime_extract>(id_col(left), type);
}

std::shared_ptr<dffunction>
datetime_extract_col(const std::shared_ptr<dffunction>& left,
                     datetime_type type) {
  return std::make_shared<dffunction_datetime_extract>(left, type);
}

std::shared_ptr<dffunction>
datetime_extract_col_as(const std::string& left, datetime_type type,
                        const std::string& as) {
  return std::make_shared<dffunction_datetime_extract>(id_col(left), type, as);
}

std::shared_ptr<dffunction>
datetime_extract_col_as(const std::shared_ptr<dffunction>& left,
                        datetime_type type, const std::string& as) {
  return std::make_shared<dffunction_datetime_extract>(left, type, as);
}


// ----- datetime_diff -----
std::shared_ptr<dfcolumn>
dffunction_datetime_diff::execute(dftable_base& t) const {
  auto left_column = left->execute(t);
  return left_column->datetime_diff(right->execute(t), type);
}

std::shared_ptr<dfcolumn>
dffunction_datetime_diff::execute(dftable_base& t1, 
                                  dftable_base& t2) const {
  auto left_column = left->execute(t1);
  auto right_column = right->execute(t2);
  auto aligned_right_column = realign_df(t1, t2, right_column);
  return left_column->datetime_diff(aligned_right_column, type);
}

std::shared_ptr<dfcolumn> dffunction_datetime_diff::aggregate
(dftable_base& table,
 node_local<std::vector<size_t>>& local_grouped_idx,
 node_local<std::vector<size_t>>& local_idx_split,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map,
 node_local<size_t>& row_sizes,
 dftable& grouped_table) {
  auto left_column = left->aggregate(table, local_grouped_idx,
                                     local_idx_split, hash_divide,
                                     merge_map, row_sizes, grouped_table);
  auto right_column = right->aggregate(table, local_grouped_idx,
                                       local_idx_split, hash_divide,
                                       merge_map, row_sizes, grouped_table);
  return left_column->datetime_diff(right_column, type);
}

std::shared_ptr<dfcolumn>
dffunction_datetime_diff::whole_column_aggregate(dftable_base& t) {
  auto left_column = left->whole_column_aggregate(t);
  return left_column->datetime_diff(right->whole_column_aggregate(t), type);
}

std::shared_ptr<dffunction> datetime_diff_col(const std::string& left,
                                              const std::string& right,
                                              datetime_type type) {
  return std::make_shared<dffunction_datetime_diff>
    (id_col(left), id_col(right), type);
}

std::shared_ptr<dffunction>
datetime_diff_col(const std::shared_ptr<dffunction>& left,
                  const std::string& right,
                  datetime_type type) {
  return std::make_shared<dffunction_datetime_diff>(left, id_col(right), type);
}

std::shared_ptr<dffunction>
datetime_diff_col(const std::string& left,
                  const std::shared_ptr<dffunction>& right,
                  datetime_type type) {
  return std::make_shared<dffunction_datetime_diff>(id_col(left), right, type);
}
 
std::shared_ptr<dffunction>
datetime_diff_col(const std::shared_ptr<dffunction>& left,
                  const std::shared_ptr<dffunction>& right,
                  datetime_type type) {
  return std::make_shared<dffunction_datetime_diff>(left, right, type);
} 

std::shared_ptr<dffunction> datetime_diff_col_as(const std::string& left,
                                                 const std::string& right,
                                                 datetime_type type,
                                                 const std::string& as) {
  return std::make_shared<dffunction_datetime_diff>
    (id_col(left), id_col(right), type, as);
}

std::shared_ptr<dffunction>
datetime_diff_col_as(const std::shared_ptr<dffunction>& left,
                     const std::string& right,
                     datetime_type type,
                     const std::string& as) {
  return std::make_shared<dffunction_datetime_diff>
    (left, id_col(right), type, as);
}

std::shared_ptr<dffunction>
datetime_diff_col_as(const std::string& left,
                     const std::shared_ptr<dffunction>& right,
                     datetime_type type,
                     const std::string& as) {
  return std::make_shared<dffunction_datetime_diff>
    (id_col(left), right, type, as);
}

std::shared_ptr<dffunction>
datetime_diff_col_as(const std::shared_ptr<dffunction>& left,
                     const std::shared_ptr<dffunction>& right,
                     datetime_type type,
                     const std::string& as) {
  return std::make_shared<dffunction_datetime_diff>(left, right, type, as);
}


// ----- datetime_diff_im -----
std::shared_ptr<dfcolumn>
dffunction_datetime_diff_im::execute(dftable_base& t) const {
  auto left_column = left->execute(t);
  return is_reversed ? left_column->rdatetime_diff_im(right, type) 
    : left_column->datetime_diff_im(right, type);
}

std::shared_ptr<dfcolumn> dffunction_datetime_diff_im::aggregate
(dftable_base& table,
 node_local<std::vector<size_t>>& local_grouped_idx,
 node_local<std::vector<size_t>>& local_idx_split,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map,
 node_local<size_t>& row_sizes,
 dftable& grouped_table) {
  auto left_column = left->aggregate(table, local_grouped_idx,
                                     local_idx_split, hash_divide,
                                     merge_map, row_sizes, grouped_table);
  return is_reversed ? left_column->rdatetime_diff_im(right, type) 
    : left_column->datetime_diff_im(right, type);
}

std::shared_ptr<dfcolumn>
dffunction_datetime_diff_im::whole_column_aggregate(dftable_base& t) {
  auto left_column = left->whole_column_aggregate(t);
  return is_reversed ? left_column->rdatetime_diff_im(right, type) 
    : left_column->datetime_diff_im(right, type);
}

std::shared_ptr<dffunction>
datetime_diff_im(const std::string& left, datetime_t right,
                 datetime_type type) {
  return std::make_shared<dffunction_datetime_diff_im>(id_col(left), right,
                                                       type);
}

std::shared_ptr<dffunction>
datetime_diff_im_as(const std::string& left, datetime_t right,
                    datetime_type type, const std::string& as) {
  return std::make_shared<dffunction_datetime_diff_im>(id_col(left), right,
                                                       type, as);
}

std::shared_ptr<dffunction>
datetime_diff_im(const std::shared_ptr<dffunction>& left, datetime_t right,
                 datetime_type type) {
  return std::make_shared<dffunction_datetime_diff_im>(left, right, type);
}

std::shared_ptr<dffunction>
datetime_diff_im_as(const std::shared_ptr<dffunction>& left, datetime_t right,
                    datetime_type type, const std::string& as) {
  return std::make_shared<dffunction_datetime_diff_im>(left, right, type, as);
}

std::shared_ptr<dffunction>
datetime_diff_im
(datetime_t left, const std::string& right, datetime_type type) {
  return std::make_shared<dffunction_datetime_diff_im>(id_col(right), left,
                                                       type, true);
}

std::shared_ptr<dffunction>
datetime_diff_im_as(datetime_t left, const std::string& right,
                    datetime_type type, const std::string& as) {
  return std::make_shared<dffunction_datetime_diff_im>(id_col(right), left,
                                                       type, as, true);
}

std::shared_ptr<dffunction>
datetime_diff_im(datetime_t left, const std::shared_ptr<dffunction>& right,
                 datetime_type type) {
  return std::make_shared<dffunction_datetime_diff_im>(right, left, type, true);
}

std::shared_ptr<dffunction>
datetime_diff_im_as(datetime_t left, const std::shared_ptr<dffunction>& right,
                    datetime_type type, const std::string& as) {
  return std::make_shared<dffunction_datetime_diff_im>(right, left, type,
                                                       as, true);
}


// ----- datetime_add -----
std::shared_ptr<dfcolumn>
dffunction_datetime_add::execute(dftable_base& t) const {
  auto left_column = left->execute(t);
  return left_column->datetime_add(right->execute(t), type);
}

std::shared_ptr<dfcolumn>
dffunction_datetime_add::execute(dftable_base& t1, 
                                 dftable_base& t2) const {
  auto left_column = left->execute(t1);
  auto right_column = right->execute(t2);
  auto aligned_right_column = realign_df(t1, t2, right_column);
  return left_column->datetime_add(aligned_right_column, type);
}

std::shared_ptr<dfcolumn> dffunction_datetime_add::aggregate
(dftable_base& table,
 node_local<std::vector<size_t>>& local_grouped_idx,
 node_local<std::vector<size_t>>& local_idx_split,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map,
 node_local<size_t>& row_sizes,
 dftable& grouped_table) {
  auto left_column = left->aggregate(table, local_grouped_idx,
                                     local_idx_split, hash_divide,
                                     merge_map, row_sizes, grouped_table);
  auto right_column = right->aggregate(table, local_grouped_idx,
                                       local_idx_split, hash_divide,
                                       merge_map, row_sizes, grouped_table);
  return left_column->datetime_add(right_column, type);
}

std::shared_ptr<dfcolumn>
dffunction_datetime_add::whole_column_aggregate(dftable_base& t) {
  auto left_column = left->whole_column_aggregate(t);
  return left_column->datetime_add(right->whole_column_aggregate(t), type);
}

std::shared_ptr<dffunction> datetime_add_col(const std::string& left,
                                             const std::string& right,
                                             datetime_type type) {
  return std::make_shared<dffunction_datetime_add>
    (id_col(left), id_col(right), type);
}

std::shared_ptr<dffunction>
datetime_add_col(const std::shared_ptr<dffunction>& left,
                 const std::string& right,
                 datetime_type type) {
  return std::make_shared<dffunction_datetime_add>(left, id_col(right), type);
}

std::shared_ptr<dffunction>
datetime_add_col(const std::string& left,
                 const std::shared_ptr<dffunction>& right,
                 datetime_type type) {
  return std::make_shared<dffunction_datetime_add>(id_col(left), right, type);
}
 
std::shared_ptr<dffunction>
datetime_add_col(const std::shared_ptr<dffunction>& left,
                 const std::shared_ptr<dffunction>& right,
                 datetime_type type) {
  return std::make_shared<dffunction_datetime_add>(left, right, type);
} 

std::shared_ptr<dffunction> datetime_add_col_as(const std::string& left,
                                                const std::string& right,
                                                datetime_type type,
                                                const std::string& as) {
  return std::make_shared<dffunction_datetime_add>
    (id_col(left), id_col(right), type, as);
}

std::shared_ptr<dffunction>
datetime_add_col_as(const std::shared_ptr<dffunction>& left,
                    const std::string& right,
                    datetime_type type,
                    const std::string& as) {
  return std::make_shared<dffunction_datetime_add>
    (left, id_col(right), type, as);
}

std::shared_ptr<dffunction>
datetime_add_col_as(const std::string& left,
                    const std::shared_ptr<dffunction>& right,
                    datetime_type type,
                    const std::string& as) {
  return std::make_shared<dffunction_datetime_add>
    (id_col(left), right, type, as);
}

std::shared_ptr<dffunction>
datetime_add_col_as(const std::shared_ptr<dffunction>& left,
                    const std::shared_ptr<dffunction>& right,
                    datetime_type type,
                    const std::string& as) {
  return std::make_shared<dffunction_datetime_add>(left, right, type, as);
}


// ----- datetime_add_im -----
std::shared_ptr<dfcolumn>
dffunction_datetime_add_im::execute(dftable_base& t) const {
  auto left_column = left->execute(t);
  return left_column->datetime_add_im(right, type);
}

std::shared_ptr<dfcolumn> dffunction_datetime_add_im::aggregate
(dftable_base& table,
 node_local<std::vector<size_t>>& local_grouped_idx,
 node_local<std::vector<size_t>>& local_idx_split,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map,
 node_local<size_t>& row_sizes,
 dftable& grouped_table) {
  auto left_column = left->aggregate(table, local_grouped_idx,
                                     local_idx_split, hash_divide,
                                     merge_map, row_sizes, grouped_table);
  return left_column->datetime_add_im(right, type);
}

std::shared_ptr<dfcolumn>
dffunction_datetime_add_im::whole_column_aggregate(dftable_base& t) {
  auto left_column = left->whole_column_aggregate(t);
  return left_column->datetime_add_im(right, type);
}

std::shared_ptr<dffunction>
datetime_add_im(const std::string& left, int right, datetime_type type) {
  return std::make_shared<dffunction_datetime_add_im>(id_col(left), right, type);
}

std::shared_ptr<dffunction>
datetime_add_im_as(const std::string& left, int right, datetime_type type,
                   const std::string& as) {
  return std::make_shared<dffunction_datetime_add_im>(id_col(left), right,
                                                      type, as);
}

std::shared_ptr<dffunction>
datetime_add_im(const std::shared_ptr<dffunction>& left, int right,
                datetime_type type) {
  return std::make_shared<dffunction_datetime_add_im>(left, right, type);
}

std::shared_ptr<dffunction>
datetime_add_im_as(const std::shared_ptr<dffunction>& left, int right,
                   datetime_type type, const std::string& as) {
  return std::make_shared<dffunction_datetime_add_im>(left, right, type, as);
}

// ----- datetime_sub -----
std::shared_ptr<dfcolumn>
dffunction_datetime_sub::execute(dftable_base& t) const {
  auto left_column = left->execute(t);
  return left_column->datetime_sub(right->execute(t), type);
}

std::shared_ptr<dfcolumn>
dffunction_datetime_sub::execute(dftable_base& t1, 
                                 dftable_base& t2) const {
  auto left_column = left->execute(t1);
  auto right_column = right->execute(t2);
  auto aligned_right_column = realign_df(t1, t2, right_column);
  return left_column->datetime_sub(aligned_right_column, type);
}

std::shared_ptr<dfcolumn> dffunction_datetime_sub::aggregate
(dftable_base& table,
 node_local<std::vector<size_t>>& local_grouped_idx,
 node_local<std::vector<size_t>>& local_idx_split,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map,
 node_local<size_t>& row_sizes,
 dftable& grouped_table) {
  auto left_column = left->aggregate(table, local_grouped_idx,
                                     local_idx_split, hash_divide,
                                     merge_map, row_sizes, grouped_table);
  auto right_column = right->aggregate(table, local_grouped_idx,
                                       local_idx_split, hash_divide,
                                       merge_map, row_sizes, grouped_table);
  return left_column->datetime_sub(right_column, type);
}

std::shared_ptr<dfcolumn>
dffunction_datetime_sub::whole_column_aggregate(dftable_base& t) {
  auto left_column = left->whole_column_aggregate(t);
  return left_column->datetime_sub(right->whole_column_aggregate(t), type);
}

std::shared_ptr<dffunction> datetime_sub_col(const std::string& left,
                                             const std::string& right,
                                             datetime_type type) {
  return std::make_shared<dffunction_datetime_sub>
    (id_col(left), id_col(right), type);
}

std::shared_ptr<dffunction>
datetime_sub_col(const std::shared_ptr<dffunction>& left,
                 const std::string& right,
                 datetime_type type) {
  return std::make_shared<dffunction_datetime_sub>(left, id_col(right), type);
}

std::shared_ptr<dffunction>
datetime_sub_col(const std::string& left,
                 const std::shared_ptr<dffunction>& right,
                 datetime_type type) {
  return std::make_shared<dffunction_datetime_sub>(id_col(left), right, type);
}
 
std::shared_ptr<dffunction>
datetime_sub_col(const std::shared_ptr<dffunction>& left,
                 const std::shared_ptr<dffunction>& right,
                 datetime_type type) {
  return std::make_shared<dffunction_datetime_sub>(left, right, type);
} 

std::shared_ptr<dffunction> datetime_sub_col_as(const std::string& left,
                                                const std::string& right,
                                                datetime_type type,
                                                const std::string& as) {
  return std::make_shared<dffunction_datetime_sub>
    (id_col(left), id_col(right), type, as);
}

std::shared_ptr<dffunction>
datetime_sub_col_as(const std::shared_ptr<dffunction>& left,
                    const std::string& right,
                    datetime_type type,
                    const std::string& as) {
  return std::make_shared<dffunction_datetime_sub>
    (left, id_col(right), type, as);
}

std::shared_ptr<dffunction>
datetime_sub_col_as(const std::string& left,
                    const std::shared_ptr<dffunction>& right,
                    datetime_type type,
                    const std::string& as) {
  return std::make_shared<dffunction_datetime_sub>
    (id_col(left), right, type, as);
}

std::shared_ptr<dffunction>
datetime_sub_col_as(const std::shared_ptr<dffunction>& left,
                    const std::shared_ptr<dffunction>& right,
                    datetime_type type,
                    const std::string& as) {
  return std::make_shared<dffunction_datetime_sub>(left, right, type, as);
}


// ----- datetime_sub_im -----
std::shared_ptr<dfcolumn>
dffunction_datetime_sub_im::execute(dftable_base& t) const {
  auto left_column = left->execute(t);
  return left_column->datetime_sub_im(right, type);
}

std::shared_ptr<dfcolumn> dffunction_datetime_sub_im::aggregate
(dftable_base& table,
 node_local<std::vector<size_t>>& local_grouped_idx,
 node_local<std::vector<size_t>>& local_idx_split,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map,
 node_local<size_t>& row_sizes,
 dftable& grouped_table) {
  auto left_column = left->aggregate(table, local_grouped_idx,
                                     local_idx_split, hash_divide,
                                     merge_map, row_sizes, grouped_table);
  return left_column->datetime_sub_im(right, type);
}

std::shared_ptr<dfcolumn>
dffunction_datetime_sub_im::whole_column_aggregate(dftable_base& t) {
  auto left_column = left->whole_column_aggregate(t);
  return left_column->datetime_sub_im(right, type);
}

std::shared_ptr<dffunction>
datetime_sub_im(const std::string& left, int right, datetime_type type) {
  return std::make_shared<dffunction_datetime_sub_im>(id_col(left), right, type);
}

std::shared_ptr<dffunction>
datetime_sub_im_as(const std::string& left, int right, datetime_type type,
                   const std::string& as) {
  return std::make_shared<dffunction_datetime_sub_im>(id_col(left), right,
                                                      type, as);
}

std::shared_ptr<dffunction>
datetime_sub_im(const std::shared_ptr<dffunction>& left, int right,
                datetime_type type) {
  return std::make_shared<dffunction_datetime_sub_im>(left, right, type);
}

std::shared_ptr<dffunction>
datetime_sub_im_as(const std::shared_ptr<dffunction>& left, int right,
                   datetime_type type, const std::string& as) {
  return std::make_shared<dffunction_datetime_sub_im>(left, right, type, as);
}


// ----- datetime_truncate -----
std::shared_ptr<dfcolumn>
dffunction_datetime_truncate::execute(dftable_base& t) const {
  auto left_column = left->execute(t);
  return left_column->datetime_truncate(type);
}

std::shared_ptr<dfcolumn> dffunction_datetime_truncate::aggregate
(dftable_base& table,
 node_local<std::vector<size_t>>& local_grouped_idx,
 node_local<std::vector<size_t>>& local_idx_split,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map,
 node_local<size_t>& row_sizes,
 dftable& grouped_table) {
  auto left_column = left->aggregate(table, local_grouped_idx,
                                     local_idx_split, hash_divide,
                                     merge_map, row_sizes, grouped_table);
  return left_column->datetime_truncate(type);
}

std::shared_ptr<dfcolumn>
dffunction_datetime_truncate::whole_column_aggregate(dftable_base& t) {
  auto left_column = left->whole_column_aggregate(t);
  return left_column->datetime_truncate(type);
}

std::shared_ptr<dffunction>
datetime_truncate_col(const std::string& left, datetime_type type) {
  return std::make_shared<dffunction_datetime_truncate>(id_col(left), type);
}

std::shared_ptr<dffunction>
datetime_truncate_col(const std::shared_ptr<dffunction>& left,
                      datetime_type type) {
  return std::make_shared<dffunction_datetime_truncate>(left, type);
}

std::shared_ptr<dffunction>
datetime_truncate_col_as(const std::string& left, datetime_type type,
                         const std::string& as) {
  return std::make_shared<dffunction_datetime_truncate>(id_col(left), type, as);
}

std::shared_ptr<dffunction>
datetime_truncate_col_as(const std::shared_ptr<dffunction>& left,
                         datetime_type type, const std::string& as) {
  return std::make_shared<dffunction_datetime_truncate>(left, type, as);
}


// ----- datetime_months_between -----
std::shared_ptr<dfcolumn>
dffunction_datetime_months_between::execute(dftable_base& t) const {
  auto left_column = left->execute(t);
  return left_column->datetime_months_between(right->execute(t));
}

std::shared_ptr<dfcolumn>
dffunction_datetime_months_between::execute(dftable_base& t1, 
                                            dftable_base& t2) const {
  auto left_column = left->execute(t1);
  auto right_column = right->execute(t2);
  auto aligned_right_column = realign_df(t1, t2, right_column);
  return left_column->datetime_months_between(aligned_right_column);
}

std::shared_ptr<dfcolumn> dffunction_datetime_months_between::aggregate
(dftable_base& table,
 node_local<std::vector<size_t>>& local_grouped_idx,
 node_local<std::vector<size_t>>& local_idx_split,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map,
 node_local<size_t>& row_sizes,
 dftable& grouped_table) {
  auto left_column = left->aggregate(table, local_grouped_idx,
                                     local_idx_split, hash_divide,
                                     merge_map, row_sizes, grouped_table);
  auto right_column = right->aggregate(table, local_grouped_idx,
                                       local_idx_split, hash_divide,
                                       merge_map, row_sizes, grouped_table);
  return left_column->datetime_months_between(right_column);
}

std::shared_ptr<dfcolumn>
dffunction_datetime_months_between::whole_column_aggregate(dftable_base& t) {
  auto left_column = left->whole_column_aggregate(t);
  return left_column->datetime_months_between(right->whole_column_aggregate(t));
}

std::shared_ptr<dffunction> datetime_months_between_col
(const std::string& left, const std::string& right) {
  return std::make_shared<dffunction_datetime_months_between>
    (id_col(left), id_col(right));
}

std::shared_ptr<dffunction>
datetime_months_between_col(const std::shared_ptr<dffunction>& left,
                            const std::string& right) {
  return std::make_shared<dffunction_datetime_months_between>
    (left, id_col(right));
}

std::shared_ptr<dffunction>
datetime_months_between_col(const std::string& left,
                            const std::shared_ptr<dffunction>& right) {
  return std::make_shared<dffunction_datetime_months_between>
    (id_col(left), right);
}
 
std::shared_ptr<dffunction>
datetime_months_between_col(const std::shared_ptr<dffunction>& left,
                            const std::shared_ptr<dffunction>& right) {
  return std::make_shared<dffunction_datetime_months_between>(left, right);
} 

std::shared_ptr<dffunction>
datetime_months_between_col_as(const std::string& left,
                               const std::string& right,
                               const std::string& as) {
  return std::make_shared<dffunction_datetime_months_between>
    (id_col(left), id_col(right), as);
}

std::shared_ptr<dffunction>
datetime_months_between_col_as(const std::shared_ptr<dffunction>& left,
                               const std::string& right,
                               const std::string& as) {
  return std::make_shared<dffunction_datetime_months_between>
    (left, id_col(right), as);
}

std::shared_ptr<dffunction>
datetime_months_between_col_as(const std::string& left,
                               const std::shared_ptr<dffunction>& right,
                               const std::string& as) {
  return std::make_shared<dffunction_datetime_months_between>
    (id_col(left), right, as);
}

std::shared_ptr<dffunction>
datetime_months_between_col_as(const std::shared_ptr<dffunction>& left,
                               const std::shared_ptr<dffunction>& right,
                               const std::string& as) {
  return std::make_shared<dffunction_datetime_months_between>(left, right, as);
}


// ----- datetime_next_day -----
std::shared_ptr<dfcolumn>
dffunction_datetime_next_day::execute(dftable_base& t) const {
  auto left_column = left->execute(t);
  return left_column->datetime_next_day(right->execute(t));
}

std::shared_ptr<dfcolumn>
dffunction_datetime_next_day::execute(dftable_base& t1, 
                                      dftable_base& t2) const {
  auto left_column = left->execute(t1);
  auto right_column = right->execute(t2);
  auto aligned_right_column = realign_df(t1, t2, right_column);
  return left_column->datetime_next_day(aligned_right_column);
}

std::shared_ptr<dfcolumn> dffunction_datetime_next_day::aggregate
(dftable_base& table,
 node_local<std::vector<size_t>>& local_grouped_idx,
 node_local<std::vector<size_t>>& local_idx_split,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map,
 node_local<size_t>& row_sizes,
 dftable& grouped_table) {
  auto left_column = left->aggregate(table, local_grouped_idx,
                                     local_idx_split, hash_divide,
                                     merge_map, row_sizes, grouped_table);
  auto right_column = right->aggregate(table, local_grouped_idx,
                                       local_idx_split, hash_divide,
                                       merge_map, row_sizes, grouped_table);
  return left_column->datetime_next_day(right_column);
}

std::shared_ptr<dfcolumn>
dffunction_datetime_next_day::whole_column_aggregate(dftable_base& t) {
  auto left_column = left->whole_column_aggregate(t);
  return left_column->datetime_next_day(right->whole_column_aggregate(t));
}

std::shared_ptr<dffunction> datetime_next_day_col(const std::string& left,
                                                  const std::string& right) {
  return std::make_shared<dffunction_datetime_next_day>
    (id_col(left), id_col(right));
}

std::shared_ptr<dffunction>
datetime_next_day_col(const std::shared_ptr<dffunction>& left,
                      const std::string& right) {
  return std::make_shared<dffunction_datetime_next_day>(left, id_col(right));
}

std::shared_ptr<dffunction>
datetime_next_day_col(const std::string& left,
                      const std::shared_ptr<dffunction>& right) {
  return std::make_shared<dffunction_datetime_next_day>(id_col(left), right);
}
 
std::shared_ptr<dffunction>
datetime_next_day_col(const std::shared_ptr<dffunction>& left,
                      const std::shared_ptr<dffunction>& right) {
  return std::make_shared<dffunction_datetime_next_day>(left, right);
} 

std::shared_ptr<dffunction> datetime_next_day_col_as(const std::string& left,
                                                     const std::string& right,
                                                     const std::string& as) {
  return std::make_shared<dffunction_datetime_next_day>
    (id_col(left), id_col(right), as);
}

std::shared_ptr<dffunction>
datetime_next_day_col_as(const std::shared_ptr<dffunction>& left,
                         const std::string& right,
                         const std::string& as) {
  return std::make_shared<dffunction_datetime_next_day>
    (left, id_col(right), as);
}

std::shared_ptr<dffunction>
datetime_next_day_col_as(const std::string& left,
                         const std::shared_ptr<dffunction>& right,
                         const std::string& as) {
  return std::make_shared<dffunction_datetime_next_day>
    (id_col(left), right, as);
}

std::shared_ptr<dffunction>
datetime_next_day_col_as(const std::shared_ptr<dffunction>& left,
                         const std::shared_ptr<dffunction>& right,
                         const std::string& as) {
  return std::make_shared<dffunction_datetime_next_day>(left, right, as);
}


// ----- datetime_next_day_im -----
std::shared_ptr<dfcolumn>
dffunction_datetime_next_day_im::execute(dftable_base& t) const {
  auto left_column = left->execute(t);
  return left_column->datetime_next_day_im(right);
}

std::shared_ptr<dfcolumn> dffunction_datetime_next_day_im::aggregate
(dftable_base& table,
 node_local<std::vector<size_t>>& local_grouped_idx,
 node_local<std::vector<size_t>>& local_idx_split,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map,
 node_local<size_t>& row_sizes,
 dftable& grouped_table) {
  auto left_column = left->aggregate(table, local_grouped_idx,
                                     local_idx_split, hash_divide,
                                     merge_map, row_sizes, grouped_table);
  return left_column->datetime_next_day_im(right);
}

std::shared_ptr<dfcolumn>
dffunction_datetime_next_day_im::whole_column_aggregate(dftable_base& t) {
  auto left_column = left->whole_column_aggregate(t);
  return left_column->datetime_next_day_im(right);
}

std::shared_ptr<dffunction>
datetime_next_day_im(const std::string& left, int right) {
  return std::make_shared<dffunction_datetime_next_day_im>(id_col(left), right);
}

std::shared_ptr<dffunction>
datetime_next_day_im_as(const std::string& left, int right,
                        const std::string& as) {
  return std::make_shared<dffunction_datetime_next_day_im>
    (id_col(left), right,as);
}

std::shared_ptr<dffunction>
datetime_next_day_im(const std::shared_ptr<dffunction>& left, int right) {
  return std::make_shared<dffunction_datetime_next_day_im>(left, right);
}

std::shared_ptr<dffunction>
datetime_next_day_im_as(const std::shared_ptr<dffunction>& left, int right,
                        const std::string& as) {
  return std::make_shared<dffunction_datetime_next_day_im>(left, right, as);
}

// ----- dffunction_when -----

std::shared_ptr<dfcolumn>
merge_column(std::vector<std::shared_ptr<dfcolumn>>& columns,
             std::vector<node_local<std::vector<size_t>>>& idx,
             node_local<std::vector<size_t>>& table_idx,
             node_local<std::vector<size_t>>& null_idx,
             bool use_null_idx);

std::shared_ptr<dfcolumn>
dffunction_when::execute(dftable_base& t) const {
  if(!(cond.size() == func.size()) && !(cond.size() + 1 == func.size())) {
    throw std::runtime_error
      ("condition size should be equal to func size or func size - 1");
  }
  if(func.size() == 0) throw std::runtime_error("func size is 0");
  std::vector<node_local<std::vector<size_t>>> new_idx(func.size());
  std::vector<std::shared_ptr<dfcolumn>> new_columns(func.size());
  auto table_idx = t.get_local_index();
  auto crnt_idx = table_idx;
  for(size_t i = 0; i < cond.size(); i++) {
    filtered_dftable crnt_table(t, crnt_idx);
    auto filtered = crnt_table.filter(cond[i]);
    new_columns[i] = func[i]->execute(filtered);
    new_idx[i] = filtered.get_local_index();
    crnt_idx = crnt_idx.map
      (+[](const std::vector<size_t>& crnt_idx,
           const std::vector<size_t>& new_idx) {
        return set_difference(crnt_idx, new_idx);
      }, new_idx[i]);
  }
  if(cond.size() + 1 == func.size()) {
    filtered_dftable crnt_table(t, crnt_idx);
    new_columns[func.size() - 1] = func[func.size() - 1]->execute(crnt_table);
    new_idx[func.size() - 1] = crnt_idx;
    node_local<std::vector<size_t>> dummy;
    return merge_column(new_columns, new_idx, table_idx, dummy, false);
  } else {
    return merge_column(new_columns, new_idx, table_idx, crnt_idx, true);
  }
}

std::shared_ptr<dfcolumn>
dffunction_when::aggregate
(dftable_base& table,
 node_local<std::vector<size_t>>& local_grouped_idx,
 node_local<std::vector<size_t>>& local_idx_split,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map,
 node_local<size_t>& row_sizes,
 dftable& grouped_table) {
  if(!(cond.size() == func.size()) && !(cond.size() + 1 == func.size())) {
    throw std::runtime_error
      ("condition size should be equal to func size or func size - 1");
  }
  if(func.size() == 0) throw std::runtime_error("func size is 0");
  std::vector<node_local<std::vector<size_t>>> new_idx(func.size());
  std::vector<std::shared_ptr<dfcolumn>> new_columns(func.size());
  auto table_idx = grouped_table.get_local_index();
  auto crnt_idx = table_idx;
  for(size_t i = 0; i < cond.size(); i++) {
    auto tmp_column = func[i]->aggregate
      (table, local_grouped_idx, local_idx_split, hash_divide,
       merge_map, row_sizes, grouped_table);
    auto cond_idx = cond[i]->aggregate_filter
      (table, local_grouped_idx, local_idx_split, hash_divide,
       merge_map, row_sizes, grouped_table);
    new_idx[i] = cond_idx.map
      (+[](std::vector<size_t>& cond_idx,
           std::vector<size_t>& crnt_idx) {
        return set_intersection(cond_idx, crnt_idx);
      }, crnt_idx);
    dftable tmp_table;
    tmp_table.append_column("tmp",tmp_column);
    filtered_dftable filtered_tmp_table(tmp_table, new_idx[i]);
    new_columns[i] = filtered_tmp_table.column("tmp");
    crnt_idx = crnt_idx.map
      (+[](const std::vector<size_t>& crnt_idx,
           const std::vector<size_t>& new_idx) {
        return set_difference(crnt_idx, new_idx);
      }, new_idx[i]);
  }
  if(cond.size() + 1 == func.size()) {
    auto tmp_column =
      func[func.size() - 1]->aggregate(table, local_grouped_idx,
                                       local_idx_split, hash_divide,
                                       merge_map, row_sizes, grouped_table);
    new_idx[func.size() - 1] = crnt_idx;
    dftable tmp_table;
    tmp_table.append_column("tmp",tmp_column);
    filtered_dftable filtered_tmp_table(tmp_table, new_idx[func.size() - 1]);
    new_columns[func.size() - 1] = filtered_tmp_table.column("tmp");
    node_local<std::vector<size_t>> dummy;
    return merge_column(new_columns, new_idx, table_idx, dummy, false);
  } else {
    return merge_column(new_columns, new_idx, table_idx, crnt_idx, true);
  }
}

std::shared_ptr<dfcolumn>
dffunction_when::whole_column_aggregate(dftable_base& t) {
  if(!(cond.size() == func.size()) && !(cond.size() + 1 == func.size())) {
    throw std::runtime_error
      ("condition size should be equal to func size or func size - 1");
  }
  if(func.size() == 0) throw std::runtime_error("func size is 0");
  std::vector<node_local<std::vector<size_t>>> new_idx(func.size());
  std::vector<std::shared_ptr<dfcolumn>> new_columns(func.size());
  std::vector<std::vector<size_t>> sizes(get_nodesize());
  sizes[0].push_back(0);
  auto table_idx = make_node_local_scatter(sizes);
  auto crnt_idx = table_idx;
  for(size_t i = 0; i < cond.size(); i++) {
    auto tmp_column = func[i]->whole_column_aggregate(t);
    auto cond_idx = cond[i]->whole_column_aggregate_filter(t);
    new_idx[i] = cond_idx.map
      (+[](std::vector<size_t>& cond_idx,
           std::vector<size_t>& crnt_idx) {
        return set_intersection(cond_idx, crnt_idx);
      }, crnt_idx);
    dftable tmp_table;
    tmp_table.append_column("tmp",tmp_column);
    filtered_dftable filtered_tmp_table(tmp_table, new_idx[i]);
    new_columns[i] = filtered_tmp_table.column("tmp");
    crnt_idx = crnt_idx.map
      (+[](const std::vector<size_t>& crnt_idx,
           const std::vector<size_t>& new_idx) {
        return set_difference(crnt_idx, new_idx);
      }, new_idx[i]);
  }
  if(cond.size() + 1 == func.size()) {
    auto tmp_column =
      func[func.size() - 1]->whole_column_aggregate(t);
    new_idx[func.size() - 1] = crnt_idx;
    dftable tmp_table;
    tmp_table.append_column("tmp",tmp_column);
    filtered_dftable filtered_tmp_table(tmp_table, new_idx[func.size() - 1]);
    new_columns[func.size() - 1] = filtered_tmp_table.column("tmp");
    node_local<std::vector<size_t>> dummy;
    return merge_column(new_columns, new_idx, table_idx, dummy, false);
  } else {
    return merge_column(new_columns, new_idx, table_idx, crnt_idx, true);
  }
}

std::shared_ptr<dffunction> 
when(const std::vector<std::shared_ptr<dfoperator>>& cond,
     const std::vector<std::shared_ptr<dffunction>>& func) {
  return std::make_shared<dffunction_when>(cond, func);
}

std::shared_ptr<dffunction> 
when(const std::vector<std::pair<std::shared_ptr<dfoperator>, 
     std::shared_ptr<dffunction>>>& cond_func_pairs) {
  auto size = cond_func_pairs.size();
  std::vector<std::shared_ptr<dfoperator>> cond(size);
  std::vector<std::shared_ptr<dffunction>> func(size);
  for(size_t i = 0; i < size; i++) {
    cond[i] = cond_func_pairs[i].first;
    func[i] = cond_func_pairs[i].second;
  }
  return std::make_shared<dffunction_when>(cond, func);
}

std::shared_ptr<dffunction> 
when(const std::vector<std::pair<std::shared_ptr<dfoperator>, 
     std::shared_ptr<dffunction>>>& cond_func_pairs,
     const std::shared_ptr<dffunction>& else_func) {
  auto size = cond_func_pairs.size();
  std::vector<std::shared_ptr<dfoperator>> cond(size);
  std::vector<std::shared_ptr<dffunction>> func(size+1);
  for(size_t i = 0; i < size; i++) {
    cond[i] = cond_func_pairs[i].first;
    func[i] = cond_func_pairs[i].second;
  }
  func[size] = else_func;
  return std::make_shared<dffunction_when>(cond, func);
}

std::pair<std::shared_ptr<dfoperator>, std::shared_ptr<dffunction>>
operator>>(const std::shared_ptr<dfoperator>& cond,
           const std::shared_ptr<dffunction>& func) {
  return make_pair(cond, func);
}


// ----- datetime_im -----
std::shared_ptr<dfcolumn>
dffunction_datetime_im::execute(dftable_base& t) const {
  auto num_rows = make_node_local_scatter(t.num_rows());
  auto nlval = num_rows.map(+[](size_t num_row, datetime_t value) {
      return std::vector<datetime_t>(num_row, value);
    }, broadcast(value));
  auto dvval = nlval.template moveto_dvector<datetime_t>();
  return std::make_shared<typed_dfcolumn<datetime>>(std::move(dvval));
}

std::shared_ptr<dfcolumn> dffunction_datetime_im::aggregate
(dftable_base& table,
 node_local<std::vector<size_t>>& local_grouped_idx,
 node_local<std::vector<size_t>>& local_idx_split,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map,
 node_local<size_t>& row_sizes,
 dftable& grouped_table) {
  auto num_rows = make_node_local_scatter(grouped_table.num_rows());
  auto nlval = num_rows.map(+[](size_t num_row, datetime_t value) {
      return std::vector<datetime_t>(num_row, value);
    }, broadcast(value));
  auto dvval = nlval.template moveto_dvector<datetime_t>();
  return std::make_shared<typed_dfcolumn<datetime>>(std::move(dvval));
}

std::shared_ptr<dfcolumn>
dffunction_datetime_im::whole_column_aggregate(dftable_base& t) {
  std::vector<datetime_t> v = {value};
  return std::make_shared<typed_dfcolumn<datetime>>(make_dvector_scatter(v));
}

std::shared_ptr<dffunction>
datetime_im(datetime_t value) {
  return std::make_shared<dffunction_datetime_im>(value);
}

std::shared_ptr<dffunction>
datetime_im_as(datetime_t value, const std::string& as) {
  return std::make_shared<dffunction_datetime_im>(value, as);
}

std::shared_ptr<dffunction>
make_date_im(int year, int month, int day) {
  auto value = makedatetime(year, month, day);
  std::string as = "make_date(" + STR(year) + "-" + STR(month) + "-"
    + STR(day) + ")";
  return datetime_im_as(value, as);
}

std::shared_ptr<dffunction>
make_date_im_as(int year, int month, int day, const std::string& as) {
  auto value = makedatetime(year, month, day);
  return datetime_im_as(value, as);
}

std::shared_ptr<dffunction>
make_datetime_im(int year, int month, int day,
                 int hour, int minute, int second) {
  auto value = makedatetime(year, month, day, hour, minute, second);
  std::string as = "make_datetime(" + STR(year) + "-" + STR(month) + "-"
    + STR(day) + " " + STR(hour) + ":" + STR(minute) + ":" + STR(second) + ")";
  return datetime_im_as(value, as);
}

std::shared_ptr<dffunction>
make_datetime_im_as(int year, int month, int day,
                    int hour, int minute, int second,
                    const std::string& as) {
  auto value = makedatetime(year, month, day, hour, minute, second);
  return datetime_im_as(value, as);
}


// ----- dic_string_im -----
std::shared_ptr<dfcolumn>
dffunction_dic_string_im::execute(dftable_base& t) const {
  auto num_rows = make_node_local_scatter(t.num_rows());
  auto nlval = num_rows.map(+[](size_t num_row) {
      return std::vector<size_t>(num_row); // dictionary index = 0
    });
  words ws;
  ws.chars = char_to_int(value);
  ws.starts = {0};
  ws.lens = {value.size()};
  auto dic = std::make_shared<dict>(make_dict(ws));
  auto nulls = make_node_local_allocate<std::vector<size_t>>();
  return std::make_shared<typed_dfcolumn<dic_string>>
    (std::move(dic), std::move(nlval), std::move(nulls));
}

std::shared_ptr<dfcolumn> dffunction_dic_string_im::aggregate
(dftable_base& table,
 node_local<std::vector<size_t>>& local_grouped_idx,
 node_local<std::vector<size_t>>& local_idx_split,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map,
 node_local<size_t>& row_sizes,
 dftable& grouped_table) {
  auto num_rows = make_node_local_scatter(grouped_table.num_rows());
  auto nlval = num_rows.map(+[](size_t num_row) {
      return std::vector<size_t>(num_row); // dictionary index = 0
    });
  words ws;
  ws.chars = char_to_int(value);
  ws.starts = {0};
  ws.lens = {value.size()};
  auto dic = std::make_shared<dict>(make_dict(ws));
  auto nulls = make_node_local_allocate<std::vector<size_t>>();
  return std::make_shared<typed_dfcolumn<dic_string>>
    (std::move(dic), std::move(nlval), std::move(nulls));
}

std::shared_ptr<dfcolumn>
dffunction_dic_string_im::whole_column_aggregate(dftable_base& t) {
  std::vector<size_t> v = {0};
  auto nlval = make_dvector_scatter(v).moveto_node_local();
  words ws;
  ws.chars = char_to_int(value);
  ws.starts = {0};
  ws.lens = {value.size()};
  auto dic = std::make_shared<dict>(make_dict(ws));
  auto nulls = make_node_local_allocate<std::vector<size_t>>();
  return std::make_shared<typed_dfcolumn<dic_string>>
    (std::move(dic), std::move(nlval), std::move(nulls));
}

std::shared_ptr<dffunction>
dic_string_im(const std::string& value) {
  return std::make_shared<dffunction_dic_string_im>(value);
}

std::shared_ptr<dffunction>
dic_string_im_as(const std::string& value, const std::string& as) {
  return std::make_shared<dffunction_dic_string_im>(value, as);
}


// ----- null_string_column -----
std::shared_ptr<dfcolumn>
dffunction_null_string_column::execute(dftable_base& t) const {
  return create_null_column<std::string>(t.num_rows());
}

std::shared_ptr<dfcolumn> dffunction_null_string_column::aggregate
(dftable_base& table,
 node_local<std::vector<size_t>>& local_grouped_idx,
 node_local<std::vector<size_t>>& local_idx_split,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map,
 node_local<size_t>& row_sizes,
 dftable& grouped_table) {
  return create_null_column<std::string>(grouped_table.num_rows());
}

std::shared_ptr<dfcolumn>
dffunction_null_string_column::whole_column_aggregate(dftable_base& t) {
  std::vector<size_t> sizes(get_nodesize());
  sizes[0] = 1;
  return create_null_column<std::string>(sizes);
}

std::shared_ptr<dffunction>
null_string_column() {
  return std::make_shared<dffunction_null_string_column>();
}

std::shared_ptr<dffunction>
null_string_column_as(const std::string& as) {
  return std::make_shared<dffunction_null_string_column>(as);
}


// ----- null_dic_string_column -----
std::shared_ptr<dfcolumn>
dffunction_null_dic_string_column::execute(dftable_base& t) const {
  return create_null_column<dic_string>(t.num_rows());
}

std::shared_ptr<dfcolumn> dffunction_null_dic_string_column::aggregate
(dftable_base& table,
 node_local<std::vector<size_t>>& local_grouped_idx,
 node_local<std::vector<size_t>>& local_idx_split,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map,
 node_local<size_t>& row_sizes,
 dftable& grouped_table) {
  return create_null_column<dic_string>(grouped_table.num_rows());
}

std::shared_ptr<dfcolumn>
dffunction_null_dic_string_column::whole_column_aggregate(dftable_base& t) {
  std::vector<size_t> sizes(get_nodesize());
  sizes[0] = 1;
  return create_null_column<dic_string>(sizes);
}

std::shared_ptr<dffunction>
null_dic_string_column() {
  return std::make_shared<dffunction_null_dic_string_column>();
}

std::shared_ptr<dffunction>
null_dic_string_column_as(const std::string& as) {
  return std::make_shared<dffunction_null_dic_string_column>(as);
}


// ----- null_datetime_column -----
std::shared_ptr<dfcolumn>
dffunction_null_datetime_column::execute(dftable_base& t) const {
  return create_null_column<datetime>(t.num_rows());
}

std::shared_ptr<dfcolumn> dffunction_null_datetime_column::aggregate
(dftable_base& table,
 node_local<std::vector<size_t>>& local_grouped_idx,
 node_local<std::vector<size_t>>& local_idx_split,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map,
 node_local<size_t>& row_sizes,
 dftable& grouped_table) {
  return create_null_column<datetime>(grouped_table.num_rows());
}

std::shared_ptr<dfcolumn>
dffunction_null_datetime_column::whole_column_aggregate(dftable_base& t) {
  std::vector<size_t> sizes(get_nodesize());
  sizes[0] = 1;
  return create_null_column<datetime>(sizes);
}

std::shared_ptr<dffunction>
null_datetime_column() {
  return std::make_shared<dffunction_null_datetime_column>();
}

std::shared_ptr<dffunction>
null_datetime_column_as(const std::string& as) {
  return std::make_shared<dffunction_null_datetime_column>(as);
}

// ----- substr -----
std::shared_ptr<dfcolumn> dffunction_substr::execute(dftable_base& t) const {
  auto left_column = left->execute(t);
  return left_column->substr(right->execute(t));
}

std::shared_ptr<dfcolumn> dffunction_substr::execute(dftable_base& t1, 
                                                     dftable_base& t2) const {
  auto left_column = left->execute(t1);
  auto right_column = right->execute(t2);
  auto aligned_right_column = realign_df(t1, t2, right_column);
  return left_column->substr(aligned_right_column);
}

std::shared_ptr<dfcolumn> dffunction_substr::aggregate
(dftable_base& table,
 node_local<std::vector<size_t>>& local_grouped_idx,
 node_local<std::vector<size_t>>& local_idx_split,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map,
 node_local<size_t>& row_sizes,
 dftable& grouped_table) {
  auto left_column = left->aggregate(table, local_grouped_idx,
                                     local_idx_split, hash_divide,
                                     merge_map, row_sizes, grouped_table);
  auto right_column = right->aggregate(table, local_grouped_idx,
                                       local_idx_split, hash_divide,
                                       merge_map, row_sizes, grouped_table);
  return left_column->substr(right_column);
}

std::shared_ptr<dfcolumn>
dffunction_substr::whole_column_aggregate(dftable_base& t) {
  auto left_column = left->whole_column_aggregate(t);
  return left_column->substr(right->whole_column_aggregate(t));
}

std::shared_ptr<dffunction> substr_col(const std::string& left,
                                       const std::string& right) {
  return std::make_shared<dffunction_substr>(id_col(left), id_col(right));
}

std::shared_ptr<dffunction>
substr_col(const std::shared_ptr<dffunction>& left, const std::string& right){
  return std::make_shared<dffunction_substr>(left, id_col(right));
}

std::shared_ptr<dffunction>
substr_col(const std::string& left, const std::shared_ptr<dffunction>& right) {
  return std::make_shared<dffunction_substr>(id_col(left), right);
}
 
std::shared_ptr<dffunction>
substr_col(const std::shared_ptr<dffunction>& left,
           const std::shared_ptr<dffunction>& right) {
  return std::make_shared<dffunction_substr>(left, right);
} 

std::shared_ptr<dffunction> substr_col_as(const std::string& left,
                                          const std::string& right,
                                          const std::string& as) {
  return std::make_shared<dffunction_substr>(id_col(left), id_col(right), as);
}

std::shared_ptr<dffunction>
substr_col_as(const std::shared_ptr<dffunction>& left, const std::string& right,
              const std::string& as) {
  return std::make_shared<dffunction_substr>(left, id_col(right), as);
}

std::shared_ptr<dffunction>
substr_col_as(const std::string& left, const std::shared_ptr<dffunction>& right,
              const std::string& as) {
  return std::make_shared<dffunction_substr>(id_col(left), right, as);
}

std::shared_ptr<dffunction>
substr_col_as(const std::shared_ptr<dffunction>& left,
              const std::shared_ptr<dffunction>& right,
              const std::string& as) {
  return std::make_shared<dffunction_substr>(left, right, as);
}

std::shared_ptr<dffunction>
right_col(const std::string& left, const std::string& right) {
  auto right_col = id_col(right);
  auto left_col = id_col(left);
  return when({(right_col == 0 && is_not_null(left_col)) >> dic_string_im("")},
              std::make_shared<dffunction_substr>
              (left_col, sub_im(0,right_col)))
    ->as("right_col(" + left + "," + right + ")");
}

std::shared_ptr<dffunction>
right_col(const std::shared_ptr<dffunction>& left,
          const std::shared_ptr<dffunction>& right) {
  return when({(right == 0 && is_not_null(left)) >> dic_string_im("")},
              std::make_shared<dffunction_substr>
              (left, sub_im(0,right)))
    ->as("right_col(" + left->get_as() + "," + right->get_as() + ")");
} 

std::shared_ptr<dffunction>
right_col_as(const std::string& left, const std::string& right,
             const std::string& as) {
  auto right_col = id_col(right);
  auto left_col = id_col(left);
  return when({(right_col == 0 && is_not_null(left_col)) >> dic_string_im("")},
              std::make_shared<dffunction_substr>
              (left_col, sub_im(0,right_col)))->as(as);
}

std::shared_ptr<dffunction>
right_col_as(const std::shared_ptr<dffunction>& left,
             const std::shared_ptr<dffunction>& right,
             const std::string& as) {
  return when({(right == 0 && is_not_null(left)) >> dic_string_im("")},
              std::make_shared<dffunction_substr>(left, sub_im(0,right)))
    ->as(as);
}

std::shared_ptr<dfcolumn>
dffunction_substr_im::execute(dftable_base& t) const {
  auto left_column = left->execute(t);
  return left_column->substr(right);
}

std::shared_ptr<dfcolumn> dffunction_substr_im::aggregate
(dftable_base& table,
 node_local<std::vector<size_t>>& local_grouped_idx,
 node_local<std::vector<size_t>>& local_idx_split,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map,
 node_local<size_t>& row_sizes,
 dftable& grouped_table) {
  auto left_column = left->aggregate(table, local_grouped_idx,
                                     local_idx_split, hash_divide,
                                     merge_map, row_sizes, grouped_table);
  return left_column->substr(right);
}

std::shared_ptr<dfcolumn>
dffunction_substr_im::whole_column_aggregate(dftable_base& t) {
  auto left_column = left->whole_column_aggregate(t);
  return left_column->substr(right);
}

std::shared_ptr<dffunction>
substr_im(const std::string& left, int right) {
  return std::make_shared<dffunction_substr_im>(id_col(left), right);
}

std::shared_ptr<dffunction>
substr_im_as(const std::string& left, int right, const std::string& as) {
  return std::make_shared<dffunction_substr_im>(id_col(left), right, as);
}

std::shared_ptr<dffunction>
substr_im(const std::shared_ptr<dffunction>& left, int right) {
  return std::make_shared<dffunction_substr_im>(left, right);
}

std::shared_ptr<dffunction>
substr_im_as(const std::shared_ptr<dffunction>& left, int right,
             const std::string& as) {
  return std::make_shared<dffunction_substr_im>(left, right, as);
}

std::shared_ptr<dffunction>
right_im(const std::string& left, int right) {
  if(right == 0) {
    return when({is_not_null(id_col(left)) >> dic_string_im("")})
      ->as("right_im(" + left + "," + STR(right) + ")");
  } else {
    return std::make_shared<dffunction_substr_im>
      (id_col(left), -right, "right_im(" + left + "," + STR(right) + ")");
  }
}

std::shared_ptr<dffunction>
right_im_as(const std::string& left, int right, const std::string& as) {
  if(right == 0) {
    return when({is_not_null(id_col(left)) >> dic_string_im("")})->as(as);
  } else {
    return std::make_shared<dffunction_substr_im>(id_col(left), -right, as);
  }
}

std::shared_ptr<dffunction>
right_im(const std::shared_ptr<dffunction>& left, int right) {
  if(right == 0) {
    return when({is_not_null(left) >> dic_string_im("")})
      ->as("right_im(" + left->get_as() + "," + STR(right) + ")");
  } else {
    return std::make_shared<dffunction_substr_im>
      (left, -right, "right_im(" + left->get_as() + "," + STR(right) + ")");
  }
}

std::shared_ptr<dffunction>
right_im_as(const std::shared_ptr<dffunction>& left, int right,
            const std::string& as) {
  if(right == 0) {
    return when({is_not_null(left) >> dic_string_im("")})->as(as);
  } else {
    return std::make_shared<dffunction_substr_im>(left, -right, as);
  }
}

std::shared_ptr<dfcolumn>
dffunction_substr_num::execute(dftable_base& t) const {
  auto left_column = left->execute(t);
  auto right_column = right->execute(t);
  auto num_column = num->execute(t);
  return left_column->substr(right_column, num_column);
}

std::shared_ptr<dfcolumn> dffunction_substr_num::aggregate
(dftable_base& table,
 node_local<std::vector<size_t>>& local_grouped_idx,
 node_local<std::vector<size_t>>& local_idx_split,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map,
 node_local<size_t>& row_sizes,
 dftable& grouped_table) {
  auto left_column = left->aggregate(table, local_grouped_idx,
                                     local_idx_split, hash_divide,
                                     merge_map, row_sizes, grouped_table);
  auto right_column = right->aggregate(table, local_grouped_idx,
                                       local_idx_split, hash_divide,
                                       merge_map, row_sizes, grouped_table);
  auto num_column = num->aggregate(table, local_grouped_idx,
                                   local_idx_split, hash_divide,
                                   merge_map, row_sizes, grouped_table);
  return left_column->substr(right_column, num_column);
}

std::shared_ptr<dfcolumn>
dffunction_substr_num::whole_column_aggregate(dftable_base& t) {
  auto left_column = left->whole_column_aggregate(t);
  auto right_column = right->whole_column_aggregate(t);
  auto num_column = num->whole_column_aggregate(t);
  return left_column->substr(right_column, num_column);
}

std::shared_ptr<dffunction>
substr_poscol_numcol(const std::string& left,
                     const std::string& right,
                     const std::string& num) {
  return std::make_shared<dffunction_substr_num>
    (id_col(left), id_col(right), id_col(num));
} 
std::shared_ptr<dffunction>
substr_poscol_numcol(const std::shared_ptr<dffunction>& left,
                     const std::shared_ptr<dffunction>& right,
                     const std::shared_ptr<dffunction>& num) {
  return std::make_shared<dffunction_substr_num>(left, right, num);
} 

std::shared_ptr<dffunction>
substr_poscol_numcol_as(const std::string& left,
                        const std::string& right,
                        const std::string& num,
                        const std::string& as) {
  return std::make_shared<dffunction_substr_num>
    (id_col(left), id_col(right), id_col(num), as);
}

std::shared_ptr<dffunction>
substr_poscol_numcol_as(const std::shared_ptr<dffunction>& left,
                        const std::shared_ptr<dffunction>& right,
                        const std::shared_ptr<dffunction>& num,
                        const std::string& as) {
  return std::make_shared<dffunction_substr_num>(left, right, num, as);
}

std::shared_ptr<dfcolumn>
dffunction_substr_posim_num::execute(dftable_base& t) const {
  auto left_column = left->execute(t);
  auto num_column = num->execute(t);
  return left_column->substr(right, num_column);
}

std::shared_ptr<dfcolumn>
dffunction_substr_posim_num::execute(dftable_base& t1,
                                     dftable_base& t2) const {
  auto left_column = left->execute(t1);
  auto num_column = num->execute(t2);
  return left_column->substr(right, num_column);
}

std::shared_ptr<dfcolumn> dffunction_substr_posim_num::aggregate
(dftable_base& table,
 node_local<std::vector<size_t>>& local_grouped_idx,
 node_local<std::vector<size_t>>& local_idx_split,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map,
 node_local<size_t>& row_sizes,
 dftable& grouped_table) {
  auto left_column = left->aggregate(table, local_grouped_idx,
                                     local_idx_split, hash_divide,
                                     merge_map, row_sizes, grouped_table);
  auto num_column = num->aggregate(table, local_grouped_idx,
                                   local_idx_split, hash_divide,
                                   merge_map, row_sizes, grouped_table);
  return left_column->substr(right, num_column);
}

std::shared_ptr<dfcolumn>
dffunction_substr_posim_num::whole_column_aggregate(dftable_base& t) {
  auto left_column = left->whole_column_aggregate(t);
  auto num_column = num->whole_column_aggregate(t);
  return left_column->substr(right, num_column);
}

std::shared_ptr<dffunction>
substr_posim_numcol(const std::string& left, int right,
                    const std::string& num) {
  return std::make_shared<dffunction_substr_posim_num>
    (id_col(left), right, id_col(num));
}

std::shared_ptr<dffunction>
substr_posim_numcol_as(const std::string& left, int right,
                       const std::string& num, const std::string& as) {
  return std::make_shared<dffunction_substr_posim_num>
    (id_col(left), right, id_col(num), as);
}

std::shared_ptr<dffunction>
substr_posim_numcol(const std::shared_ptr<dffunction>& left, int right,
                    const std::shared_ptr<dffunction>& num) {
  return std::make_shared<dffunction_substr_posim_num>(left, right, num);
}

std::shared_ptr<dffunction>
substr_posim_numcol_as(const std::shared_ptr<dffunction>& left, int right,
                       const std::shared_ptr<dffunction>& num,
                       const std::string& as) {
  return std::make_shared<dffunction_substr_posim_num>(left, right, num, as);
}

std::shared_ptr<dffunction>
left_col(const std::string& left, const std::string& num) {
  return std::make_shared<dffunction_substr_posim_num>
    (id_col(left), 0, id_col(num), "left_col(" + left + "," + num + ")");
}

std::shared_ptr<dffunction>
left_col_as(const std::string& left,
            const std::string& num, const std::string& as) {
  return std::make_shared<dffunction_substr_posim_num>
    (id_col(left), 0, id_col(num), as);
}

std::shared_ptr<dffunction>
left_col(const std::shared_ptr<dffunction>& left,
         const std::shared_ptr<dffunction>& num) {
  return std::make_shared<dffunction_substr_posim_num>
    (left, 0, num, "left_col(" + left->get_as() + "," + num->get_as() + ")");
}

std::shared_ptr<dffunction>
left_col_as(const std::shared_ptr<dffunction>& left,
            const std::shared_ptr<dffunction>& num,
            const std::string& as) {
  return std::make_shared<dffunction_substr_posim_num>(left, 0, num, as);
}

std::shared_ptr<dfcolumn>
dffunction_substr_numim::execute(dftable_base& t) const {
  auto left_column = left->execute(t);
  auto right_column = right->execute(t);
  return left_column->substr(right_column, num);
}

std::shared_ptr<dfcolumn>
dffunction_substr_numim::execute(dftable_base& t1,
                                 dftable_base& t2) const {
  auto left_column = left->execute(t1);
  auto right_column = right->execute(t2);
  return left_column->substr(right_column, num);
}

std::shared_ptr<dfcolumn> dffunction_substr_numim::aggregate
(dftable_base& table,
 node_local<std::vector<size_t>>& local_grouped_idx,
 node_local<std::vector<size_t>>& local_idx_split,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map,
 node_local<size_t>& row_sizes,
 dftable& grouped_table) {
  auto left_column = left->aggregate(table, local_grouped_idx,
                                     local_idx_split, hash_divide,
                                     merge_map, row_sizes, grouped_table);
  auto right_column = right->aggregate(table, local_grouped_idx,
                                       local_idx_split, hash_divide,
                                       merge_map, row_sizes, grouped_table);
  return left_column->substr(right_column, num);
}

std::shared_ptr<dfcolumn>
dffunction_substr_numim::whole_column_aggregate(dftable_base& t) {
  auto left_column = left->whole_column_aggregate(t);
  auto right_column = right->whole_column_aggregate(t);
  return left_column->substr(right_column, num);
}

std::shared_ptr<dffunction>
substr_poscol_numim(const std::string& left, const std::string& right,
                    int num) {
  return std::make_shared<dffunction_substr_numim>
    (id_col(left), id_col(right), num);
}

std::shared_ptr<dffunction>
substr_poscol_numim_as(const std::string& left, const std::string& right,
                       int num, const std::string& as) {
  return std::make_shared<dffunction_substr_numim>
    (id_col(left), id_col(right), num, as);
}

std::shared_ptr<dffunction>
substr_poscol_numim(const std::shared_ptr<dffunction>& left, 
                    const std::shared_ptr<dffunction>& right,
                    int num) {
  return std::make_shared<dffunction_substr_numim>(left, right, num);
}

std::shared_ptr<dffunction>
substr_poscol_numim_as(const std::shared_ptr<dffunction>& left, 
                       const std::shared_ptr<dffunction>& right,
                       int num,
                       const std::string& as) {
  return std::make_shared<dffunction_substr_numim>(left, right, num, as);
}

std::shared_ptr<dfcolumn>
dffunction_substr_posim_numim::execute(dftable_base& t) const {
  auto left_column = left->execute(t);
  return left_column->substr(right, num);
}

std::shared_ptr<dfcolumn> dffunction_substr_posim_numim::aggregate
(dftable_base& table,
 node_local<std::vector<size_t>>& local_grouped_idx,
 node_local<std::vector<size_t>>& local_idx_split,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map,
 node_local<size_t>& row_sizes,
 dftable& grouped_table) {
  auto left_column = left->aggregate(table, local_grouped_idx,
                                     local_idx_split, hash_divide,
                                     merge_map, row_sizes, grouped_table);
  return left_column->substr(right, num);
}

std::shared_ptr<dfcolumn>
dffunction_substr_posim_numim::whole_column_aggregate(dftable_base& t) {
  auto left_column = left->whole_column_aggregate(t);
  return left_column->substr(right, num);
}

std::shared_ptr<dffunction>
substr_posim_numim(const std::string& left, int right, int num) {
  return std::make_shared<dffunction_substr_posim_numim>
    (id_col(left), right, num);
}

std::shared_ptr<dffunction>
substr_posim_numim_as(const std::string& left, int right,
                      int num, const std::string& as) {
  return std::make_shared<dffunction_substr_posim_numim>
    (id_col(left), right, num, as);
}

std::shared_ptr<dffunction>
substr_posim_numim(const std::shared_ptr<dffunction>& left,
                   int right, int num) {
  return std::make_shared<dffunction_substr_posim_numim>(left, right, num);
}

std::shared_ptr<dffunction>
substr_posim_numim_as(const std::shared_ptr<dffunction>& left,
                      int right, int num, const std::string& as) {
  return std::make_shared<dffunction_substr_posim_numim>(left, right, num, as);
}

std::shared_ptr<dffunction>
left_im(const std::string& left, int num) {
  return std::make_shared<dffunction_substr_posim_numim>
    (id_col(left), 0, num, "left_im(" + left + "," + STR(num) + ")");
}

std::shared_ptr<dffunction>
left_im_as(const std::string& left, int num, const std::string& as) {
  return std::make_shared<dffunction_substr_posim_numim>
    (id_col(left), 0, num, as);
}

std::shared_ptr<dffunction>
left_im(const std::shared_ptr<dffunction>& left, int num) {
  return std::make_shared<dffunction_substr_posim_numim>
    (left, 0, num, "left_im(" + left->get_as() + "," + STR(num) + ")");
}

std::shared_ptr<dffunction>
left_im_as(const std::shared_ptr<dffunction>& left, int num,
           const std::string& as) {
  return std::make_shared<dffunction_substr_posim_numim>(left, 0, num, as);
}


// ----- make_date -----
std::shared_ptr<dfcolumn>
dffunction_make_date_helper(std::shared_ptr<typed_dfcolumn<int>>& year,
                            std::shared_ptr<typed_dfcolumn<int>>& month,
                            std::shared_ptr<typed_dfcolumn<int>>& day) {
  auto val = year->val.map
    (+[](std::vector<int>& year,
         std::vector<int>& month,
         std::vector<int>& day) {
      auto size = year.size();
      std::vector<datetime_t> ret(size);
      auto yearp = year.data();
      auto monthp = month.data();
      auto dayp = day.data();
      auto retp = ret.data();
      for(size_t i = 0; i < size; i++) {
        retp[i] = makedatetime(yearp[i], monthp[i], dayp[i]);
      }
      return ret;
    }, month->val, day->val);
  auto nulls = year->nulls.map
    (+[](std::vector<size_t>& year_nulls,
         std::vector<size_t>& month_nulls,
         std::vector<size_t>& day_nulls) {
      return set_union(set_union(year_nulls, month_nulls), day_nulls);
    }, month->nulls, day->nulls);
  val.mapv(+[](std::vector<datetime_t>& val, std::vector<size_t>& nulls) {
      auto valp = val.data();
      auto nullsp = nulls.data();
      auto nulls_size = nulls.size();
      auto max = std::numeric_limits<datetime_t>::max();
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
      for(size_t i = 0; i < nulls_size; i++) {
        valp[nullsp[i]] = max;
      }
    }, nulls);
  return std::make_shared<typed_dfcolumn<datetime>>(val, nulls);
}

std::shared_ptr<dfcolumn> dffunction_make_date::execute(dftable_base& t) const {
  auto year_column = std::dynamic_pointer_cast<typed_dfcolumn<int>>
    (year->execute(t)->type_cast("int"));
  auto month_column = std::dynamic_pointer_cast<typed_dfcolumn<int>>
    (month->execute(t)->type_cast("int"));
  auto day_column = std::dynamic_pointer_cast<typed_dfcolumn<int>>
    (day->execute(t)->type_cast("int"));
  return dffunction_make_date_helper(year_column, month_column, day_column);
}

std::shared_ptr<dfcolumn> dffunction_make_date::aggregate
(dftable_base& table,
 node_local<std::vector<size_t>>& local_grouped_idx,
 node_local<std::vector<size_t>>& local_idx_split,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map,
 node_local<size_t>& row_sizes,
 dftable& grouped_table) {
  auto year_column = std::dynamic_pointer_cast<typed_dfcolumn<int>>
    (year->aggregate(table, local_grouped_idx,
                     local_idx_split, hash_divide,
                     merge_map, row_sizes, grouped_table)
     ->type_cast("int"));
  auto month_column = std::dynamic_pointer_cast<typed_dfcolumn<int>>
    (month->aggregate(table, local_grouped_idx,
                      local_idx_split, hash_divide,
                      merge_map, row_sizes, grouped_table)
     ->type_cast("int"));
  auto day_column = std::dynamic_pointer_cast<typed_dfcolumn<int>>
    (day->aggregate(table, local_grouped_idx,
                    local_idx_split, hash_divide,
                    merge_map, row_sizes, grouped_table)
     ->type_cast("int"));
  return dffunction_make_date_helper(year_column, month_column, day_column);
}

std::shared_ptr<dfcolumn>
dffunction_make_date::whole_column_aggregate(dftable_base& t) {
  auto year_column = std::dynamic_pointer_cast<typed_dfcolumn<int>>
    (year->whole_column_aggregate(t)->type_cast("int"));
  auto month_column = std::dynamic_pointer_cast<typed_dfcolumn<int>>
    (month->whole_column_aggregate(t)->type_cast("int"));
  auto day_column = std::dynamic_pointer_cast<typed_dfcolumn<int>>
    (day->whole_column_aggregate(t)->type_cast("int"));
  return dffunction_make_date_helper(year_column, month_column, day_column);
}

std::shared_ptr<dffunction> make_date_col(const std::string& year,
                                          const std::string& month,
                                          const std::string& day) {
  return std::make_shared<dffunction_make_date>
    (id_col(year), id_col(month), id_col(day));
}

std::shared_ptr<dffunction>
make_date_col(const std::shared_ptr<dffunction>& year,
              const std::shared_ptr<dffunction>& month,
              const std::shared_ptr<dffunction>& day) {
  return std::make_shared<dffunction_make_date>(year, month, day);
} 

std::shared_ptr<dffunction> make_date_col_as(const std::string& year,
                                             const std::string& month,
                                             const std::string& day,
                                             const std::string& as) {
  return std::make_shared<dffunction_make_date>
    (id_col(year), id_col(month), id_col(day), as);
}

std::shared_ptr<dffunction>
make_date_col_as(const std::shared_ptr<dffunction>& year,
                 const std::shared_ptr<dffunction>& month,
                 const std::shared_ptr<dffunction>& day,
                 const std::string& as) {
  return std::make_shared<dffunction_make_date>(year, month, day, as);
} 


// ----- make_datetime -----
std::shared_ptr<dfcolumn>
dffunction_make_datetime_helper(std::shared_ptr<typed_dfcolumn<int>>& year,
                                std::shared_ptr<typed_dfcolumn<int>>& month,
                                std::shared_ptr<typed_dfcolumn<int>>& day,
                                std::shared_ptr<typed_dfcolumn<int>>& hour,
                                std::shared_ptr<typed_dfcolumn<int>>& minute,
                                std::shared_ptr<typed_dfcolumn<int>>& second) {
  auto val = year->val.map
    (+[](std::vector<int>& year,
         std::vector<int>& month,
         std::vector<int>& day,
         std::vector<int>& hour,
         std::vector<int>& minute,
         std::vector<int>& second) {
      auto size = year.size();
      std::vector<datetime_t> ret(size);
      auto yearp = year.data();
      auto monthp = month.data();
      auto dayp = day.data();
      auto hourp = hour.data();
      auto minutep = minute.data();
      auto secondp = second.data();
      auto retp = ret.data();
      for(size_t i = 0; i < size; i++) {
        retp[i] = makedatetime(yearp[i], monthp[i], dayp[i],
                               hourp[i], minutep[i], secondp[i]);
      }
      return ret;
    }, month->val, day->val, hour->val, minute->val, second->val);
  auto nulls = year->nulls.map
    (+[](std::vector<size_t>& year_nulls,
         std::vector<size_t>& month_nulls,
         std::vector<size_t>& day_nulls,
         std::vector<size_t>& hour_nulls,
         std::vector<size_t>& minute_nulls,
         std::vector<size_t>& second_nulls) {
      return set_union(set_union(set_union(year_nulls, month_nulls),
                                 set_union(day_nulls, hour_nulls)),
                       set_union(minute_nulls, second_nulls));
    }, month->nulls, day->nulls, hour->nulls, minute->nulls, second->nulls);
  val.mapv(+[](std::vector<datetime_t>& val, std::vector<size_t>& nulls) {
      auto valp = val.data();
      auto nullsp = nulls.data();
      auto nulls_size = nulls.size();
      auto max = std::numeric_limits<datetime_t>::max();
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
      for(size_t i = 0; i < nulls_size; i++) {
        valp[nullsp[i]] = max;
      }
    }, nulls);
  return std::make_shared<typed_dfcolumn<datetime>>(val, nulls);
}

std::shared_ptr<dfcolumn>
dffunction_make_datetime::execute(dftable_base& t) const {
  auto year_column = std::dynamic_pointer_cast<typed_dfcolumn<int>>
    (year->execute(t)->type_cast("int"));
  auto month_column = std::dynamic_pointer_cast<typed_dfcolumn<int>>
    (month->execute(t)->type_cast("int"));
  auto day_column = std::dynamic_pointer_cast<typed_dfcolumn<int>>
    (day->execute(t)->type_cast("int"));
  auto hour_column = std::dynamic_pointer_cast<typed_dfcolumn<int>>
    (hour->execute(t)->type_cast("int"));
  auto minute_column = std::dynamic_pointer_cast<typed_dfcolumn<int>>
    (minute->execute(t)->type_cast("int"));
  auto second_column = std::dynamic_pointer_cast<typed_dfcolumn<int>>
    (second->execute(t)->type_cast("int"));
  return dffunction_make_datetime_helper
    (year_column, month_column, day_column,
     hour_column, minute_column, second_column);
}

std::shared_ptr<dfcolumn> dffunction_make_datetime::aggregate
(dftable_base& table,
 node_local<std::vector<size_t>>& local_grouped_idx,
 node_local<std::vector<size_t>>& local_idx_split,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map,
 node_local<size_t>& row_sizes,
 dftable& grouped_table) {
  auto year_column = std::dynamic_pointer_cast<typed_dfcolumn<int>>
    (year->aggregate(table, local_grouped_idx,
                     local_idx_split, hash_divide,
                     merge_map, row_sizes, grouped_table)
     ->type_cast("int"));
  auto month_column = std::dynamic_pointer_cast<typed_dfcolumn<int>>
    (month->aggregate(table, local_grouped_idx,
                      local_idx_split, hash_divide,
                      merge_map, row_sizes, grouped_table)
     ->type_cast("int"));
  auto day_column = std::dynamic_pointer_cast<typed_dfcolumn<int>>
    (day->aggregate(table, local_grouped_idx,
                    local_idx_split, hash_divide,
                    merge_map, row_sizes, grouped_table)
     ->type_cast("int"));
  auto hour_column = std::dynamic_pointer_cast<typed_dfcolumn<int>>
    (hour->aggregate(table, local_grouped_idx,
                     local_idx_split, hash_divide,
                     merge_map, row_sizes, grouped_table)
     ->type_cast("int"));
  auto minute_column = std::dynamic_pointer_cast<typed_dfcolumn<int>>
    (minute->aggregate(table, local_grouped_idx,
                       local_idx_split, hash_divide,
                       merge_map, row_sizes, grouped_table)
     ->type_cast("int"));
  auto second_column = std::dynamic_pointer_cast<typed_dfcolumn<int>>
    (second->aggregate(table, local_grouped_idx,
                       local_idx_split, hash_divide,
                       merge_map, row_sizes, grouped_table)
     ->type_cast("int"));
  return dffunction_make_datetime_helper
    (year_column, month_column, day_column,
     hour_column, minute_column, second_column);
}

std::shared_ptr<dfcolumn>
dffunction_make_datetime::whole_column_aggregate(dftable_base& t) {
  auto year_column = std::dynamic_pointer_cast<typed_dfcolumn<int>>
    (year->whole_column_aggregate(t)->type_cast("int"));
  auto month_column = std::dynamic_pointer_cast<typed_dfcolumn<int>>
    (month->whole_column_aggregate(t)->type_cast("int"));
  auto day_column = std::dynamic_pointer_cast<typed_dfcolumn<int>>
    (day->whole_column_aggregate(t)->type_cast("int"));
  auto hour_column = std::dynamic_pointer_cast<typed_dfcolumn<int>>
    (hour->whole_column_aggregate(t)->type_cast("int"));
  auto minute_column = std::dynamic_pointer_cast<typed_dfcolumn<int>>
    (minute->whole_column_aggregate(t)->type_cast("int"));
  auto second_column = std::dynamic_pointer_cast<typed_dfcolumn<int>>
    (second->whole_column_aggregate(t)->type_cast("int"));
  return dffunction_make_datetime_helper
    (year_column, month_column, day_column,
     hour_column, minute_column, second_column);
}

std::shared_ptr<dffunction> make_datetime_col(const std::string& year,
                                              const std::string& month,
                                              const std::string& day,
                                              const std::string& hour,
                                              const std::string& minute,
                                              const std::string& second) {
  return std::make_shared<dffunction_make_datetime>
    (id_col(year), id_col(month), id_col(day),
     id_col(hour), id_col(minute), id_col(second));
}

std::shared_ptr<dffunction>
make_datetime_col(const std::shared_ptr<dffunction>& year,
                  const std::shared_ptr<dffunction>& month,
                  const std::shared_ptr<dffunction>& day,
                  const std::shared_ptr<dffunction>& hour,
                  const std::shared_ptr<dffunction>& minute,
                  const std::shared_ptr<dffunction>& second) {
  return std::make_shared<dffunction_make_datetime>(year, month, day,
                                                    hour, minute, second);
} 

std::shared_ptr<dffunction> make_datetime_col_as(const std::string& year,
                                                 const std::string& month,
                                                 const std::string& day,
                                                 const std::string& hour,
                                                 const std::string& minute,
                                                 const std::string& second,
                                                 const std::string& as) {
  return std::make_shared<dffunction_make_datetime>
    (id_col(year), id_col(month), id_col(day),
     id_col(hour), id_col(minute), id_col(second), as);
}

std::shared_ptr<dffunction>
make_datetime_col_as(const std::shared_ptr<dffunction>& year,
                     const std::shared_ptr<dffunction>& month,
                     const std::shared_ptr<dffunction>& day,
                     const std::shared_ptr<dffunction>& hour,
                     const std::shared_ptr<dffunction>& minute,
                     const std::shared_ptr<dffunction>& second,
                     const std::string& as) {
  return std::make_shared<dffunction_make_datetime>(year, month, day, 
                                                    hour, minute, second, as);
}


// ----- length -----
std::shared_ptr<dfcolumn>
dffunction_length::execute(dftable_base& t) const {
  auto left_column = left->execute(t);
  return left_column->length();
}

std::shared_ptr<dfcolumn> dffunction_length::aggregate
(dftable_base& table,
 node_local<std::vector<size_t>>& local_grouped_idx,
 node_local<std::vector<size_t>>& local_idx_split,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map,
 node_local<size_t>& row_sizes,
 dftable& grouped_table) {
  auto left_column = left->aggregate(table, local_grouped_idx,
                                     local_idx_split, hash_divide,
                                     merge_map, row_sizes, grouped_table);
  return left_column->length();
}

std::shared_ptr<dfcolumn>
dffunction_length::whole_column_aggregate(dftable_base& t) {
  auto left_column = left->whole_column_aggregate(t);
  return left_column->length();
}

std::shared_ptr<dffunction>
length_col(const std::string& left) {
  return std::make_shared<dffunction_length>(id_col(left));
}

std::shared_ptr<dffunction>
length_col(const std::shared_ptr<dffunction>& left) {
  return std::make_shared<dffunction_length>(left);
}

std::shared_ptr<dffunction>
length_col_as(const std::string& left, const std::string& as) {
  return std::make_shared<dffunction_length>(id_col(left), as);
}

std::shared_ptr<dffunction>
length_col_as(const std::shared_ptr<dffunction>& left,
              const std::string& as) {
  return std::make_shared<dffunction_length>(left, as);
}


// ----- char_length -----
std::shared_ptr<dfcolumn>
dffunction_char_length::execute(dftable_base& t) const {
  auto left_column = left->execute(t);
  return left_column->char_length();
}

std::shared_ptr<dfcolumn> dffunction_char_length::aggregate
(dftable_base& table,
 node_local<std::vector<size_t>>& local_grouped_idx,
 node_local<std::vector<size_t>>& local_idx_split,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map,
 node_local<size_t>& row_sizes,
 dftable& grouped_table) {
  auto left_column = left->aggregate(table, local_grouped_idx,
                                     local_idx_split, hash_divide,
                                     merge_map, row_sizes, grouped_table);
  return left_column->char_length();
}

std::shared_ptr<dfcolumn>
dffunction_char_length::whole_column_aggregate(dftable_base& t) {
  auto left_column = left->whole_column_aggregate(t);
  return left_column->char_length();
}

std::shared_ptr<dffunction>
char_length_col(const std::string& left) {
  return std::make_shared<dffunction_char_length>(id_col(left));
}

std::shared_ptr<dffunction>
char_length_col(const std::shared_ptr<dffunction>& left) {
  return std::make_shared<dffunction_char_length>(left);
}

std::shared_ptr<dffunction>
char_length_col_as(const std::string& left, const std::string& as) {
  return std::make_shared<dffunction_char_length>(id_col(left), as);
}

std::shared_ptr<dffunction>
char_length_col_as(const std::shared_ptr<dffunction>& left,
              const std::string& as) {
  return std::make_shared<dffunction_char_length>(left, as);
}


// ----- locate -----
std::shared_ptr<dfcolumn>
dffunction_locate::execute(dftable_base& t) const {
  auto left_column = left->execute(t);
  return left_column->locate(str, pos);
}

std::shared_ptr<dfcolumn> dffunction_locate::aggregate
(dftable_base& table,
 node_local<std::vector<size_t>>& local_grouped_idx,
 node_local<std::vector<size_t>>& local_idx_split,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map,
 node_local<size_t>& row_sizes,
 dftable& grouped_table) {
  auto left_column = left->aggregate(table, local_grouped_idx,
                                     local_idx_split, hash_divide,
                                     merge_map, row_sizes, grouped_table);
  return left_column->locate(str, pos);
}

std::shared_ptr<dfcolumn>
dffunction_locate::whole_column_aggregate(dftable_base& t) {
  auto left_column = left->whole_column_aggregate(t);
  return left_column->locate(str, pos);
}

std::shared_ptr<dffunction>
locate_im(const std::string& left, const std::string& str) {
  return std::make_shared<dffunction_locate>(id_col(left), str, 1);
}

std::shared_ptr<dffunction>
locate_im(const std::shared_ptr<dffunction>& left, const std::string& str) {
  return std::make_shared<dffunction_locate>(left, str, 1);
}

std::shared_ptr<dffunction>
locate_im(const std::string& left, const std::string& str, int pos) {
  return std::make_shared<dffunction_locate>(id_col(left), str, pos);
}

std::shared_ptr<dffunction>
locate_im(const std::shared_ptr<dffunction>& left, const std::string& str,
          int pos) {
  return std::make_shared<dffunction_locate>(left, str, pos);
}

std::shared_ptr<dffunction>
locate_im_as(const std::string& left, const std::string& str,
             const std::string& as) {
  return std::make_shared<dffunction_locate>(id_col(left), str, 1, as);
}

std::shared_ptr<dffunction>
locate_im_as(const std::shared_ptr<dffunction>& left, const std::string& str,
             const std::string& as) {
  return std::make_shared<dffunction_locate>(left, str, 1, as);
}

std::shared_ptr<dffunction>
locate_im_as(const std::string& left, const std::string& str,
             int pos, const std::string& as) {
  return std::make_shared<dffunction_locate>(id_col(left), str, pos, as);
}

std::shared_ptr<dffunction>
locate_im_as(const std::shared_ptr<dffunction>& left, const std::string& str,
             int pos, const std::string& as) {
  return std::make_shared<dffunction_locate>(left, str, pos, as);
}


// ----- trim -----
std::shared_ptr<dfcolumn>
dffunction_trim::execute(dftable_base& t) const {
  auto left_column = left->execute(t);
  return left_column->trim(kind, str);
}

std::shared_ptr<dfcolumn> dffunction_trim::aggregate
(dftable_base& table,
 node_local<std::vector<size_t>>& local_grouped_idx,
 node_local<std::vector<size_t>>& local_idx_split,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map,
 node_local<size_t>& row_sizes,
 dftable& grouped_table) {
  auto left_column = left->aggregate(table, local_grouped_idx,
                                     local_idx_split, hash_divide,
                                     merge_map, row_sizes, grouped_table);
  return left_column->trim(kind, str);
}

std::shared_ptr<dfcolumn>
dffunction_trim::whole_column_aggregate(dftable_base& t) {
  auto left_column = left->whole_column_aggregate(t);
  return left_column->trim(kind, str);
}

std::shared_ptr<dffunction>
ltrim_im(const std::string& left) {
  return std::make_shared<dffunction_trim>
    (id_col(left), std::string(" "), trim_type::leading);
}

std::shared_ptr<dffunction>
ltrim_im(const std::string& left, const std::string& str) {
  return std::make_shared<dffunction_trim>
    (id_col(left), str, trim_type::leading);
}

std::shared_ptr<dffunction>
rtrim_im(const std::string& left) {
  return std::make_shared<dffunction_trim>
    (id_col(left), std::string(" "), trim_type::trailing);
}

std::shared_ptr<dffunction>
rtrim_im(const std::string& left, const std::string& str) {
  return std::make_shared<dffunction_trim>
    (id_col(left), str, trim_type::trailing);
}

std::shared_ptr<dffunction>
trim_im(const std::string& left) {
  return std::make_shared<dffunction_trim>
    (id_col(left), std::string(" "), trim_type::both);
}

std::shared_ptr<dffunction>
trim_im(const std::string& left, const std::string& str) {
  return std::make_shared<dffunction_trim>
    (id_col(left), str, trim_type::both);
}

std::shared_ptr<dffunction>
trim_im(const std::string& left, const std::string& str, trim_type kind) {
  return std::make_shared<dffunction_trim>
    (id_col(left), str, kind);
}

std::shared_ptr<dffunction>
ltrim_im(const std::shared_ptr<dffunction>& left) {
  return std::make_shared<dffunction_trim>
    (left, std::string(" "), trim_type::leading);
}

std::shared_ptr<dffunction>
ltrim_im(const std::shared_ptr<dffunction>& left, const std::string& str) {
  return std::make_shared<dffunction_trim>
    (left, str, trim_type::leading);
}

std::shared_ptr<dffunction>
rtrim_im(const std::shared_ptr<dffunction>& left) {
  return std::make_shared<dffunction_trim>
    (left, std::string(" "), trim_type::trailing);
}

std::shared_ptr<dffunction>
rtrim_im(const std::shared_ptr<dffunction>& left, const std::string& str) {
  return std::make_shared<dffunction_trim>
    (left, str, trim_type::trailing);
}

std::shared_ptr<dffunction>
trim_im(const std::shared_ptr<dffunction>& left) {
  return std::make_shared<dffunction_trim>
    (left, std::string(" "), trim_type::both);
}

std::shared_ptr<dffunction>
trim_im(const std::shared_ptr<dffunction>& left, const std::string& str) {
  return std::make_shared<dffunction_trim>
    (left, str, trim_type::both);
}

std::shared_ptr<dffunction>
trim_im(const std::shared_ptr<dffunction>& left,
        const std::string& str, trim_type kind) {
  return std::make_shared<dffunction_trim>
    (left, str, kind);
}

std::shared_ptr<dffunction>
ltrim_im_as(const std::string& left, const std::string& as) {
  return std::make_shared<dffunction_trim>
    (id_col(left), std::string(" "), trim_type::leading, as);
}

std::shared_ptr<dffunction>
ltrim_im_as(const std::string& left, const std::string& str,
            const std::string& as) {
  return std::make_shared<dffunction_trim>
    (id_col(left), str, trim_type::leading, as);
}

std::shared_ptr<dffunction>
rtrim_im_as(const std::string& left, const std::string& as) {
  return std::make_shared<dffunction_trim>
    (id_col(left), std::string(" "), trim_type::trailing, as);
}

std::shared_ptr<dffunction>
rtrim_im_as(const std::string& left, const std::string& str,
            const std::string& as) {
  return std::make_shared<dffunction_trim>
    (id_col(left), str, trim_type::trailing, as);
}

std::shared_ptr<dffunction>
trim_im_as(const std::string& left, const std::string& as) {
  return std::make_shared<dffunction_trim>
    (id_col(left), std::string(" "), trim_type::both, as);
}

std::shared_ptr<dffunction>
trim_im_as(const std::string& left, const std::string& str,
           const std::string& as) {
  return std::make_shared<dffunction_trim>
    (id_col(left), str, trim_type::both, as);
}

std::shared_ptr<dffunction>
trim_im_as(const std::string& left, const std::string& str, trim_type kind,
           const std::string& as) {
  return std::make_shared<dffunction_trim>
    (id_col(left), str, kind, as);
}

std::shared_ptr<dffunction>
ltrim_im_as(const std::shared_ptr<dffunction>& left,
            const std::string& as) {
  return std::make_shared<dffunction_trim>
    (left, std::string(" "), trim_type::leading, as);
}

std::shared_ptr<dffunction>
ltrim_im_as(const std::shared_ptr<dffunction>& left, const std::string& str,
            const std::string& as) {
  return std::make_shared<dffunction_trim>
    (left, str, trim_type::leading, as);
}

std::shared_ptr<dffunction>
rtrim_im_as(const std::shared_ptr<dffunction>& left,
            const std::string& as) {
  return std::make_shared<dffunction_trim>
    (left, std::string(" "), trim_type::trailing, as);
}

std::shared_ptr<dffunction>
rtrim_im_as(const std::shared_ptr<dffunction>& left, const std::string& str,
            const std::string& as) {
  return std::make_shared<dffunction_trim>
    (left, str, trim_type::trailing, as);
}

std::shared_ptr<dffunction>
trim_im_as(const std::shared_ptr<dffunction>& left,
           const std::string& as) {
  return std::make_shared<dffunction_trim>
    (left, std::string(" "), trim_type::both, as);
}

std::shared_ptr<dffunction>
trim_im_as(const std::shared_ptr<dffunction>& left, const std::string& str,
           const std::string& as) {
  return std::make_shared<dffunction_trim>
    (left, str, trim_type::both, as);
}

std::shared_ptr<dffunction>
trim_im_as(const std::shared_ptr<dffunction>& left,
           const std::string& str, trim_type kind,
           const std::string& as) {
  return std::make_shared<dffunction_trim>
    (left, str, kind, as);
}


// ----- replace -----
std::shared_ptr<dfcolumn>
dffunction_replace::execute(dftable_base& t) const {
  auto left_column = left->execute(t);
  return left_column->replace(from, to);
}

std::shared_ptr<dfcolumn> dffunction_replace::aggregate
(dftable_base& table,
 node_local<std::vector<size_t>>& local_grouped_idx,
 node_local<std::vector<size_t>>& local_idx_split,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map,
 node_local<size_t>& row_sizes,
 dftable& grouped_table) {
  auto left_column = left->aggregate(table, local_grouped_idx,
                                     local_idx_split, hash_divide,
                                     merge_map, row_sizes, grouped_table);
  return left_column->replace(from, to);
}

std::shared_ptr<dfcolumn>
dffunction_replace::whole_column_aggregate(dftable_base& t) {
  auto left_column = left->whole_column_aggregate(t);
  return left_column->replace(from, to);
}

std::shared_ptr<dffunction>
replace_im(const std::string& left, const std::string& from,
           const std::string& to) {
  return std::make_shared<dffunction_replace>(id_col(left), from, to);
}

std::shared_ptr<dffunction>
replace_im(const std::shared_ptr<dffunction>& left, const std::string& from,
           const std::string& to) {
  return std::make_shared<dffunction_replace>(left, from, to);
}

std::shared_ptr<dffunction>
replace_im_as(const std::string& left, const std::string& from,
              const std::string& to, const std::string& as) {
  return std::make_shared<dffunction_replace>(id_col(left), from, to, as);
}

std::shared_ptr<dffunction>
replace_im_as(const std::shared_ptr<dffunction>& left, const std::string& from,
              const std::string& to, const std::string& as) {
  return std::make_shared<dffunction_replace>(left, from, to, as);
}


// ----- reverse -----
std::shared_ptr<dfcolumn>
dffunction_reverse::execute(dftable_base& t) const {
  auto left_column = left->execute(t);
  return left_column->reverse();
}

std::shared_ptr<dfcolumn> dffunction_reverse::aggregate
(dftable_base& table,
 node_local<std::vector<size_t>>& local_grouped_idx,
 node_local<std::vector<size_t>>& local_idx_split,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map,
 node_local<size_t>& row_sizes,
 dftable& grouped_table) {
  auto left_column = left->aggregate(table, local_grouped_idx,
                                     local_idx_split, hash_divide,
                                     merge_map, row_sizes, grouped_table);
  return left_column->reverse();
}

std::shared_ptr<dfcolumn>
dffunction_reverse::whole_column_aggregate(dftable_base& t) {
  auto left_column = left->whole_column_aggregate(t);
  return left_column->reverse();
}

std::shared_ptr<dffunction>
reverse_col(const std::string& left) {
  return std::make_shared<dffunction_reverse>(id_col(left));
}

std::shared_ptr<dffunction>
reverse_col(const std::shared_ptr<dffunction>& left) {
  return std::make_shared<dffunction_reverse>(left);
}

std::shared_ptr<dffunction>
reverse_col_as(const std::string& left, const std::string& as) {
  return std::make_shared<dffunction_reverse>(id_col(left), as);
}

std::shared_ptr<dffunction>
reverse_col_as(const std::shared_ptr<dffunction>& left, const std::string& as) {
  return std::make_shared<dffunction_reverse>(left, as);
}


// ----- substring_index -----
std::shared_ptr<dfcolumn>
dffunction_substring_index::execute(dftable_base& t) const {
  auto left_column = left->execute(t);
  return left_column->substring_index(str, pos);
}

std::shared_ptr<dfcolumn> dffunction_substring_index::aggregate
(dftable_base& table,
 node_local<std::vector<size_t>>& local_grouped_idx,
 node_local<std::vector<size_t>>& local_idx_split,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map,
 node_local<size_t>& row_sizes,
 dftable& grouped_table) {
  auto left_column = left->aggregate(table, local_grouped_idx,
                                     local_idx_split, hash_divide,
                                     merge_map, row_sizes, grouped_table);
  return left_column->substring_index(str, pos);
}

std::shared_ptr<dfcolumn>
dffunction_substring_index::whole_column_aggregate(dftable_base& t) {
  auto left_column = left->whole_column_aggregate(t);
  return left_column->substring_index(str, pos);
}

std::shared_ptr<dffunction>
substring_index_im(const std::string& left, const std::string& str, int pos) {
  return std::make_shared<dffunction_substring_index>(id_col(left), str, pos);
}

std::shared_ptr<dffunction>
substring_index_im(const std::shared_ptr<dffunction>& left,
                   const std::string& str, int pos) {
  return std::make_shared<dffunction_substring_index>(left, str, pos);
}

std::shared_ptr<dffunction>
substring_index_im_as(const std::string& left, const std::string& str,
                      int pos, const std::string& as) {
  return
    std::make_shared<dffunction_substring_index>(id_col(left), str, pos, as);
}

std::shared_ptr<dffunction>
substring_index_im_as(const std::shared_ptr<dffunction>& left,
                      const std::string& str, int pos, const std::string& as) {
  return std::make_shared<dffunction_substring_index>(left, str, pos, as);
}


// ----- concat -----
std::shared_ptr<dfcolumn> dffunction_concat::execute(dftable_base& t) const {
  auto left_column = left->execute(t);
  return left_column->concat(right->execute(t));
}

std::shared_ptr<dfcolumn> dffunction_concat::execute(dftable_base& t1, 
                                                     dftable_base& t2) const {
  auto left_column = left->execute(t1);
  auto right_column = right->execute(t2);
  auto aligned_right_column = realign_df(t1, t2, right_column);
  return left_column->concat(aligned_right_column);
}

std::shared_ptr<dfcolumn> dffunction_concat::aggregate
(dftable_base& table,
 node_local<std::vector<size_t>>& local_grouped_idx,
 node_local<std::vector<size_t>>& local_idx_split,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map,
 node_local<size_t>& row_sizes,
 dftable& grouped_table) {
  auto left_column = left->aggregate(table, local_grouped_idx,
                                     local_idx_split, hash_divide,
                                     merge_map, row_sizes, grouped_table);
  auto right_column = right->aggregate(table, local_grouped_idx,
                                       local_idx_split, hash_divide,
                                       merge_map, row_sizes, grouped_table);
  return left_column->concat(right_column);
}

std::shared_ptr<dfcolumn>
dffunction_concat::whole_column_aggregate(dftable_base& t) {
  auto left_column = left->whole_column_aggregate(t);
  return left_column->concat(right->whole_column_aggregate(t));
}

std::shared_ptr<dffunction> concat_col(const std::string& left,
                                       const std::string& right) {
  return std::make_shared<dffunction_concat>(id_col(left), id_col(right));
}


std::shared_ptr<dffunction>
concat_col(const std::shared_ptr<dffunction>& left,
           const std::shared_ptr<dffunction>& right) {
  return std::make_shared<dffunction_concat>(left, right);
} 

std::shared_ptr<dffunction> concat_col_as(const std::string& left,
                                          const std::string& right,
                                          const std::string& as) {
  return std::make_shared<dffunction_concat>(id_col(left), id_col(right), as);
}

std::shared_ptr<dffunction>
concat_col_as(const std::shared_ptr<dffunction>& left,
              const std::shared_ptr<dffunction>& right,
              const std::string& as) {
  return std::make_shared<dffunction_concat>(left, right, as);
}

// ----- concat_im -----
std::shared_ptr<dfcolumn> dffunction_concat_im::execute(dftable_base& t) const {
  auto left_column = left->execute(t);
  if(!is_reversed) return left_column->append_string(str);
  else return left_column->prepend_string(str);
}

std::shared_ptr<dfcolumn> dffunction_concat_im::aggregate
(dftable_base& table,
 node_local<std::vector<size_t>>& local_grouped_idx,
 node_local<std::vector<size_t>>& local_idx_split,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map,
 node_local<size_t>& row_sizes,
 dftable& grouped_table) {
  auto left_column = left->aggregate(table, local_grouped_idx,
                                     local_idx_split, hash_divide,
                                     merge_map, row_sizes, grouped_table);
  if(!is_reversed) return left_column->append_string(str);
  else return left_column->prepend_string(str);
}

std::shared_ptr<dfcolumn>
dffunction_concat_im::whole_column_aggregate(dftable_base& t) {
  auto left_column = left->whole_column_aggregate(t);
  if(!is_reversed) return left_column->append_string(str);
  else return left_column->prepend_string(str);
}

// we do not support specifying column by std::string,
// because it cannot be distinguished from appending/prepending string!

std::shared_ptr<dffunction>
concat_im(const std::shared_ptr<dffunction>& left,
          const std::string& str) {
  return std::make_shared<dffunction_concat_im>(left, str);
} 

std::shared_ptr<dffunction>
concat_im(const std::string& str,
          const std::shared_ptr<dffunction>& left) {
  return std::make_shared<dffunction_concat_im>(left, str, true);
} 

std::shared_ptr<dffunction>
concat_im_as(const std::shared_ptr<dffunction>& left,
             const std::string& str,
             const std::string& as) {
  return std::make_shared<dffunction_concat_im>(left, str, as);
} 

std::shared_ptr<dffunction>
concat_im(const std::string& str,
          const std::shared_ptr<dffunction>& left,
          const std::string& as) {
  return std::make_shared<dffunction_concat_im>(left, str, as, true);
} 

std::shared_ptr<dffunction>
concat_ws(const std::string& sep,
          const std::string& left,
          const std::string& right) {
  return std::make_shared<dffunction_concat>
    (concat_im(id_col(left), sep), id_col(right))
    ->as("concat_ws(" + sep + "," + left + "," + right + ")");
}

std::shared_ptr<dffunction>
concat_ws(const std::string& sep,
          const std::shared_ptr<dffunction>& left,
          const std::shared_ptr<dffunction>& right) {
  return std::make_shared<dffunction_concat>
    (concat_im(left, sep), right)
    ->as("concat_ws(" + sep + "," +
         left->get_as() + "," + right->get_as() + ")");
}


// ----- lower -----
std::shared_ptr<dfcolumn>
dffunction_lower::execute(dftable_base& t) const {
  auto left_column = left->execute(t);
  return left_column->lower();
}

std::shared_ptr<dfcolumn> dffunction_lower::aggregate
(dftable_base& table,
 node_local<std::vector<size_t>>& local_grouped_idx,
 node_local<std::vector<size_t>>& local_idx_split,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map,
 node_local<size_t>& row_sizes,
 dftable& grouped_table) {
  auto left_column = left->aggregate(table, local_grouped_idx,
                                     local_idx_split, hash_divide,
                                     merge_map, row_sizes, grouped_table);
  return left_column->lower();
}

std::shared_ptr<dfcolumn>
dffunction_lower::whole_column_aggregate(dftable_base& t) {
  auto left_column = left->whole_column_aggregate(t);
  return left_column->lower();
}

std::shared_ptr<dffunction>
lower_col(const std::string& left) {
  return std::make_shared<dffunction_lower>(id_col(left));
}

std::shared_ptr<dffunction>
lower_col(const std::shared_ptr<dffunction>& left) {
  return std::make_shared<dffunction_lower>(left);
}

std::shared_ptr<dffunction>
lower_col_as(const std::string& left, const std::string& as) {
  return std::make_shared<dffunction_lower>(id_col(left), as);
}

std::shared_ptr<dffunction>
lower_col_as(const std::shared_ptr<dffunction>& left, const std::string& as) {
  return std::make_shared<dffunction_lower>(left, as);
}


// ----- upper -----
std::shared_ptr<dfcolumn>
dffunction_upper::execute(dftable_base& t) const {
  auto left_column = left->execute(t);
  return left_column->upper();
}

std::shared_ptr<dfcolumn> dffunction_upper::aggregate
(dftable_base& table,
 node_local<std::vector<size_t>>& local_grouped_idx,
 node_local<std::vector<size_t>>& local_idx_split,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map,
 node_local<size_t>& row_sizes,
 dftable& grouped_table) {
  auto left_column = left->aggregate(table, local_grouped_idx,
                                     local_idx_split, hash_divide,
                                     merge_map, row_sizes, grouped_table);
  return left_column->upper();
}

std::shared_ptr<dfcolumn>
dffunction_upper::whole_column_aggregate(dftable_base& t) {
  auto left_column = left->whole_column_aggregate(t);
  return left_column->upper();
}

std::shared_ptr<dffunction>
upper_col(const std::string& left) {
  return std::make_shared<dffunction_upper>(id_col(left));
}

std::shared_ptr<dffunction>
upper_col(const std::shared_ptr<dffunction>& left) {
  return std::make_shared<dffunction_upper>(left);
}

std::shared_ptr<dffunction>
upper_col_as(const std::string& left, const std::string& as) {
  return std::make_shared<dffunction_upper>(id_col(left), as);
}

std::shared_ptr<dffunction>
upper_col_as(const std::shared_ptr<dffunction>& left, const std::string& as) {
  return std::make_shared<dffunction_upper>(left, as);
}


// ----- datetime_format_im -----
std::shared_ptr<dfcolumn>
dffunction_datetime_format_im::execute(dftable_base& t) const {
  auto left_column = left->execute(t);
  return left_column->datetime_format(fmt, type);
}

std::shared_ptr<dfcolumn> dffunction_datetime_format_im::aggregate(dftable_base& table,
          node_local<std::vector<size_t>>& local_grouped_idx,
          node_local<std::vector<size_t>>& local_idx_split,
          node_local<std::vector<std::vector<size_t>>>& hash_divide,
          node_local<std::vector<std::vector<size_t>>>& merge_map,
          node_local<size_t>& row_sizes,
          dftable& grouped_table) {
  auto left_column = left->aggregate(table, local_grouped_idx,
                                     local_idx_split, hash_divide,
                                     merge_map, row_sizes, grouped_table);
  return left_column->datetime_format(fmt, type);
}

std::shared_ptr<dfcolumn>
dffunction_datetime_format_im::whole_column_aggregate(dftable_base& t) {
  auto left_column = left->whole_column_aggregate(t);
  return left_column->datetime_format(fmt, type);
}

std::shared_ptr<dffunction>
datetime_format_im(const std::string& left,
                   const std::string& fmt,
                   const std::string& type) {
  return std::make_shared<dffunction_datetime_format_im>
    (id_col(left), fmt, type);
}

std::shared_ptr<dffunction>
datetime_format_im(const std::shared_ptr<dffunction>& left,
                   const std::string& fmt,
                   const std::string& type) {
  return std::make_shared<dffunction_datetime_format_im>(left, fmt, type);
}

std::shared_ptr<dffunction>
datetime_format_im_as(const std::string& left,
                      const std::string& fmt,
                      const std::string& type,
                      const std::string& as) {
  return std::make_shared<dffunction_datetime_format_im>
    (id_col(left), fmt, type, as);
}

std::shared_ptr<dffunction>
datetime_format_im_as(const std::shared_ptr<dffunction>& left,
                      const std::string& fmt,
                      const std::string& type,
                      const std::string& as) {
  return std::make_shared<dffunction_datetime_format_im>(left, fmt, type, as);
}

// ----- utility functions for user's direct use -----

std::shared_ptr<dffunction> col(const std::string& col) {
  return id_col(col);
}

std::shared_ptr<dffunction> operator~(const std::string& col) {
  return id_col(col);
}

}
