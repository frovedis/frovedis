#include <sys/sysinfo.h>
#include "fp_growth.hpp"

namespace frovedis {
  std::vector<std::string>
  get_target_columns(std::vector<std::string>& cols){
    auto sz = cols.size();
    std::vector<std::string> target_cols(sz-2);
    for(size_t i = 0; i < cols.size()-2; ++i){
      target_cols[i] = cols[i+1];
    }
    return target_cols;
  }
  
  dftable copy_and_rename_df(dftable& df, int iter){
    auto copy_df = df;
    auto cols = df.columns();
    auto ncol = cols.size();
    checkAssumption(ncol > 3);
    std::string new_name;
    for(size_t i = 0; i < ncol-2; ++i) {
      new_name = cols[i] + "_";
      copy_df.rename(cols[i], new_name);
    }
    new_name = "item" + std::to_string(iter);
    copy_df.rename(cols[ncol-2], new_name);
    new_name = "rank" + std::to_string(iter);
    copy_df.rename(cols[ncol-1], new_name);
    return copy_df;
  }
  
  void free_df(dftable_base& df){
    dftable tmp;
    df = tmp;
  }

  double get_table_size(int n_row, std::vector<std::string> cols){
    double tmp = 0;
    for (auto i : cols){
      if(i.compare(0, 8, "trans_id") == 0){
        tmp += 32;
      }else{
        tmp += 8;
      }
    }
    return tmp*n_row;
  }

  void print_stats(std::string name, int n_row, std::vector<std::string> cols){
    int MB = 1000000;
    struct sysinfo mem;
    sysinfo(&mem);
    std::cout << "\n" << name << "TABLE: \nRows-> " << n_row << "\nSize(MB)-> " \
              << (double)get_table_size(n_row, cols)/MB \
              << "\nUsed Sys Mem(MB)-> " \
              << (double)(mem.totalram-mem.freeram)/MB << std::endl;
  }

  dftable fp_growth_self_join(dftable& df, int iter) {
    auto copy_df = copy_and_rename_df(df, iter);
    auto cols_left  = df.columns();
    auto cols_right = copy_df.columns();
    std::vector<std::string> opt_left(iter), opt_right(iter);
    for (int i = 0; i < iter; ++i){
      opt_left[i] = cols_left[i];
      opt_right[i] = cols_right[i];
    }
    auto ncol = cols_left.size();
    auto filtered_df = df.bcast_join(copy_df, multi_eq(opt_left, opt_right))
                         .filter(neq(cols_left[ncol-2], cols_right[ncol-2]))  
                         .filter(gt(cols_left[ncol-1], cols_right[ncol-1])); 
    free_df(copy_df);
    std::vector<std::string> select_targets(ncol + 1);
    for(size_t i = 0; i < ncol - 1; ++i) select_targets[i] = cols_left[i];
    select_targets[ncol - 1] = cols_right[ncol - 2];
    select_targets[ncol] = cols_right[ncol - 1];
    return filtered_df.select(select_targets);
  }
  
  std::vector<dftable>
  generate_tables(dftable df,size_t support){ 
    //pass-by-value to avoid changes in input dftable
    int niter = 2;
    std::vector<dftable> ret;
    if(df.num_row()) ret.push_back(df);
    while(df.num_row()){
      df = fp_growth_self_join(df, niter++);
        if(df.num_row()) ret.push_back(df);
    }
    return ret;
  }
  
  fp_growth_model grow_fp_tree(dftable& t, double min_support) {
    auto col_list = t.columns();
    if (col_list.size() == 2) {
      require (col_list[0] == "trans_id" && col_list[1] == "item",
             "two columns detected: expected names - (trans_id, item)");
    }
    else if (col_list.size() == 3) {
      require (col_list[1] == "trans_id" && col_list[2] == "item",
             "three columns detected: expected names - (index, trans_id, item)");
    }
    else REPORT_ERROR(USER_ERROR, 
    "number of column must be two (trans_id, item) or three (index, trans_id, item)");

    require (min_support > 0.0 && min_support <= 1.0,
           "support value must be within the range of range 0 to 1.");

    // ensuring that each transaction has unique item (no repetition)
    auto n_unique = t.group_by({"trans_id", "item"})
                   .select({"trans_id", "item"}, {count_as("item","count")})
                   .max<size_t>("count") > 1;
    require (!n_unique, "element must be unique in each transaction");
    size_t support = ceil(t.max<int>("trans_id") * min_support);

    // getting the frequent item names with their frequency rank in order
    auto item_count = t.group_by({"item"})
                       .select({"item"},{count_as("item","count")})
                       .filter(ge_im("count",support))
                       .sort_desc("count")
                       .materialize();

    std::vector<dftable> freq_itemsets;
    freq_itemsets.push_back(item_count);
    item_count = item_count.append_rowid("rank");
    // getting the ordered item set
    auto ordered_itemset = t.bcast_join(item_count.rename("item","item_join"), 
                                  eq("item","item_join"))
                            .select({"trans_id","item","rank"})
                            .sort("trans_id");
    // getting the rank of most freqent items in each transaction
    auto most_frequent = \
	  ordered_itemset.group_by({"trans_id"})
      .select({"trans_id"},{min_as("rank","min_rank")});
  
    // filtering out the most frequest from ordered item set
    auto result = \
	    ordered_itemset.bcast_join(most_frequent.rename("trans_id","trans_id_join"), 
        eq("trans_id","trans_id_join"))
        .filter(neq("rank","min_rank"))
        .select({"trans_id","item","rank"});
    // combination of all items in each transaction
    auto combination = \
	    ordered_itemset.bcast_join(result.rename("trans_id","trans_id_join")
                                         .rename("item","item_base")
                                         .rename("rank","rank_base"), 
                                           eq("trans_id","trans_id_join"))
                                           .filter(gt("rank_base","rank"))
                                           .select({"trans_id","item_base","item","rank"})
                                           .sort("trans_id");
    free_df(ordered_itemset);
    try {
      auto join_df = generate_tables(combination,support);
      for(auto& each: join_df) { // parsing all joined dftables
        auto cols = each.columns();    
        auto tcols = get_target_columns(cols);  
        auto fitem = each.group_by(tcols)
                         .select(tcols, {count_as(tcols[tcols.size()-1],"count")})
                         .filter(ge_im("count",support))
                         .materialize();
        if(fitem.num_row()) freq_itemsets.push_back(fitem);
      }
    }
    catch(std::exception& excpt) {
      REPORT_ERROR(INTERNAL_ERROR, 
      "out-of-memory error occured during fp-tree construction!\n");
    }
    return fp_growth_model(freq_itemsets); 
  }

}
