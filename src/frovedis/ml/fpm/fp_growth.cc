#include <frovedis.hpp>
#include <frovedis/dataframe.hpp>
#include <algorithm>
#include <frovedis/dataframe/dftable_to_dvector.hpp>

#include <sys/sysinfo.h>
#include <string>
#include <stdint.h>
#include <assert.h>

#include "fp_growth_model.hpp"
#include "fp_growth.hpp"

//#define DEBUG

namespace frovedis{

  struct key{
      size_t operator()(uint32_t a, uint8_t b, uint8_t c=0, uint8_t d=0){
          uint64_t n = a;
          /*assert(b & 0xFF00 == 0);
          assert(c & 0xFF00 == 0);
          assert(d & 0xFF00 == 0);*/
          n = n << 32 | d << 24 | c << 16 | b;
          return n;
      }
      SERIALIZE_NONE
  };
  
  static std::vector<std::string>
  get_target_columns(std::vector<std::string>& cols){
    auto sz = cols.size();
    std::vector<std::string> target_cols(sz-2);
    for(size_t i = 0; i < cols.size()-2; ++i){
      target_cols[i] = cols[i+1];
    }
#ifdef DEBUG
    for(size_t i = 0; i < target_cols.size(); ++i){
       std::cout<<target_cols[i]<<"\n";
    }
#endif
    return target_cols;
  }
  
  static dftable copy_and_rename_df(dftable& df, int iter){
    auto copy_df = df;
    auto cols = df.columns();
    auto ncol = cols.size();
    checkAssumption(ncol > 3);
    std::string new_name;
    for(size_t i = 0; i < ncol-3; ++i){
      new_name = cols[i] + "_";
      copy_df.rename(cols[i], new_name);
    }
    new_name = "item" + std::to_string(iter-1);
    copy_df.rename(cols[ncol-3], new_name);
    new_name = "rank" + std::to_string(iter-1);
    copy_df.rename(cols[ncol-2], new_name);
    new_name = "hash_val" + std::to_string(iter-1);
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
      if(i.compare(0, 8, "hash_val") == 0){
        tmp += 64;
      }else if(i.compare(0, 8, "trans_id") == 0){
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
    std::cout<<"\n"<<name<<"TABLE: \nRows-> "<<n_row<<"\nSize(MB)-> " \
	         <<(double)get_table_size(n_row, cols)/MB<<"\nUsed Sys Mem(MB)-> " \
			 <<(double)(mem.totalram-mem.freeram)/MB<<std::endl;
  }

  static void fp_growth_self_join_gt_4(dftable& df, \
                                       dftable& res, int iter){
      auto cols_left  = df.columns();
      auto ncol = cols_left.size(); 
      std::vector<std::string> select_targets(ncol+1);
      for(size_t i = 0; i < ncol-1; ++i) select_targets[i] = cols_left[i];
      auto dfh =  df.calc<size_t,int,int,int,int>(std::string("hash_val"),key(),
                                               std::string(cols_left[0]),
                                               std::string(cols_left[1]),
                                               std::string(cols_left[2]),
                                               std::string(cols_left[3]));
#ifdef DEBUG
      print_stats("Hashed ", dfh.num_row(), dfh.columns());
      dfh.show();
#endif
      auto jcopy_df = copy_and_rename_df(dfh, iter+1);
      auto jcols_left  = dfh.columns();
      auto jcols_right = jcopy_df.columns();
      auto jncol = jcols_left.size();
      auto joined_df = \
	   dfh.bcast_join(jcopy_df, eq(jcols_left[jncol-1], jcols_right[jncol-1]));
#ifdef DEBUG
      print_stats("Joined ", joined_df.num_row(), joined_df.columns());
      joined_df.show();
#endif
      free_df(dfh);
      free_df(jcopy_df);
#ifdef DEBUG
      std::cout<<"jcols_left[4] -> "<<jcols_left[4]<<" jcols_right[4] -> " \
			   <<jcols_right[4]<<std::endl;
#endif
      //filter on item3 and item3_
      auto filtered_df = joined_df.filter(eq(jcols_left[4], jcols_right[4]));
      free_df(joined_df);
      int start = 4;
      size_t  eq_time = iter - start;
      for(size_t i = 0 ;i < eq_time ; i++){
#ifdef DEBUG
        std::cout<<"jcols_left[i+start] -> " \
		         <<jcols_left[i+start]<<" jcols_right[i+start] -> " \
				 <<jcols_right[i+start]<<std::endl;
#endif
        //filter equal items from corresponding item cols in both tables
        filtered_df  = \
		 filtered_df.filter(eq(jcols_left[i+start], jcols_right[i+start]));
      }
#ifdef DEBUG
      std::cout<<"jcols_left[jncol-3] -> " \
	           <<jcols_left[jncol-3]<<" jcols_right[jncol-3] -> " \
			   <<jcols_right[jncol-3]<<std::endl;
#endif
      //filter on unequal item4 and item5 and greater rank
      auto  df_filtered =  \
	   filtered_df.filter(neq(jcols_left[jncol-3], jcols_right[jncol-3])).
           filter(gt(jcols_left[jncol-2], jcols_right[jncol-2]));
      free_df(filtered_df);
      select_targets[ncol-1] = jcols_right[jncol-3];
      select_targets[ncol] = jcols_right[jncol-2];
      res = df_filtered.select(select_targets);
      free_df(df_filtered);
#ifdef DEBUG
      std::cout<<"\nRES TABLE: "<<res.num_row()<<std::endl;
      res.show();
#endif
  }

  static void fp_growth_self_join_le_4(dftable df, \
                                       dftable& res, int iter){
      dftable dfh;
      auto cols_left  = df.columns();
      auto ncol = cols_left.size(); 
      std::vector<std::string> select_targets(ncol+1);
      for(size_t i = 0; i < ncol-1; ++i) select_targets[i] = cols_left[i];
      if(iter == 2){
        dfh =  df.calc<size_t,int,int>(std::string("hash_val"),key(),
                                                 cols_left[0],
                                                 cols_left[1]);
      }else if(iter == 3){
        dfh =  df.calc<size_t,int,int,int>(std::string("hash_val"),key(),
                                                 std::string(cols_left[0]),
                                                 std::string(cols_left[1]),
                                                  std::string(cols_left[2]));
      }else if(iter == 4){
        dfh =  df.calc<size_t,int,int,int,int>(std::string("hash_val"),key(),
                                                 std::string(cols_left[0]),
                                                 std::string(cols_left[1]),
                                                 std::string(cols_left[2]),
                                                 std::string(cols_left[3]));
      }
#ifdef DEBUG
      print_stats("Hashed ", dfh.num_row(), dfh.columns());
      dfh.show();
#endif
      auto jcopy_df = copy_and_rename_df(dfh, iter+1);
      auto jcols_left  = dfh.columns();
      auto jcols_right = jcopy_df.columns();
      auto jncol = jcols_left.size();
#ifdef DEBUG
      std::cout<<"\nJOINING ON: jcols_left[jncol-1], jcols_right[jncol-1]" \
	           <<jcols_left[jncol-1]<<" jcols_right[jncol-1] " \
			   <<jcols_right[jncol-1]<<std::endl;
#endif
      auto joined_df = \
	   dfh.bcast_join(jcopy_df, eq(jcols_left[jncol-1], jcols_right[jncol-1]));
#ifdef DEBUG
      print_stats("Joined ", joined_df.num_row(), joined_df.columns());
      joined_df.show();
#endif
      free_df(dfh);
      free_df(jcopy_df);
      //filtering on item col and greater rank
      //filter on unequal item4 and item5 and greater rank
      /*auto filtered_df = \
	      joined_df.filter(neq(jcols_left[jncol-3], jcols_right[jncol-3])).
          filter(gt(jcols_left[jncol-2], jcols_right[jncol-2]));
      free_df(filtered_df);*/
      auto filtered_df1 = \
	   joined_df.filter(neq(jcols_left[jncol-3], jcols_right[jncol-3]));
      free_df(joined_df);
      //filtering on rank
      auto filtered_df = \
	   filtered_df1.filter(gt(jcols_left[jncol-2], jcols_right[jncol-2]));
      free_df(filtered_df1);
      select_targets[ncol-1] = jcols_right[jncol-3];
      select_targets[ncol] = jcols_right[jncol-2];
      auto f_cols = filtered_df.columns();
      res = filtered_df.select(select_targets);
      free_df(filtered_df);
#ifdef DEBUG
      std::cout<<"\nRES TABLE: "<<res.num_row()<<std::endl;
      res.show();
#endif
  }

  static dftable fp_growth_self_join(dftable& df, int iter){
#ifdef DEBUG
    std::cout<<"\nNITER====="<<iter;
#endif
    dftable res;
    //implementation to reduce join over head
    if(iter>4){
      fp_growth_self_join_gt_4(df, res, iter);
    }else{
      fp_growth_self_join_le_4(df, res, iter);
    }
    return res; 
  }
  
  static std::vector<dftable>
  generate_tables(dftable df,size_t support){ 
  //pass-by-value to avoid changes in input dftable
#ifdef DEBUG
    std::cout << "enerating generate_tables\n";
#endif
    int niter = 2;
    std::vector<dftable> ret;
    if(df.num_row()) ret.push_back(df);
    while(df.num_row()){
      df = fp_growth_self_join(df, niter++);
#ifdef DEBUG
      std::cout<<"TABLE BEFORE COUNT\n";
      df.show();
#endif
      if(df.num_row()) ret.push_back(df);
    }
    return ret;
  }
  
  fp_growth_model grow_fp_tree(dftable& t, double support_e){
    std::vector<dftable> freq_itemsets;
    auto col_list = t.columns();
    if(col_list.size()!=2)
         throw std::runtime_error("Number of Column Must be two");
    if(col_list[0]!="trans_id" || col_list[1]!="item")
         throw std::runtime_error("Column name must be trans_id and item");
    auto trow = t.group_by({"trans_id", "item"})
                         .select({"trans_id", "item"}, {count_as("item","count")})
                         .max<size_t>("count");
    if(trow > 1){
      throw std::runtime_error("Element must be unique in a transaction");
    }
    if((support_e < 0) || (support_e > 1)){
      throw std::runtime_error("Support value must range between 0 and 1.");
    }
    size_t support = ceil(t.max<int>("trans_id") * support_e);
#ifdef DEBUG
    std::cout<<"SUPPORT: "<<support<<std::endl;
#endif  
    // getting the frequent item names with their frequency rank in order
    auto item_count = t.group_by({"item"})
                       .select({"item"},{count_as("item","count")})
                       .filter(ge_im("count",support))
                       .sort_desc("count")
                       .materialize();
    freq_itemsets.push_back(item_count);
    item_count = item_count.append_rowid("rank");
#ifdef DEBUG
    std::cout << "Table T:\n";
    t.show();
    std::cout << "Item_count\n";
    item_count.show();
#endif  
    // getting the ordered item set
    auto ordered_itemset = t.bcast_join(item_count.rename("item","item_join"), 
                                  eq("item","item_join"))
                            .select({"trans_id","item","rank"})
                            .sort("trans_id");
#ifdef DEBUG
    std::cout << "Ordered_itemset done\n";
    ordered_itemset.show();
#endif  
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
#ifdef DEBUG
    std::cout<<"COMBINATION TABLE"<<"\n";
    combination.show();
#endif  
    auto join_df = generate_tables(combination,support);
    for(auto& each: join_df){ // parsing all joined dftables
      auto cols = each.columns();    
      auto tcols = get_target_columns(cols);  
      auto fitem = each.group_by(tcols)
                       .select(tcols, {count_as(tcols[tcols.size()-1],"count")})
                       .filter(ge_im("count",support))
                       .materialize();
      if(fitem.num_row()) freq_itemsets.push_back(fitem);
    }
    free_df(combination);
    for(auto& each: join_df){
      free_df(each);
    }
    return fp_growth_model(freq_itemsets); 
  }

  association_rule
  generate_association_rules(std::vector<dftable>& freq_itemsets, double con){
    std::vector<dftable> ass_rule;
#ifdef DEBUG
    std::cout<<"\nNUM FREQ ITEM SETS: "<<freq_itemsets.size()<<std::endl;
    std::cout<<"\nFREQ ITEM SETS:\n";
    for(dftable i:freq_itemsets){
        i.show();
    }
#endif
    for(size_t i = 1; i < freq_itemsets.size(); ++i){ 
	//freq-itemset 1 will not be considered
       auto n = freq_itemsets[i].num_col();
       for(size_t j=0; j < n-1; j++){
         auto res =  create_antacedent(freq_itemsets[i], j);
         auto res_con = calculate_confidence(freq_itemsets[i-1],res,con);
         if(res_con.num_row()){
           ass_rule.push_back(res_con);
         }
       }
     }
     return association_rule(ass_rule);
  }
}
