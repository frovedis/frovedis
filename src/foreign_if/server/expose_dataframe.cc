#include "exrpc_data_storage.hpp"
#include "exrpc_dataframe.hpp"
#include "exrpc_dffunction.hpp"

using namespace frovedis;

void expose_frovedis_dataframe_functions() {
  // --- frovedis dataframe ---
  expose(create_dataframe);
  expose(create_dataframe_from_local_vectors);
  expose(calc_memory_size);
  expose(release_data<dftable_base>);
  expose(release_data<grouped_dftable>);
  expose(show_dataframe);
  expose(get_dfoperator<int>);
  expose(get_dfoperator<long>);
  expose(get_dfoperator<unsigned long>);
  expose(get_dfoperator<float>);
  expose(get_dfoperator<double>);
  expose(get_str_dfoperator);
  expose(get_dfANDoperator);
  expose(get_dfORoperator);
  expose(get_dfNOToperator);
  expose((release_data<std::shared_ptr<dfoperator>>));
  expose(filter_df);
  expose(select_df);
  expose(fselect_df);
  expose(isnull_df);
  expose(drop_df_cols);
  expose(drop_df_rows<int>);
  expose(drop_df_rows<long>);
  expose(drop_df_rows<unsigned long>);
  expose(drop_df_rows<float>);
  expose(drop_df_rows<double>);
  expose(drop_df_rows<std::string>);
  expose(sort_df);
  expose(join_df);
  expose(group_by_df);
  expose(fgroup_by_df);
  expose(frovedis_df_size);
  expose(frovedis_df_cnt);
  expose(frovedis_df_sum);
  expose(frovedis_df_avg);
  expose(frovedis_df_min);
  expose(frovedis_df_max);
  expose(frovedis_df_std);
  expose(frovedis_df_sem);
  expose(frovedis_df_median);
  expose(frovedis_df_var);
  expose(frovedis_df_mad);
  expose(frovedis_df_rename);
  expose(get_df_col<int>);
  expose(get_df_col<long>);
  expose(get_df_col<unsigned long>);
  expose(get_df_col<float>);
  expose(get_df_col<double>);
  expose(get_df_string_col);
  expose(frovedis_gdf_aggr);
  expose(frovedis_gdf_select);
  expose(frovedis_gdf_fselect);
  expose(frovedis_gdf_agg_select);
  expose(frovedis_gdf_agg_fselect);
  expose(df_to_colmajor_float);
  expose(df_to_colmajor_double);
  expose(df_to_rowmajor_float);
  expose(df_to_rowmajor_double);
  expose(df_to_crs_float);
  expose(df_to_crs_double);
  expose(df_to_crs_float_using_info);
  expose(df_to_crs_double_using_info);
  expose(load_sparse_conversion_info);
  expose(save_sparse_conversion_info);
  expose(release_sparse_conversion_info);
  expose(frov_multi_eq_dfopt);
  expose(frov_cross_join_dfopt);
  expose(frov_load_dataframe_from_csv);
  expose(get_dataframe_length);
  expose(frov_df_append_column);
  expose(frov_df_add_index);
  expose(frov_df_reset_index);
  expose(frov_df_set_index);
  expose(frov_df_drop_duplicates);
  expose(get_df_column_pointer<int>);          // for spark
  expose(get_df_column_pointer<long>);         // for spark
  expose(get_df_column_pointer<float>);        // for spark
  expose(get_df_column_pointer<double>);       // for spark
  expose(get_df_column_pointer<std::string>);  // for spark
  expose(get_df_column_pointer<int64_t>);      // for spark
  expose(frov_df_copy_index);
  expose(frov_df_copy_column);
  expose(frov_df_union);
  expose(frov_df_set_col_order);
  expose(frov_df_astype);
  expose(frov_df_abs);
  expose(frov_df_binary_operation);
  expose(frov_df_immed_binary_operation);
  expose(frov_df_fillna);
  expose(frov_df_to_string);
  expose(frov_df_dropna_by_rows);
  expose(frov_df_dropna_by_cols<int>);
  expose(frov_df_dropna_by_cols<long>);
  expose(frov_df_dropna_by_cols<unsigned long>);
  expose(frov_df_dropna_by_cols<float>);
  expose(frov_df_dropna_by_cols<double>);
  expose(frov_df_dropna_by_cols<std::string>);
  expose(frov_df_head);
  expose(frov_df_tail);
  expose(frov_df_slice_range);
  expose(frov_df_get_index_loc);
  expose(frov_df_countna);
  expose(frov_df_ksort);
  expose(frov_df_mean);
  expose(frov_df_var);
  expose(frov_df_mad);
  expose(frov_df_std);
  expose(frov_df_sem);
  expose(frov_df_median);
  expose(frov_df_cov);
  expose(frov_col_cov);  // TODO: remove (if unused) 
  expose(frov_col2_cov); // TODO: remove (if unused)
  expose(frov_series_cov);
  expose(frov_df_sum<int>);
  expose(frov_df_sum<long>);
  expose(frov_df_sum<unsigned long>);
  expose(frov_df_sum<float>);
  expose(frov_df_sum<double>);
  expose(frov_df_first_element);
  expose(frov_df_last_element);
  expose(frov_df_min<int>);
  expose(frov_df_min<long>);
  expose(frov_df_min<unsigned long>);
  expose(frov_df_min<float>);
  expose(frov_df_min<double>);
  expose(frov_df_max<int>);
  expose(frov_df_max<long>);
  expose(frov_df_max<unsigned long>);
  expose(frov_df_max<float>);
  expose(frov_df_max<double>);
  expose(frovedis_gdf_aggr_with_ddof);
  expose(frovedis_gdf_aggr_with_mincount);
  expose(frov_df_mode_cols);
  expose(frov_df_mode_rows);
  expose(frov_df_clip);
  expose(frov_df_clip_axis1_numeric);
  expose(frov_df_clip_axis1_str);
  expose(frov_df_datetime_operation);
  expose(frov_df_concat_columns);
  //loc
  expose(frov_df_sel_rows_by_val<int>);
  expose(frov_df_sel_rows_by_val<long>);
  expose(frov_df_sel_rows_by_val<unsigned long>);
  expose(frov_df_sel_rows_by_val<float>);
  expose(frov_df_sel_rows_by_val<double>);
  expose(frov_df_sel_rows_by_val<std::string>);
  //iloc
  expose(frov_df_sel_rows_by_indices);

  // -- for spark wrapper ---
  expose(append_scalar);
  expose(get_dffunc_id);
  expose(get_dffunc_im<int>);
  expose(get_dffunc_im<long>);
  expose(get_dffunc_im<unsigned long>);
  expose(get_dffunc_im<float>);
  expose(get_dffunc_im<double>);
  expose(get_dffunc_bool_im);
  expose(get_dffunc_string_im);
  expose(get_dffunc_opt);
  expose(get_immed_dffunc_opt<int>);
  expose(get_immed_dffunc_opt<long>);
  expose(get_immed_dffunc_opt<float>);
  expose(get_immed_dffunc_opt<double>);
  expose(get_immed_int_dffunc_opt);
  expose(get_immed_string_dffunc_opt);
  expose(get_immed_substr);
  expose(get_immed_substr_index);
  expose(get_col_substr);
  expose(set_dffunc_asCol_name);
  expose(execute_dffunc);
  expose(get_dffunc_agg);
  expose(set_dfagg_asCol_name);
  expose(execute_dfagg);
  expose(copy_spark_column);
  expose(get_col_concat_multi);
  expose(get_immed_pad);
  expose(get_immed_locate);
  expose(get_immed_from_to_opt);
  // -----------------------
  expose(append_when_condition);
  expose(frov_get_bool_mask);
  expose(frov_df_filter_using_mask);
  expose(frov_df_distinct);
  expose(frovedis_series_string_methods);
  expose(frovedis_series_slice);
  expose(frovedis_series_pad);
  expose(frovedis_df_to_csv);
  expose(frovedis_set_datetime_type_for_add_sub_op);
}
