#ifndef _FP_GROWTH_MODEL_
#define _FP_GROWTH_MODEL_

//#define FP_DEBUG

#include <frovedis/dataframe.hpp>

namespace frovedis {
  struct association_rule{
    association_rule() {}
    association_rule(std::vector<dftable>& rs): rule(rs){}
    void debug_print();
    void save (const std::string& fname);
    void savebinary (const std::string& fname);
    void load (const std::string& fname);
    void loadbinary (const std::string& fname);
    std::vector<dftable> rule;
    SERIALIZE(rule)
  };
  
  struct fp_growth_model {
    fp_growth_model() {}
    fp_growth_model(std::vector<dftable>& fp_tree,
                    std::vector<dftable>& tree_info): 
                    item(fp_tree), tree_info(tree_info)  {}
    void debug_print();
    size_t get_count();
    size_t get_depth();
    void clear();
    std::vector<dftable> get_frequent_itemset();
    association_rule generate_rules(double confidence);
    void load (const std::string& dir);
    void loadbinary (const std::string& dir);
    void save (const std::string& dir);
    void savebinary (const std::string& dir);

    std::vector<dftable> item, tree_info;
    SERIALIZE(item, tree_info)
  };
  
  dftable create_antacedent(dftable, int);
  dftable calculate_confidence(dftable&, dftable&, double);
  association_rule 
  generate_association_rules(std::vector<dftable>& freq_itemsets,
                             double con);
  void free_df(dftable_base&);

  template <class T>
  void show(const std::string& msg,
            const std::vector<T>& vec,
            const int& limit = 10) {
    std::cout << msg; debug_print_vector(vec, limit);
  }

}
#endif
