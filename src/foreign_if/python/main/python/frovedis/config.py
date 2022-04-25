"""
config: to manipulate several configuration parameters
"""
from .matrix.dtype import DTYPE

class global_config(object):
    __param_key = \
      { 
        "rawsend_enabled": True,
        "string_dvector_as": DTYPE.WORDS
      }

    __param_type = \
      {
        "rawsend_enabled": type(True),
        "string_dvector_as": type(DTYPE.WORDS)
      }

    def __init__(self):
        '''  initializes global configuration '''
        pass

    @staticmethod
    def set(key, value):
        ''' sets the config parameter '''
        if key not in global_config.__param_key:
            raise ValueError(\
            "set: '{}' not foudn in global configuration".format(key))

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
            "set: '{}' not found in global configuration".format(key))
        return global_config.__param_key[key]

