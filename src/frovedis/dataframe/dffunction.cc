#include "dffunction.hpp"

namespace frovedis {

bool check_distribution(dftable_base& left, dftable_base& right,
                        std::vector<size_t>& left_sizes) {
  checkAssumption(left.num_col() && right.num_col());
  left_sizes = left.num_rows();
  auto right_sizes = right.num_rows();
  return left_sizes == right_sizes;
}

// re-aligns right as per left
dftable_base& realign_df(dftable_base& left, 
                         dftable_base& right,
                         const std::vector<std::string>& right_cols) {
  dftable_base& ret = right;
  std::vector<size_t> left_sizes;
  if (!check_distribution(left, right, left_sizes)) 
    ret = right.select(right_cols).align_as(left_sizes);
  return ret;
}

std::shared_ptr<dfcolumn> dffunction_add::execute(dftable_base& t) const {
  auto left_column = t.column(left);
  return left_column->add(t.column(right));
}

std::shared_ptr<dfcolumn> dffunction_add::execute(dftable_base& t1, 
                                                  dftable_base& t2) const {
  auto left_column = t1.column(left);
  auto&& t2_ = realign_df(t1, t2, {right});
  return left_column->add(t2_.column(right));
}

std::shared_ptr<dffunction> add_col(const std::string& left,
                                    const std::string& right) {
  return std::make_shared<dffunction_add>(left, right);
}

std::shared_ptr<dffunction> add_col_as(const std::string& left,
                                       const std::string& right,
                                       const std::string& as) {
  return std::make_shared<dffunction_add>(left, right, as);
}


std::shared_ptr<dfcolumn> dffunction_sub::execute(dftable_base& t) const {
  auto left_column = t.column(left);
  return left_column->sub(t.column(right));
}

std::shared_ptr<dfcolumn> dffunction_sub::execute(dftable_base& t1, 
                                                  dftable_base& t2) const {
  auto left_column = t1.column(left);
  auto&& t2_ = realign_df(t1, t2, {right});
  return left_column->sub(t2_.column(right));
}

std::shared_ptr<dffunction> sub_col(const std::string& left,
                                    const std::string& right) {
  return std::make_shared<dffunction_sub>(left, right);
}

std::shared_ptr<dffunction> sub_col_as(const std::string& left,
                                       const std::string& right,
                                       const std::string& as) {
  return std::make_shared<dffunction_sub>(left, right, as);
}

std::shared_ptr<dfcolumn> dffunction_mul::execute(dftable_base& t) const {
  auto left_column = t.column(left);
  return left_column->mul(t.column(right));
}

std::shared_ptr<dfcolumn> dffunction_mul::execute(dftable_base& t1, 
                                                  dftable_base& t2) const {
  auto left_column = t1.column(left);
  auto&& t2_ = realign_df(t1, t2, {right});
  return left_column->mul(t2_.column(right));
}

std::shared_ptr<dffunction> mul_col(const std::string& left,
                                    const std::string& right) {
  return std::make_shared<dffunction_mul>(left, right);
}

std::shared_ptr<dffunction> mul_col_as(const std::string& left,
                                       const std::string& right,
                                       const std::string& as) {
  return std::make_shared<dffunction_mul>(left, right, as);
}

std::shared_ptr<dfcolumn> dffunction_fdiv::execute(dftable_base& t) const {
  auto left_column = t.column(left);
  return left_column->fdiv(t.column(right));
}

std::shared_ptr<dfcolumn> dffunction_fdiv::execute(dftable_base& t1, 
                                                   dftable_base& t2) const {
  auto left_column = t1.column(left);
  auto&& t2_ = realign_df(t1, t2, {right});
  return left_column->fdiv(t2_.column(right));
}

std::shared_ptr<dffunction> fdiv_col(const std::string& left,
                                    const std::string& right) {
  return std::make_shared<dffunction_fdiv>(left, right);
}

std::shared_ptr<dffunction> fdiv_col_as(const std::string& left,
                                       const std::string& right,
                                       const std::string& as) {
  return std::make_shared<dffunction_fdiv>(left, right, as);
}

std::shared_ptr<dfcolumn> dffunction_idiv::execute(dftable_base& t) const {
  auto left_column = t.column(left);
  return left_column->idiv(t.column(right));
}

std::shared_ptr<dfcolumn> dffunction_idiv::execute(dftable_base& t1, 
                                                   dftable_base& t2) const {
  auto left_column = t1.column(left);
  auto&& t2_ = realign_df(t1, t2, {right});
  return left_column->idiv(t2_.column(right));
}

std::shared_ptr<dffunction> idiv_col(const std::string& left,
                                    const std::string& right) {
  return std::make_shared<dffunction_idiv>(left, right);
}

std::shared_ptr<dffunction> idiv_col_as(const std::string& left,
                                       const std::string& right,
                                       const std::string& as) {
  return std::make_shared<dffunction_idiv>(left, right, as);
}

std::shared_ptr<dfcolumn> dffunction_mod::execute(dftable_base& t) const {
  auto left_column = t.column(left);
  return left_column->mod(t.column(right));
}

std::shared_ptr<dfcolumn> dffunction_mod::execute(dftable_base& t1, 
                                                  dftable_base& t2) const {
  auto left_column = t1.column(left);
  auto&& t2_ = realign_df(t1, t2, {right});
  return left_column->mod(t2_.column(right));
}

std::shared_ptr<dffunction> mod_col(const std::string& left,
                                    const std::string& right) {
  return std::make_shared<dffunction_mod>(left, right);
}

std::shared_ptr<dffunction> mod_col_as(const std::string& left,
                                       const std::string& right,
                                       const std::string& as) {
  return std::make_shared<dffunction_mod>(left, right, as);
}

std::shared_ptr<dfcolumn> dffunction_pow::execute(dftable_base& t) const {
  auto left_column = t.column(left);
  return left_column->pow(t.column(right));
}

std::shared_ptr<dfcolumn> dffunction_pow::execute(dftable_base& t1,
                                                  dftable_base& t2) const {
  auto left_column = t1.column(left);
  auto&& t2_ = realign_df(t1, t2, {right});
  return left_column->pow(t2_.column(right));
}

std::shared_ptr<dffunction> pow_col(const std::string& left,
                                    const std::string& right) {
  return std::make_shared<dffunction_pow>(left, right);
}

std::shared_ptr<dffunction> pow_col_as(const std::string& left,
                                       const std::string& right,
                                       const std::string& as) {
  return std::make_shared<dffunction_pow>(left, right, as);
}

std::shared_ptr<dfcolumn> dffunction_abs::execute(dftable_base& t) const {
  auto left_column = t.column(left);
  return left_column->abs();
}

std::shared_ptr<dffunction>
abs_col(const std::string& left) {
  return std::make_shared<dffunction_abs>(left);
}

std::shared_ptr<dffunction>
abs_col_as(const std::string& left, const std::string& as) {
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
