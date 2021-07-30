#include "exrpc_data_storage.hpp"
#include "exrpc_dataframe.hpp"
#include "exrpc_dffunction.hpp"

using namespace frovedis;

void expose_frovedis_dataframe_functions() {
  // --- frovedis dataframe ---
  expose(create_dataframe);
  expose(release_data<dftable>);
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
  expose(frovedis_df_size);
  expose(frovedis_df_cnt);
  expose(frovedis_df_sum);
  expose(frovedis_df_avg);
  expose(frovedis_df_min);
  expose(frovedis_df_max);
  expose(frovedis_df_std);
  expose(frovedis_df_rename);
  expose(get_df_int_col);
  expose(get_df_long_col);
  expose(get_df_ulong_col);
  expose(get_df_float_col);
  expose(get_df_double_col);
  expose(get_df_string_col);
  expose(frovedis_gdf_aggr);
  expose(frovedis_gdf_select);
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
  expose(frov_df_copy_index);
  expose(frov_df_copy_column);
  expose(frov_df_union);
  expose(frov_df_set_col_order);
  expose(frov_df_astype);
  expose(frov_df_abs);
  expose(frov_df_binary_operation);
  expose(frov_df_immed_binary_operation);
  expose(frov_df_fillna);
  expose(frov_df_dropna);
  expose(frov_df_to_string);
}
