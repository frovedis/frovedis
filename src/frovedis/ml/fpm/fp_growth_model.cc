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
      std::cout << "--- tree[" << i << "] ---\n";
      item[i].show(); 
      std::cout << std::endl;
      std::cout << "--- tree_info[" << i << "] ---\n";
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
    require(!directory_exists(dir),
    "save: a directory with same name already exists!\n");
    make_directory(dir);
    dftable old_info;
    for (size_t i = 0; i < item.size(); ++i) {
      auto part_dname = dir + "/tree_" + std::to_string(i);
      auto fis = decompress_impl(item[i], tree_info[i], old_info);
      fis.save(part_dname);
    }
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
      //show("names: ", names);
      //show("types: ", types);
      tree_item[i] = make_dftable_loadtext(tree_data, types, names, ' ');
      tree_info[i] = dftable(); // empty info (since decompressed tree is saved)
    }
    *this = fp_growth_model(std::move(tree_item), std::move(tree_info));
  }

  void fp_growth_model::loadbinary (const std::string& dir) {
    require(directory_exists(dir), "load: directory does not exist!\n");
    auto depth = count_non_hidden_files(dir);
    RLOG(INFO) << "load: tree-depth found: " << depth << std::endl;
    clear();
    std::vector<dftable> tree_item(depth), tree_info(depth);
    for(size_t i = 0; i < depth; ++i) {
      auto tree_dir = dir + "/tree_" + std::to_string(i);
      tree_item[i] = dftable();
      tree_item[i].load(tree_dir);
      tree_info[i] = dftable(); // empty info (since decompressed tree is saved)
    }
    *this = fp_growth_model(std::move(tree_item), std::move(tree_info));
  }
  
  struct diff {
    double operator()(size_t a, size_t b) { 
      return static_cast<double>(a) / static_cast<double>(b);
    }
    SERIALIZE_NONE
  };

  void calculate_confidence(dftable& pre, dftable& post,
                            double conf, int depth_level,
                            std::vector<dftable>& result) {
    std::string count = "count";
    std::string union_count = "union_count";
    std::string consequent = "consequent";
    std::string confidence = "confidence";
    std::string antacedent = "antacedent";
    auto pre_col = pre.columns();
    auto post_col = post.columns();
    auto iter = pre_col.size() - 1;
    auto post_ncol = post_col.size();
    std::vector<std::string> opt_left(iter), opt_right(iter);
    std::vector<std::string> targets1(post_ncol + 1), targets2(post_ncol);
    for(size_t i = 0; i < iter; ++i) {
      opt_left[i] = pre_col[i];
      auto ant_name = antacedent + std::to_string(i+1);
      opt_right[i] = targets1[i] = targets2[i] = ant_name;
    }
    targets1[iter] = targets2[iter] = consequent; // ant1, ... antN, consequent
    targets1[iter + 1] = union_count; 
    targets1[iter + 2] = count;      // + union_count, count
    targets2[iter + 1] = confidence; // + confidence
#ifdef FP_DEBUG
    show("opt-left: ", opt_left);
    show("opt-right: ", opt_right);
    show("select-targets1: ", targets1);
    show("select-targets2: ", targets2);
#endif

    post.rename(post_col[post_ncol - 1], union_count); // count -> union_count
    auto tmp_post_col = post.columns();
    for(size_t i = 0; i < post_ncol - 1; ++i) {
      if (i == 0) {
        size_t k = 1;
        for (size_t j = 0; j < post_ncol - 1; ++j) {
          if (i == j) post.rename(tmp_post_col[j], consequent); 
          else post.rename(tmp_post_col[j], antacedent + std::to_string(k++));
        }
      }
      else { // swap name of ith column with i-1th column
        auto t1 = tmp_post_col[i];
        auto t2 = tmp_post_col[i - 1];
        post.rename(t2, "tmp");
        post.rename(t1, t2); // consequent
        post.rename("tmp", t1);
      }
      tmp_post_col = post.columns(); // renamed columns
#ifdef FP_DEBUG
      show("pre-col: ", pre_col);
      show("post-col: ", tmp_post_col);
      std::cout << " ------ Pre ---" << std::endl;   pre.show();
      std::cout << " ------ Post ---" << std::endl;  post.show();
#endif
      dftable rule;
      if(post.num_row() > pre.num_row()) {
        // std::cout << "join: " << post.num_row() << " x " << pre.num_row() << "\n";
        rule = post.bcast_join(pre, multi_eq(opt_right, opt_left))
                   .select(targets1); // ant1, ..., antN, consequent, union_count, count
      }
      else {
        // std::cout << "join: " << pre.num_row() << " x " << post.num_row() << "\n";
        rule = pre.bcast_join(post, multi_eq(opt_left, opt_right))
                  .select(targets1); // ant1, ..., antN, consequent, union_count, count
      }
      rule = rule.calc<double,size_t,size_t>(confidence, diff(),
                                             union_count, count)
                 .filter(ge_im(confidence, conf))
                 .select(targets2); // ant1, ..., antN, consequent, confidence
#ifdef FP_DEBUG
         std::cout << "--- [depth: " << depth_level 
                   << "; iter: " << i << "] rule --- \n";
         rule.show();
#endif
      if (rule.num_row()) result.push_back(std::move(rule));
    }
    for(size_t i = 0; i < post_ncol; ++i) {
      post.rename(tmp_post_col[i], post_col[i]); // rename-back to original
    }
  }

  association_rule
  generate_association_rules(std::vector<dftable>& item,
                             std::vector<dftable>& tree_info,
                             double conf) {
    require(conf >= 0.0 && conf <= 1.0, 
    "generate_rules: confidence should be in between 0 to 1.\n");
    std::vector<dftable> rules;
    auto depth = item.size();
    if (depth <= 1) return rules;
    try {
      dftable old_info;
      auto pre_fis = decompress_impl(item[0], tree_info[0], old_info);
      for(size_t i = 1; i < depth; ++i) {
        auto post_fis = decompress_impl(item[i], tree_info[i], old_info);
        calculate_confidence(pre_fis, post_fis, conf, i, rules);
        pre_fis = std::move(post_fis);
      }
    }
    catch(std::exception& excpt) {
      std::string msg = excpt.what();
      if(msg.find("bad_alloc") != std::string::npos ) {
        std::string e = "out-of-memory error occured during rule mining!\n";
        e += "retry building tree with higher min_suuport value.\n";
        REPORT_ERROR(INTERNAL_ERROR, e);
      }
      else REPORT_ERROR(INTERNAL_ERROR, msg);
    }
    return association_rule(rules);
  }

  association_rule fp_growth_model::generate_rules(double conf) {
    return generate_association_rules(item, tree_info, conf);
  }
 
  void association_rule::clear () {
    for(size_t i = 0; i < rule.size(); ++i) free_df(rule[i]); 
    rule.clear();
  }

  size_t association_rule::get_depth () { return rule.size(); }

  size_t association_rule::get_count() {
    size_t count = 0;
    for(size_t i = 0; i < rule.size(); ++i) count += rule[i].num_row();
    return count;
  }

  void association_rule::debug_print() {
    for(size_t i = 0; i < rule.size(); ++i) {
      std::cout << "--- rule[" << i << "] ---\n";
      rule[i].show(); 
      std::cout << std::endl;
    }
  }
  
  void association_rule::save (const std::string& dir) {
    require(!directory_exists(dir),
    "save: a directory with same name already exists!\n");
    make_directory(dir);
    size_t precision = 6;
    std::string datetime_fmt = "%Y-%m-%d";
    std::string sep = " ";
    for (size_t i = 0; i < rule.size(); ++i) {
      auto part_dname = dir + "/rule_" + std::to_string(i);
      make_directory(part_dname);
      auto rule_data = part_dname + "/data";
      auto rule_schema = part_dname + "/schema";
      auto dt = rule[i].savetext(rule_data, precision, datetime_fmt, sep);

      std::ofstream schema_str;
      schema_str.exceptions(std::ofstream::failbit | std::ofstream::badbit);
      schema_str.open(rule_schema.c_str());
      for(auto& e: dt) schema_str << e.first << "\n" << e.second << "\n";
    }
  }
  
  void association_rule::savebinary (const std::string& dir) {
    require(!directory_exists(dir),
    "save: a directory with same name already exists!\n");
    make_directory(dir);
    for (size_t i = 0; i < rule.size(); ++i) {
      auto part_dname = dir + "/rule_" + std::to_string(i);
      rule[i].save(part_dname);
    }
  }
  
  void association_rule::load (const std::string& dir) {
    require(directory_exists(dir), "load: directory does not exist!\n");
    auto depth = count_non_hidden_files(dir);
    RLOG(INFO) << "load: set of rules found: " << depth << std::endl;
    clear();  this->rule.resize(depth);
    std::string tmp;
    for(size_t i = 0; i < depth; ++i) {
      auto rule_data = dir + "/rule_" + std::to_string(i) + "/data";
      auto rule_schema = dir + "/rule_" + std::to_string(i) + "/schema";
      std::ifstream schema_str(rule_schema.c_str());
      std::vector<std::string> types, names;
      while(std::getline(schema_str, tmp)) {
        names.push_back(tmp);
        std::getline(schema_str, tmp); types.push_back(tmp);
      }
      //show("names: ", names);
      //show("types: ", types);
      rule[i] = make_dftable_loadtext(rule_data, types, names, ' ');
    }
  }
  
  void association_rule::loadbinary (const std::string& dir) {
    require(directory_exists(dir), "load: directory does not exist!\n");
    auto depth = count_non_hidden_files(dir);
    RLOG(INFO) << "load: set of rules found: " << depth << std::endl;
    clear();  this->rule.resize(depth);
    for(size_t i = 0; i < depth; ++i) {
      auto rule_dir = dir + "/rule_" + std::to_string(i);
      rule[i] = dftable();
      rule[i].load(rule_dir);
    }
  }

}
