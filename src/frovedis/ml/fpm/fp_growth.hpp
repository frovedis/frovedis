#ifndef _FP_GROWTH_
#define _FP_GROWTH_

#include <frovedis/dataframe.hpp>
#include "fp_growth_model.hpp"

namespace frovedis {
  fp_growth_model
  grow_fp_tree(dftable& df, double min_support);

  association_rule
  generate_association_rules(std::vector<dftable>& freq_itemsets,
                             double con);
  template <class T>
  std::vector<std::pair<std::vector<T>, long>>
  frovedis_to_spark_model(std::vector<dftable>& freq_itemset );

  template <class T>
  std::vector<std::pair<std::vector<T>,std::pair<T, double>>>
  frovedis_to_spark_ass_rule(std::vector<dftable>& freq);
} 
#endif
