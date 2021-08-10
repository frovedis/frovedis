#ifndef _FP_GROWTH_MODEL_
#define _FP_GROWTH_MODEL_

//#define FP_DLOG

#include <frovedis/dataframe.hpp>

namespace frovedis {
  struct association_rule{
    association_rule() {}
    association_rule(const std::vector<dftable>& rs): rule(rs) {}
    association_rule(std::vector<dftable>&& rs) { rule.swap(rs); }
    void clear();
    void debug_print();
    size_t get_count();
    size_t get_depth();
    void save (const std::string& dir);
    void savebinary (const std::string& dir);
    void load (const std::string& dir);
    void loadbinary (const std::string& dir);
    std::vector<dftable> rule;
    SERIALIZE(rule)
  };
  
  struct fp_growth_model {
    fp_growth_model() {}
    fp_growth_model(size_t n_trans_, 
                    const std::vector<dftable>& fp_tree,
                    const std::vector<dftable>& t_info): 
      n_trans(n_trans_), item(fp_tree), tree_info(t_info) {}
    fp_growth_model(size_t n_trans_,
                    std::vector<dftable>&& fp_tree,
                    std::vector<dftable>&& t_info) {
      n_trans = n_trans_;
      item.swap(fp_tree); 
      tree_info.swap(t_info);
    }

    void clear();
    void debug_print();
    size_t get_count();
    size_t get_depth();
    dftable& get_item_support();
    std::vector<dftable> get_frequent_itemset();
    dftable transform(dftable& transactions);
    association_rule generate_rules(double confidence);
    void load (const std::string& dir);
    void loadbinary (const std::string& dir);
    void save (const std::string& dir);
    void savebinary (const std::string& dir);

    size_t n_trans;
    std::vector<dftable> item, tree_info;
    dftable item_support;
    association_rule rules;
    SERIALIZE(n_trans, item, tree_info, item_support, rules)
  };
  
  void free_df(dftable_base&);
  int len (size_t num);
}
#endif
