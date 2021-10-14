#include <cmath>
#include "fp_growth_model.hpp"

namespace frovedis {

  void free_df(dftable_base& df) { dftable tmp; df = tmp; }
  int len (size_t num) { return num == 0 ? 1 : trunc(log10(num)) + 1; } 

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
    free_df(item_support);
    n_trans = 0;
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
    if (item_support.num_row()) {
      std::cout << "--- item_support ---\n"; 
      item_support.show(); std::cout << std::endl;
    }
    for (size_t i = 0; i < item.size(); ++i) {
      std::cout << "--- tree[" << i << "] ---\n";
      item[i].show(); std::cout << std::endl;
      std::cout << "--- tree_info[" << i << "] ---\n";
      tree_info[i].show(); std::cout << std::endl;
    }
    std::cout << "total #FIS: " << get_count() << "\n";
  }
 
  void fp_growth_model::save (const std::string& dir) {
    require(!directory_exists(dir),
    "save: a directory with same name already exists!\n");
    make_directory(dir);
    size_t precision = 6;
    std::string datetime_fmt = "%Y-%m-%d";
    std::string sep = " ";
    dftable old_info;
    auto depth = item.size();
    for (size_t i = 0; i < depth; ++i) {
      auto part_dname = dir + "/tree_" + STR(i, len(depth));
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
    std::string metadata = dir + "/metadata";
    std::ofstream mdstr;
    mdstr.exceptions(std::ofstream::failbit | std::ofstream::badbit);
    mdstr.open(metadata.c_str());
    mdstr << n_trans << "\n" << get_count() << std::endl;    
  }

  void fp_growth_model::savebinary (const std::string& dir) {
    require(!directory_exists(dir),
    "save: a directory with same name already exists!\n");
    make_directory(dir);
    dftable old_info;
    auto depth = item.size();
    for (size_t i = 0; i < depth; ++i) {
      auto part_dname = dir + "/tree_" + STR(i, len(depth));
      auto fis = decompress_impl(item[i], tree_info[i], old_info);
      fis.save(part_dname);
    }
    std::string metadata = dir + "/metadata";
    std::ofstream mdstr;
    mdstr.exceptions(std::ofstream::failbit | std::ofstream::badbit);
    mdstr.open(metadata.c_str());
    mdstr << n_trans << "\n" << get_count() << std::endl;    
  }

  void fp_growth_model::load (const std::string& dir) {
    require(directory_exists(dir), "load: directory does not exist!\n");
    auto depth = count_files_with_regex(dir, "(tree)(.*)");
    clear();
    std::string tmp;
    std::vector<dftable> tree_item(depth), tree_info(depth);
    for(size_t i = 0; i < depth; ++i) {
      auto tree_data = dir + "/tree_" + STR(i, len(depth)) + "/data";
      auto tree_schema = dir + "/tree_" + STR(i, len(depth)) + "/schema";
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
    size_t count = 0;
    std::string metadata = dir + "/metadata";
    std::ifstream mdstr(metadata.c_str()); mdstr >> n_trans >> count;    
    RLOG(INFO) << "load: tree-depth found: " << depth 
               << "; #FIS: " << count << std::endl;
    *this = fp_growth_model(n_trans, std::move(tree_item), 
                            std::move(tree_info));
  }

  void fp_growth_model::loadbinary (const std::string& dir) {
    require(directory_exists(dir), "load: directory does not exist!\n");
    auto depth = count_files_with_regex(dir, "(tree)(.*)");
    clear();
    std::vector<dftable> tree_item(depth), tree_info(depth);
    for(size_t i = 0; i < depth; ++i) {
      auto tree_dir = dir + "/tree_" + STR(i, len(depth));
      tree_item[i] = dftable();
      tree_item[i].load(tree_dir);
      tree_info[i] = dftable(); // empty info (since decompressed tree is saved)
    }
    size_t count = 0;
    std::string metadata = dir + "/metadata";
    std::ifstream mdstr(metadata.c_str()); mdstr >> n_trans >> count;    
    RLOG(INFO) << "load: tree-depth found: " << depth 
               << "; #FIS: " << count << std::endl;
    *this = fp_growth_model(n_trans, std::move(tree_item), 
                            std::move(tree_info));
  }

  struct conf_helper {
    double operator()(size_t ucnt, size_t cnt) { 
      return static_cast<double>(ucnt) / static_cast<double>(cnt);
    }
    SERIALIZE_NONE
  };

  struct lift_helper_check_divzero {
    double operator()(double conf, double consequentSupport) { 
      if (consequentSupport == 0.0) return std::numeric_limits<double>::max();
      else return conf / consequentSupport;
    }
    SERIALIZE_NONE
  };

  struct lift_helper {
    double operator()(double conf, double consequentSupport) { 
      return conf / consequentSupport;
    }
    SERIALIZE_NONE
  };

  struct support_helper {
    support_helper() {}
    support_helper(size_t n): n_trans(n) {}
    double operator()(size_t ucnt) { 
      return static_cast<double>(ucnt) / static_cast<double>(n_trans);
    }
    size_t n_trans;
    SERIALIZE(n_trans)
  };

  struct conviction_helper {
    double operator()(double consequentSupport, double conf) { 
      if (conf == 1.0) return std::numeric_limits<double>::max();
      return ((1.0 - consequentSupport) / (1.0 - conf));
    }
    SERIALIZE_NONE
  };

  void swap_col_names(dftable& df, 
                      const std::string& c1, 
                      const std::string& c2) {
    // assumes c1 and c2 are existing columns
    df.rename(c1, "tmp")  // c1, c2 -> tmp, c2
      .rename(c2, c1)     // tmp, c2 -> tmp, c1
      .rename("tmp", c2);  // tmp, c1 -> c2, c1
  }

  dftable join_pre_post(dftable& pre, dftable& post,
                        const std::vector<std::string>& opt_left,
                        const std::vector<std::string>& opt_right,
                        const std::vector<std::string>& select_targets) {
    dftable ret;
    if(post.num_row() > pre.num_row()) {
      // std::cout << "join: " << post.num_row() << " x " << pre.num_row() << "\n";
      ret = post.bcast_join(pre, multi_eq(opt_right, opt_left))
                .select(select_targets);
    }
    else {
      // std::cout << "join: " << pre.num_row() << " x " << post.num_row() << "\n";
      ret = pre.bcast_join(post, multi_eq(opt_left, opt_right))
               .select(select_targets);
    }
    return ret;
  }

  // https://en.wikipedia.org/wiki/Association_rule_learning#Confidence
  filtered_dftable 
  calculate_confidence(dftable& rule,
                       double min_confidence,
                       std::string& confidence, 
                       std::string& union_count,
                       std::string& count) {
    // adding confidence column in rule: confidence = union_count / count
    rule.calc<double,size_t,size_t>(confidence, conf_helper(),
                                    union_count, count); 
    return rule.filter(ge_im(confidence, min_confidence)); 
  }

  // https://en.wikipedia.org/wiki/Association_rule_learning#Lift
  dftable calculate_other_measures(filtered_dftable& with_conf,
                                   dftable& item_support,
                                   const std::shared_ptr<dfoperator>& opt,
                                   std::vector<std::string> select_targets,
                                   std::string& lift,
                                   std::string& confidence,
                                   std::string& cons_support,
                                   std::string& support,
                                   std::string& union_count,
                                   size_t n_trans,
                                   std::string& conviction, 
                                   bool check_divzero) {
    dftable ret; // ant1, ..., antN, consequent, union_count, confidence, cons_support
    ret = with_conf.bcast_join(item_support, opt).select(select_targets);
    // adding 'lift' column in ret: lift = confidence / cons_support
    if (check_divzero) {
      ret.calc<double,double,double>(lift, lift_helper_check_divzero(),
                                     confidence, cons_support, true); 
    }
    else {
      ret.calc<double,double,double>(lift, lift_helper(), 
                                     confidence, cons_support);
    }
    // adding 'support' column in ret: support = union_count / n_trans
    ret.calc<double,size_t>(support, support_helper(n_trans), union_count);
    // adding 'conviction' column in ret: conviction = (1 - cons_support) / (1 - confidence)
    ret.calc<double,double,double>(conviction, conviction_helper(),
                                   cons_support, confidence, true);
    ret.drop(union_count).drop(cons_support); // ant1, ..., antN, consequent, confidence, lift, support, conviction
    return ret;
  }
    
  void generate_association_rules_helper(dftable& pre, dftable& post,
                                         double min_conf, int depth_level,
                                         dftable& item_support, 
                                         size_t n_trans,
                                         bool check_divzero,
                                         std::vector<dftable>& result) {
    std::string count = "count";
    std::string union_count = "union_count";
    std::string consequent = "consequent";
    std::string confidence = "confidence";
    std::string antecedent = "antecedent";
    std::string lift = "lift";
    std::string support = "support";
    std::string conviction = "conviction";
    std::string cons_support = "item_support";
    auto pre_col = pre.columns();
    auto post_col = post.columns();
    auto iter = pre_col.size() - 1;
    auto post_ncol = post_col.size();
    std::vector<std::string> opt_left(iter), opt_right(iter);
    std::vector<std::string> targets1(post_ncol + 1); 
    std::vector<std::string> targets2(post_ncol + 2); 
    for(size_t i = 0; i < iter; ++i) {
      opt_left[i] = pre_col[i];
      auto ant_name = antecedent + STR(i + 1);
      opt_right[i] = targets1[i] = targets2[i] = ant_name;
    }
    targets1[iter] = targets2[iter] = consequent; // ant1, ... antN, consequent
    targets1[iter + 1] = union_count; 
    targets1[iter + 2] = count;      // + union_count, count
    targets2[iter + 1] = union_count; 
    targets2[iter + 2] = confidence; 
    targets2[iter + 3] = cons_support; // + union_count, confidence, cons_support
    #ifdef FP_DLOG
      show("opt-left: ", opt_left);
      show("opt-right: ", opt_right);
      show("select-targets1: ", targets1);
      show("select-targets2: ", targets2);
    #endif

    // renaming post for mining
    post.rename(post_col[0], consequent); // item -> consequent
    // item1 -> antecedent1; item2 -> antecedent2, ...
    for (size_t i = 1; i < post_ncol - 1; ++i) {
      post.rename(post_col[i], antecedent + STR(i));
    }
    post.rename(post_col[post_ncol - 1], union_count); // count -> union_count

    for(size_t i = 0; i < post_ncol - 1; ++i) {
      if(i != 0) swap_col_names(post, consequent, antecedent + STR(i));
      // rule: ant1, ..., antN, consequent, union_count, count
      auto rule = join_pre_post(pre, post, opt_left, opt_right, targets1);
      // with_conf: ant1, ..., antN, consequent, union_count, count, confidence
      auto with_conf = calculate_confidence(rule, min_conf, confidence,
                                            union_count, count);
      // rule: ant1, ..., antN, consequent, confidence, lift, support, conviction
      rule = calculate_other_measures(with_conf, item_support,
                                      eq(consequent, "item"), targets2,
                                      lift, confidence, cons_support,
                                      support, union_count, n_trans, 
                                      conviction, check_divzero);
      #ifdef FP_DLOG
        std::cout << "--- [depth: " << depth_level 
                  << "; iter: " << i 
                  << "] rule --- \n"; rule.show();
        std::cout << " --- Pre ---\n";   pre.show();
        std::cout << " --- Post ---\n";  post.show();
      #endif
      if (rule.num_row()) result.push_back(std::move(rule));
    }

    // post is renaming back to original names
    auto tmp_post_col = post.columns(); // renamed col names
    for(size_t i = 0; i < post_ncol; ++i) {
      post.rename(tmp_post_col[i], post_col[i]);
    }
  }

  association_rule
  generate_association_rules(std::vector<dftable>& item,
                             std::vector<dftable>& tree_info,
                             dftable& item_support,
                             double min_conf,
                             size_t n_trans) {
    require(min_conf >= 0.0 && min_conf <= 1.0, 
    "generate_rules: min-confidence should be in between 0 to 1.\n");
    std::vector<dftable> rules;
    auto depth = item.size();
    if (depth <= 1) return rules;
    try {
      // divzero might occur when some item has 
      // very less ocuurence (C) in high no. of transactions (N)
      // and provided min-support is also very small (maybe zero)
      // C << N (C / N can be close to zero or zero)
      auto check_divzero = item_support.min<double>("item_support") == 0.0;
      dftable old_info;
      auto pre_fis = decompress_impl(item[0], tree_info[0], old_info);
      for(size_t i = 1; i < depth; ++i) {
        auto post_fis = decompress_impl(item[i], tree_info[i], old_info);
        generate_association_rules_helper(pre_fis, post_fis, min_conf, i, 
                                          item_support, n_trans,
                                          check_divzero, rules);
        pre_fis = std::move(post_fis);
      }
    }
    catch(std::exception& excpt) {
      std::string msg = excpt.what();
      if(msg.find("bad_alloc") != std::string::npos ) {
        std::string e = "out-of-memory error occured during rule mining!\n";
        e += "retry building tree with higher min_support value.\n";
        REPORT_ERROR(INTERNAL_ERROR, e);
      }
      else REPORT_ERROR(INTERNAL_ERROR, msg);
    }
    return association_rule(rules);
  }

  dftable& fp_growth_model::get_item_support() {
    require(!item.empty(), 
    "get_item_support: attribute is available only after fit!\n");
    if(item_support.num_row() == 0) { // first-time computation only
      item_support = item[0];
      item_support.calc<double,size_t>
                    ("item_support", 
                     support_helper(n_trans),
                     "count") // item_support = count / n_trans
                  .drop("count");
    }
    return item_support; // item, item_support
  }
  
  dftable fp_growth_model:: transform(dftable& trans){
    std::vector<dftable> trans_table_v;
    std::vector<dftable> res_table;
    dftable x;
    for (auto& rule: rules.rule) {
      //antacedent columns = total number of columns - (consequent, confidence,
      //                                                lift, support, 
      //                                                conviction)
      auto ant_n = rule.num_col() - 5; // #antecedent columns
      std::vector<std::string> ant_cols(ant_n);
      res_table.clear();
      auto tmp = rule.columns();
      for (size_t i = 0; i < ant_n; ++i) ant_cols[i] = tmp[i];
      std::vector<std::string> targets = {"trans_id", "consequent"};
      // targets = trans_id consequent <all antecedent columns except for the 
      //                                first, as it would no longer be needed 
      //                                in further calculations>
      targets.insert(std::end(targets), std::begin(ant_cols) + 1, 
                     std::end(ant_cols));

      // transactions = trans_id item rank
      // trans = trans_id item
      // rule = <antecedents> consequent confidence lift support conviction
      auto j_df = trans.bcast_join(rule, eq("item", "antecedent1"))
                   .filter(neq("item", "consequent"))
                   .select(targets)
                   .rename("trans_id", "trans_id_");
      for (size_t i = 1; i < ant_n; ++i) {
        targets = {"trans_id_", "consequent"};
        targets.insert(std::end(targets), 
                       std::begin(ant_cols) + i + 1, 
                       std::end(ant_cols));
        j_df = trans.bcast_join(j_df, eq("item", ant_cols[i]))
                .filter(neq("item", "consequent"))
                .filter(eq("trans_id", "trans_id_"))
                .select(targets);
        if (j_df.num_row() == 0) {
          //No further computation possible, this antecedent group in rules 
          //does not lead to any possible predictions.
          break; //break if table empty.
        }
      }
      if (j_df.num_row() > 0) {
        j_df.append_rowid("id");
        auto x = trans.bcast_join(j_df, 
                                  multi_eq({"trans_id", "item"}, 
                                   {"trans_id_", "consequent"}))
                      .select({"id"});
        j_df.rename("trans_id_", "trans_id");
        if (x.num_row() > 0) {
          auto id = x.column("id")->as_dvector<size_t>().gather();
          j_df = j_df.drop_rows("id", id);
        }
        j_df.drop("id");
      }
      if (j_df.num_row() > 0) trans_table_v.push_back(j_df.align_block()); 
    }
    if (trans_table_v.size()) {
      x = trans_table_v.back();
      trans_table_v.pop_back();
      x = x.union_tables(trans_table_v, false);
      //group_by & select
      return x.distinct();
    } 
    else {
      dftable dummy;
      std::vector<int> v = {};
      auto dv = make_dvector_scatter(v);
      dummy.append_column("trans_id", dv);
      dummy.append_column("consequent", dv);
      return dummy;
    }
  }


  association_rule fp_growth_model::generate_rules(double min_conf) {
    require(!item.empty(),
    "generate_rules: can be called only after fit!\n");
    auto item_support = get_item_support();
    rules =  generate_association_rules(item, tree_info, item_support,
                                      min_conf, n_trans);
    return rules;
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
      rule[i].show(); std::cout << std::endl; 
    }
  }
  
  void association_rule::save (const std::string& dir) {
    require(!directory_exists(dir),
    "save: a directory with same name already exists!\n");
    make_directory(dir);
    size_t precision = 6;
    std::string datetime_fmt = "%Y-%m-%d";
    std::string sep = " ";
    auto rule_depth = rule.size();
    for (size_t i = 0; i < rule_depth; ++i) {
      auto part_dname = dir + "/rule_" + STR(i, len(rule_depth));
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
    auto rule_depth = rule.size();
    for (size_t i = 0; i < rule_depth; ++i) {
      auto part_dname = dir + "/rule_" + STR(i, len(rule_depth));
      rule[i].save(part_dname);
    }
  }
  
  void association_rule::load (const std::string& dir) {
    require(directory_exists(dir), "load: directory does not exist!\n");
    auto depth = count_files_with_regex(dir, "(rule)(.*)");
    RLOG(INFO) << "load: set of rules found: " << depth << std::endl;
    clear();  this->rule.resize(depth);
    std::string tmp;
    for(size_t i = 0; i < depth; ++i) {
      auto rule_data = dir + "/rule_" + STR(i, len(depth)) + "/data";
      auto rule_schema = dir + "/rule_" + STR(i, len(depth)) + "/schema";
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
    auto depth = count_files_with_regex(dir, "(rule)(.*)");
    RLOG(INFO) << "load: set of rules found: " << depth << std::endl;
    clear();  this->rule.resize(depth);
    for(size_t i = 0; i < depth; ++i) {
      auto rule_dir = dir + "/rule_" + STR(i, len(depth));
      rule[i] = dftable();
      rule[i].load(rule_dir);
    }
  }

}
