#include "fp_growth.hpp"
#include <typeinfo>

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
  new_name = "item" + STR(iter);
  copy_df.rename(cols[ncol-2], new_name);
  new_name = "rank" + STR(iter);
  copy_df.rename(cols[ncol-1], new_name);
  return copy_df;
}

dftable fp_growth_self_join(dftable& df, int iter, 
                            int compression_point,
                            int nproc) {
  auto copy_df = copy_and_rename_df(df, iter);
  auto cols_left  = df.columns();
  auto cols_right = copy_df.columns();
  auto ncol = cols_left.size();
  std::vector<std::string> opt_left, opt_right;
  if (iter <= compression_point) { 
    // <= because till compression_point, there will be all columns 
    // in input df for fp_growth_self_join(...)
    opt_left.resize(iter);
    opt_right.resize(iter);
    for (size_t i = 0 ; i < iter; i++) {
      opt_left[i] = cols_left[i];
      opt_right[i] = cols_right[i];
    }
  }
  else {
    opt_left = {cols_left[0], cols_left[1]};    // trans_id, cid
    opt_right = {cols_right[0], cols_right[1]}; // trans_id_, cid_
  }
  auto filtered_df = df.bcast_join(copy_df, multi_eq(opt_left, opt_right))
                       .filter(gt(cols_left[ncol-1], cols_right[ncol-1]));
  free_df(copy_df);
  std::vector<std::string> select_targets(ncol + 1);
  for(size_t i = 0; i < ncol - 1; ++i) select_targets[i] = cols_left[i];
  select_targets[ncol - 1] = cols_right[ncol - 2];
  select_targets[ncol] = cols_right[ncol - 1];
  auto ret = filtered_df.select(select_targets)
                        .rename(select_targets[ncol], "rank");
  return (nproc > 1) ? ret.align_block() : ret;
}

std::vector<std::string> 
get_target_cols(std::vector<std::string>& cols) {
  auto sz = cols.size();
  std::vector<std::string> target_cols(sz - 2);
  for(size_t i = 0; i < sz - 2; ++i) target_cols[i] = cols[i + 1];
  return target_cols;
}

std::vector<std::string>
rename_compression_input(dftable& input, 
                         const std::vector<std::string>& cols) {
  auto ncol = cols.size();
  std::vector<std::string> ret(ncol);
  for(size_t i = 0; i < ncol; ++i) {
    ret[i] = cols[i] + "_";
    input.rename(cols[i], ret[i]); // renamed in-place
  }
  return ret;
}

void compress_dftable(dftable& input, int niter, 
                      int compression_point,
                      dftable& compressed_info) {
  if (niter < compression_point) return; // no compression - quick return
  auto cols = input.columns();
  auto ncol = cols.size();
  std::vector<std::string> targets;
  if (niter == compression_point) {
    targets.resize(compression_point);
    for(int i = 0; i < compression_point; ++i) targets[i] = cols[i + 1];
  }
  else { // already a compressed table having "cid" column
    input.rename(cols[1], cols[1] + "_prev"); // cid -> cid_prev
    targets.resize(2);
    targets[0] = cols[1] + "_prev";
    targets[1] = cols[2];
  }
  compressed_info = input.group_by(targets)
                         .select(targets).append_rowid("cid");
  auto tid = cols[0];         // trans_id
  auto item = cols[ncol - 2]; // itemK
  auto rank = cols[ncol - 1]; // rank
  auto renamed_cols = rename_compression_input(input, targets);
  input = input.bcast_join(compressed_info, multi_eq(renamed_cols, targets))
               .select({tid, "cid", item, rank});
}

fp_growth_model 
generate_tables(dftable& item_count,
                dftable& df, size_t support,
                size_t n_trans,
                int tree_depth,
                int compression_point = 4,
                int mem_opt_level = 0) {
  /*
   * This function attempts to generate all possible frequent itemsets
   * by performing self-join on input 'df' till the resultant joined table 
   * becomes empty or tree is constructed till specified 'tree_depth'. 
   * The tree schemas can be visualized as follows:
   *
   * iter 0: tid item rank 
   * iter 1: tid item item1 rank
   * iter 2: tid item item1 item2 rank
   * iter 3: tid item item1 item2 item3 rank
   * iter 4: tid item item1 item2 item3 item4 rank
   * :
   *
   * the above trees will be compressed from ith iteration point,
   * where "i" equals to user given compression_point (by default = 4). 
   * For example, if compression_point = 2, then the above trees 
   * will be compressed from iteration 2 as follows:
   *
   * iter 0: tid item rank       (tree_info: empty)
   * iter 1: tid item item1 rank (tree_info: empty)
   * iter 2: tid cid item2 rank  (tree_info: cid -> item item1)
   * iter 3: tid cid item3 rank  (tree_info: cid -> cid_prev item2 -> item item1 item2)
   * iter 4: tid cid item4 rank  (tree_info: cid -> cid_prev item3 -> item item1 item2 item3)
   * :
   *
   */
  dftable compressed_info;
  std::vector<dftable> tree, tree_info;
  tree.push_back(std::move(item_count)); // depth-0
  tree_info.push_back(std::move(compressed_info));
  auto nproc = get_nodesize();

  time_spent gen_t(DEBUG);
  for (int niter = 1; niter < tree_depth; ++niter) {
    df = fp_growth_self_join(df, niter, compression_point, nproc);
    // df would be compressed in-place if niter >= compression_point
    compress_dftable(df, niter, compression_point, compressed_info);
    
    auto cols = df.columns();
    std::vector<std::string> tcols = get_target_cols(cols);
    auto combination = df.group_by(tcols)
                         .select(tcols, {count_as(tcols[tcols.size() - 1], "count")})
                         .filter(ge_im("count", support))
                         .materialize();
    if(nproc > 1) combination.align_block();
    if(!combination.num_row()) break; // no further depth is possible
    if (mem_opt_level == 1) { // further optimizing tree only when level is set to 1
      auto sz = tcols.size();
      std::vector<std::string> opt_right(sz);
      for (size_t i = 0; i < sz; ++i) {
        opt_right[i] = tcols[i] + "_new";
        df.rename(tcols[i], opt_right[i]);
      }
      df = df.bcast_join(combination, multi_eq(opt_right, tcols)).select(cols);
      if(nproc > 1) df.align_block();
      if (niter >= compression_point) {
        auto rem = combination.group_by({"cid"}).select({"cid"})
                              .rename("cid", "cid_rem");
        auto info_cols = compressed_info.columns();
        compressed_info = compressed_info.bcast_join(rem, eq("cid", "cid_rem"))
                                         .select(info_cols);
        if(nproc > 1) compressed_info.align_block();
      }
    }
    tree.push_back(std::move(combination));
    tree_info.push_back(std::move(compressed_info));
    gen_t.show(std::string("  niter-") + STR(niter) + ": ");
  }
  return fp_growth_model(n_trans, std::move(tree), std::move(tree_info));
}

fp_growth_model 
grow_fp_tree(dftable& t, 
             double min_support,
             int tree_depth,
             int compression_point,
             int mem_opt_level) { 
  RLOG(INFO) << "fp-growth hyper-parameters:: "
             << "min_support: " << min_support
             << "; tree_depth: " << tree_depth
             << "; compression_point: " << compression_point
             << "; mem_opt_level: " << mem_opt_level << std::endl; 
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

  require (min_support >= 0.0 && min_support <= 1.0,  
           "support value must be within the range of range 0 to 1.");

  require(tree_depth >= 1, 
  "minimum allowed value for 'tree_depth' is 1.\n"); // at-least item-count table

  require(compression_point >= 2, 
  "minimum allowed value for 'compression_point' is 2.\n");

  require(mem_opt_level == 0 || mem_opt_level == 1,
  "supported mem_opt_level is either 0 or 1.\n");

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
  item_count.rename("item", "item_join");

  auto ordered_itemset = t.bcast_join(item_count, 
                                      eq("item","item_join"))
                          .select({"trans_id","item","rank"});
  item_count.rename("item_join", "item") // renaming back
            .drop("rank");  // rank is no longer required in item_count
  fp_t.show("ordered-itemset: ");
  fp_growth_model m;
  try {
    m = generate_tables(item_count, ordered_itemset, 
                        support, n_trans, tree_depth,
                        compression_point, mem_opt_level);
  }
  catch(std::exception& excpt) {
    std::string msg = excpt.what();
    if(msg.find("bad_alloc") != std::string::npos ) {
      std::string e = "out-of-memory error occured during fp-tree construction!\n";
      e += "retry with smaller tree_depth value.\n";
      if (min_support < 1.0) e += "or with higher min_support value.\n";
      if (mem_opt_level == 0) e += "or by setting mem_opt_level = 1.\n";
      REPORT_ERROR(INTERNAL_ERROR, e);
    }
    else REPORT_ERROR(INTERNAL_ERROR, msg);
  }
  fp_t.show("generate-trees: ");
  return m;
}

}
