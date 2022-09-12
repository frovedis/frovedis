"""
	model.py: wrapper of frovedis ARIMA
"""
import warnings
import datetime
import copy
import numpy as np
from pandas import (
    DataFrame,
    DatetimeIndex,
    Index,
    Period,
    PeriodIndex,
    RangeIndex,
    Series,
    Timestamp,
    date_range,
    period_range,
)
from ....exrpc import rpclib
from ....base import BaseEstimator
from ....exrpc.server import FrovedisServer, set_association, \
                           check_association, do_if_active_association
from ....matrix.dvector import FrovedisDvector
from ....matrix.dtype import str_encode, TypeUtil, DTYPE
from ...model_util import M_KIND, ModelID, GLM
from ....dataframe import df as fd

class ARIMA(BaseEstimator):
    """
    DESC: AR-Auto Regressive(p) I-Integrated(d) MA-Moving Average(q) model
    A time series model that is used to forecast data based on the dataset of
    past to predict/forecast the future.

    HYPERPARAMS: endog -> TYPE: {array-like of shape (n_samples,)
                          or pandas DataFrame having single column},
                          it contains the time series data.
                 exog -> DEFAULT: None, (Unused)
                 order -> TYPE: Tuple, DEFAULT: (1,0,0), (p,d,q) order of the
                          model for the autoregressive, differences, and
                          moving average components where p,d,q are integers.
                          Currently, autoregressive order cannot be 0.
                 seasonal_order -> DEFAULT: (0,0,0,0), (Unused)
                 trend -> DEFAULT: None, (Unused)
                 enforce_stationarity -> DEFAULT: True, (Unused)
                 enforce_invertibility -> DEFAULT: True, (Unused)
                 concentrate_scale -> DEFAULT: False, (Unused)
                 trend_offset -> DEFAULT: 1, (Unused)
                 dates -> DEFAULT: None, (Unused)
                 freq -> TYPE: {str, pandas DateOffset}, DEFAULT: None,
                         It is optional. It is used only when time series data
                         is a pandas Series or a Dataframe having an index. In
                         case, frequency information is not provided by given
                         endog, then the frequency of the time series may be
                         specified here as a offset string or Pandas offset.
                         For example, freq = '3D' or freq = to_offset('3D')
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
                               This is optional.
                 solver -> TYPE: str, DEFAULT: 'lapack', it uses 'lapack',
                           'lbfgs', 'sag', 'scalapack' as solver to perform
                           linear regression.
                 verbose -> TYPE: int, DEFAULT: 0, it specifies the log level
                            to use. It uses 0 (for INFO mode), 1 (for DEBUG
                            mode) or 2 (for TRACE mode) for getting training
                            time logs from frovedis server.
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
        self.__ar_lag = None
        self.__diff_order = None
        self.__ma_lag = None
        self._metadata = None #Used to hold Series/Dataframe index information

    def _check_index(self):
        default_index = Index(range(self.endog.shape[0]))
        final_index = copy.deepcopy(self.endog.index)
        if isinstance(final_index, RangeIndex):
            return (final_index, True)
        if isinstance(final_index, Index) and isinstance(final_index[0], str):
            final_index = DatetimeIndex(final_index)
        if isinstance(final_index, (DatetimeIndex, PeriodIndex)):
            if not final_index.is_monotonic_increasing:
                warnings.warn("A date index has been provided, but it " + \
                              "is not monotonic and so will be ignored" + \
                              " when e.g. forecasting.")
                return (default_index, False)
            if self.freq is None and final_index.freq is None:
                final_index.freq = final_index.inferred_freq
                if final_index.freq is not None:
                    warnings.warn("No frequency information was " + \
                                  "provided, so inferred frequency " + \
                                  final_index.inferred_freq + " will be used.")
                else:
                    warnings.warn("A date index has been provided, but" + \
                                  " it has no associated frequency" + \
                                  " information and so will be ignored" + \
                                  " when e.g. forecasting.")
                    return (default_index, False)
            elif self.freq is not None:
                try:
                    final_index.freq = self.freq
                except ValueError as verr:
                    if final_index.freq is None:
                        raise ValueError("The given frequency could not be " + \
                                        "matched to the given index.") from verr
                    raise ValueError("The given frequency argument is " + \
                                 "incompatible with the given index.") from verr
        elif isinstance(final_index, Index) and \
             final_index.dtype in [np.int64, np.int32]:
            if not final_index.is_monotonic_increasing or \
               not final_index.equals(default_index):
                warnings.warn("An unsupported index was provided and " + \
                              "will be ignored when e.g. forecasting.")
                return (default_index, False)
        else:
            warnings.warn("An unsupported index was provided and " + \
                          "will be ignored when e.g. forecasting.")
            return (default_index, False)
        return (final_index, True)

    def _valid_input(self):
        endog = None
        if isinstance(self.endog, fd.DataFrame):
            raise NotImplementedError("Currently, endog cannot be passed as" + \
                                      " frovedis DataFrame!")
        elif isinstance(self.endog, DataFrame):
            shape = np.shape(self.endog)
            if len(shape) == 2 and shape[1] == 1:
                if shape[0] < (self.__ar_lag + self.__diff_order + \
                               self.__ma_lag + self.seasonal + 2):
                    raise ValueError("Number of samples in input is too " + \
                                     "less for time series analysis!")
                self._metadata = self._check_index()
                df_dtype = TypeUtil.to_id_dtype(self.endog.iloc[:, 0].dtype)
                if df_dtype in [DTYPE.INT, DTYPE.LONG]:
                    endog = FrovedisDvector(self.endog.iloc[:, 0],
                                            dtype = np.float64)
                else:
                    endog = FrovedisDvector(self.endog.iloc[:, 0])
                self._endog_len = shape[0]
            else:
                raise ValueError("Frovedis ARIMA models require univariate " + \
                                 "`endog`. Got shape {0}".format(shape))
        elif isinstance(self.endog, Series):
            shape = np.shape(self.endog.values)
            if len(shape) == 1:
                if shape[0] < (self.__ar_lag + self.__diff_order + \
                               self.__ma_lag + self.seasonal + 2):
                    raise ValueError("Number of samples in input is too " + \
                                     "less for time series analysis!")
                self._metadata = self._check_index()
                df_dtype = TypeUtil.to_id_dtype(self.endog.dtype)
                if df_dtype in [DTYPE.INT, DTYPE.LONG]:
                    endog = FrovedisDvector(self.endog.values,
                                            dtype = np.float64)
                else:
                    endog = FrovedisDvector(self.endog.values)
                self._endog_len = shape[0]
            else:
                raise ValueError("Frovedis ARIMA models require univariate " + \
                                 "`endog`. Got shape {0}".format(shape))
        elif isinstance(self.endog, FrovedisDvector):
            inp_data = self.endog.to_numpy_array()
            shape = np.shape(inp_data)
            if shape[0] < (self.__ar_lag + self.__diff_order + \
               self.__ma_lag + self.seasonal + 2):
                raise ValueError("Number of samples in input is too less " + \
                                 "for time series analysis!")
            self._endog_len = shape[0]
            df_dtype = TypeUtil.to_id_dtype(self.endog.get_numpy_dtype())
            if df_dtype in [DTYPE.INT, DTYPE.LONG]:
                endog = FrovedisDvector(inp_data, dtype = np.float64)
            else:
                endog = self.endog
        elif isinstance(self.endog, (np.ndarray, list, tuple)):
            shape = np.shape(self.endog)
            if len(shape) == 1 or (len(shape) == 2 and shape[1] == 1):
                if np.shape(self.endog)[0] < (self.__ar_lag + \
                   self.__diff_order + self.__ma_lag + self.seasonal + 2):
                    raise ValueError("Number of samples in input is too " + \
                                     "less for time series analysis!")
                endog = np.ravel(self.endog)
                self._endog_len = shape[0]
                if endog.dtype in [np.int32, np.int64]:
                    endog = FrovedisDvector().as_dvec(endog, dtype=np.float64)
                else:
                    endog = FrovedisDvector().as_dvec(endog)
            else:
                raise ValueError("Frovedis ARIMA models require univariate " + \
                                 "`endog`. Got shape {0}".format(shape))
        else:
            raise ValueError("unrecognized data " + \
                             "structures: %s" % type(self.endog))
        return endog

    def _validate_params(self):
        if len(self.order) != 3:
            raise ValueError("`order` argument must be an iterable with " + \
                             "three elements.")
        self.__ar_lag = 1 if self.order[0] is None else self.order[0]
        self.__diff_order = 0 if self.order[1] is None else self.order[1]
        self.__ma_lag = 0 if self.order[2] is None else self.order[2]
        if self.__ar_lag < 1:
            raise ValueError("Terms in the AR order cannot be less than 1.")
        if self.__diff_order < 0:
            raise ValueError("Cannot specify negative differencing.")
        if self.__ma_lag < 0:
            raise ValueError("Terms in the MA order cannot be negative.")
        if self.seasonal is None:
            self.seasonal = 0
        elif self.seasonal < 0:
            raise ValueError("The seasonal differencing interval cannot " + \
                             "be negative, given: " + str(self.seasonal))
        if self.solver == 'sag':
            self.solver = 'sgd'
        elif self.solver not in ['lapack', 'lbfgs', 'scalapack']:
            raise ValueError("Unknown solver: " + self.solver + " for time " + \
                             "series analysis!")
        if self.freq is not None and \
           not isinstance(self.endog, (Series, DataFrame)):
            raise ValueError("Frequency provided without associated index.")

    @set_association
    def fit(self):
        """
        DESC: Fit (estimate) the parameters of the model.
        During fitting, if endog has any type of index, then it will behave
        differently based on the given index type.
        If it is a numeric index with values 0,1,...,N-1, where `N` is n_samples
        or if it is (coerceable to) a DatetimeIndex or PeriodIndex with an
        associated frequency, then it is called a "Supported" index. Otherwise
        it is called an "Unsupported" index.
        For numeric indices type, it can be Index, RangeIndex instances.
        Here, for RangeIndex instances the indices can be in form:
          1. Indices with 0,2,4,....,N-2
          2. Indices with 10,12,14,....,N-2
        They should be monotonically increasing and not need to start with 0.
        When Index instance is used as numeric indices, then they have to be
        monotonically increaing and starting with 0.
        NOTE: A warning will be given when unsupported indices are used for
        training.
        """
        self.reset_metadata()
        if self.validate_specification:
            self._validate_params()
        else:
            raise ValueError("Currently validate_specification=True " + \
                             "is only supported!")
        endog = self._valid_input()
        self.__mdtype = endog.get_dtype()
        self.__mkind = M_KIND.ARM
        (host, port) = FrovedisServer.getServerInstance()
        rpclib.arima_fit(host, port, endog.get(), self.__ar_lag, \
                         self.__diff_order, self.__ma_lag, self.seasonal, \
                         self.auto_arima, str_encode(self.solver), \
                         self.verbose, self.__mid, self.__mdtype)
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        self.isfitted = True
        return self

    def _make_supported_index(self, key, index):
        key_oos = False
        if isinstance(index, DatetimeIndex):
            if isinstance(key, int):
                if key < 0:
                    if self._endog_len >= abs(key):
                        key = self._endog_len + key
                    else:
                        raise IndexError('index' + str(key) + ' is out of ' + \
                                         'bounds for axis 0 with ' + \
                                         'size ' + str(self._endog_len))
                elif key > self._endog_len:
                    index = date_range(start = index[0], \
                                       periods = int(key + 1), \
                                       freq = index.inferred_freq)
                    key = len(index) - 1
                    key_oos = True
            elif isinstance(key, (datetime.datetime, str)):
                try:
                    key = Timestamp(key)
                    if key > index[-1]:
                        index = date_range(start = index[0], end = key, \
                                           freq = index.inferred_freq)
                        key = len(index) - 1
                        key_oos = True
                    else:
                        key = index.get_loc(key)
                except KeyError as kerr:
                    raise KeyError(key) from kerr
            else:
                try:
                    key = index.get_loc(key)
                except:
                    raise KeyError(key)
        elif isinstance(index, PeriodIndex):
            if isinstance(key, int):
                if key < 0:
                    if self._endog_len >= abs(key):
                        key = self._endog_len + key
                    else:
                        raise KeyError(key)
                elif key > self._endog_len:
                    index = period_range(start = index[0], \
                                         periods = int(key + 1), \
                                         freq = index.freq)
                    key = len(index) - 1
                    key_oos = True
            elif isinstance(key, (datetime.datetime, str)):
                key = Period(key, freq = index.freq)
                try:
                    if key > index[-1]:
                        index = period_range(start = index[0], end = key, \
                                             freq = index.freq)
                        key = len(index) - 1
                        key_oos = True

                    else:
                        key = index.get_loc(key)
                except:
                    raise KeyError(key)
            else:
                try:
                    key = index.get_loc(key)
                except:
                    raise ValueError("Value must be Period, string, " + \
                                     "integer, or datetime")
        elif isinstance(index, RangeIndex):
            if isinstance(key, int):
                if key < 0:
                    if self._endog_len >= abs(key):
                        key = self._endog_len + key
                    else:
                        raise KeyError(key)
                elif key > self._endog_len:
                    index = RangeIndex(start = index.start, \
                                       stop = index.start + (key + 1) *
                                       index.step, step = index.step)
                    key = len(index) - 1
                    key_oos = True
        elif isinstance(index, Index) and index.dtype in [np.int64, np.int32]:
            if isinstance(key, int):
                if key < 0:
                    if self._endog_len >= abs(key):
                        key = self._endog_len + key
                    else:
                        raise KeyError(key)
                elif key >= self._endog_len:
                    index = Index(np.arange(index[0], int(key + 1)))
                    key = len(index) - 1
                    key_oos = True
        else:
            raise NotImplementedError("Invalid index type")
        return key, index, key_oos

    def _make_unsupported_index(self, key, index):
        key_oos = False
        if isinstance(index, RangeIndex):
            if isinstance(key, int):
                if key <= self._endog_len:
                    index = self.endog.index
                if key < 0:
                    if self._endog_len >= abs(key):
                        key = self._endog_len + key
                        index = self.endog.index
                    else:
                        raise IndexError('index' + str(key) + ' is out of ' + \
                                         'bounds for axis 0 with ' + \
                                         'size ' + str(self._endog_len))
                elif key > self._endog_len:
                    end = index.start + (key + 1) * index.step
                    index = RangeIndex(start = index.start, stop = end, \
                                       step = index.step)
                    key = index[key]
                    key_oos = True
            else:
                key = self.endog.index.get_loc(key)
                index = self.endog.index
        return key, index, key_oos

    def _make_pred_index(self, key, index, valid_index):
        if valid_index:
            key, index, key_oos = self._make_supported_index(key, index)
        else:
            key, index, key_oos = self._make_unsupported_index(key, index)
        return key, index, key_oos

    @check_association
    def predict(self, start=None, end=None, dynamic=False, **kwargs):
        """
        DESC: Perform in-sample prediction and out-of-sample prediction.
        PARAMS: start -> TYPE: {int, str or datetime},  DEFAULT: None,
                         it specifies the staring index after which the values
                         are to be predicted. If the date index does not have a
                         fixed frequency, start must be an integer index during
                         out-of-sample prediction.
                stop -> TYPE: {int, str or datetime}, DEFAULT: None,
                        it specifies the index till which the values are to
                        be predicted. If the date index does not have a fixed
                        frequency, end must be an integer index during out-of-
                        sample prediction.
                dynamic -> DEFAULT: False, (Unused)
                **kwargs -> (Unused)
        NOTE: When endog is array-like, start and end must be an integer. If it
              is a dataframe having date indices with associated frequency (also
              known as Supported index), then start and end can also be dates
              as string or datetime instance during in-sample prediction and
              out-of-sample prediction.

              When endog has Unsupported indices, only in-sample prediction can
              be done with dates as string or datetime instance. For performing
              out-of-sample prediction, only integer can be used.

              In case start or stop are negative then predicted values are
              returned from fitted values as long as indexes are accessible (in
              range). This is in sync with statsmodel behaviour. It is valid
              for both when endog is array-like or a pandas DataFrame.
        RETURNS: TYPE: {ndarray of shape (n_samples,) or Series}, it returns the
                 predicted values
        """
        if self._metadata is None:
            if start is None:
                start = 0
            elif not isinstance(start, int):
                raise ValueError("Cannot use an array-like `endog` with " + \
                                 "start of {} type.".format(type(start)))
            elif start < 0:
                if self._endog_len >= abs(start):
                    start = self._endog_len + start
                else:
                    raise KeyError("The `start` argument could not be " + \
                                   "matched to a location related to the " + \
                                   "index of the data.")
            if end is None:
                end = max(start, self._endog_len - 1)
            elif not isinstance(end, int):
                raise ValueError("Cannot use an array-like `endog` with " + \
                                 "end of {} type.".format(type(end)))
            elif end < 0:
                if self._endog_len >= abs(end):
                    end = self._endog_len + end
                else:
                    raise KeyError("The `end` argument could not be " + \
                                   "matched to a location related to the " + \
                                   "index of the data.")
        else:
            if start is None:
                start = 0
            try:
                start, start_index, start_oos = self._make_pred_index(start, \
                                                          self._metadata[0], \
                                                          self._metadata[1])
            except KeyError as kerr:
                raise KeyError("The `start` argument could not be  " + \
                               "matched to a location related to the " + \
                               "index of the data.") from kerr
            if end is None:
                end = max(start, self._endog_len - 1)
            try:
                end, end_index, end_oos = self._make_pred_index(end, \
                                                  self._metadata[0], \
                                                  self._metadata[1])
            except KeyError as kerr:
                raise KeyError("The `end` argument could not be matched " + \
                               "to a location related to the index of " + \
                               "the data.") from kerr
        if end < start:
            raise ValueError("In prediction `end` must not be less than `start`!")
        if dynamic:
            raise ValueError("Currently, ARIMA.predict() does not support " + \
                             "dynamic = True!")
        (host, port) = FrovedisServer.getServerInstance()
        arima_pred = rpclib.arima_predict(host, port, start, end, \
                                          self.__mid, self.__mdtype)
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        if self._metadata is not None:
            if not start_oos and not end_oos:
                pred_index = end_index[start : end + 1]
            else:
                pred_index = end_index[start:]
            return Series(arima_pred, index = pred_index, \
                          name = "predicted_mean")
        return np.asarray(arima_pred, dtype = np.float64)

    def _fcast_supported_index(self, steps, index):
        if isinstance(index, (DatetimeIndex, PeriodIndex)):
            date_func = (date_range
                         if isinstance(index, DatetimeIndex) else
                         period_range)
            if isinstance(steps, int):
                if steps <= 0:
                    raise ValueError("Prediction must have `end` " + \
                                     "after `start`!")
                index = date_func(start = index[-1], \
                                  periods = int(steps + 1), \
                                  freq = index.freq)
            elif isinstance(steps, (datetime.datetime, str)):
                steps = (Timestamp(steps)
                         if isinstance(index, DatetimeIndex) else
                         Period(steps, freq=index.freq))
                if steps in index.values:
                    raise ValueError("Prediction must have `end` " + \
                                     "after `start`!")
                index = date_func(start = index[-1], \
                                  end = steps, \
                                  freq = index.freq)
                if index.empty:
                    raise KeyError(steps)
                steps = len(index) - 1
            else:
                if isinstance(index, PeriodIndex):
                    raise ValueError("Value must be Period, string, " + \
                                     "integer, or datetime")
                raise KeyError(steps)
        elif isinstance(index, RangeIndex):
            if isinstance(steps, int):
                if steps <= 0:
                    raise ValueError("Prediction must have `end` " + \
                                     "after `start`!")
                end = (steps * index.step) + index.stop
                index = RangeIndex(start = index.stop - index.step,
                                   stop = end, step = index.step)
                steps = len(index) - 1
        elif isinstance(index, Index) and index.dtype in [np.int64, np.int32]:
            if isinstance(steps, int):
                if steps <= 0:
                    raise ValueError("Prediction must have `end` " + \
                                     "after `start`!")
                end = index[-1] + int(steps + 1)
                index = Index(np.arange(start=index[-1], stop=end))
                steps = len(index) - 1
        else:
            raise NotImplementedError("Invalid index type")
        return steps, index

    def _fcast_unsupported_index(self, steps, index):
        if isinstance(index, RangeIndex):
            if isinstance(steps, int):
                if steps <= 0:
                    raise ValueError("Prediction must have `end` " + \
                                     "after `start`!")
                end = steps + index.stop + 1
                index = RangeIndex(start = index.stop, stop = end)
                steps = len(index) - 1
            else:
                raise KeyError(steps)
        return steps, index

    def _fcast_index(self, steps, index, valid_index):
        if valid_index:
            steps, index = self._fcast_supported_index(steps, index)
        else:
            steps, index = self._fcast_unsupported_index(steps, index)
        return steps, index

    @check_association
    def forecast(self, steps=1, exog=None, alpha=0.05):
        """
        DESC: Perform out-of-sample forecasting.
        PARAMS: steps -> TYPE: {int, str, datetime}, DEFAULT: 1, it specifies
                         the number of out-of-sample values to be predicted.
                exog -> DEFAULT: None, (Unused)
                alpha -> DEFAULT: 0.05, (Unused)
        NOTE: When endog is array-like, steps must be an integer. If it is
              a dataframe having date indices with associated frequency (known
              as Supported index), then steps can also be dates as string or
              datetime instance during out-of-sample forecasting.

              When endog has unsupported indices, only integer must be used for
              out-of-sample forecasting. Also, a warning will be given when such
              indices are used for forecasting.
        RETURNS: TYPE: {ndarray of shape (n_samples,) or Series, returns out-of-
                 sample forecasted values.
        """
        if exog is not None:
            raise ValueError("Currently, exog is not supported by forecast()!")
        if alpha != 0.05:
            raise ValueError("Currently, alpha is not supported by forecast()!")
        if steps is None:
            steps = 1
        step_index = None
        if self._metadata is None and not isinstance(steps, int):
            raise ValueError("Cannot use an array-like `endog` with " + \
                             "steps of {} type.".format(type(steps)))
        if self._metadata is not None:
            try:
                steps, step_index = self._fcast_index(steps, \
                                           self._metadata[0], \
                                           self._metadata[1])
            except KeyError as kerr:
                raise KeyError("'The `end` argument could not be " + \
                               "matched to a location related to the index" + \
                               " of the data.'") from kerr
        (host, port) = FrovedisServer.getServerInstance()
        fcast = rpclib.arima_forecast(host, port, steps, self.__mid, \
                                      self.__mdtype)
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        if self._metadata is not None and self._metadata[1]:
            return Series(fcast, index = step_index[1:steps + 1], \
                          name = "predicted_mean")
        if self._metadata is not None and not self._metadata[1]:
            return Series(fcast, index = step_index[:steps], \
                          name = "predicted_mean")
        return np.asarray(fcast, dtype = np.float64)

    @property
    @check_association
    def fittedvalues(self):
        """
        DESC: fittedvalues getter
        RETURNS: TYPE: {ndarray of shape (n_samples,) or Series}, it returns the
                 fitted values of the model
        """
        if self._fittedvalues is None:
            (host, port) = FrovedisServer.getServerInstance()
            ret = rpclib.get_fitted_vector(host, port, self.__mid, \
                                           self.__mkind, self.__mdtype)
            excpt = rpclib.check_server_exception()
            if excpt["status"]:
                raise RuntimeError(excpt["info"])
            if self._metadata is None:
                self._fittedvalues = np.asarray(ret, dtype = np.float64)
            else:
                self._fittedvalues = Series(data = ret, \
                                            index = self.endog.index)
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
        self._metadata = None
        self._endog_len = None
        self.__ar_lag = None
        self.__diff_order = None
        self.__ma_lag = None

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
