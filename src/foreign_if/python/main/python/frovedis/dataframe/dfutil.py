""" dfutil: module containing utility programs for dataframe """

import numpy as np
from collections import Iterable
from ..matrix.dtype import DTYPE, TypeUtil, get_result_type

def infer_column_type_from_first_notna(df, col, is_index=False):
    if is_index: #infers type of index assuming it contains all non-na
        dtype = type(df.index.values[0]).__name__
    else:
        valid_idx = col.first_valid_index()
        tidx = df.index.get_loc(valid_idx)
        if isinstance(tidx, slice):
            valid_int_idx = tidx.start
        elif isinstance(tidx, Iterable):
            valid_int_idx = np.where(tidx == True)[0][0]
        elif isinstance(tidx, int):
            valid_int_idx = tidx
        else:
            raise TypeError(\
            "pandas df.get_loc() return type could not be understood!")
        dtype = type(col.values[valid_int_idx]).__name__
    return dtype 
     
def get_string_typename(numpy_type):
    """
    return frovedis types from numpy types
    """
    numpy_to_string_type = { "int32": "int",
                             "int64": "long",
                             "float32": "float",
                             "float64": "double",
                             "str": "dic_string",
                             "string": "dic_string",
                             "bool": "dic_string",
                             "uint64": "unsigned long"}
    if numpy_type not in numpy_to_string_type:
        raise TypeError("data type '{0}'' not understood\n"
                        .format(numpy_type))
    return numpy_to_string_type[numpy_type]

def get_null_value(dtype):
    """
    return frovedis-like null values for given 'dtype'
    """
    null_val = {}
    null_val[DTYPE.DOUBLE] = np.finfo(np.float64).max
    null_val[DTYPE.FLOAT] = np.finfo(np.float32).max
    null_val[DTYPE.ULONG] = np.iinfo(np.uint).max
    null_val[DTYPE.LONG] = np.iinfo(np.int64).max
    null_val[DTYPE.INT] = np.iinfo(np.int32).max
    null_val[DTYPE.BOOL] = np.iinfo(np.int32).max
    null_val[DTYPE.STRING] = "NULL"
    if dtype not in null_val:
        raise TypeError("data type '{0}'' not understood\n".format(dtype))
    return null_val[dtype]
    
def union_lists(data):
    """ performs union on list of lists """
    return list(set().union(*data))

def infer_dtype(dfs, colname):
    """ 
    infers the dtype of the resultant column when
    the given column, 'colname' of input dataframes, 'dfs'
    are appended together
    """
    dtypes = [df.get_dtype(colname) for df in dfs if colname in df.columns]
    return get_result_type(dtypes)

def check_if_consistent(dfs, cast_info):
    """
    checks name and types of columns of each dataframes in 'dfs',
    returns False, in case any one of them if found to be inconsistent,
    otherwise returns True.
    """
    index_names = np.asarray([df.index.name if df.has_index() \
                          else None for df in dfs])
    is_index_names_consistent = np.all(index_names == index_names[0])
        
    index_dtypes = np.asarray([df.index.dtype if df.has_index() \
                    else None for df in dfs])
    is_index_dtypes_consistent = np.all(index_dtypes == index_dtypes[0])
       
    is_column_names_consistent = True
    is_column_dtypes_consistent = True 
    for c, t in cast_info.items():
        if is_column_names_consistent or is_column_dtypes_consistent:
            for df in dfs:
                if c not in df.columns:
                    is_column_names_consistent = False   # missing-column
                elif df.get_dtype(c) != t:
                    is_column_dtypes_consistent = False  # type-mismatch
        else:
            break

    return [is_index_names_consistent, is_index_dtypes_consistent, \
            is_column_names_consistent, is_column_dtypes_consistent]

def add_null_column_and_type_cast(dfs, is_frov_df, cast_info):
    """
    adds null column to each df in 'dfs' if any column in 'cast_info.keys()'
    is missing and performs typecasting in case requested type
    in 'cast_info' is different than existing type
    'is_frov_df' should be a list of size = len(dfs) of boolean type
    indicating which all dataframes in 'dfs' are actually user constructed 
    frovedis DataFrame object, [used internally by DataFrame.append()]
    """
    if len(dfs) < 1:
        raise ValueError("dfs: input is empty!")

    if len(dfs) != len(is_frov_df):
        raise ValueError("add_null_column_and_type_cast: length "
                         "of 'dfs' and 'is_frov_df' differ!")

    chk_list = check_if_consistent(dfs, cast_info)
    is_consistent = np.sum(chk_list) == 4
    if is_consistent:
        #print("all consistent")
        return dfs # early return: ready for append -> no modification required

    # copying input dataframes() only when they are frovedis Datarame instances
    ret = [dfs[i].copy() if is_frov_df[i] else dfs[i] \
           for i in range(0, len(dfs))]

    # handling of index-column (frovedis expects all same index names)
    if not chk_list[0]: # is_index_names_consistent
        for df in ret:
            if df.has_index():
                if df.index.name != "index":
                    df.rename_index("index", inplace=True)
            else:
                df.add_index("index")
        index_col = "index"
    else:
        index_col = ret[0].index.name # all index name is same

    # handling of missing columns
    if not chk_list[2]: # is_column_names_consistent
        for i in range(0, len(ret)):
            df = ret[i]
            for col in cast_info.keys():
                if col not in df.columns:
                    ctype = TypeUtil.to_id_dtype(cast_info[col])
                    df[col] = get_null_value(ctype)

    # inferring index type in case of inconsistency
    if not chk_list[1]: # is_index_dtypes_consistent
        index_dtypes = [df.get_dtype(index_col) for df in ret]
        inferred_index_type = get_result_type(index_dtypes)
        cast_info[index_col] = inferred_index_type
        #print(index_col + ":" + str(inferred_index_type))

    # handling of type-casting (if type-mismatch either in index or in column)
    if not (chk_list[1] and chk_list[3]): 
        for i in range(0, len(ret)):
            df = ret[i]
            if not chk_list[1] and \
               df.get_dtype(index_col) != cast_info[index_col]:
                # this covers index as well as other mismatched columns
                ret[i] = df.astype(cast_info) 
            else:
                for col in cast_info.keys():
                    if df.get_dtype(col) != cast_info[col]: 
                        ret[i] = df.astype(cast_info)
                        break
    
    ''' 
    for df in ret: 
        print(df)
        print(df.dtypes)
        print(df.index.dtype)
        print("---------------")
    ''' 
    return ret

def get_python_scalar_type(val):
    """ returns type of the input scalar """
    if not np.isscalar(val):
        raise ValueError("input must be a scalar value!")
    dt = type(val).__name__
    if dt == "int": # all integer numbers in python3 is typed as 'int'
        dt = "long" 
    elif dt == "float": # all floating point numbers in pythin3 is typed as 'float'
        dt = "double" 
    return dt

def check_string_or_array_like(by, func): 
    """ checks if the given input 'by' is a string or array-like """
    if isinstance(by, str):
        ret_by = [by]
    elif isinstance(by, (list, tuple, np.ndarray)): #iterable
        unq, idx = np.unique(by, return_index=True) # excludes redundant values, if any
        sorted_idx = np.sort(idx) 
        ret_by = [by[i] for i in sorted_idx]
    else:
        raise TypeError(func + ": expected: string or array-like; "\
                        "received: %s" % (type(by).__name__))
    return ret_by

def check_stat_error(axis, skipna, level):
    """ 
    checks the given parameters for the statistical functions
    like sum, mean, var etc. 
    """
    if level is not None:
        raise ValueError("'level' parameter is not cutrrently supported!\n")

    if axis not in [None, 0, 1, "index", "columns"]:
        raise ValueError("No axis named '%s' for DataFrame object" % str(axis))
    if axis is None or axis == "index":
        axis_ = 0
    elif axis == "columns":
        axis_ = 1
    else:
        axis_ = axis

    if skipna not in [None, True, False]:
        raise ValueError(\
        "skipna='%s' is not supported currently!\n" % str(skipna))
    if skipna is None:
        skipna_ = True
    else:
        skipna_ = skipna

    return axis_, skipna_
