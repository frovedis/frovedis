#!/usr/bin/env python

# This source provides the interfaces to call the C/C++ functions from python code

import numpy as np
from ctypes import *
from numpy.ctypeslib import ndpointer
from scipy.sparse import csr_matrix

# A dynamic library containing implementation of server side code
try:
  lib = CDLL("libfrovedis_client_python.so")
except OSError: 
  try: 
    lib = CDLL("../lib/libfrovedis_client_python.so")
  except OSError:
    raise OSError("libfrovedis_client_python.so: No such dll found (set LD_LIBRARY_PATH)")

# --- Frovedis Server ---
initialize_server = lib.initialize_server
initialize_server.argtypes = [c_char_p]
initialize_server.restype = py_object

get_worker_size = lib.get_worker_size
get_worker_size.argtypes = [c_char_p, c_int]
get_worker_size.restype = c_int

clean_server = lib.clean_server
clean_server.argtypes = [c_char_p, c_int]

finalize_server = lib.finalize_server
finalize_server.argtypes = [c_char_p, c_int]

check_server_exception = lib.check_server_exception
check_server_exception.restype = py_object

# --- Frovedis dvector ---
# create from numpy array
create_frovedis_int_dvector = lib.create_frovedis_int_dvector
create_frovedis_int_dvector.argtypes = [c_char_p, c_int,
                                  ndpointer(c_int,ndim=1,flags="C_CONTIGUOUS"),
                                  c_int]
create_frovedis_int_dvector.restype = py_object

create_frovedis_long_dvector = lib.create_frovedis_long_dvector
create_frovedis_long_dvector.argtypes = [c_char_p, c_int,
                                  ndpointer(c_long,ndim=1,flags="C_CONTIGUOUS"),
                                  c_int]
create_frovedis_long_dvector.restype = py_object

create_frovedis_float_dvector = lib.create_frovedis_float_dvector
create_frovedis_float_dvector.argtypes = [c_char_p, c_int,
                                  ndpointer(c_float,ndim=1,flags="C_CONTIGUOUS"),
                                  c_int]
create_frovedis_float_dvector.restype = py_object

create_frovedis_double_dvector = lib.create_frovedis_double_dvector
create_frovedis_double_dvector.argtypes = [c_char_p, c_int,
                                  ndpointer(c_double,ndim=1,flags="C_CONTIGUOUS"),
                                  c_int]
create_frovedis_double_dvector.restype = py_object

create_frovedis_string_dvector = lib.create_frovedis_string_dvector
create_frovedis_string_dvector.argtypes = [c_char_p, c_int, POINTER(c_char_p), c_int]
create_frovedis_string_dvector.restype = py_object

#To Print Created dvector

show_frovedis_dvector = lib.show_frovedis_dvector
show_frovedis_dvector.argtypes = [c_char_p, c_int, c_long, c_int]

release_frovedis_dvector = lib.release_frovedis_dvector
release_frovedis_dvector.argtypes = [c_char_p, c_int, c_long, c_int]

#----Frovedis Dataframe from Python--------------------
create_frovedis_dataframe = lib.create_frovedis_dataframe
create_frovedis_dataframe.argtypes = [c_char_p, c_int, POINTER(c_short),
                                      POINTER(c_char_p), POINTER(c_long), c_int]
create_frovedis_dataframe.restype = c_long

show_frovedis_dataframe = lib.show_frovedis_dataframe
show_frovedis_dataframe.argtypes = [c_char_p, c_int, c_long]

release_frovedis_dataframe = lib.release_frovedis_dataframe
release_frovedis_dataframe.argtypes = [c_char_p, c_int, c_long]

release_dfoperator = lib.release_dfoperator
release_dfoperator.argtypes = [c_char_p, c_int, c_long]

get_frovedis_dfoperator = lib.get_frovedis_dfoperator
get_frovedis_dfoperator.argtypes = [c_char_p, c_int, c_char_p, c_char_p, c_short, c_short, c_bool]
get_frovedis_dfoperator.restype = c_long

get_dfANDoperator = lib.get_frovedis_dfANDoperator
get_dfANDoperator.argtypes = [c_char_p, c_int, c_long, c_long]
get_dfANDoperator.restype = c_long

get_dfORoperator = lib.get_frovedis_dfORoperator
get_dfORoperator.argtypes = [c_char_p, c_int, c_long, c_long]
get_dfORoperator.restype = c_long

filter_frovedis_dataframe = lib.filter_frovedis_dataframe
filter_frovedis_dataframe.argtypes = [c_char_p, c_int, c_long, c_long]
filter_frovedis_dataframe.restype = c_long

select_frovedis_dataframe = lib.select_frovedis_dataframe
select_frovedis_dataframe.argtypes = [c_char_p, c_int, c_long, POINTER(c_char_p), c_int]
select_frovedis_dataframe.restype = c_long

sort_frovedis_dataframe = lib.sort_frovedis_dataframe
sort_frovedis_dataframe.argtypes = [c_char_p, c_int, c_long, POINTER(c_char_p), c_int, c_bool]
sort_frovedis_dataframe.restype = c_long

group_frovedis_dataframe = lib.group_frovedis_dataframe
group_frovedis_dataframe.argtypes = [c_char_p, c_int, c_long, POINTER(c_char_p), c_int]
group_frovedis_dataframe.restype = c_long

agg_grouped_dataframe = lib.agg_grouped_dataframe
agg_grouped_dataframe.argtypes = [c_char_p, c_int, c_long, 
                                  POINTER(c_char_p), c_ulong,
                                  POINTER(c_char_p), POINTER(c_char_p), 
                                  POINTER(c_char_p), c_ulong]
agg_grouped_dataframe.restype = c_long

merge_frovedis_dataframe = lib.join_frovedis_dataframe
merge_frovedis_dataframe.argtypes = [c_char_p, c_int, c_long, c_long, c_long,
                                    c_char_p, c_char_p]
merge_frovedis_dataframe.restype = c_long

rename_frovedis_dataframe = lib.rename_frovedis_dataframe
rename_frovedis_dataframe.argtypes = [c_char_p, c_int, c_long, 
                                      POINTER(c_char_p), POINTER(c_char_p), c_int]
rename_frovedis_dataframe.restype = c_long

get_min_frovedis_dataframe = lib.min_frovedis_dataframe
get_min_frovedis_dataframe.argtypes = [c_char_p, c_int, c_long, 
                                       POINTER(c_char_p), POINTER(c_short), c_int]
get_min_frovedis_dataframe.restype = py_object 

get_max_frovedis_dataframe = lib.max_frovedis_dataframe
get_max_frovedis_dataframe.argtypes = [c_char_p, c_int, c_long, 
                                       POINTER(c_char_p), POINTER(c_short), c_int]
get_max_frovedis_dataframe.restype = py_object

get_sum_frovedis_dataframe = lib.sum_frovedis_dataframe
get_sum_frovedis_dataframe.argtypes = [c_char_p, c_int, c_long, 
                                       POINTER(c_char_p), POINTER(c_short), c_int]
get_sum_frovedis_dataframe.restype = py_object

get_avg_frovedis_dataframe = lib.avg_frovedis_dataframe
get_avg_frovedis_dataframe.argtypes = [c_char_p, c_int, c_long, 
                                       POINTER(c_char_p), c_int]
get_avg_frovedis_dataframe.restype = py_object

get_cnt_frovedis_dataframe = lib.cnt_frovedis_dataframe
get_cnt_frovedis_dataframe.argtypes = [c_char_p, c_int, c_long, 
                                       POINTER(c_char_p), c_int]
get_cnt_frovedis_dataframe.restype = py_object

get_std_frovedis_dataframe = lib.std_frovedis_dataframe
get_std_frovedis_dataframe.argtypes = [c_char_p, c_int, c_long, 
                                       POINTER(c_char_p), POINTER(c_short), c_int]
get_std_frovedis_dataframe.restype = py_object

get_frovedis_col = lib.get_frovedis_col
get_frovedis_col.argtypes = [c_char_p, c_int, c_long, c_char_p, c_short] 
get_frovedis_col.restype = py_object

# --- Frovedis sparse matrices ---
# create from scipy matrix
create_frovedis_crs_II_matrix = lib.create_frovedis_crs_II_matrix
create_frovedis_crs_II_matrix.argtypes = [c_char_p, c_int,
                                          c_ulong, c_ulong,
                                          ndpointer(c_int,ndim=1,flags="C_CONTIGUOUS"),
                                          ndpointer(c_int,ndim=1,flags="C_CONTIGUOUS"),
                                          ndpointer(c_long,ndim=1,flags="C_CONTIGUOUS"),
                                          c_ulong]
create_frovedis_crs_II_matrix.restype = py_object

create_frovedis_crs_IL_matrix = lib.create_frovedis_crs_IL_matrix
create_frovedis_crs_IL_matrix.argtypes = [c_char_p, c_int,
                                          c_ulong, c_ulong,
                                          ndpointer(c_int,ndim=1,flags="C_CONTIGUOUS"),
                                          ndpointer(c_long,ndim=1,flags="C_CONTIGUOUS"),
                                          ndpointer(c_long,ndim=1,flags="C_CONTIGUOUS"),
                                          c_ulong]
create_frovedis_crs_IL_matrix.restype = py_object

create_frovedis_crs_LI_matrix = lib.create_frovedis_crs_LI_matrix
create_frovedis_crs_LI_matrix.argtypes = [c_char_p, c_int,
                                          c_ulong, c_ulong,
                                          ndpointer(c_long,ndim=1,flags="C_CONTIGUOUS"),
                                          ndpointer(c_int,ndim=1,flags="C_CONTIGUOUS"),
                                          ndpointer(c_long,ndim=1,flags="C_CONTIGUOUS"),
                                          c_ulong]
create_frovedis_crs_LI_matrix.restype = py_object

create_frovedis_crs_LL_matrix = lib.create_frovedis_crs_LL_matrix
create_frovedis_crs_LL_matrix.argtypes = [c_char_p, c_int,
                                          c_ulong, c_ulong,
                                          ndpointer(c_long,ndim=1,flags="C_CONTIGUOUS"),
                                          ndpointer(c_long,ndim=1,flags="C_CONTIGUOUS"),
                                          ndpointer(c_long,ndim=1,flags="C_CONTIGUOUS"),
                                          c_ulong]
create_frovedis_crs_LL_matrix.restype = py_object

create_frovedis_crs_FI_matrix = lib.create_frovedis_crs_FI_matrix
create_frovedis_crs_FI_matrix.argtypes = [c_char_p, c_int,
                                          c_ulong, c_ulong,
                                          ndpointer(c_float,ndim=1,flags="C_CONTIGUOUS"),
                                          ndpointer(c_int,ndim=1,flags="C_CONTIGUOUS"),
                                          ndpointer(c_long,ndim=1,flags="C_CONTIGUOUS"),
                                          c_ulong]
create_frovedis_crs_FI_matrix.restype = py_object

create_frovedis_crs_FL_matrix = lib.create_frovedis_crs_FL_matrix
create_frovedis_crs_FL_matrix.argtypes = [c_char_p, c_int,
                                          c_ulong, c_ulong,
                                          ndpointer(c_float,ndim=1,flags="C_CONTIGUOUS"),
                                          ndpointer(c_long,ndim=1,flags="C_CONTIGUOUS"),
                                          ndpointer(c_long,ndim=1,flags="C_CONTIGUOUS"),
                                          c_ulong]
create_frovedis_crs_FL_matrix.restype = py_object

create_frovedis_crs_DI_matrix = lib.create_frovedis_crs_DI_matrix
create_frovedis_crs_DI_matrix.argtypes = [c_char_p, c_int,
                                          c_ulong, c_ulong,
                                          ndpointer(c_double,ndim=1,flags="C_CONTIGUOUS"),
                                          ndpointer(c_int,ndim=1,flags="C_CONTIGUOUS"),
                                          ndpointer(c_long,ndim=1,flags="C_CONTIGUOUS"),
                                          c_ulong]
create_frovedis_crs_DI_matrix.restype = py_object

create_frovedis_crs_DL_matrix = lib.create_frovedis_crs_DL_matrix
create_frovedis_crs_DL_matrix.argtypes = [c_char_p, c_int,
                                          c_ulong, c_ulong,
                                          ndpointer(c_double,ndim=1,flags="C_CONTIGUOUS"),
                                          ndpointer(c_long,ndim=1,flags="C_CONTIGUOUS"),
                                          ndpointer(c_long,ndim=1,flags="C_CONTIGUOUS"),
                                          c_ulong]
create_frovedis_crs_DL_matrix.restype = py_object

# load from text/bin file
load_frovedis_crs_matrix = lib.load_frovedis_crs_matrix
load_frovedis_crs_matrix.argtypes = [c_char_p, c_int, 
                                     c_char_p, c_bool, 
                                     c_short, c_short]
load_frovedis_crs_matrix.restype = py_object

save_frovedis_crs_matrix = lib.save_frovedis_crs_matrix
save_frovedis_crs_matrix.argtypes = [c_char_p, c_int, 
                                     c_long, c_char_p, c_bool, 
                                     c_short, c_short]

release_frovedis_crs_matrix = lib.release_frovedis_crs_matrix
release_frovedis_crs_matrix.argtypes = [c_char_p, c_int, c_long, 
                                        c_short, c_short]

show_frovedis_crs_matrix = lib.show_frovedis_crs_matrix
show_frovedis_crs_matrix.argtypes = [c_char_p, c_int, c_long, 
                                     c_short, c_short]

# --- Frovedis Dense matrices ---
# create from numpy matrix
create_frovedis_double_dense_matrix = lib.create_frovedis_double_dense_matrix
create_frovedis_double_dense_matrix.argtypes = [c_char_p, c_int,
                                       c_ulong, c_ulong,
                                       ndpointer(c_double,ndim=1,flags="C_CONTIGUOUS"),
                                       c_char]
create_frovedis_double_dense_matrix.restype = py_object

create_frovedis_float_dense_matrix = lib.create_frovedis_float_dense_matrix
create_frovedis_float_dense_matrix.argtypes = [c_char_p, c_int,
                                       c_ulong, c_ulong,
                                       ndpointer(c_float,ndim=1,flags="C_CONTIGUOUS"),
                                       c_char]
create_frovedis_float_dense_matrix.restype = py_object



create_frovedis_long_dense_matrix = lib.create_frovedis_long_dense_matrix
create_frovedis_long_dense_matrix.argtypes = [c_char_p, c_int,
                                       c_ulong, c_ulong,
                                       ndpointer(c_long,ndim=1,flags="C_CONTIGUOUS"),
                                       c_char]
create_frovedis_long_dense_matrix.restype = py_object



create_frovedis_int_dense_matrix = lib.create_frovedis_int_dense_matrix
create_frovedis_int_dense_matrix.argtypes = [c_char_p, c_int,
                                       c_ulong, c_ulong,
                                       ndpointer(c_int,ndim=1,flags="C_CONTIGUOUS"),
                                       c_char]
create_frovedis_int_dense_matrix.restype = py_object


# load from text/bin file
load_frovedis_dense_matrix = lib.load_frovedis_dense_matrix
load_frovedis_dense_matrix.argtypes = [c_char_p, c_int, 
                                       c_char_p, c_bool, c_char, c_short]
load_frovedis_dense_matrix.restype = py_object

save_frovedis_dense_matrix = lib.save_frovedis_dense_matrix
save_frovedis_dense_matrix.argtypes = [c_char_p, c_int, 
                                       c_long, c_char_p, 
                                       c_bool, c_char, c_short]

transpose_frovedis_dense_matrix = lib.transpose_frovedis_dense_matrix
transpose_frovedis_dense_matrix.argtypes = [c_char_p, c_int, 
                                            c_long, c_char, c_short]
transpose_frovedis_dense_matrix.restype = py_object

copy_frovedis_dense_matrix = lib.copy_frovedis_dense_matrix
copy_frovedis_dense_matrix.argtypes = [c_char_p, c_int, c_long, c_char, c_short]
copy_frovedis_dense_matrix.restype = py_object

release_frovedis_dense_matrix = lib.release_frovedis_dense_matrix
release_frovedis_dense_matrix.argtypes = [c_char_p, c_int, c_long, c_char, c_short]

show_frovedis_dense_matrix = lib.show_frovedis_dense_matrix
show_frovedis_dense_matrix.argtypes = [c_char_p, c_int, c_long, c_char, c_short]

get_frovedis_rowmatrix = lib.get_frovedis_rowmatrix
get_frovedis_rowmatrix.argtypes = [c_char_p, c_int, c_long, 
                                   c_ulong, c_ulong, 
                                   c_char, c_short]
get_frovedis_rowmatrix.restype = py_object

get_double_rowmajor_array = lib.get_double_rowmajor_array
get_double_rowmajor_array.argtypes = [c_char_p, c_int, c_long, c_char,
                               ndpointer(c_double,ndim=1,flags="C_CONTIGUOUS"),
                               c_ulong]

get_float_rowmajor_array = lib.get_float_rowmajor_array
get_float_rowmajor_array.argtypes = [c_char_p, c_int, c_long, c_char,
                               ndpointer(c_float,ndim=1,flags="C_CONTIGUOUS"),
                               c_ulong]

get_long_rowmajor_array = lib.get_long_rowmajor_array
get_long_rowmajor_array.argtypes = [c_char_p, c_int, c_long, c_char,
                               ndpointer(c_long,ndim=1,flags="C_CONTIGUOUS"),
                               c_ulong]

get_int_rowmajor_array = lib.get_int_rowmajor_array
get_int_rowmajor_array.argtypes = [c_char_p, c_int, c_long, c_char,
                               ndpointer(c_int,ndim=1,flags="C_CONTIGUOUS"),
                               c_ulong]

# --- Frovedis ML Models ---

parallel_float_glm_predict = lib.parallel_float_glm_predict
parallel_float_glm_predict.argtypes = [c_char_p, c_int, c_int, c_short, c_long,
                                 ndpointer(c_float,ndim=1,flags="C_CONTIGUOUS"),
                                 c_ulong, c_bool, c_short, c_bool]

parallel_double_glm_predict = lib.parallel_double_glm_predict
parallel_double_glm_predict.argtypes = [c_char_p, c_int, c_int, c_short, c_long,
                                 ndpointer(c_double,ndim=1,flags="C_CONTIGUOUS"),
                                 c_ulong, c_bool, c_short, c_bool]

# kmeans predict returns int always:
parallel_kmeans_predict = lib.parallel_kmeans_predict
parallel_kmeans_predict.argtypes = [c_char_p, c_int, c_int, 
                                    c_short, c_long,
                                    ndpointer(c_int,ndim=1,flags="C_CONTIGUOUS"),
                                    c_ulong, c_short, c_bool]

als_float_predict = lib.als_float_predict
als_float_predict.argtypes = [c_char_p, c_int, c_int, 
                        ndpointer(c_int,ndim=1,flags="C_CONTIGUOUS"),
                        ndpointer(c_float,ndim=1,flags="C_CONTIGUOUS"),
                        c_ulong]

als_double_predict = lib.als_double_predict
als_double_predict.argtypes = [c_char_p, c_int, c_int, 
                        ndpointer(c_int,ndim=1,flags="C_CONTIGUOUS"),
                        ndpointer(c_double,ndim=1,flags="C_CONTIGUOUS"),
                        c_ulong]

als_float_rec_users = lib.als_float_rec_users
als_float_rec_users.argtypes = [c_char_p, c_int, c_int, c_int, c_int,
                          ndpointer(c_int,ndim=1,flags="C_CONTIGUOUS"),
                          ndpointer(c_float,ndim=1,flags="C_CONTIGUOUS")]

als_double_rec_users = lib.als_double_rec_users
als_double_rec_users.argtypes = [c_char_p, c_int, c_int, c_int, c_int,
                          ndpointer(c_int,ndim=1,flags="C_CONTIGUOUS"),
                          ndpointer(c_double,ndim=1,flags="C_CONTIGUOUS")]

als_float_rec_prods = lib.als_float_rec_prods
als_float_rec_prods.argtypes = [c_char_p, c_int, c_int, c_int, c_int,
                          ndpointer(c_int,ndim=1,flags="C_CONTIGUOUS"),
                          ndpointer(c_float,ndim=1,flags="C_CONTIGUOUS")]

als_double_rec_prods = lib.als_double_rec_prods
als_double_rec_prods.argtypes = [c_char_p, c_int, c_int, c_int, c_int,
                          ndpointer(c_int,ndim=1,flags="C_CONTIGUOUS"),
                          ndpointer(c_double,ndim=1,flags="C_CONTIGUOUS")]

release_frovedis_model = lib.release_frovedis_model
release_frovedis_model.argtypes = [c_char_p,c_int,c_int,c_short,c_short]

show_frovedis_model = lib.show_frovedis_model
show_frovedis_model.argtypes = [c_char_p,c_int,c_int,c_short,c_short]

load_frovedis_model = lib.load_frovedis_model
load_frovedis_model.argtypes = [c_char_p,c_int,c_int,c_short,c_short,c_char_p]

load_frovedis_nbm = lib.load_frovedis_nbm
load_frovedis_nbm.argtypes = [c_char_p,c_int,c_int,c_short,c_char_p]
load_frovedis_nbm.restype = py_object

load_frovedis_mfm = lib.load_frovedis_mfm
load_frovedis_mfm.argtypes = [c_char_p,c_int,c_int,c_short,c_char_p]
load_frovedis_mfm.restype = py_object

save_frovedis_model = lib.save_frovedis_model
save_frovedis_model.argtypes = [c_char_p,c_int,c_int,c_short,c_short,c_char_p]

# --- Frovedis ML Trainers ---
lr_sgd = lib.lr_sgd
lr_sgd.argtypes = [c_char_p, c_int, c_long, c_long, c_int, c_double,
                   c_int, c_bool, c_double, c_int, c_int,
                   c_short, c_short, c_bool]

lr_lbfgs = lib.lr_lbfgs
lr_lbfgs.argtypes = [c_char_p, c_int, c_long, c_long, c_int, c_double,
                     c_int, c_bool, c_double, c_int, c_int,
                     c_short, c_short, c_bool]

svm_sgd = lib.svm_sgd
svm_sgd.argtypes = [c_char_p, c_int, c_long, c_long, c_int, c_double,
                    c_int, c_bool, c_double, c_int, c_int,
                    c_short, c_short, c_bool]

svm_lbfgs = lib.svm_lbfgs
svm_lbfgs.argtypes = [c_char_p, c_int, c_long, c_long, c_int, c_double,
                      c_int, c_bool, c_double, c_int, c_int,
                      c_short, c_short, c_bool]

dt_train = lib.dt_trainer
dt_train.argtypes = [c_char_p, c_int, c_long,
                     c_long, c_char_p, c_char_p,
                     c_int, c_int, c_int, c_int,
                     c_float, c_int, c_int,
                     c_short, c_short, c_bool]

nb_train = lib.nb_trainer
nb_train.argtypes = [c_char_p, c_int, c_long,
                     c_long, c_double, c_int, c_char_p, c_int,
		     c_short, c_short, c_bool]

lnr_sgd = lib.lnr_sgd
lnr_sgd.argtypes = [c_char_p, c_int, c_long, c_long, c_bool, c_int, c_int,
                    c_short, c_short, c_bool]

lnr_lbfgs = lib.lnr_lbfgs
lnr_lbfgs.argtypes = [c_char_p, c_int, c_long, c_long, c_bool, c_int, c_int,
                      c_short, c_short, c_bool]

lasso_sgd = lib.lasso_sgd
lasso_sgd.argtypes = [c_char_p, c_int, c_long, c_long, c_int, c_double,
                      c_bool, c_double, c_int, c_int,
                      c_short, c_short, c_bool]

lasso_lbfgs = lib.lasso_lbfgs
lasso_lbfgs.argtypes = [c_char_p, c_int, c_long, c_long, c_int, c_double,
                        c_bool, c_double, c_int, c_int,
                        c_short, c_short, c_bool]

ridge_sgd = lib.ridge_sgd
ridge_sgd.argtypes = [c_char_p, c_int, c_long, c_long, c_int, c_double,
                      c_bool, c_double, c_int, c_int,
                      c_short, c_short, c_bool]

ridge_lbfgs = lib.ridge_lbfgs
ridge_lbfgs.argtypes = [c_char_p, c_int, c_long, c_long, c_int, c_double,
                        c_bool, c_double, c_int, c_int,
                        c_short, c_short, c_bool]

kmeans_train = lib.kmeans_train
kmeans_train.argtypes= [c_char_p, c_int, c_long, c_int, 
                        c_int, c_long, c_double, c_int, c_int,
                        c_short, c_short, c_bool]

# als will always be trained with sparse data
als_train = lib.als_train
als_train.argtypes = [c_char_p, c_int, c_long, c_int, c_int,
                      c_double, c_double, c_long, c_int, c_int,
                      c_short, c_short]

fm_train = lib.fm_trainer
fm_train.argtypes = [c_char_p, c_int, 
                     c_long, c_long, 
                     c_double, c_int,
                     c_double, c_char_p, 
                     c_bool, c_bool, c_int, 
                     c_double, c_double, c_double, 
                     c_int, c_int, c_bool, c_int,
                     c_short, c_short]

# --- Frovedis PBLAS Wrappers ---
pswap = lib.pswap
pswap.argtypes = [c_char_p, c_int, c_long, c_long]

pcopy = lib.pcopy
pcopy.argtypes = [c_char_p, c_int, c_long, c_long]

pscal = lib.pscal
pscal.argtypes = [c_char_p, c_int, c_long, c_double]

paxpy = lib.paxpy
paxpy.argtypes = [c_char_p, c_int, c_long, c_long, c_double]

pdot = lib.pdot
pdot.argtypes = [c_char_p, c_int, c_long, c_long]
pdot.restype = c_double

pnrm2 = lib.pnrm2
pnrm2.argtypes = [c_char_p, c_int, c_long]
pnrm2.restype = c_double

pgemv = lib.pgemv
pgemv.argtypes = [c_char_p, c_int, c_long, c_long, 
                  c_bool, c_double, c_double]
pgemv.restype = py_object

pger = lib.pger
pger.argtypes = [c_char_p, c_int, c_long, c_long, c_double]
pger.restype = py_object

pgemm = lib.pgemm
pgemm.argtypes = [c_char_p, c_int, c_long, c_long, 
                  c_bool, c_bool, c_double, c_double]
pgemm.restype = py_object

pgeadd = lib.pgeadd
pgeadd.argtypes = [c_char_p, c_int, c_long, c_long, 
                   c_bool, c_double, c_double]

# --- Frovedis SCALAPACK Wrappers ---
pgetrf = lib.pgetrf
pgetrf.argtypes = [c_char_p, c_int, c_long]
pgetrf.restype = py_object

pgetri = lib.pgetri
pgetri.argtypes = [c_char_p, c_int, c_long, c_long]
pgetri.restype = c_int

pgetrs = lib.pgetrs
pgetrs.argtypes = [c_char_p, c_int, c_long, c_long, c_long, c_bool]
pgetrs.restype = c_int

pgesv = lib.pgesv
pgesv.argtypes = [c_char_p, c_int, c_long, c_long]
pgesv.restype = c_int

pgels = lib.pgels
pgels.argtypes = [c_char_p, c_int, c_long, c_long, c_bool]
pgels.restype = c_int

pgesvd = lib.pgesvd
pgesvd.argtypes = [c_char_p, c_int, c_long, c_bool, c_bool]
pgesvd.restype = py_object

# --- Frovedis ARPACK Wrappers ---
compute_sparse_svd = lib.compute_sparse_svd
compute_sparse_svd.argtypes = [c_char_p, c_int, c_long, c_int,
                               c_short, c_short]
compute_sparse_svd.restype = py_object

# --- Scalapack Results ---
release_ipiv = lib.release_ipiv
release_ipiv.argtypes = [c_char_p, c_int, c_char, c_long]

get_double_array = lib.get_double_array
get_double_array.argtypes = [c_char_p, c_int, c_long, 
                             ndpointer(c_double,ndim=1,flags="C_CONTIGUOUS")]

release_double_array = lib.release_double_array
release_double_array.argtypes = [c_char_p, c_int, c_long]

save_as_diag_matrix = lib.save_as_diag_matrix
save_as_diag_matrix.argtypes = [c_char_p, c_int, c_long, c_char_p, c_bool]

get_svd_results_from_file = lib.get_svd_results_from_file
get_svd_results_from_file.argtypes = [c_char_p, c_int,
                                      c_char_p, c_char_p, c_char_p,
                                      c_bool, c_bool, c_bool, c_char]
get_svd_results_from_file.restype = py_object
