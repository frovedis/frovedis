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
  
  dftable create_antacedent(dftable df, int j){ 
  //pass-by-value to avoid changes in input dftable
      auto col_name = df.columns();
      std::vector<std::string> cols_list;
      df.rename(col_name[j],"consequent");
      df.rename("count","union_count");
      int k = 1;
      for(int i=0; i < col_name.size()-1; i++){
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
  dftable calculate_confidence(dftable& pre, dftable& post, double con){
    auto pre_col = pre.columns();
    auto post_col = post.columns();
    auto col_list1 = post_col;
	//will contain all antacedants, consequent, union_count and count
    col_list1.push_back("count"); 
    auto col_list2 = post_col;
    col_list2.pop_back(); //popping union_counts
	// will conatin all antacedants, consequent and confidence
    col_list2.push_back("confidence"); 
    auto iter = pre.num_col()-1;
    auto result = pre.bcast_join(post,eq(pre_col[0],post_col[0])).materialize();
    if(iter > 1){
      auto fdf = result.filter(eq(pre_col[1],post_col[1]));
      free_df(result);
      for(int i = 2; i < iter; ++i){
        fdf = fdf.filter(eq(pre_col[i], post_col[i]));
      }
      result = fdf.materialize();
    }  
    return result.select(col_list1)
                .calc<double,size_t,size_t>(std::string("confidence"),ratio_op(),
                                            std::string("union_count"),
                                            std::string("count"))
                .filter(ge_im("confidence",con))
                .select(col_list2);
  }
}

