"""
ensemble.py: wrapper of frovedis ensemble models - GBT
"""

# !/usr/bin/env python
import os.path
import pickle
import numpy as np
from ..exrpc import rpclib
from ..base import *
from ..exrpc.server import FrovedisServer
from ..matrix.ml_data import FrovedisLabeledPoint
from ..matrix.dtype import TypeUtil
from ..mllib.metrics import accuracy_score, r2_score
from ..mllib.model_util import M_KIND, ModelID, GLM


class GradientBoostingClassifier(BaseEstimator):
    """A python wrapper of Frovedis Gradient boosted trees: classifier"""
    # max_bins: added
    # verbose: added
    def __init__(self, loss="deviance", learning_rate=0.1, n_estimators=100,
                 subsample=1.0, criterion="friedman_mse", min_samples_split=2,
                 min_samples_leaf=1, min_weight_fraction_leaf=0.,
                 max_depth=3, min_impurity_decrease=0.,
                 min_impurity_split=None, init=None,
                 random_state=None, max_features=None,
                 verbose=0,
                 max_leaf_nodes=None, warm_start=False,
                 presort="deprecated", validation_fraction=0.1,
                 n_iter_no_change=None, tol=1e-4, ccp_alpha=0.0,
                 max_bins=32):
        self.loss = loss
        self.learning_rate = learning_rate
        self.n_estimators = n_estimators
        self.subsample = subsample
        self.criterion = criterion
        self.min_samples_split = min_samples_split
        self.min_samples_leaf = min_samples_leaf
        self.min_weight_fraction_leaf = min_weight_fraction_leaf
        self.max_depth = max_depth
        self.min_impurity_decrease = min_impurity_decrease # min_info_gain
        self.min_impurity_split = min_impurity_split
        self.init = init
        self.random_state = random_state
        self.max_features = max_features
        self.max_leaf_nodes = max_leaf_nodes
        self.warm_start = warm_start
        self.presort = presort
        self.validation_fraction = validation_fraction
        self.n_iter_no_change = n_iter_no_change
        self.tol = tol
        self.ccp_alpha = ccp_alpha
        self.verbose = verbose
        # extra
        self.__mid = None
        self.__mdtype = None
        self.__mkind = M_KIND.GBT
        self.label_map = None
        self.n_classes_ = None
        # Frovedis side parameters
        self.max_bins = max_bins
        self.algo = "Classification"

    def validate(self):
        """
        NAME: validate
        validating the params, if invalid raise ValueError
        """
        supported_losses = ("deviance", "default")
        supported_impurities = ("friedman_mse", "mae", "mse")

        if self.loss not in supported_losses:
            raise ValueError("Loss '{0:s}' not supported. ".format(self.loss))

        if self.learning_rate <= 0.0:
            raise ValueError("learning_rate must be greater than 0 but "
                             "was %r" % self.learning_rate)

        if self.n_estimators <= 0:
            raise ValueError("n_estimators must be greater than 0 but "
                             "was %r" % self.n_estimators)

        if not (0.0 < self.subsample <= 1.0):
            raise ValueError("subsample must be in (0,1] but "
                             "was %r" % self.subsample)

        if self.criterion not in supported_impurities:
            raise ValueError("Invalid criterion for GradientBoostingClassifier:"
                             + "'{}'".format(self.criterion))

        if self.max_depth < 0:
            raise ValueError("max depth can not be negative !")

        if self.min_impurity_decrease < 0:
            raise ValueError("Value of min_impurity_decrease should be "
                             "greater than 0")

        if self.max_bins < 0:
            raise ValueError("Value of max_bin should be greater than 0")

        if self.n_classes_ < 0:
            raise ValueError("Value of number of classes should be +ve integer"
                             " or zero!")

        if self.random_state is None:
            self.random_state = -1

        if(isinstance(self.max_features, int)):
            self.feature_subset_strategy = "customrate"
            self.feature_subset_rate = (self.max_features*1.0)/self.n_features
        elif(isinstance(self.max_features, float)):
            self.feature_subset_strategy = "customrate"
            self.feature_subset_rate = self.max_features
        elif(self.max_features is None):
            self.feature_subset_strategy = "all"
            self.feature_subset_rate = self.n_features
        elif(self.max_features == "auto"):
            self.feature_subset_strategy = "auto"
            self.feature_subset_rate = np.sqrt(self.n_features)
        elif(self.max_features == "sqrt"):
            self.feature_subset_strategy = "sqrt"
            self.feature_subset_rate = np.sqrt(self.n_features)
        elif(self.max_features == "log2"):
            self.feature_subset_strategy = "log2"
            self.feature_subset_rate = np.log2(self.n_features)
        else:
            raise ValueError("validate: unsupported max_features is encountered!")

        # mapping frovedis loss types with sklearn
        self.loss_map = {"deviance": "logloss", "exponential": "exponential",
                         "default": "default"}

    def fit(self, X, y):
        """
        NAME: fit
        fit for Gradient Boost Classifier
        """
        # release old model, if any
        self.release()
        # perform the fit
        self.__mid = ModelID.get()
        inp_data = FrovedisLabeledPoint(X, y)
        (X, y) = inp_data.get()
        dtype = inp_data.get_dtype()
        itype = inp_data.get_itype()
        dense = inp_data.is_dense()
        unique_labels = inp_data.get_distinct_labels()
        self.n_features = inp_data.numCols()
        self.n_classes_ = unique_labels.size
        self.classes_ = unique_labels
        self.n_estimators_ = self.n_estimators # TODO: confirm whether frovedis supports n_iter_no_change
        self.__mdtype = dtype
        if self.n_classes_ > 2:
            encoded_y, logic = y.encode(need_logic=True)
        elif self.n_classes_ == 2:
            target = [-1, 1]
            encoded_y, logic = y.encode(unique_labels, target, need_logic=True)
        else:
            raise ValueError("fit: number of unique labels in y are "
                             "less than 2")
        self.label_map = logic

        self.validate()
        (host, port) = FrovedisServer.getServerInstance()
        rpclib.gbt_train(host, port, X.get(), encoded_y.get(),
                         self.algo.encode('ascii'),
                         self.loss_map[self.loss].encode('ascii'),
                         self.criterion.lower().encode('ascii'),
                         self.learning_rate,  # double
                         self.max_depth,  # int
                         self.min_impurity_decrease,  # double
                         self.random_state,  # int seed
                         self.tol,  # double,
                         self.max_bins,  # int
                         self.subsample,  # double
                         self.feature_subset_strategy.encode('ascii'),
                         self.feature_subset_rate,
                         self.n_estimators, self.n_classes_, 
                         self.verbose, self.__mid,
                         dtype, itype, dense)

        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        return self

    def predict(self, X):
        """
        NAME: predict
        Perform classification on an array of test vectors X.
        """
        if self.__mid is not None:
            frov_pred = GLM.predict(X, self.__mid, self.__mkind, self.__mdtype,
                                    False)
            new_pred = [self.label_map[frov_pred[i]]
                        for i in range(0, len(frov_pred))]
            return np.asarray(new_pred, dtype=np.int64)
        else:
            raise ValueError("predict is called before calling fit, or the "
                             "model is released.")

    def load(self, fname, dtype=None):
        """
        NAME: load
        Load the model from a file
        """
        if not os.path.exists(fname):
            raise ValueError("the model with name %s does not exist!" % fname)
        self.release()
        target = open(fname + "/label_map", "rb")
        self.label_map = pickle.load(target)
        target.close()
        metadata = open(fname + "/metadata", "rb")
        self.n_classes_, self.__mkind, self.__mdtype = pickle.load(metadata)
        metadata.close()
        if dtype is not None:
            mdt = TypeUtil.to_numpy_dtype(self.__mdtype)
            if dtype != mdt:
                raise ValueError("load: type mismatches detected!" +
                                 "expected type: " + str(mdt) +
                                 "; given type: " + str(dtype))
        self.__mid = ModelID.get()
        GLM.load(self.__mid, self.__mkind, self.__mdtype, fname + "/model")
        return self

    def score(self, X, y):
        """
        NAME: score
        check the accuracy for the model
        """
        if self.__mid is not None:
            return accuracy_score(y, self.predict(X))

    def save(self, fname):
        """
        NAME: save
        Save model to a file
        """
        if self.__mid is not None:
            if os.path.exists(fname):
                raise ValueError("another model with %s name already"
                                 " exists!" % fname)
            else:
                os.makedirs(fname)
            GLM.save(self.__mid, self.__mkind, self.__mdtype, fname + "/model")
            target = open(fname + "/label_map", "wb")
            pickle.dump(self.label_map, target)
            target.close()
            metadata = open(fname + "/metadata", "wb")
            pickle.dump((self.n_classes_, self.__mkind,
                         self.__mdtype), metadata)
            metadata.close()
        else:
            raise AttributeError("save: requested model might have "
                                 "been released!")

    def debug_print(self):
        """
        NAME: debug_print
        Show the model
        """
        if self.__mid is not None:
            GLM.debug_print(self.__mid, self.__mkind, self.__mdtype)

    def release(self):
        """
        NAME: release
        Release the model-id to generate new model-id
        """
        if self.__mid is not None:
            GLM.release(self.__mid, self.__mkind, self.__mdtype)
            self.__mid = None
            self.__mdtype = None
            self.label_map = None
            self.n_classes_ = None

    def __del__(self):
        """
        NAME: __del__
        Check FrovedisServer is up then release
        """
        if FrovedisServer.isUP():
            self.release()


class GradientBoostingRegressor(BaseEstimator):
    """A python wrapper of Frovedis Gradient boosted trees: regressor"""

    def __init__(self, loss='ls', learning_rate=0.1, n_estimators=100,
                 subsample=1.0, criterion='friedman_mse', min_samples_split=2,
                 min_samples_leaf=1, min_weight_fraction_leaf=0.,
                 max_depth=3, min_impurity_decrease=0.,
                 min_impurity_split=None, init=None, random_state=None,
                 max_features=None, alpha=0.9, verbose=0, max_leaf_nodes=None,
                 warm_start=False, presort='deprecated',
                 validation_fraction=0.1,
                 n_iter_no_change=None, tol=1e-4, ccp_alpha=0.0,
                 max_bins=32):
        self.loss = loss
        self.learning_rate = learning_rate
        self.n_estimators = n_estimators
        self.subsample = subsample
        self.criterion = criterion
        self.min_samples_split = min_samples_split
        self.min_samples_leaf = min_samples_leaf
        self.min_weight_fraction_leaf = min_weight_fraction_leaf
        self.max_depth = max_depth
        self.min_impurity_decrease = min_impurity_decrease # min_info_gain
        self.min_impurity_split = min_impurity_split
        self.init = init
        self.random_state = random_state
        self.max_features = max_features
        self.alpha = alpha
        self.max_leaf_nodes = max_leaf_nodes
        self.warm_start = warm_start
        self.presort = presort
        self.validation_fraction = validation_fraction
        self.n_iter_no_change = n_iter_no_change
        self.tol = tol
        self.ccp_alpha = ccp_alpha
        self.verbose = verbose
        # extra
        self.__mid = None
        self.__mdtype = None
        self.__mkind = M_KIND.GBT
        self.max_bins = max_bins
        self.algo = "Regression"

    def validate(self):
        """
        NAME: validate
        validating the params, if invalid raise ValueError
        """
        supported_losses = ("ls", "lad", "default")
        supported_impurities = ("friedman_mse", "mae", "mse")

        if self.loss not in supported_losses:
            raise ValueError("Loss '{0:s}' not supported. ".format(self.loss))

        if self.learning_rate <= 0.0:
            raise ValueError("learning_rate must be greater than 0 but "
                             "was %r" % self.learning_rate)

        if self.n_estimators <= 0:
            raise ValueError("n_estimators must be greater than 0 but "
                             "was %r" % self.n_estimators)

        if not (0.0 < self.subsample <= 1.0):
            raise ValueError("subsample must be in (0,1] but "
                             "was %r" % self.subsample)

        if self.criterion not in supported_impurities:
            raise ValueError("Invalid criterion for GradientBoostingClassifier:"
                             + "'{}'".format(self.criterion))

        if self.max_depth < 0:
            raise ValueError("max depth can not be negative !")

        if self.min_impurity_decrease < 0:
            raise ValueError("Value of min_impurity_decrease should be "
                             "greater than 0")

        if self.max_bins < 0:
            raise ValueError("Value of max_bin should be greater than 0")

        if self.random_state is None:
            self.random_state = -1

        if(isinstance(self.max_features, int)):
            self.feature_subset_strategy = "customrate"
            self.feature_subset_rate = (self.max_features*1.0)/self.n_features
        elif(isinstance(self.max_features, float)):
            self.feature_subset_strategy = "customrate"
            self.feature_subset_rate = self.max_features
        elif(self.max_features is None):
            self.feature_subset_strategy = "all"
            self.feature_subset_rate = self.n_features
        elif(self.max_features == "auto"):
            self.feature_subset_strategy = "auto"
            self.feature_subset_rate = np.sqrt(self.n_features)
        elif(self.max_features == "sqrt"):
            self.feature_subset_strategy = "sqrt"
            self.feature_subset_rate = np.sqrt(self.n_features)
        elif(self.max_features == "log2"):
            self.feature_subset_strategy = "log2"
            self.feature_subset_rate = np.log2(self.n_features)
        else:
            raise ValueError("validate: unsupported max_features is encountered!")

        # mapping frovedis loss types with sklearn
        self.loss_map = {"ls": "leastsquareserror", "lad": "leastabsoluteerror",
                         "default": "default"}

    def fit(self, X, y):
        """
        NAME: fit
        fit for Gradient Boost Classifier
        """
        # release old model, if any
        self.release()
        # perform the fit
        self.__mid = ModelID.get()
        inp_data = FrovedisLabeledPoint(X, y)
        (X, y) = inp_data.get()
        dtype = inp_data.get_dtype()
        itype = inp_data.get_itype()
        dense = inp_data.is_dense()
        self.n_estimators_ = self.n_estimators # TODO: confirm whether frovedis supports n_iter_no_change
        self.__mdtype = dtype
        self.n_features = inp_data.numCols()
        self.validate()
        (host, port) = FrovedisServer.getServerInstance()
        rpclib.gbt_train(host, port, X.get(), y.get(),
                         self.algo.encode('ascii'),
                         self.loss_map[self.loss].encode('ascii'),
                         self.criterion.lower().encode('ascii'),
                         self.learning_rate,  # double
                         self.max_depth,  # int
                         self.min_impurity_decrease,  # double
                         self.random_state,  # int seed
                         self.tol,  # double,
                         self.max_bins,  # int
                         self.subsample,  # double
                         self.feature_subset_strategy.encode('ascii'),
                         self.feature_subset_rate,
                         self.n_estimators, -1, # -1 for n_classes , as regressor  
                         self.verbose, self.__mid,
                         dtype, itype, dense)
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        return self

    def predict(self, X):
        """
        NAME: predict
        Perform classification on an array of test vectors X.
        """
        if self.__mid is not None:
            frov_pred = GLM.predict(X, self.__mid, self.__mkind, self.__mdtype,
                                    False)
            return np.asarray(frov_pred, dtype=np.float64)
        else:
            raise ValueError("predict is called before calling fit, or the "
                             "model is released.")

    def load(self, fname, dtype=None):
        """
        NAME: load
        Load the model from a file
        """
        if not os.path.exists(fname):
            raise ValueError("the model with name %s does not exist!" % fname)
        self.release()
        metadata = open(fname + "/metadata", "rb")
        self.__mkind, self.__mdtype = pickle.load(metadata)
        metadata.close()
        if dtype is not None:
            mdt = TypeUtil.to_numpy_dtype(self.__mdtype)
            if dtype != mdt:
                raise ValueError("load: type mismatches detected!" +
                                 "expected type: " + str(mdt) +
                                 "; given type: " + str(dtype))
        self.__mid = ModelID.get()
        GLM.load(self.__mid, self.__mkind, self.__mdtype, fname + "/model")
        return self

    def score(self, X, y):
        """
        NAME: score
        check the r2 score for the model
        """
        if self.__mid is not None:
            return r2_score(y, self.predict(X))

    def save(self, fname):
        """
        NAME: save
        Save model to a file
        """
        if self.__mid is not None:
            if os.path.exists(fname):
                raise ValueError("another model with %s name already"
                                 " exists!" % fname)
            else:
                os.makedirs(fname)
            GLM.save(self.__mid, self.__mkind, self.__mdtype, fname + "/model")
            metadata = open(fname + "/metadata", "wb")
            pickle.dump((self.__mkind,
                         self.__mdtype), metadata)
            metadata.close()
        else:
            raise AttributeError("save: requested model might have "
                                 "been released!")

    def debug_print(self):
        """
        NAME: debug_print
        Show the model
        """
        if self.__mid is not None:
            GLM.debug_print(self.__mid, self.__mkind, self.__mdtype)

    def release(self):
        """
        NAME: release
        Release the model-id to generate new model-id
        """
        if self.__mid is not None:
            GLM.release(self.__mid, self.__mkind, self.__mdtype)
            self.__mid = None
            self.__mdtype = None
            self.label_map = None

    def __del__(self):
        """
        NAME: __del__
        Check FrovedisServer is up then release
        """
        if FrovedisServer.isUP():
            self.release()

