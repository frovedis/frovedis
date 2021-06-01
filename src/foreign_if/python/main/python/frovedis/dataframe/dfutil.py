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
    indices = np.asarray([df.index.name if df.has_index() \
                          else None for df in dfs])
    # index must exist in each dataframe and their names should be all same
    if not np.all(indices == indices[0]): 
        return False

    for col in dfs[0].columns:
        dtype_base = dfs[0].get_dtype(col)
        for df in dfs[1:]:
            try:
                dtype_other = df.get_dtype(col)
                if dtype_other != dtype_base:
                    return False
            except ValueError as e: # in case 'col' not found in 'e'
                return False
    return True # everything seems consistent

def add_null_column_and_type_cast(dfs, is_frov_df, cast_info):
    """
    adds null column to each df in 'dfs' if any column in 'cast_info.keys()'
    is missing and performs typecasting in case requested type
    in 'cast_info' is different than existing type
    'is_frov_df' should be a list of size = len(dfs) of boolean type
    indicating which all dataframes in 'dfs' are actually user constructed 
    frovedis DataFrame object, [used internally by DataFrame.append()]
    """
    null_replacement = {}
    null_replacement[DTYPE.DOUBLE] = np.finfo(np.float64).max
    null_replacement[DTYPE.FLOAT] = np.finfo(np.float32).max
    null_replacement[DTYPE.ULONG] = np.iinfo(np.uint).max
    null_replacement[DTYPE.LONG] = np.iinfo(np.int64).max
    null_replacement[DTYPE.INT] = np.iinfo(np.int32).max
    null_replacement[DTYPE.BOOL] = np.iinfo(np.int32).max
    null_replacement[DTYPE.STRING] = "NULL"

    if len(dfs) < 1:
        raise ValueError("dfs: input is empty!")

    if len(dfs) != len(is_frov_df):
        raise ValueError("add_null_column_and_type_cast: length "
                         "of 'dfs' and 'is_frov_df' differ!")

    # copying input dataframes() on need basis
    is_consistent = check_if_consistent(dfs, cast_info)
    if is_consistent:
        return dfs # early return: ready for append, no modification required

    ret = [None] * len(dfs)
    for i in range(0, len(dfs)):
        if is_frov_df[i]:
            if not is_consistent:
                ret[i] = dfs[i].copy()
            else:
                ret[i] = dfs[i]
        else:
            ret[i] = dfs[i]

    # handling of index-column (frovedis expects all same index names)
    index_names = np.asarray([df.index.name if df.has_index() \
                              else None for df in ret])
    if not np.all(index_names == index_names[0]):
        for df in ret:
            if df.has_index():
                if df.index.name != "index":
                    df.rename_index("index", inplace=True)
            else:
                df.add_index("index")
        index_col = "index"
    else:
        index_col = index_names[0]

    cols = list(cast_info.keys())
    index_dtypes = [df.get_dtype(index_col) for df in ret]
    inferred_index_type = get_result_type(index_dtypes)
    cast_info[index_col] = inferred_index_type
    #print(index_col + ":" + str(inferred_index_type))

    # handling of missing columns
    for i in range(0, len(ret)):
        df = ret[i]
        for col in cols:
            if col not in df.columns:
                df[col] = \
                null_replacement[TypeUtil.to_id_dtype(cast_info[col])]

    # handling of type-casting
    for i in range(0, len(ret)):
        df = ret[i]
        for col in cast_info.keys(): # includes index
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
