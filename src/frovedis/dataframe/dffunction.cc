#include "dffunction.hpp"

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


// ----- id -----
std::shared_ptr<dffunction> id_col(const std::string& left) {
  return std::make_shared<dffunction_id>(left);
}

std::shared_ptr<dffunction> id_col_as(const std::string& left,
                                      const std::string& as) {
  return std::make_shared<dffunction_id>(left, as);
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

std::shared_ptr<dfcolumn> dffunction_abs::execute(dftable_base& t) const {
  auto left_column = left->execute(t);
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

}
