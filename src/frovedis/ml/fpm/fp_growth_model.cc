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
  
  std::vector<std::string> 
  get_info_columns(dftable& cur_info, dftable& old_info) {
    // cid_ in c2 => cid_prev in c1
    auto c1 = cur_info.columns(); // cid_prev, item_N+1, cid
    auto c2 = old_info.columns(); // item, item1, ..., itemN, cid_
    //std::cout << "cur: "; debug_print_vector(c1);
    //std::cout << "old: "; debug_print_vector(c2);
    auto c1sz = c1.size();
    auto c2sz = c2.size();
    auto retsz = (c1sz - 1) + (c2sz - 1); // removing cid_ from both old and cid_prev from cur
    std::vector<std::string> ret(retsz);
    for(size_t i = 0; i < c2sz - 1; ++i) ret[i] = c2[i]; // item, item1, ..., itemN
    ret[retsz - 2] = c1[1]; // item_N+1
    ret[retsz - 1] = c1[2]; // cid
    return ret;
  }

  std::vector<std::string>
  get_item_columns(dftable& item, dftable& tinfo) {
    auto c1 = item.columns();  // cid, item_N+1, count
    auto c2 = tinfo.columns(); // item, item1, ..., itemN, cid_
    //std::cout << "item: "; debug_print_vector(c1);
    //std::cout << "info: "; debug_print_vector(c2);
    auto c1sz = c1.size();
    auto c2sz = c2.size();
    auto retsz = (c1sz - 1) + (c2sz - 1); // removing cid_ from tinfo and cid from item
    std::vector<std::string> ret(retsz);
    for(size_t i = 0; i < c2sz - 1; ++i) ret[i] = c2[i]; // item, item1, ..., itemN
    ret[retsz - 2] = c1[1]; // item_N+1
    ret[retsz - 1] = c1[2]; // count
    return ret;
  }

  dftable decompress_impl(dftable& item, 
                          dftable& tree_info, 
                          dftable& old_info) {
    if(tree_info.num_row()) {
      if (!old_info.num_row()) old_info = tree_info.rename("cid", "cid_");
      else {
        auto merged_info = tree_info.bcast_join(old_info, eq("cid_prev", "cid_"))
                                    .select(get_info_columns(tree_info, old_info));
        old_info = merged_info.rename("cid", "cid_");
      }
      return item.bcast_join(old_info, eq("cid", "cid_"))
                 .select(get_item_columns(item, old_info));
    }
    else return item;
  }

  size_t fp_growth_model::get_depth () { return item.size() + 1; }

  size_t fp_growth_model::get_count() { 
    size_t count = 0;
    for(size_t i = 0; i < item.size(); ++i) count += item[i].num_row();
    return count;
  }

  std::vector<dftable> 
  fp_growth_model::get_frequent_itemset() {
    std::vector<dftable> ret; 
    dftable old_info;
    for (size_t i = 0; i < item.size(); ++i) {
      ret.push_back(decompress_impl(item[i], tree_info[i], old_info));
    }
    return ret;
  }

  void fp_growth_model::debug_print() {
    for (size_t i = 0; i < item.size(); ++i) {
      std::cout << "---tree[" << i << "]---\n";
      item[i].show(); 
      std::cout << std::endl;
      std::cout << "---tree_info[" << i << "]---\n";
      tree_info[i].show();
      std::cout << std::endl;
    }
  }
 
  void fp_growth_model::save (const std::string& dir) {
    require(!directory_exists(dir),
    "save: a directory with same name already exists!\n");
    make_directory(dir);
    size_t precision = 6;
    std::string datetime_fmt = "%Y-%m-%d";
    std::string sep = " ";
    dftable old_info;
    for (size_t i = 0; i < item.size(); ++i) {
      auto fname = dir + "/tree_" + std::to_string(i);
      auto fis = decompress_impl(item[i], tree_info[i], old_info);
      fis.savetext(fname, precision, datetime_fmt, sep);
    }
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
  
  //pass-by-value to avoid changes in input dftable
  dftable create_antacedent(dftable df, int j){ 
      auto col_name = df.columns();
      std::vector<std::string> cols_list;
      df.rename(col_name[j],"consequent");
      df.rename("count","union_count");
      int k = 1;
      for(size_t i = 0; i < col_name.size()-1; i++) {
        if(i != j) {
          auto new_name = "antacedent" + std::to_string(k++);
          df.rename(col_name[i],new_name);
          cols_list.push_back(new_name);
        }
      }
      cols_list.push_back("consequent");
      cols_list.push_back("union_count");
      std::cout << " --- ANT::Cols list:" << std::endl;
      for (auto col: cols_list) std::cout << col << std::endl;
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
    std::cout << "Pre col:" << std::endl;
    for (auto col: pre_col) std::cout << col << std::endl;
    auto post_col = post.columns();
    std::cout << "Post col:" << std::endl;
    for (auto col: post_col) std::cout << col << std::endl;
    auto col_list1 = post_col;
    //will contain all antacedants, consequent, union_count and count
    col_list1.push_back("count"); 
    auto col_list2 = post_col;
    col_list2.pop_back(); //popping union_counts
    // will conatin all antacedants, consequent and confidence
    col_list2.push_back("confidence"); 
    auto iter = pre.num_col()-1;
    std::cout << "Col list1:" << std::endl;
    for (auto col: col_list1) std::cout << col << std::endl;
    std::cout << "Col list2:" << std::endl;
    for (auto col: col_list2) std::cout << col << std::endl;
    std::vector<std::string> opt_left(iter), opt_right(iter);
    for (size_t i = 0; i < iter; ++i){
      opt_left[i] = pre_col[i];
      opt_right[i] = post_col[i];
    }
    std::cout << "Opt left:" << std::endl;
    for (auto col: opt_left) std::cout << col << std::endl;
    std::cout << "Opt right:" << std::endl;
    for (auto col: opt_right) std::cout << col << std::endl;
    std::cout << " ------ Pre ---" << std::endl;
    pre.show();
    std::cout << " ------ Post ---" << std::endl;
    post.show();
    auto result = pre.bcast_join(post,multi_eq(opt_left, opt_right));
    return result.select(col_list1)
                 .calc<double,size_t,size_t>(std::string("confidence"),diff(),
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
         std::cout << "--- res_con: " << std::endl;
         res_con.show();
         if(res_con.num_row()) ass_rule.push_back(res_con);
       }
    }
    return association_rule(ass_rule);
  }

}
