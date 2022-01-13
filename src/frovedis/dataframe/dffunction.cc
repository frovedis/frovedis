#include "dffunction.hpp"
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

std::shared_ptr<dffunction>
datetime_im(datetime_t value) {
  return std::make_shared<dffunction_datetime_im>(value);
}

std::shared_ptr<dffunction>
datetime_im_as(datetime_t value, const std::string& as) {
  return std::make_shared<dffunction_datetime_im>(value, as);
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

// ----- utility functions for user's direct use -----

std::shared_ptr<dffunction> col(const std::string& col) {
  return id_col(col);
}

std::shared_ptr<dffunction> operator~(const std::string& col) {
  return id_col(col);
}

std::shared_ptr<dffunction> operator+(const std::shared_ptr<dffunction>& a,
                                      const std::shared_ptr<dffunction>& b) {
  return add_col(a,b);
}

std::shared_ptr<dffunction> operator*(const std::shared_ptr<dffunction>& a,
                                      const std::shared_ptr<dffunction>& b) {
  return mul_col(a,b);
}

std::shared_ptr<dffunction> operator-(const std::shared_ptr<dffunction>& a,
                                      const std::shared_ptr<dffunction>& b) {
  return sub_col(a,b);
}

std::shared_ptr<dffunction> operator/(const std::shared_ptr<dffunction>& a,
                                      const std::shared_ptr<dffunction>& b) {
  return fdiv_col(a,b);
}

std::shared_ptr<dffunction> operator%(const std::shared_ptr<dffunction>& a,
                                      const std::shared_ptr<dffunction>& b) {
  return mod_col(a,b);
}


}
