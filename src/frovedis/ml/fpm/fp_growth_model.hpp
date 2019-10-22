#ifndef _FP_GROWTH_MODEL_
#define _FP_GROWTH_MODEL_

#include <frovedis/dataframe.hpp>
#include <typeinfo>

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
    fp_growth_model(std::vector<dftable>& it ): item(it)  {}
    void debug_print();
    std::vector<dftable> get_frequent_itemset();
    association_rule generate_rules(double confidence);
    void load (const std::string& fname);
    void loadbinary (const std::string& fname);
    void save (const std::string& fname);
    void savebinary (const std::string& fname);
    std::vector<dftable> item;
    SERIALIZE(item)
  };
  
  dftable create_antacedent(dftable, int);
  dftable calculate_confidence(dftable&, dftable&, double);
}

#endif


