#include "fp_growth_model.hpp"

namespace frovedis {

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
  
  dftable create_antacedent(dftable df, int j){ 
  //pass-by-value to avoid changes in input dftable
      auto col_name = df.columns();
      std::vector<std::string> cols_list;
      df.rename(col_name[j],"consequent");
      df.rename("count","union_count");
      int k = 1;
      for(size_t i = 0; i < col_name.size()-1; i++){
        if(i != j){
          auto new_name = "antacedent" + std::to_string(k++);
          df.rename(col_name[i],new_name);
          cols_list.push_back(new_name);
        }
      }
      cols_list.push_back("consequent");
      cols_list.push_back("union_count");
      return df.select(cols_list);
  }  

  struct diff {
    double operator()(size_t a, size_t b) {
      return static_cast<double>(a) / static_cast<double>(b);
    }
    SERIALIZE_NONE
  };

  dftable calculate_confidence(dftable& pre, dftable& post, double con){
    auto pre_col = pre.columns();
    auto post_col = post.columns();
    auto col_list1 = post_col;
    col_list1.push_back("count"); //will contain all antacedants, consequent, union_count and count
    auto col_list2 = post_col;
    col_list2.pop_back(); //popping union_counts
    col_list2.push_back("confidence"); // will conatin all antacedants, consequent and confidence
    auto iter = pre.num_col() - 1;
    std::vector<std::string> opt_left(iter), opt_right(iter);
    for (size_t i = 0; i < iter; ++i){
      opt_left[i] = pre_col[i];
      opt_right[i] = post_col[i];
    }
    auto result = pre.bcast_join(post,multi_eq(opt_left, opt_right));
    return result.select(col_list1)
                 .calc<double,size_t,size_t>(std::string("confidence"), diff(),
                                             std::string("union_count"),
                                             std::string("count"))
                 .filter(ge_im("confidence",con))
                 .select(col_list2);
  }

  association_rule
  generate_association_rules(std::vector<dftable>& freq_itemsets,
                             double con) {
    std::vector<dftable> ass_rule;
#ifdef FP_DEBUG
    std::cout << "NUM FREQ ITEM SETS: " << freq_itemsets.size() << std::endl;
    std::cout << "FREQ ITEM SETS:\n";
    for(dftable& i: freq_itemsets)  i.show();
#endif
    for(size_t i = 1; i < freq_itemsets.size(); ++i) {
       //freq-itemset 1 will not be considered
       auto n = freq_itemsets[i].num_col();
       for(size_t j = 0; j < n - 1; j++) {
         auto res =  create_antacedent(freq_itemsets[i], j);
         auto res_con = calculate_confidence(freq_itemsets[i-1],res,con);
         if(res_con.num_row()) ass_rule.push_back(res_con);
       }
    }
    return association_rule(ass_rule);
  }
}
