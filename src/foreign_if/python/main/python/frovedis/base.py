"""
base.py
"""

#!/usr/bin/env python

from collections import defaultdict

class BaseEstimator(object):
    """Base class for all estimators in frovedis, with support for
    get_params(), and set_params()
    Notes
    -----
    All estimators should specify all the parameters that can be set
    at the class level in their ``__init__`` as explicit keyword
    argument. Also, if user supports python 2, please install funcsigs,
    incase of python 3 install inspect.
    """

    @classmethod
    def _get_param_names(cls):
        """Get parameter names for the estimator"""
        init = getattr(cls.__init__, 'deprecated_original', cls.__init__)
        if init is object.__init__:
            return []

        import sys
        if sys.version_info[0] < 3:
            import funcsigs
            init_signature = funcsigs.signature(init)
        else:
            import inspect
            init_signature = inspect.signature(init)
        parameters = [p for p in init_signature.parameters.values()
                      if p.name != 'self' and p.kind != p.VAR_KEYWORD]
        for p in parameters:
            if p.kind == p.VAR_POSITIONAL:
                raise RuntimeError("frovedis estimators should always "
                                   "specify their parameters in the signature"
                                   " of their __init__ (no varargs)."
                                   " %s with constructor %s doesn't "
                                   " follow this convention."
                                   % (cls, init_signature))
        return sorted([p.name for p in parameters])

    def get_params(self, deep=True):
        """
        Get parameters for this estimator.
        Parameters
        ----------
        deep : bool, default=True
            If True, will return the parameters for this estimator and
            contained subobjects that are estimators.
        Returns
        -------
        params : mapping of string to any
            Parameter names mapped to their values.
        """
        ret = dict()
        for key in self._get_param_names():
            try:
                value = getattr(self, key)
            except AttributeError:
                value = None
            if deep and hasattr(value, 'get_params'):
                deep_i = value.get_params().items()
                ret.update((key + '__' + k, val) for k, val in deep_i)
            ret[key] = value
        return ret

    def set_params(self, **params):
        """
        Set the parameters of this estimator.
        Parameters
        ----------
        **params : dict
            Estimator parameters.
        Returns
        -------
        self : object
            Estimator instance.
        """
        if not params:
            return self
        params_list = self.get_params(deep=True)

        nested_params = defaultdict(dict)  # grouped by prefix
        for key, value in params.items():
            key, delim, sub_key = key.partition('__')
            if key not in params_list:
                raise ValueError('Invalid parameter %s for estimator %s. '
                                 'Check the list of available parameters '
                                 'with `estimator.get_params().keys()`.' %
                                 (key, self))

            if delim:
                nested_params[key][sub_key] = value
            else:
                setattr(self, key, value)
                params_list[key] = value

        for key, internal_params in nested_params.items():
            params_list[key].set_params(**internal_params)

        return self

