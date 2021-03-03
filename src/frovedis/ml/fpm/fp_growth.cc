#include <sys/sysinfo.h>
#include "fp_growth.hpp"

namespace frovedis {

dftable copy_and_rename_df(dftable& df, int iter) {
  auto copy_df = df;
  auto cols = df.columns();
  auto ncol = cols.size();
  std::string new_name;
  checkAssumption(ncol >= 3); // starts from ordered_set
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

dftable fp_growth_self_join(dftable& df, int iter, bool to_compress_out) {
  auto copy_df = copy_and_rename_df(df, iter);
  auto cols_left  = df.columns();
  auto cols_right = copy_df.columns();
  auto ncol = cols_left.size();
  std::vector<std::string> opt_left, opt_right;
  if ((iter < 3) || (!to_compress_out)) {
    opt_left.resize(iter);
    opt_right.resize(iter);
    for (size_t i = 0 ; i < iter; i++) {
      opt_left[i] = cols_left[i];
      opt_right[i] = cols_right[i];
    }
  }
  else {
    opt_left = {cols_left[0], cols_left[1]}; 
    opt_right = {cols_right[0], cols_right[1]}; 
  }
  auto filtered_df = df.bcast_join(copy_df, multi_eq(opt_left, opt_right))
                       .filter(gt(cols_left[ncol-1], cols_right[ncol-1])); //.materialize();
  free_df(copy_df);
  std::vector<std::string> select_targets(ncol + 1);
  for(size_t i = 0; i < ncol - 1; ++i) select_targets[i] = cols_left[i];
  select_targets[ncol - 1] = cols_right[ncol - 2];
  select_targets[ncol] = cols_right[ncol - 1];
  return filtered_df.select(select_targets)
                    .rename(select_targets[ncol], "rank");
}

std::vector<std::string> 
get_target_cols(std::vector<std::string>& cols) {
  auto sz = cols.size();
  std::vector<std::string> target_cols(sz - 2);
  for(size_t i = 0; i < sz - 2; ++i) target_cols[i] = cols[i + 1];
  return target_cols;
}

void compress_dftable(dftable& input, int niter, 
                      dftable& compressed_info) {
  if (niter < 2) return;
  auto cols = input.columns();
  if (niter > 2) { //already a compressed table with 'cid' column
    input.rename(cols[1], cols[1] + "_prev"); // cid -> cid_prev
    cols[1] = cols[1] + "_prev";
  }
  auto n_cols = cols.size();
  auto c1 = cols[1]; // item
  auto c2 = cols[2]; // item1
  auto c1_ = c1 + "_"; // for renaming
  auto c2_ = c2 + "_"; // for renaming
  auto tid = cols[0]; // tid
  auto item = cols[n_cols - 2]; //item2
  auto rank = cols[n_cols - 1]; //rank
  compressed_info = input.group_by({c1, c2}).select({c1, c2}).append_rowid("cid");
  input.rename(c1, c1_).rename(c2, c2_);
  input = input.bcast_join(compressed_info, multi_eq({c1_, c2_}, {c1, c2}))
               .select({tid, "cid", item, rank});
}

fp_growth_model 
generate_tables(dftable& item_count, 
                dftable& df, size_t support,
                bool to_compress_out,
                int mem_opt_level = 0) {
  /*
   * This function attempts to generate all possible frequent itemsets
   * by performing self join on input 'df' till the resultant join table 
   * becomes empty. The table structure can be visualized as follows:
   *
   * iter 0: tid item rank 
   * iter 1: tid item item1 rank
   * iter 2: tid item item1 item2 rank
   * iter 3: tid item item1 item2 item3 rank
   * iter 4: tid item item1 item2 item3 item4 rank
   * :
   *
   * if to_compress_out = true, then the above tables will be compressed 
   * from iteration 2 as follows:
   *
   * iter 0: tid item rank       (tree_info: empty)
   * iter 1: tid item item1 rank (tree_info: empty)
   * iter 2: tid cid item2 rank  (tree_info: cid -> item item1)
   * iter 3: tid cid item3 rank  (tree_info: cid -> item item1 item2)
   * iter 4: tid cid item4 rank  (tree_info: cid -> item item1 item2 item3)
   * :
   *
   */
  int niter = 0;
  dftable compressed_info;
  std::vector<dftable> tree, tree_info;
  tree.push_back(std::move(item_count));
  tree_info.push_back(std::move(compressed_info));
  time_spent gen_t(DEBUG);
  while (true) {
    niter++;
    df = fp_growth_self_join(df, niter, to_compress_out);
    if (to_compress_out) compress_dftable(df, niter, compressed_info);
    
    auto cols = df.columns();
    std::vector<std::string> tcols = get_target_cols(cols);
    auto combination = df.group_by(tcols)
                         //.select(tcols, {count_as(tcols[0], "count")})
                         .select(tcols, {count_as(tcols[tcols.size() - 1], "count")})
                         .filter(ge_im("count", support)).materialize();
    if(combination.num_row()) {
      if (mem_opt_level == 1) { // further optimizing tree only when level is set to 1
        auto sz = tcols.size();
        std::vector<std::string> opt_right(sz);
        for (size_t i = 0; i < sz; ++i) {
          opt_right[i] = tcols[i] + "_new";
          df.rename(tcols[i], opt_right[i]);
        }
        df = df.bcast_join(combination, multi_eq(opt_right, tcols)).select(cols);
        if ((niter > 1) && (to_compress_out)) {
          auto rem = combination.group_by({"cid"}).select({"cid"}).rename("cid", "cid_rem");
          auto info_cols = compressed_info.columns();
          compressed_info = compressed_info.bcast_join(rem, eq("cid", "cid_rem"))
                                           .select(info_cols);
        }
      }
      tree.push_back(std::move(combination));
      tree_info.push_back(std::move(compressed_info));
      gen_t.show(std::string("  niter-") + std::to_string(niter) + ": ");
    } 
    else  break;
  }
  return fp_growth_model(tree, tree_info);
}

fp_growth_model 
grow_fp_tree(dftable& t, 
             double min_support,
             bool to_compress_out,
             int mem_opt_level) { 
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
  auto is_unique = t.group_by({"trans_id", "item"})
                    .select({"trans_id", "item"}, {count_as("item","count")});
  if (is_unique.max<size_t>("count") > 1) {
    std::cout << "\nbelow transactions with repeated items are detected in input data: \n";
    is_unique.filter(gt_im("count", 1)).show(); std::cout << std::endl;
    REPORT_ERROR(USER_ERROR, "items must be unique in each transaction!\n");
  }

  auto n_trans = t.group_by({"trans_id"}).num_row();
  size_t support = ceil(n_trans * min_support);

  time_spent fp_t(DEBUG);
  // getting the frequent item names with their frequency rank in order
  auto item_count = t.group_by({"item"})
                     .select({"item"},{count_as("item","count")})
                     .filter(ge_im("count",support))
                     .sort_desc("count")
                     .append_rowid("rank");
  fp_t.show("item-count: ");
 
  // getting the ordered item set
  auto ordered_itemset = t.bcast_join(item_count.rename("item", "item_join"), 
                                      eq("item","item_join"))
                          .select({"trans_id","item","rank"});
  item_count.rename("item_join", "item") // renaming back
            .drop("rank");  // rank is no longer required in item_count
  fp_t.show("ordered-itemset: ");

  fp_growth_model m;
  try {
    m = generate_tables(item_count, ordered_itemset, 
                        support, to_compress_out, mem_opt_level);
  }
  catch(std::exception& excpt) {
    std::string msg = excpt.what();
    if(msg.find("bad_alloc") != std::string::npos ) {
      std::string e = "out-of-memory error occured during fp-tree construction!\n";
      if (to_compress_out) {
        if (min_support < 1.0) e += "retry with higher min support value.\n";
        REPORT_ERROR(INTERNAL_ERROR, e);
      }
      else {
        e += "retry by setting to_compression_out = 'true'.\n";
        REPORT_ERROR(INTERNAL_ERROR, e);
      }
    }
    else REPORT_ERROR(INTERNAL_ERROR, msg);
  }
  fp_t.show("generate-trees: ");
  return m;
}

}
