"""
config: to manipulate several configuration parameters
"""

class global_config(object):
    __param_key = \
      { 
        "rawsend_enabled": True
      }

    __param_type = \
      {
        "rawsend_enabled": type(True)
      }

    def __init__(self):
        '''  initializes global configuration '''
        pass

    @staticmethod
    def set(key, value):
        ''' sets the config parameter '''
        if key not in global_config.__param_key:
            raise ValueError(f"set: '{key}' not foudn in global configuration")

        expected_t = global_config.__param_type[key]
        if type(value) != expected_t:
            raise ValueError(f"set: expected '{key}' to be of type'" + \
                             f"{expected_t}'; received: '{type(value)}'")
        global_config.__param_key[key] = value

    @staticmethod
    def get(key):
        ''' return the set value for the given config key '''
        if key not in global_config.__param_key:
            raise ValueError(f"set: '{key}' not found in global configuration")
        return global_config.__param_key[key]

