"""
config: to manipulate several configuration parameters
"""
import numpy as np
import pandas as pd
from .matrix.dtype import DTYPE

def get_pandas_nat():
  return pd.to_datetime("").value

class global_config(object):
    __param_key = \
      { 
        "rawsend_enabled": True,
        "string_dvector_as": DTYPE.WORDS,
        # since TimeDelta are teated as nanosecond when tranfering to server side
        "datetime_type_for_add_sub_op": "day", #TODO: make it nanosecond
        "NaT": get_pandas_nat()
      }

    __param_type = \
      {
        "rawsend_enabled": type(True),
        "string_dvector_as": type(DTYPE.WORDS),
        "datetime_type_for_add_sub_op": type("day"), #TODO: make it nanosecond
        "NaT": type(get_pandas_nat())
      }

    read_only = ["NaT"]

    def __init__(self):
        '''  initializes global configuration '''
        pass

    @staticmethod
    def set(key, value):
        ''' sets the config parameter '''
        if key in global_config.read_only:
            raise ValueError(\
            "set: '{}' is prohibited being a read-only parameter".format(key))

        if key not in global_config.__param_key:
            raise ValueError(\
            "set: '{}' not found in global configuration".format(key))

        expected_t = global_config.__param_type[key]
        if type(value) != expected_t:
            raise ValueError("set: expected '{}' to be of type'".format(key) + \
                        "{}'; received: '{}'".format(expected_t, type(value)))
        global_config.__param_key[key] = value

    @staticmethod
    def get(key):
        ''' return the set value for the given config key '''
        if key not in global_config.__param_key:
            raise ValueError(\
            "get: '{}' not found in global configuration".format(key))
        return global_config.__param_key[key]

