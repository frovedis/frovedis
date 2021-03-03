#include "fp_growth_model.hpp"

namespace frovedis {

  void free_df(dftable_base& df) { dftable tmp; df = tmp; }

  std::vector<std::string> 
  get_info_columns(dftable& cur_info, dftable& old_info) {
    // cid_ in c2 => cid_prev in c1
    auto c1 = cur_info.columns(); // cid_prev, item_N+1, cid
    auto c2 = old_info.columns(); // item, item1, ..., itemN, cid_
    //show("cur: ", c1);
    //show("old: ", c2);
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
    //show("item: ", c1);
    //show("info: ", c2);
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
      if (!old_info.num_row()) {
        old_info = tree_info;
        old_info.rename("cid", "cid_");
      }
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

  void fp_growth_model::clear () { 
    for(size_t i = 0; i < item.size(); ++i) {
      free_df(item[i]); free_df(tree_info[i]);
    }
    item.clear();
    tree_info.clear();
  }

  size_t fp_growth_model::get_depth () { return item.size(); }

  size_t fp_growth_model::get_count() { 
    size_t count = 0;
    for(size_t i = 0; i < item.size(); ++i) count += item[i].num_row();
    return count;
  }

  std::vector<dftable> 
  fp_growth_model::get_frequent_itemset() {
    auto depth = get_depth();
    std::vector<dftable> ret(depth); 
    dftable old_info;
    for (size_t i = 0; i < depth; ++i) {
      ret[i] = decompress_impl(item[i], tree_info[i], old_info);
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
      auto part_dname = dir + "/tree_" + std::to_string(i);
      make_directory(part_dname);
      auto tree_data = part_dname + "/data";
      auto tree_schema = part_dname + "/schema";
      auto fis = decompress_impl(item[i], tree_info[i], old_info);
      auto dt = fis.savetext(tree_data, precision, datetime_fmt, sep);

      std::ofstream schema_str;
      schema_str.exceptions(std::ofstream::failbit | std::ofstream::badbit);
      schema_str.open(tree_schema.c_str());
      for(auto& e: dt) schema_str << e.first << "\n" << e.second << "\n";
    }
  }

  void fp_growth_model::savebinary (const std::string& dir) {
    save(dir); // for wrapper
  }

  void fp_growth_model::load (const std::string& dir) {
    require(directory_exists(dir), "load: directory does not exist!\n");
    auto depth = count_non_hidden_files(dir);
    RLOG(INFO) << "load: tree-depth found: " << depth << std::endl;
    clear();
    std::string tmp;
    std::vector<dftable> tree_item(depth), tree_info(depth);
    for(size_t i = 0; i < depth; ++i) {
      auto tree_data = dir + "/tree_" + std::to_string(i) + "/data";
      auto tree_schema = dir + "/tree_" + std::to_string(i) + "/schema";
      std::ifstream schema_str(tree_schema.c_str()); 
      std::vector<std::string> types, names;
      while(std::getline(schema_str, tmp)) {
        names.push_back(tmp);
        std::getline(schema_str, tmp); types.push_back(tmp);
      }
#ifdef FP_DEBUG
      show("names: ", names);
      show("types: ", types);
#endif
      tree_item[i] = make_dftable_loadtext(tree_data, types, names, ' ');
      tree_info[i] = dftable(); // empty info (since decompressed tree is saved)
    }
    *this = fp_growth_model(std::move(tree_item), std::move(tree_info));
  }
  void fp_growth_model::loadbinary (const std::string& dir) {
    load(dir); // for wrapper
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
#ifdef FP_DEBUG
      show("ANT::Cols list: ", cols_list);
#endif
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
    //will contain all antacedants, consequent, union_count and count
    col_list1.push_back("count"); 
    auto col_list2 = post_col;
    col_list2.pop_back(); //popping union_counts
    // will conatin all antacedants, consequent and confidence
    col_list2.push_back("confidence"); 
    auto iter = pre.num_col()-1;
    std::vector<std::string> opt_left(iter), opt_right(iter);
    for (size_t i = 0; i < iter; ++i){
      opt_left[i] = pre_col[i];
      opt_right[i] = post_col[i];
    }
#ifdef FP_DEBUG
    show("pre-col: ", pre_col);
    show("post-col: ", post_col);
    show("col-list1: ", col_list1);
    show("col-list2: ", col_list2);
    show("opt-left: ", opt_left);
    show("opt-right: ", opt_right);
    std::cout << " ------ Pre ---" << std::endl;   pre.show();
    std::cout << " ------ Post ---" << std::endl;  post.show();
#endif
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
#ifdef FP_DEBUG
         std::cout << "--- res_con: " << std::endl; res_con.show();
#endif
         if(res_con.num_row()) ass_rule.push_back(res_con);
       }
    }
    return association_rule(ass_rule);
  }

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
  
}
