"""
	model.py: wrapper of frovedis ARIMA
"""
import numpy as np
from frovedis.exrpc import rpclib
from frovedis.base import BaseEstimator
from frovedis.exrpc.server import FrovedisServer, set_association, \
                           check_association, do_if_active_association
from frovedis.matrix.dvector import FrovedisDvector
from frovedis.matrix.crs import FrovedisCRSMatrix
from frovedis.matrix.dense import FrovedisDenseMatrix, \
                                  FrovedisColmajorMatrix, \
                                  FrovedisRowmajorMatrix
from frovedis.matrix.dtype import str_encode
from frovedis.mllib.model_util import M_KIND, ModelID, GLM

class ARIMA(BaseEstimator):
    """
    DESC: AR-Auto Regressive(p) I-Integrated(d) MA-Moving Average(q) model
    A time series model that is used to forecast data based on the dataset of
    past to predict/forecast the future

    HYPERPARAMS: endog -> TYPE: array-like, contains the time series data
                 exog -> DEFAULT: None, (Unused)
                 order -> TYPE: Tuple, DEFAULT: (1,0,0), (p,d,q) order of the
                          model for the autoregressive, differences, and
                          moving average components where p,d,q are integers
                 seasonal_order -> DEFAULT: (0,0,0,0), (Unused)
                 trend -> DEFAULT: None, (Unused)
                 enforce_stationarity -> DEFAULT: True, (Unused)
                 enforce_invertibility -> DEFAULT: True, (Unused)
                 concentrate_scale -> DEFAULT: False, (Unused)
                 trend_offset -> DEFAULT: 1, (Unused)
                 dates -> DEFAULT: None, (Unused)
                 freq -> DEFAULT: None, (Unused)
                 missing -> DEFAULT: 'none', (Unused)
                 validate_specification -> DEFAULT: True, (Unused)
                 seasonal -> TYPE: int, DEFAULT: None, it specifies the
                             interval of seasonal differencing i.e if the data
                             has some seasonality, then it can also handle it.
                             This is optional.
                 auto_arima -> TYPE: bool, DEFAULT: False, it specifies whether
                               to use auto (brute) ARIMA. If true it treats
                               the p and q parameter as the highest limit for
                               its iteration and auto fit lags with best RMSE.
                               This is optional
                 solver -> TYPE: str, DEFAULT: 'lapack', it uses 'lapack',
                           'lbfgs', 'sag', 'scalapack' as solver to perform
                           linear regression
                 verbose -> TYPE: int, DEFAULT: 0, it specifies the log level
                            to use. It uses 0 (for INFO mode), 1 (for DEBUG
                            mode) or 2 (for TRACE mode) for getting training
                            time logs from frovedis server
    """
    def __init__(self, endog, exog=None, order=(1, 0, 0),
                 seasonal_order=(0, 0, 0, 0), trend=None,
                 enforce_stationarity=True, enforce_invertibility=True,
                 concentrate_scale=False, trend_offset=1, dates=None,freq=None,
                 missing='none', validate_specification=True, seasonal=None,
                 auto_arima=False, solver='lapack', verbose=0):
        self.endog = endog
        self.exog = exog
        self.order = order
        self.ar_lag = 1 if order[0] is None else order[0]
        self.diff_order = 0 if order[1] is None else order[1]
        self.ma_lag = 0 if order[2] is None else order[2]
        self.seasonal_order = seasonal_order
        self.trend = trend
        self.enforce_stationarity = enforce_stationarity
        self.enforce_invertibility = enforce_invertibility
        self.concentrate_scale = concentrate_scale
        self.trend_offset = trend_offset
        self.dates = dates
        self.freq = freq
        self.missing = missing
        self.validate_specification = validate_specification
        self.seasonal = seasonal
        self.auto_arima = auto_arima
        self.solver = solver
        self.verbose = verbose
        # extra
        self.__mid = ModelID.get()
        self.__mdtype = None
        self.__mkind = None
        self._fittedvalues = None
        self.isfitted = False
        self._endog_len = None

    @set_association
    def fit(self):
        """
        DESC: Fit (estimate) the parameters of the model
        RETURNS: TYPE: list of shape (n_samples,), returns the fitted values
                 of the model
        """
        self.reset_metadata()
        if len(self.order) != 3:
            raise ValueError("`order` argument must be an iterable with " + \
                             "three elements.")
        if self.ar_lag < 1:
            raise ValueError("Terms in the AR order cannot be less than 1.")
        if self.diff_order < 0:
            raise ValueError("Cannot specify negative differencing.")
        if self.ma_lag < 0:
            raise ValueError("Terms in the MA order cannot be negative.")
        if self.seasonal is None:
            self.seasonal = 0
        elif self.seasonal < 0:
            raise ValueError("The seasonal differencing interval cannot " + \
                             "be negative, given: " + str(self.seasonal))
        if self.auto_arima is True and self.ma_lag <= 0:
            raise ValueError("Currently, auto_arima cannot start with a " + \
                             "moving average component having value less " + \
                             "than 1!")
        if self.solver == 'sag':
            self.solver = 'sgd'
        elif self.solver not in ['lapack', 'lbfgs', 'scalapack']:
            raise ValueError("Unknown solver: " + self.solver + " for time " + \
                             "series analysis!")
        if isinstance(self.endog, FrovedisDvector):
            self.__mdtype = self.endog.get_dtype()
            inp_data = self.endog.to_numpy_array()
            shape = np.shape(inp_data)
            if np.shape(inp_data)[0] < (self.ar_lag + self.diff_order + \
               self.ma_lag + self.seasonal + 2):
                raise ValueError("Number of samples in input is too less " + \
                                 "for time series analysis!")
            self._endog_len = shape[0]
        elif isinstance(self.endog, (FrovedisCRSMatrix, FrovedisDenseMatrix,
                           FrovedisRowmajorMatrix, FrovedisColmajorMatrix)):
            raise TypeError("endog can only be FrovedisDvector, " + \
                            "not {}".format(self.endog))
        else:
            shape = np.shape(self.endog)
            if len(shape) == 1 or (len(shape) == 2 and shape[1] == 1):
                if np.shape(self.endog)[0] < (self.ar_lag + self.diff_order + \
                   self.ma_lag + self.seasonal + 2):
                    raise ValueError("Number of samples in input is too " + \
                                     "less for time series analysis!")
                self.endog = np.ravel(self.endog)
                self._endog_len = shape[0]
                self.endog = FrovedisDvector().as_dvec(self.endog)
                self.__mdtype = self.endog.get_dtype()
            else:
                raise ValueError("Frovedis ARIMA models require univariate " + \
                                 "`endog`. Got shape {0}".format(shape))
        self.__mkind = M_KIND.ARM
        (host, port) = FrovedisServer.getServerInstance()
        rpclib.arima_fit(host, port, self.endog.get(), self.ar_lag, \
                         self.diff_order, self.ma_lag, self.seasonal, \
                         self.auto_arima, str_encode(self.solver), \
                         self.verbose, self.__mid, self.__mdtype)
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        self.isfitted = True
        return self

    @check_association
    def predict(self, start=None, end=None, dynamic=False, **kwargs):
        """
        DESC: Perform in-sample prediction and out-of-sample forecasting
        PARAMS: start -> TYPE: int, DEFAULT: None, it specifies the staring
                         index after which the values are to be predicted
                stop -> TYPE: int, DEFAULT: None, it specifies the index till
                        which the values are to be predicted
                dynamic -> DEFAULT: False, (Unused)
                **kwargs -> (Unused)
        NOTE: In case start or stop are negative then predicted values are
              returned from fitted values as long as indexes are accessible(in
              range). This is in sync with statsmodel behaviour.
        RETURNS: TYPE: ndarray of shape (n_samples,), it returns the predicted
                 values
        """
        if start is None:
            start = 0
        elif start < 0:
            if self._endog_len >= abs(start):
                start = self._endog_len + start
            else:
                raise KeyError("The `start` argument could not be matched " + \
                               "to a location related to the index of " + \
                               "the data.")
        if end is None:
            end = self._endog_len - 1
        elif end < 0:
            if self._endog_len >= abs(end):
                end = self._endog_len + end
            else:
                raise KeyError("The `end` argument could not be matched to " + \
                               "a location related to the index of the data.")
        if end < start:
            raise ValueError("Prediction must have `end` after `start`!")
        if dynamic:
            raise ValueError("Currently, ARIMA.predict() does not support " + \
                             "dynamic = True!")
        (host, port) = FrovedisServer.getServerInstance()
        arima_pred = rpclib.arima_predict(host, port, start, end, \
                                          self.__mid, self.__mdtype)
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        return np.asarray(arima_pred, dtype = np.float64)

    @check_association
    def forecast(self, steps=1, exog=None, alpha=0.05):
        """
        DESC: Perform out-of-sample forecasting
        PARAMS: steps -> TYPE: int, DEFAULT: 1, it specifies the number of
                         out-of-sample values to be predicted
                exog -> DEFAULT: None, (Unused)
                alpha -> DEFAULT: 0.05, (Unused)
        RETURNS: TYPE: ndarray of shape (n_samples,), returns out of sample
                 forecasted values
        """
        if exog is not None:
            raise ValueError("Currently, exog is not supported by forecast()!")
        if alpha != 0.05:
            raise ValueError("Currently, alpha is not supported by forecast()!")
        if steps is None:
            steps = 1
        elif steps <= 0:
            raise ValueError("Prediction must have `end` after `start`!")
        (host, port) = FrovedisServer.getServerInstance()
        fcast = rpclib.arima_forecast(host, port, steps, self.__mid, \
                                      self.__mdtype)
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        return np.asarray(fcast, dtype = np.float64)

    @property
    @check_association
    def fittedvalues(self):
        """
        DESC: fittedvalues getter
        RETURNS: TYPE: ndarray of shape (n_samples,), returns the fitted
                 values of the model
        """
        if self._fittedvalues is None:
            (host, port) = FrovedisServer.getServerInstance()
            ret = rpclib.get_fitted_vector(host, port, self.__mid, \
                                           self.__mkind, self.__mdtype)
            excpt = rpclib.check_server_exception()
            if excpt["status"]:
                raise RuntimeError(excpt["info"])
            self._fittedvalues = np.asarray(ret, dtype=np.float64)
        return self._fittedvalues

    @fittedvalues.setter
    def fittedvalues(self, val):
        """
        DESC: fittedvalues setter
        """
        raise AttributeError(\
            "attribute 'fittedvalues' of ARIMA object is not writable")

    def reset_metadata(self):
        """
        DESC: resets after-fit populated attributes to None
        """
        self._fittedvalues = None
        self.isfitted = False

    def release(self):
        """
        DESC: resets after-fit populated attributes to None
              along with relasing server side memory
        """
        self.__release_server_heap()
        self.reset_metadata()

    @do_if_active_association
    def __release_server_heap(self):
        """
        DESC: to release model pointer from server heap
        """
        GLM.release(self.__mid, self.__mkind, self.__mdtype)

    def __del__(self):
        """
        DESC: destructs the python object
        """
        self.release()

    def is_fitted(self):
        """
        DESC: function to confirm if the ARIMA model is already fitted
        """
        return self.isfitted
