#include "dffunction.hpp"

namespace frovedis {

std::shared_ptr<dfcolumn> dffunction_add::execute(dftable_base& t) const {
  auto left_column = t.column(left);
  return left_column->add(t.column(right));
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

std::shared_ptr<dffunction> mod_col(const std::string& left,
                                    const std::string& right) {
  return std::make_shared<dffunction_mod>(left, right);
}

std::shared_ptr<dffunction> mod_col_as(const std::string& left,
                                       const std::string& right,
                                       const std::string& as) {
  return std::make_shared<dffunction_mod>(left, right, as);
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

}
