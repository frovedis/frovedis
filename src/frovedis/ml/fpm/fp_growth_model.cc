#include <frovedis.hpp>
#include <frovedis/dataframe.hpp>

#include "fp_growth_model.hpp"
#include "fp_growth.hpp"

namespace frovedis{

void association_rule::debug_print() {
  for(auto& r: rule) r.show();
}

void association_rule::save (const std::string& fname) {
  std::cout << "save request for fp-growth model with fname: " << fname << std::endl;
}

void association_rule::savebinary (const std::string& fname) {
  std::cout << "savebinary request for fp-growth model with fname: " << fname << std::endl;
}

void association_rule::load (const std::string& fname) {
  std::cout << "load request for fp-growth model with fname: " << fname << std::endl;
}

void association_rule::loadbinary (const std::string& fname) {
  std::cout << "loadbinary request for fp-growth model with fname: " << fname << std::endl;
}

association_rule fp_growth_model::generate_rules(double con){
  return generate_association_rules(item,con);
}

void fp_growth_model::debug_print(){
  for(auto& i: item) i.show();
}

void fp_growth_model::save (const std::string& fname) {
  std::cout << "save request for fp-growth model with fname: " << fname << std::endl;
}
void fp_growth_model::savebinary (const std::string& fname) {
  std::cout << "savebinary request for fp-growth model with fname: " << fname << std::endl;
}
void fp_growth_model::load (const std::string& fname) {
  std::cout << "load request for fp-growth model with fname: " << fname << std::endl;
}
void fp_growth_model::loadbinary (const std::string& fname) {
  std::cout << "loadbinary request for fp-growth model with fname: " << fname << std::endl;
}

}

