import warnings

def deprecated(message):
  def deprecated_decorator(func):
      def deprecated_func(*args, **kwargs):
          warnings.simplefilter('always', DeprecationWarning)
          warnings.warn("{}() is a deprecated function. {}".format(func.__name__, message),
                        category=DeprecationWarning,
                        stacklevel=2)
          warnings.simplefilter('default', DeprecationWarning)
          return func(*args, **kwargs)
      return deprecated_func
  return deprecated_decorator

def is_notebook():
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
