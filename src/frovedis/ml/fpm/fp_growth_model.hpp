#ifndef _FP_GROWTH_MODEL_
#define _FP_GROWTH_MODEL_

//#define FP_DEBUG

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
    fp_growth_model(const std::vector<dftable>& fp_tree,
                    const std::vector<dftable>& t_info): 
      item(fp_tree), tree_info(t_info) {}
    fp_growth_model(std::vector<dftable>&& fp_tree,
                    std::vector<dftable>&& t_info) {
      item.swap(fp_tree); 
      tree_info.swap(t_info);
    }
    void clear();
    void debug_print();
    size_t get_count();
    size_t get_depth();
    std::vector<dftable> get_frequent_itemset();
    association_rule generate_rules(double confidence);
    void load (const std::string& dir);
    void loadbinary (const std::string& dir);
    void save (const std::string& dir);
    void savebinary (const std::string& dir);

    std::vector<dftable> item, tree_info;
    SERIALIZE(item, tree_info)
  };
  
  void free_df(dftable_base&);

  // show() for debugging...
  template <class T>
  void show(const std::string& msg,
            const std::vector<T>& vec,
            const int& limit = 10) {
    std::cout << msg; debug_print_vector(vec, limit);
  }

}
#endif
