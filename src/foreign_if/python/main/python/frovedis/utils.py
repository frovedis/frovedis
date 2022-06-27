"""
utils.py: module containing utils like deprecated,
          is_notebook, check_sample_weight
"""
import warnings
import numbers
import numpy as np
from .config import global_config
from .exrpc.server import FrovedisServer
from .exrpc import rpclib

def deprecated(message):
    """ definition for deprecated methods """
    def deprecated_decorator(func):
        def deprecated_func(*args, **kwargs):
            warnings.simplefilter('always', DeprecationWarning)
            warnings.warn(\
              "{}() is a deprecated function. {}".format(func.__name__, \
                                                         message),
              category=DeprecationWarning,
              stacklevel=2)
            warnings.simplefilter('default', DeprecationWarning)
            return func(*args, **kwargs)
        return deprecated_func
    return deprecated_decorator

def is_notebook():
    """checks whether execution environment is an ipython/jupyter notebook"""
    try:
        shell = get_ipython().__class__.__name__
        if shell == 'ZMQInteractiveShell':
            return True   # Jupyter notebook or qtconsole
        elif shell == 'TerminalInteractiveShell':
            return False  # Terminal running IPython
        else:
            return False  # Other type (?)
    except NameError:
        return False      # Probably standard Python interpreter

def check_sample_weight(self, sample_weight):
    """checks input X and y"""
    if sample_weight is None:
        weight = np.array([], dtype=np.float64)
    elif isinstance(sample_weight, numbers.Number):
        weight = np.full(self.n_samples, sample_weight, dtype=np.float64)
    else:
        weight = np.ravel(sample_weight)
        if len(weight) != self.n_samples:
            raise ValueError("sample_weight.shape == {}, expected {}!"\
                   .format(sample_weight.shape, (self.n_samples,)))
    return np.asarray(weight, dtype=np.float64)

def str_type():
    return global_config.get("string_dvector_as")

def exec_rpc_call(func=None, rpc_args=()):
    '''
    DESC: rpclib call wrapper
    PARAMS: func - a function pointer/object
            rpc_args - a tuple containing function parameters
    RETURN VALUE: It returns result given by rpclib execution for given func
    '''
    if callable(func):
        (host, port) = FrovedisServer.getServerInstance()
        res = func(host, port, *rpc_args)
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        return res
    else:
        raise ValueError("func should be a valid function pointer/object, " + \
                         "but received {}".format(func))

