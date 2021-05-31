"""
rpclib.py
"""
#!/usr/bin/env python

# This source provides the interfaces to call the C/C++ functions from python
# code

import numpy as np
from ctypes import c_char_p, c_int, c_ulong, c_short, c_float, c_double,\
                   c_long, c_bool, c_char, c_void_p, CDLL, py_object, POINTER,\
                   c_size_t
from numpy.ctypeslib import ndpointer
from scipy.sparse.csr import csr_matrix

# A dynamic library containing implementation of server side code
try:
    LIB = CDLL("libfrovedis_client_python.so")
except OSError:
    try:
        LIB = CDLL("../lib/libfrovedis_client_python.so")
    except OSError:
        raise OSError("libfrovedis_client_python.so: No such dll found " + \
                      "(set LD_LIBRARY_PATH)")

# --- Frovedis Server ---
initialize_server = LIB.initialize_server
initialize_server.argtypes = [c_char_p]
initialize_server.restype = py_object

get_worker_size = LIB.get_worker_size
get_worker_size.argtypes = [c_char_p, c_int]
get_worker_size.restype = c_int

clean_server = LIB.clean_server
clean_server.argtypes = [c_char_p, c_int]

finalize_server = LIB.finalize_server
finalize_server.argtypes = [c_char_p, c_int]

check_server_exception = LIB.check_server_exception
check_server_exception.restype = py_object

# --- Frovedis dvector ---
# create from numpy array
create_frovedis_int_dvector = LIB.create_frovedis_int_dvector
create_frovedis_int_dvector.argtypes = [c_char_p, c_int,\
                            ndpointer(c_int, ndim=1, flags="C_CONTIGUOUS"),\
                                  c_ulong]
create_frovedis_int_dvector.restype = py_object

create_frovedis_long_dvector = LIB.create_frovedis_long_dvector
create_frovedis_long_dvector.argtypes = [c_char_p, c_int,\
                           ndpointer(c_long, ndim=1, flags="C_CONTIGUOUS"),\
                                  c_ulong]
create_frovedis_long_dvector.restype = py_object

create_frovedis_ulong_dvector = LIB.create_frovedis_ulong_dvector
create_frovedis_ulong_dvector.argtypes = [c_char_p, c_int,\
                           ndpointer(c_ulong, ndim=1, flags="C_CONTIGUOUS"),\
                                  c_ulong]
create_frovedis_ulong_dvector.restype = py_object

create_frovedis_float_dvector = LIB.create_frovedis_float_dvector
create_frovedis_float_dvector.argtypes = [c_char_p, c_int,\
                          ndpointer(c_float, ndim=1, flags="C_CONTIGUOUS"),\
                                  c_ulong]
create_frovedis_float_dvector.restype = py_object

create_frovedis_double_dvector = LIB.create_frovedis_double_dvector
create_frovedis_double_dvector.argtypes = [c_char_p, c_int,\
                         ndpointer(c_double, ndim=1, flags="C_CONTIGUOUS"),\
                                  c_ulong]
create_frovedis_double_dvector.restype = py_object

create_frovedis_string_dvector = LIB.create_frovedis_string_dvector
create_frovedis_string_dvector.argtypes = [c_char_p, c_int, POINTER(c_char_p),\
                                           c_ulong]
create_frovedis_string_dvector.restype = py_object

#To Print Created dvector
show_frovedis_dvector = LIB.show_frovedis_dvector
show_frovedis_dvector.argtypes = [c_char_p, c_int, c_long, c_int]

release_frovedis_dvector = LIB.release_frovedis_dvector
release_frovedis_dvector.argtypes = [c_char_p, c_int, c_long, c_int]

#------ Frovedis Vector
create_frovedis_int_vector = LIB.create_frovedis_int_vector
create_frovedis_int_vector.argtypes = [c_char_p, c_int,
                                       POINTER(c_int), c_ulong, c_short]
create_frovedis_int_vector.restype = py_object

create_frovedis_long_vector = LIB.create_frovedis_long_vector
create_frovedis_long_vector.argtypes = [c_char_p, c_int,
                                        POINTER(c_long), c_ulong, c_short]
create_frovedis_long_vector.restype = py_object

create_frovedis_float_vector = LIB.create_frovedis_float_vector
create_frovedis_float_vector.argtypes = [c_char_p, c_int,
                                         POINTER(c_float), c_ulong, c_short]
create_frovedis_float_vector.restype = py_object

create_frovedis_double_vector = LIB.create_frovedis_double_vector
create_frovedis_double_vector.argtypes = [c_char_p, c_int,
                                          POINTER(c_double), c_ulong, c_short]
create_frovedis_double_vector.restype = py_object

create_frovedis_string_vector = LIB.create_frovedis_string_vector
create_frovedis_string_vector.argtypes = [c_char_p, c_int,
                                          POINTER(c_char_p), c_ulong, c_short]
create_frovedis_string_vector.restype = py_object

get_frovedis_array = LIB.get_frovedis_array
get_frovedis_array.argtypes = [c_char_p, c_int, c_long, c_short]
get_frovedis_array.restype = py_object

get_float_array = LIB.get_float_array
get_float_array.argtypes = [c_char_p, c_int, c_long,
                            ndpointer(c_float, ndim=1, flags="C_CONTIGUOUS")]

get_double_array = LIB.get_double_array
get_double_array.argtypes = [c_char_p, c_int, c_long,
                             ndpointer(c_double, ndim=1, flags="C_CONTIGUOUS")]

save_frovedis_vector_client = LIB.save_frovedis_vector_client
save_frovedis_vector_client.argtypes = [c_char_p, c_int, # host, port
                                        c_long,  # data pointer
                                        c_char_p, #path
                                        c_bool, #is_binary
                                        c_short #data type
                                      ]

load_frovedis_vector_client = LIB.load_frovedis_vector_client
load_frovedis_vector_client.argtypes = [c_char_p, c_int, # host, port
                                        c_char_p, #path
                                        c_bool, #is_binary
                                        c_short #data type
                                      ]
load_frovedis_vector_client.restype = py_object

release_frovedis_array = LIB.release_frovedis_array
release_frovedis_array.argtypes = [c_char_p, c_int, c_long, c_short]


#----Frovedis Dataframe from Python--------------------
create_frovedis_dataframe = LIB.create_frovedis_dataframe
create_frovedis_dataframe.argtypes = [c_char_p, c_int, POINTER(c_short),
                                      POINTER(c_char_p), POINTER(c_long), 
                                      c_ulong]
create_frovedis_dataframe.restype = c_long

show_frovedis_dataframe = LIB.show_frovedis_dataframe
show_frovedis_dataframe.argtypes = [c_char_p, c_int, c_long]

release_frovedis_dataframe = LIB.release_frovedis_dataframe
release_frovedis_dataframe.argtypes = [c_char_p, c_int, c_long]

release_dfoperator = LIB.release_dfoperator
release_dfoperator.argtypes = [c_char_p, c_int, c_long]

get_frovedis_dfoperator = LIB.get_frovedis_dfoperator
get_frovedis_dfoperator.argtypes = [c_char_p, c_int, c_char_p, c_char_p,\
                                    c_short, c_short, c_bool]
get_frovedis_dfoperator.restype = c_long

get_dfANDoperator = LIB.get_frovedis_dfANDoperator
get_dfANDoperator.argtypes = [c_char_p, c_int, c_long, c_long]
get_dfANDoperator.restype = c_long

get_dfORoperator = LIB.get_frovedis_dfORoperator
get_dfORoperator.argtypes = [c_char_p, c_int, c_long, c_long]
get_dfORoperator.restype = c_long

get_dfNOToperator = LIB.get_frovedis_dfNOToperator
get_dfNOToperator.argtypes = [c_char_p, c_int, c_long]
get_dfNOToperator.restype = c_long

filter_frovedis_dataframe = LIB.filter_frovedis_dataframe
filter_frovedis_dataframe.argtypes = [c_char_p, c_int, c_long, c_long]
filter_frovedis_dataframe.restype = c_long

drop_frovedis_dataframe_columns = LIB.drop_frovedis_dataframe_columns
drop_frovedis_dataframe_columns.argtypes = [c_char_p, c_int, c_long, 
                                            POINTER(c_char_p), c_ulong]

drop_frovedis_duplicate_rows = LIB.drop_frovedis_duplicate_rows
drop_frovedis_duplicate_rows.argtypes = [c_char_p, c_int, c_long, 
                                         POINTER(c_char_p), c_ulong,
                                         c_char_p, c_bool]
drop_frovedis_duplicate_rows.restype = py_object

drop_frovedis_dataframe_rows_int = LIB.drop_frovedis_dataframe_rows_int
drop_frovedis_dataframe_rows_int.argtypes = [c_char_p, c_int, c_long, 
                                             POINTER(c_int), c_ulong,
                                             c_char_p]
drop_frovedis_dataframe_rows_int.restype = py_object

drop_frovedis_dataframe_rows_long = LIB.drop_frovedis_dataframe_rows_long
drop_frovedis_dataframe_rows_long.argtypes = [c_char_p, c_int, c_long, 
                                              POINTER(c_long), c_ulong,
                                              c_char_p]
drop_frovedis_dataframe_rows_long.restype = py_object

drop_frovedis_dataframe_rows_ulong = LIB.drop_frovedis_dataframe_rows_ulong
drop_frovedis_dataframe_rows_ulong.argtypes = [c_char_p, c_int, c_long, 
                                               POINTER(c_ulong), c_ulong,
                                               c_char_p]
drop_frovedis_dataframe_rows_ulong.restype = py_object

drop_frovedis_dataframe_rows_float = LIB.drop_frovedis_dataframe_rows_float
drop_frovedis_dataframe_rows_float.argtypes = [c_char_p, c_int, c_long, 
                                               POINTER(c_float), c_ulong,
                                               c_char_p]
drop_frovedis_dataframe_rows_float.restype = py_object

drop_frovedis_dataframe_rows_double = LIB.drop_frovedis_dataframe_rows_double
drop_frovedis_dataframe_rows_double.argtypes = [c_char_p, c_int, c_long, 
                                                POINTER(c_double), c_ulong,
                                                c_char_p]
drop_frovedis_dataframe_rows_double.restype = py_object

drop_frovedis_dataframe_rows_str = LIB.drop_frovedis_dataframe_rows_str
drop_frovedis_dataframe_rows_str.argtypes = [c_char_p, c_int, c_long, 
                                             POINTER(c_char_p), c_ulong,
                                             c_char_p]
drop_frovedis_dataframe_rows_str.restype = py_object

df_reset_index = LIB.df_reset_index
df_reset_index.argtypes = [c_char_p, c_int, c_long, c_bool, c_bool]
df_reset_index.restype = py_object

df_set_index = LIB.df_set_index
df_set_index.argtypes = [c_char_p, c_int, c_long, c_char_p, c_char_p, \
                         c_bool, c_bool]
df_set_index.restype = py_object

df_copy_index = LIB.df_copy_index
df_copy_index.argtypes = [c_char_p, c_int, c_long, c_long,
                          c_char_p, c_bool, c_short]
df_copy_index.restype = py_object

df_astype = LIB.df_astype
df_astype.argtypes = [c_char_p, c_int, c_long, 
                      POINTER(c_char_p),
                      POINTER(c_short), c_ulong]
df_astype.restype = py_object

select_frovedis_dataframe = LIB.select_frovedis_dataframe
select_frovedis_dataframe.argtypes = [c_char_p, c_int, c_long, \
                                      POINTER(c_char_p), c_ulong]
select_frovedis_dataframe.restype = c_long

isnull_frovedis_dataframe = LIB.isnull_frovedis_dataframe
isnull_frovedis_dataframe.argtypes = [c_char_p, c_int, c_long, \
                                      POINTER(c_char_p), c_ulong]
isnull_frovedis_dataframe.restype = c_long

sort_frovedis_dataframe = LIB.sort_frovedis_dataframe
sort_frovedis_dataframe.argtypes = [c_char_p, c_int, c_long, \
                                    POINTER(c_char_p), \
                                    ndpointer(c_int, ndim=1, \
                                    flags="C_CONTIGUOUS"), c_int]
sort_frovedis_dataframe.restype = c_long

group_frovedis_dataframe = LIB.group_frovedis_dataframe
group_frovedis_dataframe.argtypes = [c_char_p, c_int, c_long, \
                                     POINTER(c_char_p), c_ulong]
group_frovedis_dataframe.restype = c_long

select_grouped_dataframe = LIB.select_grouped_dataframe
select_grouped_dataframe.argtypes = [c_char_p, c_int, c_long,
                                     POINTER(c_char_p), c_ulong]
select_grouped_dataframe.restype = c_long

agg_grouped_dataframe = LIB.agg_grouped_dataframe
agg_grouped_dataframe.argtypes = [c_char_p, c_int, c_long,
                                  POINTER(c_char_p), c_ulong,
                                  POINTER(c_char_p), POINTER(c_char_p),
                                  POINTER(c_char_p), c_ulong]
agg_grouped_dataframe.restype = c_long

merge_frovedis_dataframe = LIB.join_frovedis_dataframe
merge_frovedis_dataframe.argtypes = [c_char_p, c_int, c_long, c_long, c_long,
                                     c_char_p, c_char_p]
merge_frovedis_dataframe.restype = c_long

rename_frovedis_dataframe = LIB.rename_frovedis_dataframe
rename_frovedis_dataframe.argtypes = [c_char_p, c_int, c_long,
                                      POINTER(c_char_p), POINTER(c_char_p),\
                                      c_ulong, c_bool]
rename_frovedis_dataframe.restype = c_long

get_min_frovedis_dataframe = LIB.min_frovedis_dataframe
get_min_frovedis_dataframe.argtypes = [c_char_p, c_int, c_long,
                                       POINTER(c_char_p), POINTER(c_short),\
                                       c_int]
get_min_frovedis_dataframe.restype = py_object

get_max_frovedis_dataframe = LIB.max_frovedis_dataframe
get_max_frovedis_dataframe.argtypes = [c_char_p, c_int, c_long,
                                       POINTER(c_char_p), POINTER(c_short),\
                                       c_int]
get_max_frovedis_dataframe.restype = py_object

get_sum_frovedis_dataframe = LIB.sum_frovedis_dataframe
get_sum_frovedis_dataframe.argtypes = [c_char_p, c_int, c_long,
                                       POINTER(c_char_p), POINTER(c_short),\
                                       c_int]
get_sum_frovedis_dataframe.restype = py_object

get_avg_frovedis_dataframe = LIB.avg_frovedis_dataframe
get_avg_frovedis_dataframe.argtypes = [c_char_p, c_int, c_long,
                                       POINTER(c_char_p), c_ulong]
get_avg_frovedis_dataframe.restype = py_object

get_cnt_frovedis_dataframe = LIB.cnt_frovedis_dataframe
get_cnt_frovedis_dataframe.argtypes = [c_char_p, c_int, c_long,
                                       POINTER(c_char_p), c_ulong]
get_cnt_frovedis_dataframe.restype = py_object

get_std_frovedis_dataframe = LIB.std_frovedis_dataframe
get_std_frovedis_dataframe.argtypes = [c_char_p, c_int, c_long,
                                       POINTER(c_char_p), c_ulong]
get_std_frovedis_dataframe.restype = py_object

get_frovedis_col = LIB.get_frovedis_col
get_frovedis_col.argtypes = [c_char_p, c_int, c_long, c_char_p, c_short]
get_frovedis_col.restype = py_object

df_to_rowmajor = LIB.df_to_rowmajor
df_to_rowmajor.argtypes = [c_char_p, c_int, c_long,    #host, port, proxy
                           POINTER(c_char_p), c_ulong, #t_cols_arr, size
                           c_short]                    #dtype
df_to_rowmajor.restype = py_object

df_to_colmajor = LIB.df_to_colmajor
df_to_colmajor.argtypes = [c_char_p, c_int, c_long,    #host, port, proxy
                           POINTER(c_char_p), c_ulong, #t_cols_arr, size
                           c_short]                    #dtype
df_to_colmajor.restype = py_object

df_to_crs = LIB.df_to_crs
df_to_crs.argtypes = [c_char_p, c_int, c_long,    #host, port, proxy
                      POINTER(c_char_p), c_ulong, #t_cols_arr, size1
                      POINTER(c_char_p), c_ulong, #cat_cols_arr, size2
                      c_long, c_short]            #info_id, dtype
df_to_crs.restype = py_object

df_to_crs_using_info = LIB.df_to_crs_using_info
df_to_crs_using_info.argtypes = [c_char_p, c_int, c_long,  #host, port, proxy
                                 c_long, c_short]          #info_id, dtype
df_to_crs_using_info.restype = py_object

# multi_eq df opt
get_multi_eq_dfopt = LIB.get_multi_eq_dfopt
get_multi_eq_dfopt.argtypes = [c_char_p, c_int, # host, port
                               POINTER(c_char_p), # left_on
                               POINTER(c_char_p), # right_on
                               c_ulong] # size
get_multi_eq_dfopt.restype = c_long

load_dataframe_from_csv = LIB.load_dataframe_from_csv
load_dataframe_from_csv.argtypes = [c_char_p, c_int, # host, port
                                   c_char_p, #filename
                                   POINTER(c_char_p), POINTER(c_char_p), # types, names
                                   c_ulong, c_ulong, # types_size, names_size
                                   c_char, c_char_p, c_char_p, #seperator, nullstr, comments
                                   c_size_t, c_double, #rows_to_see, seperate_mb
                                   c_bool, #partial_type_info
                                   POINTER(c_char_p), POINTER(c_char_p), # dtype_keys, dtype_vals
                                   c_ulong, c_bool, c_bool, # dtypes_dict_size, low_memory, add_index
                                   ndpointer(c_int, ndim=1, flags="C_CONTIGUOUS"), #usecols-id
                                   c_ulong,        # usecols-len
                                   c_bool, c_bool, # verbose, mangle_dupe_cols
                                   c_int, POINTER(c_char_p), c_ulong] # index_col, bool_cols, bool_cols_len
load_dataframe_from_csv.restype = py_object

get_frovedis_dataframe_length = LIB.get_frovedis_dataframe_length
get_frovedis_dataframe_length.argtypes = [c_char_p, c_int, # host, port
                                          c_long]          # proxy
get_frovedis_dataframe_length.restype = c_long

df_convert_dicstring_to_bool = LIB.df_convert_dicstring_to_bool
df_convert_dicstring_to_bool.argtypes = [c_char_p, c_int, c_long, # host, port, df_proxy
                                        POINTER(c_char_p), c_ulong, # col_names, sz,
                                        c_char_p, c_bool] # nullstr, need_materialize
df_convert_dicstring_to_bool.restype = py_object

df_append_column = LIB.df_append_column
df_append_column.argtypes = [c_char_p, c_int, c_long, # host, port, df_proxy
                            c_char_p, c_short, c_long,  # col_name, type, dvec
                            c_int, c_bool, c_bool] # position, need_materialize, drop_old
df_append_column.restype = py_object

df_add_index = LIB.df_add_index
df_add_index.argtypes = [c_char_p, c_int, # host, port
                        c_long, c_char_p, # proxy, name
                        c_bool] # need_materialize
df_add_index.restype = py_object

df_union = LIB.df_union
df_union.argtypes = [c_char_p, c_int, # host, port
                    c_long, POINTER(c_long), # df_proxy, proxies
                    c_int, c_bool, # size, ignore_index
                    c_bool, c_bool] # verify_integrity, sort
df_union.restype = py_object

df_union2 = LIB.df_union2
df_union2.argtypes = [c_char_p, c_int, # host, port
                      c_long, POINTER(c_long), # df_proxy, proxies
                      c_ulong, POINTER(c_char_p), # size, names
                      c_ulong, c_bool] # names_size, verify_integrity
df_union2.restype = py_object

df_set_col_order = LIB.df_set_col_order
df_set_col_order.argtypes = [c_char_p, c_int, # host, port
                            c_long, POINTER(c_char_p), # df_proxy, new_cols
                            c_ulong] # size
df_set_col_order.restype = py_object

# --- Frovedis dftable_to_sparse_info ---
load_dftable_to_sparse_info = LIB.load_dftable_to_sparse_info
load_dftable_to_sparse_info.argtypes = [c_char_p, c_int,  #host, port
                                        c_long, c_char_p] #info_id, dirname

save_dftable_to_sparse_info = LIB.save_dftable_to_sparse_info
save_dftable_to_sparse_info.argtypes = [c_char_p, c_int,  #host, port
                                        c_long, c_char_p] #info_id, dirname

release_dftable_to_sparse_info = LIB.release_dftable_to_sparse_info
release_dftable_to_sparse_info.argtypes = [c_char_p, c_int, c_long]  #host,\
                                           # port, info_id

# --- Frovedis sparse matrices ---
# create from scipy matrix
create_frovedis_crs_II_matrix = LIB.create_frovedis_crs_II_matrix
create_frovedis_crs_II_matrix.argtypes = [c_char_p, c_int,
                                          c_ulong, c_ulong,
                                          ndpointer(c_int, ndim=1,\
                                          flags="C_CONTIGUOUS"),\
                                          ndpointer(c_int, ndim=1,\
                                          flags="C_CONTIGUOUS"),\
                                          ndpointer(c_long, ndim=1,\
                                          flags="C_CONTIGUOUS"),\
                                          c_ulong]
create_frovedis_crs_II_matrix.restype = py_object

create_frovedis_crs_IL_matrix = LIB.create_frovedis_crs_IL_matrix
create_frovedis_crs_IL_matrix.argtypes = [c_char_p, c_int,
                                          c_ulong, c_ulong,
                                          ndpointer(c_int, ndim=1,\
                                          flags="C_CONTIGUOUS"),\
                                          ndpointer(c_long, ndim=1,\
                                          flags="C_CONTIGUOUS"),\
                                          ndpointer(c_long, ndim=1,\
                                          flags="C_CONTIGUOUS"),\
                                          c_ulong]
create_frovedis_crs_IL_matrix.restype = py_object

create_frovedis_crs_LI_matrix = LIB.create_frovedis_crs_LI_matrix
create_frovedis_crs_LI_matrix.argtypes = [c_char_p, c_int,
                                          c_ulong, c_ulong,
                                          ndpointer(c_long, ndim=1,\
                                          flags="C_CONTIGUOUS"),\
                                          ndpointer(c_int, ndim=1,\
                                          flags="C_CONTIGUOUS"),\
                                          ndpointer(c_long, ndim=1,\
                                          flags="C_CONTIGUOUS"),\
                                          c_ulong]
create_frovedis_crs_LI_matrix.restype = py_object

create_frovedis_crs_LL_matrix = LIB.create_frovedis_crs_LL_matrix
create_frovedis_crs_LL_matrix.argtypes = [c_char_p, c_int,
                                          c_ulong, c_ulong,
                                          ndpointer(c_long, ndim=1,\
                                          flags="C_CONTIGUOUS"),\
                                          ndpointer(c_long, ndim=1,\
                                          flags="C_CONTIGUOUS"),\
                                          ndpointer(c_long, ndim=1,\
                                          flags="C_CONTIGUOUS"),\
                                          c_ulong]
create_frovedis_crs_LL_matrix.restype = py_object

create_frovedis_crs_FI_matrix = LIB.create_frovedis_crs_FI_matrix
create_frovedis_crs_FI_matrix.argtypes = [c_char_p, c_int,
                                          c_ulong, c_ulong,
                                          ndpointer(c_float, ndim=1,\
                                          flags="C_CONTIGUOUS"),\
                                          ndpointer(c_int, ndim=1,\
                                          flags="C_CONTIGUOUS"),\
                                          ndpointer(c_long, ndim=1,\
                                          flags="C_CONTIGUOUS"),\
                                          c_ulong]
create_frovedis_crs_FI_matrix.restype = py_object

create_frovedis_crs_FL_matrix = LIB.create_frovedis_crs_FL_matrix
create_frovedis_crs_FL_matrix.argtypes = [c_char_p, c_int,
                                          c_ulong, c_ulong,
                                          ndpointer(c_float, ndim=1,\
                                          flags="C_CONTIGUOUS"),\
                                          ndpointer(c_long, ndim=1,\
                                          flags="C_CONTIGUOUS"),\
                                          ndpointer(c_long, ndim=1,\
                                          flags="C_CONTIGUOUS"),\
                                          c_ulong]
create_frovedis_crs_FL_matrix.restype = py_object

create_frovedis_crs_DI_matrix = LIB.create_frovedis_crs_DI_matrix
create_frovedis_crs_DI_matrix.argtypes = [c_char_p, c_int,
                                          c_ulong, c_ulong,
                                          ndpointer(c_double, ndim=1,\
                                          flags="C_CONTIGUOUS"),\
                                          ndpointer(c_int, ndim=1,\
                                          flags="C_CONTIGUOUS"),
                                          ndpointer(c_long, ndim=1,\
                                          flags="C_CONTIGUOUS"),
                                          c_ulong]
create_frovedis_crs_DI_matrix.restype = py_object

create_frovedis_crs_DL_matrix = LIB.create_frovedis_crs_DL_matrix
create_frovedis_crs_DL_matrix.argtypes = [c_char_p, c_int,
                                          c_ulong, c_ulong,
                                          ndpointer(c_double, ndim=1,\
                                          flags="C_CONTIGUOUS"),\
                                          ndpointer(c_long, ndim=1,\
                                          flags="C_CONTIGUOUS"),\
                                          ndpointer(c_long, ndim=1,\
                                          flags="C_CONTIGUOUS"),
                                          c_ulong]
create_frovedis_crs_DL_matrix.restype = py_object

get_crs_matrix_components = LIB.get_crs_matrix_components
get_crs_matrix_components.argtypes = [c_char_p, c_int,\
                                         c_ulong,\
                                         c_void_p,\
                                         c_void_p,\
                                         c_void_p,\
                                         c_short, c_short,\
                                         c_ulong, c_ulong]

transpose_frovedis_sparse_matrix = LIB.transpose_frovedis_sparse_matrix
transpose_frovedis_sparse_matrix.argtypes = [c_char_p, c_int,\
                                            c_long, c_short, c_short]
transpose_frovedis_sparse_matrix.restype = py_object

csr_to_rowmajor_matrix = LIB.csr_to_rowmajor_matrix
csr_to_rowmajor_matrix.argtypes = [c_char_p, #host 
                                   c_int,    #port
                                   c_long,   #data
                                   c_short,  #dtype
                                   c_short]  #itype
csr_to_rowmajor_matrix.restype = py_object

csr_to_colmajor_matrix = LIB.csr_to_colmajor_matrix
csr_to_colmajor_matrix.argtypes = [c_char_p, #host
                                   c_int,    #port
                                   c_long,   #data
                                   c_short,  #dtype
                                   c_short]  #itype
csr_to_colmajor_matrix.restype = py_object

compute_spmv = LIB.compute_spmv
compute_spmv.argtypes = [c_char_p, c_int, \
                         c_long, c_long, \
                         c_short, c_short]
compute_spmv.restype = py_object

'''
#get crs II matrix
get_crs_II_matrix_components = LIB.get_crs_II_matrix_components
get_crs_II_matrix_components.argtypes = [c_char_p, c_int,\
                                         c_ulong,\
                                         ndpointer(c_int, ndim=1,\
                                         flags="C_CONTIGUOUS"),\
                                         ndpointer(c_int, ndim=1,\
                                         flags="C_CONTIGUOUS"),\
                                         ndpointer(c_long, ndim=1,\
                                         flags="C_CONTIGUOUS")]

#get crs IL matrix
get_crs_IL_matrix_components = LIB.get_crs_IL_matrix_components
get_crs_IL_matrix_components.argtypes = [c_char_p, c_int,\
                                         c_ulong,\
                                         ndpointer(c_int, ndim=1,\
                                         flags="C_CONTIGUOUS"),\
                                         ndpointer(c_long, ndim=1,\
                                         flags="C_CONTIGUOUS"),\
                                         ndpointer(c_long, ndim=1,\
                                         flags="C_CONTIGUOUS")]

#get crs LI matrix
get_crs_LI_matrix_components = LIB.get_crs_LI_matrix_components
get_crs_LI_matrix_components.argtypes = [c_char_p, c_int,\
                                         c_ulong,\
                                         ndpointer(c_long, ndim=1,\
                                         flags="C_CONTIGUOUS"),\
                                         ndpointer(c_int, ndim=1,\
                                         flags="C_CONTIGUOUS"),\
                                         ndpointer(c_long, ndim=1,\
                                         flags="C_CONTIGUOUS")]

#get crs LL matrix
get_crs_LL_matrix_components = LIB.get_crs_LL_matrix_components
get_crs_LL_matrix_components.argtypes = [c_char_p, c_int,\
                                         c_ulong,\
                                         ndpointer(c_long, ndim=1,\
                                         flags="C_CONTIGUOUS"),\
                                         ndpointer(c_long, ndim=1,\
                                         flags="C_CONTIGUOUS"),\
                                         ndpointer(c_long, ndim=1,\
                                         flags="C_CONTIGUOUS")]

#get crs FI matrix
get_crs_FI_matrix_components = LIB.get_crs_FI_matrix_components
get_crs_FI_matrix_components.argtypes = [c_char_p, c_int,\
                                         c_ulong,\
                                         ndpointer(c_float, ndim=1,\
                                         flags="C_CONTIGUOUS"),\
                                         ndpointer(c_int, ndim=1,\
                                         flags="C_CONTIGUOUS"),\
                                         ndpointer(c_long, ndim=1,\
                                         flags="C_CONTIGUOUS")]


#get crs FL matrix
get_crs_FL_matrix_components = LIB.get_crs_FL_matrix_components
get_crs_FL_matrix_components.argtypes = [c_char_p, c_int,\
                                         c_ulong,\
                                         ndpointer(c_float, ndim=1,\
                                         flags="C_CONTIGUOUS"),\
                                         ndpointer(c_long, ndim=1,\
                                         flags="C_CONTIGUOUS"),\
                                         ndpointer(c_long, ndim=1,\
                                         flags="C_CONTIGUOUS")]


#get crs DI matrix
get_crs_DI_matrix_components = LIB.get_crs_DI_matrix_components
get_crs_DI_matrix_components.argtypes = [c_char_p, c_int,\
                                         c_ulong,\
                                         ndpointer(c_double, ndim=1,\
                                         flags="C_CONTIGUOUS"),\
                                         ndpointer(c_int, ndim=1,\
                                         flags="C_CONTIGUOUS"),\
                                         ndpointer(c_long, ndim=1,\
                                         flags="C_CONTIGUOUS")]


#get crs DL matrix
get_crs_DL_matrix_components = LIB.get_crs_DL_matrix_components
get_crs_DL_matrix_components.argtypes = [c_char_p, c_int,\
                                         c_ulong,\
                                         ndpointer(c_double, ndim=1,\
                                         flags="C_CONTIGUOUS"),\
                                         ndpointer(c_long, ndim=1,\
                                         flags="C_CONTIGUOUS"),\
                                         ndpointer(c_long, ndim=1,\
                                         flags="C_CONTIGUOUS")]

'''
# load from text/bin file
load_frovedis_crs_matrix = LIB.load_frovedis_crs_matrix
load_frovedis_crs_matrix.argtypes = [c_char_p, c_int,
                                     c_char_p, c_bool,
                                     c_short, c_short]
load_frovedis_crs_matrix.restype = py_object

save_frovedis_crs_matrix = LIB.save_frovedis_crs_matrix
save_frovedis_crs_matrix.argtypes = [c_char_p, c_int,
                                     c_long, c_char_p, c_bool,
                                     c_short, c_short]

release_frovedis_crs_matrix = LIB.release_frovedis_crs_matrix
release_frovedis_crs_matrix.argtypes = [c_char_p, c_int, c_long,
                                        c_short, c_short]

show_frovedis_crs_matrix = LIB.show_frovedis_crs_matrix
show_frovedis_crs_matrix.argtypes = [c_char_p, c_int, c_long,
                                     c_short, c_short]

# --- Frovedis Dense matrices ---
# create from numpy matrix
create_frovedis_double_dense_matrix = LIB.create_frovedis_double_dense_matrix
create_frovedis_double_dense_matrix.argtypes = [c_char_p, c_int,\
                                       c_ulong, c_ulong,\
                                       ndpointer(c_double, ndim=1,\
                                       flags="C_CONTIGUOUS"),\
                                       c_char]
create_frovedis_double_dense_matrix.restype = py_object

create_frovedis_float_dense_matrix = LIB.create_frovedis_float_dense_matrix
create_frovedis_float_dense_matrix.argtypes = [c_char_p, c_int,\
                                       c_ulong, c_ulong,\
                                       ndpointer(c_float, ndim=1,\
                                       flags="C_CONTIGUOUS"),\
                                       c_char]
create_frovedis_float_dense_matrix.restype = py_object



create_frovedis_long_dense_matrix = LIB.create_frovedis_long_dense_matrix
create_frovedis_long_dense_matrix.argtypes = [c_char_p, c_int,\
                                       c_ulong, c_ulong,\
                                       ndpointer(c_long, ndim=1,\
                                       flags="C_CONTIGUOUS"),\
                                       c_char]
create_frovedis_long_dense_matrix.restype = py_object



create_frovedis_int_dense_matrix = LIB.create_frovedis_int_dense_matrix
create_frovedis_int_dense_matrix.argtypes = [c_char_p, c_int,\
                                       c_ulong, c_ulong,\
                                       ndpointer(c_int, ndim=1,\
                                       flags="C_CONTIGUOUS"),\
                                       c_char]
create_frovedis_int_dense_matrix.restype = py_object


# load from text/bin file
load_frovedis_dense_matrix = LIB.load_frovedis_dense_matrix
load_frovedis_dense_matrix.argtypes = [c_char_p, c_int,
                                       c_char_p, c_bool, c_char, c_short]
load_frovedis_dense_matrix.restype = py_object

save_frovedis_dense_matrix = LIB.save_frovedis_dense_matrix
save_frovedis_dense_matrix.argtypes = [c_char_p, c_int,
                                       c_long, c_char_p,
                                       c_bool, c_char, c_short]

transpose_frovedis_dense_matrix = LIB.transpose_frovedis_dense_matrix
transpose_frovedis_dense_matrix.argtypes = [c_char_p, c_int,
                                            c_long, c_char, c_short]
transpose_frovedis_dense_matrix.restype = py_object

copy_frovedis_dense_matrix = LIB.copy_frovedis_dense_matrix
copy_frovedis_dense_matrix.argtypes = [c_char_p, c_int, c_long, c_char,\
                                       c_short]
copy_frovedis_dense_matrix.restype = py_object

release_frovedis_dense_matrix = LIB.release_frovedis_dense_matrix
release_frovedis_dense_matrix.argtypes = [c_char_p, c_int, c_long, c_char,\
                                          c_short]

show_frovedis_dense_matrix = LIB.show_frovedis_dense_matrix
show_frovedis_dense_matrix.argtypes = [c_char_p, c_int, c_long, c_char,\
                                       c_short]

get_frovedis_rowmatrix = LIB.get_frovedis_rowmatrix
get_frovedis_rowmatrix.argtypes = [c_char_p, c_int, c_long,
                                   c_ulong, c_ulong,
                                   c_char, c_short]
get_frovedis_rowmatrix.restype = py_object

get_double_rowmajor_array_as_int_array = LIB.get_double_rowmajor_array_as_int_array
get_double_rowmajor_array_as_int_array.argtypes = [\
                             c_char_p, c_int, c_long, c_char,\
                             ndpointer(c_int, ndim=1, flags="C_CONTIGUOUS"),\
                             c_ulong]

get_double_rowmajor_array_as_long_array = LIB.get_double_rowmajor_array_as_long_array
get_double_rowmajor_array_as_long_array.argtypes = [\
                             c_char_p, c_int, c_long, c_char,\
                             ndpointer(c_long, ndim=1, flags="C_CONTIGUOUS"),\
                             c_ulong]

get_double_rowmajor_array_as_float_array = LIB.get_double_rowmajor_array_as_float_array
get_double_rowmajor_array_as_float_array.argtypes = [\
                             c_char_p, c_int, c_long, c_char,\
                             ndpointer(c_float, ndim=1, flags="C_CONTIGUOUS"),\
                             c_ulong]

get_double_rowmajor_array_as_double_array = LIB.get_double_rowmajor_array_as_double_array
get_double_rowmajor_array_as_double_array.argtypes = [\
                             c_char_p, c_int, c_long, c_char,\
                             ndpointer(c_double, ndim=1, flags="C_CONTIGUOUS"),\
                             c_ulong]

get_float_rowmajor_array_as_int_array = LIB.get_float_rowmajor_array_as_int_array
get_float_rowmajor_array_as_int_array.argtypes = [\
                             c_char_p, c_int, c_long, c_char,\
                             ndpointer(c_int, ndim=1, flags="C_CONTIGUOUS"),\
                             c_ulong]

get_float_rowmajor_array_as_long_array = LIB.get_float_rowmajor_array_as_long_array
get_float_rowmajor_array_as_long_array.argtypes = [\
                             c_char_p, c_int, c_long, c_char,\
                             ndpointer(c_long, ndim=1, flags="C_CONTIGUOUS"),\
                             c_ulong]

get_float_rowmajor_array_as_float_array = LIB.get_float_rowmajor_array_as_float_array
get_float_rowmajor_array_as_float_array.argtypes = [\
                             c_char_p, c_int, c_long, c_char,\
                             ndpointer(c_float, ndim=1, flags="C_CONTIGUOUS"),\
                             c_ulong]

get_float_rowmajor_array_as_double_array = LIB.get_float_rowmajor_array_as_double_array
get_float_rowmajor_array_as_double_array.argtypes = [\
                             c_char_p, c_int, c_long, c_char,\
                             ndpointer(c_double, ndim=1, flags="C_CONTIGUOUS"),\
                             c_ulong]

get_long_rowmajor_array_as_int_array = LIB.get_long_rowmajor_array_as_int_array
get_long_rowmajor_array_as_int_array.argtypes = [\
                             c_char_p, c_int, c_long, c_char,\
                             ndpointer(c_int, ndim=1, flags="C_CONTIGUOUS"),\
                             c_ulong]

get_long_rowmajor_array_as_long_array = LIB.get_long_rowmajor_array_as_long_array
get_long_rowmajor_array_as_long_array.argtypes = [\
                             c_char_p, c_int, c_long, c_char,\
                             ndpointer(c_long, ndim=1, flags="C_CONTIGUOUS"),\
                             c_ulong]

get_long_rowmajor_array_as_float_array = LIB.get_long_rowmajor_array_as_float_array
get_long_rowmajor_array_as_float_array.argtypes = [\
                             c_char_p, c_int, c_long, c_char,\
                             ndpointer(c_float, ndim=1, flags="C_CONTIGUOUS"),\
                             c_ulong]

get_long_rowmajor_array_as_double_array = LIB.get_long_rowmajor_array_as_double_array
get_long_rowmajor_array_as_double_array.argtypes = [\
                             c_char_p, c_int, c_long, c_char,\
                             ndpointer(c_double, ndim=1, flags="C_CONTIGUOUS"),\
                             c_ulong]

get_int_rowmajor_array_as_int_array = LIB.get_int_rowmajor_array_as_int_array
get_int_rowmajor_array_as_int_array.argtypes = [\
                             c_char_p, c_int, c_long, c_char,\
                             ndpointer(c_int, ndim=1, flags="C_CONTIGUOUS"),\
                             c_ulong]

get_int_rowmajor_array_as_long_array = LIB.get_int_rowmajor_array_as_long_array
get_int_rowmajor_array_as_long_array.argtypes = [\
                             c_char_p, c_int, c_long, c_char,\
                             ndpointer(c_long, ndim=1, flags="C_CONTIGUOUS"),\
                             c_ulong]

get_int_rowmajor_array_as_float_array = LIB.get_int_rowmajor_array_as_float_array
get_int_rowmajor_array_as_float_array.argtypes = [\
                             c_char_p, c_int, c_long, c_char,\
                             ndpointer(c_float, ndim=1, flags="C_CONTIGUOUS"),\
                             c_ulong]

get_int_rowmajor_array_as_double_array = LIB.get_int_rowmajor_array_as_double_array
get_int_rowmajor_array_as_double_array.argtypes = [\
                             c_char_p, c_int, c_long, c_char,\
                             ndpointer(c_double, ndim=1, flags="C_CONTIGUOUS"),\
                             c_ulong]

# INT to OTHERS
I2I_cast_and_copy_array = LIB.I2I_cast_and_copy_array
I2I_cast_and_copy_array.argtypes = [\
                                ndpointer(c_int, ndim=1, flags="C_CONTIGUOUS"),\
                                ndpointer(c_int, ndim=1, flags="C_CONTIGUOUS"),\
                                c_ulong]

I2L_cast_and_copy_array = LIB.I2L_cast_and_copy_array
I2L_cast_and_copy_array.argtypes = [\
                                ndpointer(c_int, ndim=1, flags="C_CONTIGUOUS"),\
                                ndpointer(c_long, ndim=1, flags="C_CONTIGUOUS"),\
                                c_ulong]

I2F_cast_and_copy_array = LIB.I2F_cast_and_copy_array
I2F_cast_and_copy_array.argtypes = [\
                                ndpointer(c_int, ndim=1, flags="C_CONTIGUOUS"),\
                                ndpointer(c_float, ndim=1, flags="C_CONTIGUOUS"),\
                                c_ulong]

I2D_cast_and_copy_array = LIB.I2D_cast_and_copy_array
I2D_cast_and_copy_array.argtypes = [\
                                ndpointer(c_int, ndim=1, flags="C_CONTIGUOUS"),\
                                ndpointer(c_double, ndim=1, flags="C_CONTIGUOUS"),\
                                c_ulong]

# LONG to OTHERS
L2I_cast_and_copy_array = LIB.L2I_cast_and_copy_array
L2I_cast_and_copy_array.argtypes = [\
                                ndpointer(c_long, ndim=1, flags="C_CONTIGUOUS"),\
                                ndpointer(c_int, ndim=1, flags="C_CONTIGUOUS"),\
                                c_ulong]

L2L_cast_and_copy_array = LIB.L2L_cast_and_copy_array
L2L_cast_and_copy_array.argtypes = [\
                                ndpointer(c_long, ndim=1, flags="C_CONTIGUOUS"),\
                                ndpointer(c_long, ndim=1, flags="C_CONTIGUOUS"),\
                                c_ulong]

L2F_cast_and_copy_array = LIB.L2F_cast_and_copy_array
L2F_cast_and_copy_array.argtypes = [\
                                ndpointer(c_long, ndim=1, flags="C_CONTIGUOUS"),\
                                ndpointer(c_float, ndim=1, flags="C_CONTIGUOUS"),\
                                c_ulong]

L2D_cast_and_copy_array = LIB.L2D_cast_and_copy_array
L2D_cast_and_copy_array.argtypes = [\
                                ndpointer(c_long, ndim=1, flags="C_CONTIGUOUS"),\
                                ndpointer(c_double, ndim=1, flags="C_CONTIGUOUS"),\
                                c_ulong]

# FLOAT to OTHERS
F2I_cast_and_copy_array = LIB.F2I_cast_and_copy_array
F2I_cast_and_copy_array.argtypes = [\
                                ndpointer(c_float, ndim=1, flags="C_CONTIGUOUS"),\
                                ndpointer(c_int, ndim=1, flags="C_CONTIGUOUS"),\
                                c_ulong]

F2L_cast_and_copy_array = LIB.F2L_cast_and_copy_array
F2L_cast_and_copy_array.argtypes = [\
                                ndpointer(c_float, ndim=1, flags="C_CONTIGUOUS"),\
                                ndpointer(c_long, ndim=1, flags="C_CONTIGUOUS"),\
                                c_ulong]

F2F_cast_and_copy_array = LIB.F2F_cast_and_copy_array
F2F_cast_and_copy_array.argtypes = [\
                                ndpointer(c_float, ndim=1, flags="C_CONTIGUOUS"),\
                                ndpointer(c_float, ndim=1, flags="C_CONTIGUOUS"),\
                                c_ulong]

F2D_cast_and_copy_array = LIB.F2D_cast_and_copy_array
F2D_cast_and_copy_array.argtypes = [\
                                ndpointer(c_float, ndim=1, flags="C_CONTIGUOUS"),\
                                ndpointer(c_double, ndim=1, flags="C_CONTIGUOUS"),\
                                c_ulong]

# DOUBLE to OTHERS
D2I_cast_and_copy_array = LIB.D2I_cast_and_copy_array
D2I_cast_and_copy_array.argtypes = [\
                                ndpointer(c_double, ndim=1, flags="C_CONTIGUOUS"),\
                                ndpointer(c_int, ndim=1, flags="C_CONTIGUOUS"),\
                                c_ulong]

D2L_cast_and_copy_array = LIB.D2L_cast_and_copy_array
D2L_cast_and_copy_array.argtypes = [\
                                ndpointer(c_double, ndim=1, flags="C_CONTIGUOUS"),\
                                ndpointer(c_long, ndim=1, flags="C_CONTIGUOUS"),\
                                c_ulong]

D2F_cast_and_copy_array = LIB.D2F_cast_and_copy_array
D2F_cast_and_copy_array.argtypes = [\
                                ndpointer(c_double, ndim=1, flags="C_CONTIGUOUS"),\
                                ndpointer(c_float, ndim=1, flags="C_CONTIGUOUS"),\
                                c_ulong]

D2D_cast_and_copy_array = LIB.D2D_cast_and_copy_array
D2D_cast_and_copy_array.argtypes = [\
                                ndpointer(c_double, ndim=1, flags="C_CONTIGUOUS"),\
                                ndpointer(c_double, ndim=1, flags="C_CONTIGUOUS"),\
                                c_ulong]

# --- Frovedis ML data preparation ---
distinct_count = LIB.get_distinct_count
distinct_count.argtypes = [c_char_p, c_int, c_long, c_short] #host, port,\
                           #proxy, dtype
distinct_count.restype = c_int

get_distinct_elements = LIB.distinct_elements
get_distinct_elements.argtypes = [c_char_p, c_int, c_long, c_short] #host, port,\
                                  #proxy, dtype
get_distinct_elements.restype = py_object

dvector_to_numpy_array = LIB.dvector_to_numpy_array
dvector_to_numpy_array.argtypes = [c_char_p, c_int,  # host, port
                                   c_long, c_short,  # proxy, dtype
                                   c_ulong]          # size
dvector_to_numpy_array.restype = py_object

encode_frovedis_dvector_zero_based = LIB.encode_frovedis_dvector_zero_based
encode_frovedis_dvector_zero_based.argtypes = [c_char_p, c_int, #host, port
                                               c_long, c_short] #proxy, dtype
encode_frovedis_dvector_zero_based.restype = c_long # out proxy

encode_frovedis_int_dvector = LIB.encode_frovedis_int_dvector
encode_frovedis_int_dvector.argtypes = [c_char_p, c_int, c_long,  #host, port, proxy
                                        POINTER(c_int), POINTER(c_int), #src, target
                                        c_ulong] # size
encode_frovedis_int_dvector.restype = c_long # out proxy

encode_frovedis_long_dvector = LIB.encode_frovedis_long_dvector
encode_frovedis_long_dvector.argtypes = [c_char_p, c_int, c_long,  #host, port, proxy
                                         POINTER(c_long), POINTER(c_long), #src, target
                                         c_ulong] # size
encode_frovedis_long_dvector.restype = c_long # out proxy

encode_frovedis_ulong_dvector = LIB.encode_frovedis_ulong_dvector
encode_frovedis_ulong_dvector.argtypes = [c_char_p, c_int, c_long,  #host, port, proxy
                                         POINTER(c_ulong), POINTER(c_ulong), #src, target
                                         c_ulong] # size
encode_frovedis_ulong_dvector.restype = c_long # out proxy

encode_frovedis_float_dvector = LIB.encode_frovedis_float_dvector
encode_frovedis_float_dvector.argtypes = [c_char_p, c_int, c_long,  #host, port, proxy
                                          POINTER(c_float), POINTER(c_float), #src, target
                                          c_ulong] # size
encode_frovedis_float_dvector.restype = c_long # out proxy

encode_frovedis_double_dvector = LIB.encode_frovedis_double_dvector
encode_frovedis_double_dvector.argtypes = [c_char_p, c_int, c_long,  #host, port, proxy
                                           POINTER(c_double), POINTER(c_double), #src, target
                                           c_ulong] # size
encode_frovedis_double_dvector.restype = c_long # out proxy

# --- generic model functions ---
show_frovedis_model = LIB.show_frovedis_model
show_frovedis_model.argtypes = [c_char_p, c_int, c_int, c_short, c_short]

load_frovedis_model = LIB.load_frovedis_model
load_frovedis_model.argtypes = [c_char_p, c_int, c_int, c_short, c_short,\
                                c_char_p]

save_frovedis_model = LIB.save_frovedis_model
save_frovedis_model.argtypes = [c_char_p, c_int, c_int, c_short, c_short,\
                                c_char_p]

release_frovedis_model = LIB.release_frovedis_model
release_frovedis_model.argtypes = [c_char_p, c_int, c_int, c_short, c_short,\
                                    c_short, c_bool,]

get_weight_vector = LIB.get_frovedis_weight_vector
get_weight_vector.argtypes = [c_char_p, c_int, c_int, c_short, c_short]
get_weight_vector.restype = py_object

get_intercept_vector = LIB.get_frovedis_intercept_vector
get_intercept_vector.argtypes = [c_char_p, c_int, c_int, c_short, c_short]
get_intercept_vector.restype = py_object

parallel_float_glm_predict = LIB.parallel_float_glm_predict
parallel_float_glm_predict.argtypes = [c_char_p, c_int, c_int, c_short, c_long,\
                                 ndpointer(c_float, ndim=1,\
                                 flags="C_CONTIGUOUS"),\
                                 c_ulong, c_bool, c_short, c_bool]

parallel_double_glm_predict = LIB.parallel_double_glm_predict
parallel_double_glm_predict.argtypes = [c_char_p, c_int, c_int, c_short, c_long,\
                                 ndpointer(c_double, ndim=1,\
                                 flags="C_CONTIGUOUS"),\
                                 c_ulong, c_bool, c_short, c_bool]

# --- clustering APIs --- 

# 1. KMeans
kmeans_fit = LIB.kmeans_fit
kmeans_fit.argtypes = [c_char_p, c_int, c_long, c_int, #host, port, dptr, k
                       c_int, c_int, c_double,c_long,  #max_iter,n_init,tol,seed
                       c_int, c_int,                   #verbose, mid
                       c_short, c_short, c_bool,       #dtype, itype, dense,
                       c_bool]                         #use_shrink
kmeans_fit.restype = py_object

kmeans_fit_transform = LIB.kmeans_fit_transform
kmeans_fit_transform.argtypes = [c_char_p, c_int,      #host, port
                       c_long, c_int,                  #dptr, k
                       c_int, c_int, c_double,c_long,  #max_iter,n_init,tol,seed
                       c_int, c_int,                   #verbose, mid
                       c_short, c_short, c_bool,       #dtype, itype, dense,
                       c_bool]                         #use_shrink
kmeans_fit_transform.restype = py_object

kmeans_transform = LIB.kmeans_transform
kmeans_transform.argtypes = [c_char_p, c_int,          #host, port
                             c_int, c_short,           #mid, mdtype
                             c_long, c_short, c_bool]  #dptr, itype, dense
kmeans_transform.restype = py_object

get_kmeans_centroid = LIB.get_kmeans_centroid
get_kmeans_centroid.argtypes = [c_char_p, c_int,  #host, port
                                c_int, c_short]   #mid, mdtype
get_kmeans_centroid.restype = py_object           #centroid as 1D python list

parallel_kmeans_predict = LIB.parallel_kmeans_predict
parallel_kmeans_predict.argtypes = [c_char_p, c_int, c_int,
                                    c_short, c_long,
                                    ndpointer(c_int, ndim=1,\
                                    flags="C_CONTIGUOUS"),\
                                    c_ulong, c_short, c_bool]

kmeans_score = LIB.kmeans_score
kmeans_score.argtypes = [c_char_p, c_int,         #host, port
                         c_int, c_short,          #mid, mdtype
                         c_long, c_short, c_bool] #dptr, itype, dense
kmeans_score.restype = c_float

# 2. Agglomerative
aca_train = LIB.aca_train
aca_train.argtypes = [c_char_p, c_int, c_long, # host, port, data_proxy
                      c_int, c_char_p, # n_cluster, linkage
                      c_double, ndpointer(c_long, ndim=1, flags="C_CONTIGUOUS"), c_long,\
                      #ret, ret_size
                      c_int, c_int, # verbose, mid
                      c_short, c_short, c_bool] #dtype, itype, dense

acm_predict = LIB.acm_predict
acm_predict.argtypes = [c_char_p, c_int, #host, port
                        c_int, c_short,  #mid, mtype
                        c_int,           #ncluster
                        ndpointer(c_long, ndim=1, flags="C_CONTIGUOUS"), c_long]\
                        #ret, ret_size

get_acm_children = LIB.get_acm_children_vector
get_acm_children.argtypes = [c_char_p, c_int, c_int, c_short]
get_acm_children.restype = py_object

get_acm_distances = LIB.get_acm_distances_vector
get_acm_distances.argtypes = [c_char_p, c_int, c_int, c_short]
get_acm_distances.restype = py_object

get_acm_n_components = LIB.get_acm_n_connected_components
get_acm_n_components.argtypes = [c_char_p, c_int, c_int, c_short]
get_acm_n_components.restype = c_int

get_acm_n_clusters = LIB.get_acm_no_clusters
get_acm_n_clusters.argtypes = [c_char_p, c_int, c_int, c_short]
get_acm_n_clusters.restype = c_int

load_frovedis_acm = LIB.load_frovedis_acm
load_frovedis_acm.argtypes = [c_char_p, c_int, c_int, c_short, c_char_p, \
                              ndpointer(c_long, ndim=1, flags="C_CONTIGUOUS"), c_long]

# 3. spectral clustering
sca_train = LIB.sca_train
sca_train.argtypes = [c_char_p, #host
                      c_int, #port
                      c_long,#data
                      c_int, #n_clusters
                      c_int, #n_comp
                      c_int, #n_iter
                      c_double, #eps
                      c_double,#gamma
                      c_bool, #precomputed
                      c_bool, #norm_laplacian
                      c_int, #mode
                      c_bool, #drop_first
                      ndpointer(c_int, ndim=1, flags="C_CONTIGUOUS"),#labels
                      c_long, #labels array length
                      c_int, #verbose
                      c_int, #mid
                      c_short, #dtype
                      c_short, #itype
                      c_bool #dense
                     ]

get_scm_affinity_matrix = LIB.get_scm_aff_matrix
get_scm_affinity_matrix.argtypes = [c_char_p, c_int, c_int, c_short]
get_scm_affinity_matrix.restype = py_object

load_frovedis_scm = LIB.load_frovedis_scm
load_frovedis_scm.argtypes = [c_char_p, c_int, c_int, c_short, c_char_p]
load_frovedis_scm.restype = py_object

# 4. DBSCAN
dbscan_train = LIB.dbscan_train
dbscan_train.argtypes = [c_char_p, #host
                         c_int, #port
                         c_long,#data
                         ndpointer(c_double, ndim=1, flags="C_CONTIGUOUS"),#sample_weight
                         c_long,#sample_weight length
                         c_double, #eps
                         c_int, #min_pts
                         ndpointer(c_long, ndim=1, flags="C_CONTIGUOUS"),#labels
                         c_long, #labels array length
                         c_int, #verbose
                         c_int, #mid
                         c_short, #dtype
                         c_short, #itype
                         c_bool #dense
                        ]

get_dbscan_core_sample_indices = LIB.get_frovedis_dbscan_core_sample_indices
get_dbscan_core_sample_indices.argtypes = [c_char_p, c_int, c_int, c_short, c_short]
get_dbscan_core_sample_indices.restype = py_object

get_dbscan_components = LIB.get_frovedis_dbscan_components
get_dbscan_components.argtypes = [c_char_p, c_int, c_int, c_short, c_short]
get_dbscan_components.restype = py_object


# 5. Gaussian Mixture
gmm_train = LIB.gmm_train
gmm_train.argtypes = [c_char_p, c_int, c_long, # host, port, data_proxy
                      c_int, c_char_p, # n_components, covariance type 
                      c_double, c_int , c_int, #tol, max_iter, n_init
                      c_char_p, c_long, c_int, # init_param, seed, verbose
                      c_int, c_short, c_short, c_bool] #mid, dtype, itype, dense
gmm_train.restype = py_object

gmm_predict = LIB.gmm_predict
gmm_predict.argtypes = [c_char_p, c_int, c_int,
                        c_short, c_long,
                        ndpointer(c_long, ndim=1,\
                        flags="C_CONTIGUOUS"),\
                        c_ulong]

gmm_predict_proba = LIB.gmm_predict_proba
gmm_predict_proba.argtypes = [c_char_p, c_int, c_int,
                              c_short, c_long]

get_gmm_weights = LIB.get_gmm_weights_vector
get_gmm_weights.argtypes = [c_char_p, c_int, c_int, c_short]
get_gmm_weights.restype = py_object

get_gmm_covariances = LIB.get_gmm_covariances_vector
get_gmm_covariances.argtypes = [c_char_p, c_int, c_int, c_short]
get_gmm_covariances.restype = py_object

get_gmm_means = LIB.get_gmm_means_vector
get_gmm_means.argtypes = [c_char_p, c_int, c_int, c_short]
get_gmm_means.restype = py_object

get_gmm_converged = LIB.get_gmm_converged_bool
get_gmm_converged.argtypes = [c_char_p, c_int, c_int, c_short]
get_gmm_converged.restype = c_bool

get_gmm_lower_bound = LIB.get_gmm_lower_bound_val
get_gmm_lower_bound.argtypes = [c_char_p, c_int, c_int, c_short]
get_gmm_lower_bound.restype = c_double

get_gmm_score = LIB.get_gmm_score_val
get_gmm_score.argtypes = [c_char_p, c_int, c_int,
                              c_short, c_long]
get_gmm_score.restype = c_double

get_gmm_score_samples = LIB.get_gmm_score_samples_vec
get_gmm_score_samples.argtypes = [c_char_p, c_int, c_int,
                              c_short, c_long]
get_gmm_score_samples.restype = py_object

# --- classification APIs ---

# 1. Logistic Regression
lr = LIB.lr
lr.argtypes = [c_char_p, c_int, c_long, c_long,   #host,port,X,y
                   ndpointer(c_double, ndim=1, flags="C_CONTIGUOUS"),#sample_weight
                   c_long,#sample_weight length
                   c_int, c_double,                   #iter, lr_rate
                   c_int, c_double, c_bool,           #rtype, rparam, is_mult
                   c_bool, c_double, c_int, c_int,    #fit_icpt, tol, vb, mid
                   c_short, c_short, c_bool,          #dtype, itype, dense
                   c_char_p, c_bool,                  #solver, shrinking
                   c_bool]                            #warm_start
lr.restype = c_int #n_iter

# 2. Linear SVM Classification
svm = LIB.svm
svm.argtypes = [c_char_p, c_int, c_long, c_long, #host,port,X,y
                ndpointer(c_double, ndim=1, flags="C_CONTIGUOUS"),#sample_weight
                c_long,#sample_weight length
                c_int, c_double,                 #iter, lr_rate
                c_int, c_double,                 #rtype, rparam
                c_bool, c_double, c_int, c_int,  #fit_icpt, tol, vb, mid
                c_short, c_short, c_bool,        #dtype, itype, dense
                c_char_p, c_bool]                #solver, warm_start
svm.restype = c_int #n_iter

# 3. Kernel SVM Classification
frovedis_svc = LIB.frovedis_svc
frovedis_svc.argtypes = [c_char_p, c_int, c_long, c_long,  #host,port,X,y
                         c_double, c_double, c_int, c_int, #tol, C, cache, max_itr
                         c_char_p, c_double, c_double,     #kernel, gamma, coef
                         c_int, c_int, c_int,              #degree, vb, mid
                         c_short, c_short, c_bool]         #dtype, itype, dense

get_support_idx = LIB.get_frovedis_support_idx
get_support_idx.argtypes = [c_char_p, c_int, c_int, c_short, c_short]
get_support_idx.restype = py_object

get_support_vector = LIB.get_frovedis_support_vector
get_support_vector.argtypes = [c_char_p, c_int, c_int, c_short, c_short]
get_support_vector.restype = py_object

# 4. Naive Bayes
nb_train = LIB.nb_trainer
nb_train.argtypes = [c_char_p, c_int, c_long,
                     c_long, c_double, c_bool,
                     ndpointer(c_double, ndim=1, flags="C_CONTIGUOUS"),#class_prior
                     c_long,#lenght of class_prior
                     ndpointer(c_double, ndim=1, flags="C_CONTIGUOUS"),#sample_weight
                     c_long,#sample_weight length
                     c_int, c_char_p, c_double,
                     c_int, c_short, c_short, c_bool]

load_frovedis_nbm = LIB.load_frovedis_nbm
load_frovedis_nbm.argtypes = [c_char_p, c_int, c_int, c_short, c_char_p]
load_frovedis_nbm.restype = py_object

get_pi_vector = LIB.get_frovedis_pi_vector
get_pi_vector.argtypes = [c_char_p, c_int, c_int, c_short, c_short]
get_pi_vector.restype = py_object

get_feature_count = LIB.get_frovedis_feature_count
get_feature_count.argtypes = [c_char_p, c_int, c_int, c_short, c_short]
get_feature_count.restype = py_object

get_theta_vector = LIB.get_frovedis_theta_vector
get_theta_vector.argtypes = [c_char_p, c_int, c_int, c_short, c_short]
get_theta_vector.restype = py_object

get_cls_counts_vector = LIB.get_frovedis_cls_counts_vector
get_cls_counts_vector.argtypes = [c_char_p, c_int, c_int, c_short, c_short]
get_cls_counts_vector.restype = py_object

# --- regression APIs ---

# 1. Linear Regression
lnr_lapack = LIB.lnr_lapack
lnr_lapack.argtypes = [c_char_p, c_int,  # host, port
                       c_long, c_long,   # X, y
                       ndpointer(c_double, ndim=1, flags="C_CONTIGUOUS"),#sample_weight
                       c_long,#sample_weight length
                       c_bool, c_int,  # fit_icpt, vb
                       c_int, c_short]   # mid, dtype
lnr_lapack.restype = py_object

lnr_scalapack = LIB.lnr_scalapack
lnr_scalapack.argtypes = [c_char_p, c_int,  # host, port
                          c_long, c_long,   # X, y
                          ndpointer(c_double, ndim=1, flags="C_CONTIGUOUS"),#sample_weight
                          c_long,#sample_weight length
                          c_bool, c_int,  # fit_icpt, vb
                          c_int, c_short]   # mid, dtype

lnr = LIB.lnr
lnr.argtypes = [c_char_p, c_int, c_long, c_long, #host,port,X,y
                    ndpointer(c_double, ndim=1, flags="C_CONTIGUOUS"),#sample_weight
                    c_long,#sample_weight length
                    c_int, c_double,                 #iter, lr_rate
                    c_bool, c_double, c_int, c_int,  #fit_icpt, tol, vb, mid
                    c_short, c_short, c_bool,        #dtype, itype, dense
                    c_char_p, c_bool]                #solver, warm_start
lnr.restype = c_int #n_iter

# 2. Lasso Regression (L2)
lasso = LIB.lasso
lasso.argtypes = [c_char_p, c_int, c_long, c_long, #host,port,X,y
                      ndpointer(c_double, ndim=1, flags="C_CONTIGUOUS"),#sample_weight
                      c_long,#sample_weight length
                      c_int, c_double,                 #iter, lr_rate
                      c_double,                        #regparam
                      c_bool, c_double, c_int, c_int,  #fit_icpt, tol, vb, mid
                      c_short, c_short, c_bool,        #dtype, itype, dense
                      c_char_p, c_bool]                #solver, warm_start
lasso.restype = c_int #n_iter

# 3. Ridge Regression (L1)
ridge = LIB.ridge
ridge.argtypes = [c_char_p, c_int, c_long, c_long, #host,port,X,y
                      ndpointer(c_double, ndim=1, flags="C_CONTIGUOUS"),#sample_weight
                      c_long,#sample_weight length
                      c_int, c_double,                 #iter, lr_rate
                      c_double,                        #regparam
                      c_bool, c_double, c_int, c_int,  #fit_icpt, tol, vb, mid
                      c_short, c_short, c_bool,        #dtype, itype, dense
                      c_char_p, c_bool]                #solver, warm_start
ridge.restype = c_int #n_iter

# 4. SGDRegressor: SGDClassifier with "squared_loss"
lnr2_sgd = LIB.lnr2_sgd
lnr2_sgd.argtypes = [c_char_p, c_int, c_long, c_long, #host,port,X,y
                     ndpointer(c_double, ndim=1, flags="C_CONTIGUOUS"),#sample_weight
                     c_long,#sample_weight length
                     c_int, c_double,                 #iter, lr_rate
                     c_int, c_double,                 #rtype, rparam
                     c_bool, c_double, c_int, c_int,  #fit_icpt, tol, vb, mid
                     c_short, c_short, c_bool,        #dtype, itype, dense
                     c_bool]                          #warm_start
lnr2_sgd.restype = c_int #n_iter

# 5. Linear SVM Regression
svm_regressor = LIB.svm_regressor
svm_regressor.argtypes = [c_char_p, c_int, c_long, c_long, #host,port,X,y
                          ndpointer(c_double, ndim=1, flags="C_CONTIGUOUS"),#sample_weight
                          c_long,#sample_weight length
                          c_int, c_double,                 #iter, lr_rate
                          c_double,                        #eps
                          c_int, c_double,                 #rtype, rparam
                          c_bool, c_double,                #fit_icpt, tol
                          c_int, c_int, c_int,             #loss, vb, mid
                          c_short, c_short, c_bool,        #dtype, itype, dense
                          c_char_p, c_bool]                #solver, warm_start
svm_regressor.restype = c_int #n_iter

# --- tree/ensemble APIs ---

# 1. Decision Tree
dt_train = LIB.dt_trainer
dt_train.argtypes = [c_char_p, c_int, c_long,    # host, port, X_proxy 
                     c_long, c_char_p, c_char_p, # y_proxy, algo, criterion
                     c_int, c_int, c_int, c_int, # depth, ncls, bins, min-leaf
                     c_float, c_int, c_int,    # impurity-dec, verbose, mid
                     ndpointer(c_int, ndim=1, flags="C_CONTIGUOUS"), # cat-key
                     ndpointer(c_int, ndim=1, flags="C_CONTIGUOUS"), # cat-val
                     c_ulong, # info-dict-size
                     c_short, c_short, c_bool] # dtype, itype, dense

# 2. Random Forest
rf_train = LIB.rf_trainer
rf_train.argtypes = [c_char_p, c_int, c_long,         #host,port,X
                     c_long, c_char_p, c_char_p,      #y,algo,criterion
                     c_int, c_int, c_int, c_char_p,   #n_est,max_dep,n_cl,feature_subset_strat
                     c_double, #feature_subset_rate
                     c_int, c_int, c_double, c_long,   #mx_bin ,min_sample_leaf,min_impurity_decrease,seed
                     c_int, c_int, c_short, c_short,  #vb,mid,dtype,itype
                     c_bool]                          #dense

# 3. GBDT
gbt_train = LIB.gbt_trainer
gbt_train.argtypes = [c_char_p, c_int, # host, port
                     c_long, c_long, # xptr, yptr
                     c_char_p, c_char_p, c_char_p, # algo, loss, impurity
                     c_double, c_int, c_double, # learning_rate, max_depth, min_impurity_decrease
                     c_int, c_double, c_int, # seed, tol, max_bins
                     c_double, c_char_p, c_double, #subsampling rate, strategy, subset_features
                     c_int, c_int, # n_estimators, nclasses
                     c_int, c_int, # verbose, mid
                     c_short, c_short, c_bool # dtype, itype, dense
                     ]

# --- recommendation APIs ---

# 1. ALS
# als will always be trained with sparse data
als_train = LIB.als_train
als_train.argtypes = [c_char_p, c_int, c_long, c_int, c_int,
                      c_double, c_double, c_double, c_long, c_int, c_int,
                      c_short, c_short]
als_float_predict = LIB.als_float_predict
als_float_predict.argtypes = [c_char_p, c_int, c_int,\
                        ndpointer(c_int, ndim=1, flags="C_CONTIGUOUS"),\
                        ndpointer(c_float, ndim=1, flags="C_CONTIGUOUS"),\
                        c_ulong]

als_double_predict = LIB.als_double_predict
als_double_predict.argtypes = [c_char_p, c_int, c_int,\
                        ndpointer(c_int, ndim=1, flags="C_CONTIGUOUS"),\
                        ndpointer(c_double, ndim=1, flags="C_CONTIGUOUS"),\
                        c_ulong]

als_float_rec_users = LIB.als_float_rec_users
als_float_rec_users.argtypes = [c_char_p, c_int, c_int, c_int, c_int,\
                          ndpointer(c_int, ndim=1, flags="C_CONTIGUOUS"),\
                          ndpointer(c_float, ndim=1, flags="C_CONTIGUOUS")]

als_double_rec_users = LIB.als_double_rec_users
als_double_rec_users.argtypes = [c_char_p, c_int, c_int, c_int, c_int,\
                          ndpointer(c_int, ndim=1, flags="C_CONTIGUOUS"),\
                          ndpointer(c_double, ndim=1, flags="C_CONTIGUOUS")]

als_float_rec_prods = LIB.als_float_rec_prods
als_float_rec_prods.argtypes = [c_char_p, c_int, c_int, c_int, c_int,\
                          ndpointer(c_int, ndim=1, flags="C_CONTIGUOUS"),\
                          ndpointer(c_float, ndim=1, flags="C_CONTIGUOUS")]

als_double_rec_prods = LIB.als_double_rec_prods
als_double_rec_prods.argtypes = [c_char_p, c_int, c_int, c_int, c_int,\
                          ndpointer(c_int, ndim=1, flags="C_CONTIGUOUS"),\
                          ndpointer(c_double, ndim=1, flags="C_CONTIGUOUS")]

load_frovedis_mfm = LIB.load_frovedis_mfm
load_frovedis_mfm.argtypes = [c_char_p, c_int, c_int, c_short, c_char_p]
load_frovedis_mfm.restype = py_object

# 2. Factorization Machines (currently supports classification and regression)
fm_train = LIB.fm_trainer
fm_train.argtypes = [c_char_p, c_int,
                     c_long, c_long,
                     c_double, c_int,
                     c_double, c_char_p,
                     c_bool, c_bool, c_int,
                     c_double, c_double, c_double,
                     c_int, c_int, c_bool, c_int,
                     c_short, c_short]

# --- pattern mining APIs ---

# 1. FP Growth
fpgrowth_generate_fis = LIB.fpgrowth_generate_fis
fpgrowth_generate_fis.argtypes = [c_char_p, c_int, c_long, # host, port, fdata
                                  c_int, c_double,  # mid, minSupport
                                  c_int, c_int,     # tree_depth, c-point
                                  c_int, c_int]     # opt-level, verbose
fpgrowth_generate_fis.restype = c_int # returns fis count as int

get_fpgrowth_fis = LIB.fpgrowth_freq_items
get_fpgrowth_fis.argtypes = [c_char_p, c_int, c_int] #host, port, mid
get_fpgrowth_fis.restype = py_object

fpgrowth_generate_rules = LIB.fpgrowth_generate_rules
fpgrowth_generate_rules.argtypes = [c_char_p, c_int, # host, port,
                                    c_int, c_int,    # model-mid, rule-mid
                                    c_double]        # minConfidence
fpgrowth_generate_rules.restype = c_int # returns rules count as int

get_association_rules = LIB.fpgrowth_rules
get_association_rules.argtypes = [c_char_p, c_int, c_int] #host, port, mid
get_association_rules.restype = py_object

load_fp_model = LIB.load_fp_model
load_fp_model.argtypes = [c_char_p, c_int, c_int, # host, port, mid
                          c_short, c_char_p]      # mkind, fname
load_fp_model.restype = c_int  # returns fis/rule (depends on mkind) count as int

# --- natual language APIs ---

# 1. Word2Vec
w2v_build_vocab_and_dump = LIB.w2v_build_vocab_and_dump
w2v_build_vocab_and_dump.argtypes = [c_char_p, c_char_p, #text, encode
                                     c_char_p, c_char_p, #vocab, count
                                     c_int]              #minCount

w2v_train = LIB.w2v_train
w2v_train.argtypes = [c_char_p, c_int,              #host, port
                      c_char_p, c_char_p,           #encode, count
                      c_int, c_int, c_float, c_int, #hidden, window, thr, neg
                      c_int, c_float, c_float,      #iter, lr, syncperiod
                      c_int, c_int,                 #syncWords, syncTimes,
                      c_int, c_int]                 #msgSize, nthreads
w2v_train.restype = py_object

# --- Frovedis PBLAS Wrappers ---
pswap = LIB.pswap
pswap.argtypes = [c_char_p, c_int, c_long, c_long, c_short]

pcopy = LIB.pcopy
pcopy.argtypes = [c_char_p, c_int, c_long, c_long, c_short]

pscal = LIB.pscal
pscal.argtypes = [c_char_p, c_int, c_long, c_double, c_short]

paxpy = LIB.paxpy
paxpy.argtypes = [c_char_p, c_int, c_long, c_long, c_double, c_short]

pdot = LIB.pdot
pdot.argtypes = [c_char_p, c_int, c_long, c_long, c_short]
pdot.restype = c_double

pnrm2 = LIB.pnrm2
pnrm2.argtypes = [c_char_p, c_int, c_long, c_short]
pnrm2.restype = c_double

pgemv = LIB.pgemv
pgemv.argtypes = [c_char_p, c_int, c_long, c_long,
                  c_bool, c_double, c_double, c_short]
pgemv.restype = py_object

pger = LIB.pger
pger.argtypes = [c_char_p, c_int, c_long, c_long, c_double, c_short]
pger.restype = py_object

pgemm = LIB.pgemm
pgemm.argtypes = [c_char_p, c_int, c_long, c_long,
                  c_bool, c_bool, c_double, c_double, c_short]
pgemm.restype = py_object

pgeadd = LIB.pgeadd
pgeadd.argtypes = [c_char_p, c_int, c_long, c_long,
                   c_bool, c_double, c_double, c_short]

# --- Frovedis SCALAPACK Wrappers ---
pgetrf = LIB.pgetrf
pgetrf.argtypes = [c_char_p, c_int, c_long, c_short]
pgetrf.restype = py_object

pgetri = LIB.pgetri
pgetri.argtypes = [c_char_p, c_int, c_long, c_long, c_short]
pgetri.restype = c_int

pgetrs = LIB.pgetrs
pgetrs.argtypes = [c_char_p, c_int, c_long, c_long, c_long, c_bool, c_short]
pgetrs.restype = c_int

pgesv = LIB.pgesv
pgesv.argtypes = [c_char_p, c_int, c_long, c_long, c_short]
pgesv.restype = c_int

pgels = LIB.pgels
pgels.argtypes = [c_char_p, c_int, c_long, c_long, c_bool, c_short]
pgels.restype = c_int

pgesvd = LIB.pgesvd
pgesvd.argtypes = [c_char_p, c_int, c_long, c_bool, c_bool, c_short]
pgesvd.restype = py_object

# --- decomposition APIs ---

# 1. SVD
compute_var_sum = LIB.sum_of_variance
compute_var_sum.argtypes = [c_char_p, c_int, # host, port
                            c_long, c_bool,  # mptr, sample_variance
                            c_bool, c_short] # isdense, dtype
compute_var_sum.restype = c_double

compute_truncated_svd = LIB.compute_truncated_svd
compute_truncated_svd.argtypes = [c_char_p, c_int, c_long, c_int,
                                  c_short, c_short, c_bool, c_bool]
compute_truncated_svd.restype = py_object

compute_svd_self_transform = LIB.compute_svd_self_transform
compute_svd_self_transform.argtypes = [c_char_p, c_int, # host, port
                                       c_long, c_long,  # umat_ptr, sval_ptr
                                       c_short]         # dtype 
compute_svd_self_transform.restype = py_object


compute_svd_transform = LIB.compute_svd_transform
compute_svd_transform.argtypes = [c_char_p, c_int,  # host, port
                                  c_long, c_short,  # Xptr, dtype 
                                  c_short, c_bool,  # itype, isDense
                                  c_long]           # comp_ptr
compute_svd_transform.restype = py_object

compute_svd_inverse_transform = LIB.compute_svd_inverse_transform
compute_svd_inverse_transform.argtypes = [c_char_p, c_int,  # host, port
                                          c_long, c_short,  # Xptr, dtype 
                                          c_short, c_bool,  # itype, isDense
                                          c_long]           # comp_ptr
compute_svd_inverse_transform.restype = py_object

# svd result structures
release_ipiv = LIB.release_ipiv
release_ipiv.argtypes = [c_char_p, c_int, c_char, c_long]

save_as_diag_matrix = LIB.save_as_diag_matrix
save_as_diag_matrix.argtypes = [c_char_p, c_int, c_long, c_char_p, c_bool,\
                                c_char]

get_svd_results_from_file = LIB.get_svd_results_from_file
get_svd_results_from_file.argtypes = [c_char_p, c_int,
                                      c_char_p, c_char_p, c_char_p,
                                      c_bool, c_bool, c_bool, c_char, c_char]
get_svd_results_from_file.restype = py_object

# 2. PCA
compute_pca = LIB.compute_pca
compute_pca.argtypes = [ c_char_p, #host
                         c_int,  #port
                         c_long, #data
                         c_int,  #k
                         c_bool, #whiten
                         c_short, #dtype
                         c_bool, #to_copy 
                         c_bool #movable 
                       ]
compute_pca.restype = py_object

pca_transform = LIB.pca_transform
pca_transform.argtypes = [ c_char_p, #host
                           c_int,  #port
                           c_long, #data
                           c_long, #pca _directions
                           c_long, #explained variance
                           c_long, #mean
                           c_short, #dtype
                           c_bool #whiten
                         ]
pca_transform.restype = py_object

pca_inverse_transform = LIB.pca_inverse_transform
pca_inverse_transform.argtypes = [ c_char_p, #host
                                   c_int,  #port
                                   c_long, #data
                                   c_long, #pca _directions
                                   c_long, #explained variance
                                   c_long, #mean
                                   c_short, #dtype
                                   c_bool #whiten
                                 ]
pca_inverse_transform.restype = py_object

# 3. LDA
compute_lda_train = LIB.compute_lda_train
compute_lda_train.argtypes = [c_char_p, c_int,\
                              c_ulong, c_double,\
                              c_double, c_int,\
                              c_int, c_char_p,\
                              c_int, c_int,\
                              c_short, c_short,\
                              c_int, c_int]

compute_lda_transform = LIB.compute_lda_transform
compute_lda_transform.argtypes = [c_char_p, c_int,\
                                  c_ulong, c_double,\
                                  c_double, c_int,\
                                  c_char_p,\
                                  c_int, c_int,\
                                  c_short, c_short]
compute_lda_transform.restype = py_object

compute_lda_component = LIB.compute_lda_component
compute_lda_component.argtypes = [c_char_p, c_int,\
                                  c_int, c_short]
compute_lda_component.restype = py_object

# --- manifold APIs ---

# 1. TSNE
compute_tsne = LIB.compute_tsne
compute_tsne.argtypes = [ c_char_p, #host
                         c_int,  #port
                         c_long, #data
                         c_double,  #perplexity
                         c_double, #early_exaggeration
                         c_double, #min_grad_norm
                         c_double, #learning_rate
                         c_int, #n_components
                         c_int, #n_iter
                         c_int, #n_iter_without_progress
                         c_char_p, #metric
                         c_char_p, #method
                         c_char_p, #init
                         c_bool, #verbose
                         c_short #dtype
                       ]
compute_tsne.restype = py_object

# 2. spectral embedding
sea_train = LIB.sea_train
sea_train.argtypes = [c_char_p, #host
                      c_int,  #port
                      c_long,#data
                      c_int, #n_components
                      c_double,#gamma
                      c_bool, #precomputed
                      c_bool, #norm_laplacian
                      c_int, #mode
                      c_bool, #drop_first
                      c_int, #verbose
                      c_int, #mid
                      c_short, #dtype
                      c_short,#itype
                      c_bool #dense
                     ]

get_sem_affinity_matrix = LIB.get_sem_aff_matrix
get_sem_affinity_matrix.argtypes = [c_char_p, c_int, c_int, c_short]
get_sem_affinity_matrix.restype = py_object

get_sem_embedding_matrix = LIB.get_sem_embed_matrix
get_sem_embedding_matrix.argtypes = [c_char_p, c_int, c_int, c_short]
get_sem_embedding_matrix.restype = py_object

# --- neighbor APIs ---

# 1. KNN - with both regressor and classifier
knn_fit = LIB.knn_fit
knn_fit.argtypes = [  c_char_p, #host
                      c_int,  #port
                      c_long, #data
                      c_int, #k
                      c_float, #radius
                      c_char_p, #algorithm
                      c_char_p, # metric
                      c_float, #chunk_size
                      c_int, #vb
                      c_int, #mid
                      c_short, #dtype
                      c_short, #itype
                      c_bool #dense
                   ]

knc_fit = LIB.knc_fit
knc_fit.argtypes = [  c_char_p, #host
                      c_int,  #port
                      c_long, #data - mat
                      c_long, #data - labels
                      c_int, #k
                      c_char_p, #algorithm
                      c_char_p, # metric
                      c_float, #chunk_size
                      c_int, #vb
                      c_int, #mid
                      c_short, #dtype
                      c_short, #itype
                      c_bool #dense
                   ]

knr_fit = LIB.knr_fit
knr_fit.argtypes = [  c_char_p, #host
                      c_int,  #port
                      c_long, #data - mat
                      c_long, #data - labels
                      c_int, #k
                      c_char_p, #algorithm
                      c_char_p, # metric
                      c_float, #chunk_size
                      c_int, #vb
                      c_int, #mid
                      c_short, #dtype
                      c_short, #itype
                      c_bool #dense
                   ]

knc_kneighbors = LIB.knc_kneighbors
knc_kneighbors.argtypes = [c_char_p, #host
                           c_int,  #port
                           c_long, #data
                           c_int, #k
                           c_int, #mid
                           c_bool, #need distance,
                           c_short, #dtype
                           c_short, #modelitype
                           c_bool #modeldense
                          ]
knc_kneighbors.restype = py_object

knr_kneighbors = LIB.knr_kneighbors
knr_kneighbors.argtypes = [ c_char_p, #host
                            c_int,  #port
                            c_long, #data
                            c_int, #k
                            c_int, #mid
                            c_bool, #need distance,
                            c_short, #dtype
                            c_short, #itype
                            c_bool, #dense
                            c_short, #modelitype
                            c_bool #modeldense
                          ]
knr_kneighbors.restype = py_object

knc_kneighbors_graph = LIB.knc_kneighbors_graph
knc_kneighbors_graph.argtypes = [ c_char_p, #host
                                  c_int,  #port
                                  c_long, #data
                                  c_int, #k
                                  c_int, #mid
                                  c_char_p, #mode,
                                  c_short, #dtype
                                  c_short, #itype
                                  c_bool, #dense
                                  c_short, #modelitype
                                  c_bool #modeldense
                                ]
knc_kneighbors_graph.restype = py_object

knr_kneighbors_graph = LIB.knr_kneighbors_graph
knr_kneighbors_graph.argtypes = [ c_char_p, #host
                                  c_int,  #port
                                  c_long, #data
                                  c_int, #k
                                  c_int, #mid
                                  c_char_p, #mode,
                                  c_short, #dtype
                                  c_short, #itype
                                  c_bool, #dense
                                  c_short, #modelitype
                                  c_bool #modeldense
                                ]
knr_kneighbors_graph.restype = py_object

knc_float_predict = LIB.knc_float_predict
knc_float_predict.argtypes = [ c_char_p, #host
                               c_int,  #port
                               c_long, #data
                               c_int, #mid
                               c_bool, # save_proba
                               ndpointer(c_float, ndim=1, flags="C_CONTIGUOUS"),
                               c_long, # ret length
                               c_short, #itype
                               c_bool, #dense
                               c_short, #modelitype
                               c_bool #modeldense
                             ]

knc_double_predict = LIB.knc_double_predict
knc_double_predict.argtypes = [ c_char_p, #host
                                c_int,  #port
                                c_long, #data
                                c_int, #mid
                                c_bool, # save_proba
                                ndpointer(c_double, ndim=1, flags="C_CONTIGUOUS"),
                                c_long, # ret length
                                c_short, #itype
                                c_bool, #dense
                                c_short, #modelitype
                                c_bool #modeldense
                              ]

knr_float_predict = LIB.knr_float_predict
knr_float_predict.argtypes = [ c_char_p, #host
                               c_int,  #port
                               c_long, #data
                               c_int, #mid
                               ndpointer(c_float, ndim=1, flags="C_CONTIGUOUS"),
                               c_long, # ret length
                               c_short, #itype
                               c_bool, #dense
                               c_short, #modelitype
                               c_bool #modeldense
                             ]

knr_double_predict = LIB.knr_double_predict
knr_double_predict.argtypes = [ c_char_p, #host
                                c_int,  #port
                                c_long, #data
                                c_int, #mid
                                ndpointer(c_double, ndim=1, flags="C_CONTIGUOUS"),
                                c_long, # ret length
                                c_short, #itype
                                c_bool, #dense
                                c_short, #modelitype
                                c_bool #modeldense
                              ]

knc_predict_proba = LIB.knc_predict_proba
knc_predict_proba.argtypes = [ c_char_p, #host
                               c_int,  #port
                               c_long, #data
                               c_int, #mid
                               c_short, #dtype
                               c_short, #itype
                               c_bool, #dense
                               c_short, #modelitype
                               c_bool #modeldense
                             ]
knc_predict_proba.restype = py_object

knr_model_score = LIB.knr_model_score
knr_model_score.argtypes = [ c_char_p, #host
                             c_int,  #port
                             c_long, #data - mat
                             c_long, #data - labels
                             c_int, #mid
                             c_short, #dtype
                             c_short, #itype
                             c_bool, #dense
                             c_short, #modelitype
                             c_bool #modeldense
                           ]
knr_model_score.restype = c_float

knc_model_score = LIB.knc_model_score
knc_model_score.argtypes = [ c_char_p, #host
                             c_int,  #port
                             c_long, #data - mat
                             c_long, #data - labels
                             c_int, #mid
                             c_short, #dtype
                             c_short, #itype
                             c_bool, #dense
                             c_short, #modelitype
                             c_bool #modeldense
                           ]
knc_model_score.restype = c_float

knn_kneighbors = LIB.knn_kneighbors
knn_kneighbors.argtypes = [c_char_p, #host
                           c_int,  #port
                           c_long, #data
                           c_int, #k
                           c_int, #mid
                           c_bool, #need distance,
                           c_short, #dtype
                           c_short, #itype
                           c_bool, #dense
                           c_short, #modelitype
                           c_bool #modeldense
                          ]
knn_kneighbors.restype = py_object

knn_kneighbors_graph = LIB.knn_kneighbors_graph
knn_kneighbors_graph.argtypes = [ c_char_p, #host
                                  c_int,  #port
                                  c_long, #data
                                  c_int, #k
                                  c_int, #mid
                                  c_char_p, #mode,
                                  c_short, #dtype
                                  c_short, #itype
                                  c_bool, #dense
                                  c_short, #modelitype
                                  c_bool #modeldense
                                ]
knn_kneighbors_graph.restype = py_object

knn_radius_neighbors = LIB.knn_radius_neighbors
knn_radius_neighbors.argtypes = [ c_char_p, c_int, #host ,port
                                  c_long, c_float, c_int, #data, radius, mid
                                  c_short,  #dtype
                                  c_short, #itype
                                  c_bool, #dense
                                  c_short, #modelitype
                                  c_bool #modeldense
                                ]
knn_radius_neighbors.restype = py_object

knn_radius_neighbors_graph = LIB.knn_radius_neighbors_graph
knn_radius_neighbors_graph.argtypes = [ c_char_p, c_int, #host ,port
                                        c_long, c_float, c_int, #data, radius, mid
                                        c_char_p, c_short,  #mode, dtype
                                        c_short, #itype
                                        c_bool, #dense
                                        c_short, #modelitype
                                        c_bool #modeldense
                                      ]
knn_radius_neighbors_graph.restype = py_object

# --- networkx (graph) APIs ---

# 1. common graph functionalities
set_graph_data= LIB.set_graph_data
set_graph_data.argtypes = [c_char_p, c_int,\
                           c_ulong]
set_graph_data.restype = c_ulong

get_graph_data= LIB.get_graph_data
get_graph_data.argtypes = [c_char_p, c_int,\
                           c_ulong]
get_graph_data.restype = c_ulong

show_graph_py= LIB.show_graph_py
show_graph_py.argtypes = [c_char_p, c_int,\
                           c_ulong]

release_graph_py= LIB.release_graph_py
release_graph_py.argtypes = [c_char_p, c_int,\
                           c_ulong]

save_graph_py= LIB.save_graph_py
save_graph_py.argtypes = [c_char_p, c_int,\
                 c_ulong, c_char_p]

load_graph_from_text_file= LIB.load_graph_from_text_file
load_graph_from_text_file.argtypes = [c_char_p, c_int,\
                                      c_char_p]
load_graph_from_text_file.restype = py_object

copy_graph_py= LIB.copy_graph_py
copy_graph_py.argtypes = [c_char_p, c_int,\
                          c_ulong]
copy_graph_py.restype = c_ulong

# 2. PageRank
call_frovedis_pagerank = LIB.call_frovedis_pagerank
call_frovedis_pagerank.argtypes = [c_char_p, c_int,\
                                  c_long, c_double,\
                                  c_double, c_int, c_int]
call_frovedis_pagerank.restype = py_object                 # dist of lists

# 3. Single Source Shortest Path (bellman-ford)
call_frovedis_sssp= LIB.call_frovedis_sssp
call_frovedis_sssp.argtypes = [c_char_p, c_int,            # host, port
                               c_ulong,                    # graph
                               c_ulong]                    # nvertices, source
call_frovedis_sssp.restype = py_object                     # sssp result dict of lists

# 4. BFS
call_frovedis_bfs = LIB.call_frovedis_bfs
call_frovedis_bfs.argtypes = [c_char_p, c_int,             # host, port
                              c_ulong,                     # graph
                              c_ulong,                     # source
                              c_int, c_double,             # opt-level, hyb-threshold
                              c_ulong]                     # depth-limit
call_frovedis_bfs.restype = py_object                      # bfs result dict of lists

bfs_descendants_at_distance = LIB.bfs_descendants_at_distance
bfs_descendants_at_distance.argtypes = [c_char_p, c_int,   # host, port
                              c_ulong,                     # graph
                              c_ulong,                     # source
                              c_int, c_double,             # opt-level, hyb-threshold
                              c_ulong]                     # depth-limit
bfs_descendants_at_distance.restype = py_object            # python list

# 5. Connected Components (Weekly)
call_frovedis_cc = LIB.call_frovedis_cc
call_frovedis_cc.argtypes = [c_char_p, c_int,             # host, port
                             c_ulong,                     # graph
                             ndpointer(c_long, ndim=1,    # nodes_in_which_cc
                                       flags="C_CONTIGUOUS"),
                             ndpointer(c_long, ndim=1,    # nodes_dist
                                       flags="C_CONTIGUOUS"),
                             c_ulong,                     # nvertices
                             c_int, c_double]             # opt-level, hyb-threshold
call_frovedis_cc.restype = py_object

