#!/usr/bin/env python
"""
wrapper of frovedis Random Forest (classifier and regressor)
"""

import pickle
import os.path
import numpy as np
from ...base import *
from ...exrpc import rpclib
from ...exrpc.server import FrovedisServer, check_server_state
from ...matrix.ml_data import FrovedisLabeledPoint
from ...matrix.dtype import TypeUtil
from ..metrics import accuracy_score, r2_score
from ..model_util import M_KIND, ModelID, GLM

#Random Forest Classifier Class
class RandomForestClassifier(BaseEstimator):
    """A python wrapper of Frovedis Random Forest Classifier
    parameter   		:   default value
    n_estimators	   	:   10
    criterion or impurity   	:   'gini'
    min_samples_split  		:   2
    min_samples_leaf   		:   1
    min_weight_fraction_leaf 	:   0.0
    min_impurity_decrease	:   0.0
    bootstrap     		:   False
    verbose      		:   0
    """
    # defaults are as per Frovedis/scikit-learn
    # Random Forest Classifier constructor
    # added: max_bins
    def __init__(self, n_estimators=100, criterion='gini', max_depth=None,
                 min_samples_split=2, min_samples_leaf=1,
                 min_weight_fraction_leaf=0.0, max_features='auto',
                 max_leaf_nodes=None, min_impurity_decrease=0.0,
                 min_impurity_split=None, bootstrap=True, oob_score=False,
                 n_jobs=None, random_state=None, verbose=0, warm_start=False,
                 class_weight=None, ccp_alpha=0.0, max_samples=None,
                 max_bins=32):
        self.n_estimators = n_estimators
        self.criterion = criterion
        self.max_depth = max_depth
        self.min_samples_split = min_samples_split
        self.min_samples_leaf = min_samples_leaf
        self.min_weight_fraction_leaf = min_weight_fraction_leaf
        self.max_features = max_features
        self.max_leaf_nodes = max_leaf_nodes
        self.min_impurity_decrease = min_impurity_decrease
        self.min_impurity_split = min_impurity_split
        self.bootstrap = bootstrap
        self.oob_score = oob_score
        self.n_jobs = n_jobs
        self.random_state = random_state
        self.verbose = verbose
        self.warm_start = warm_start
        self.class_weight = class_weight
        self.ccp_alpha = ccp_alpha
        self.max_samples = max_samples
        # extra
        self.__mid = None
        self.__mdtype = None
        self.__mkind = M_KIND.RFM
        self.label_map = None
        self.n_classes_ = None
        # Frovedis side parameters
        self.max_bins = max_bins
        self.algo = "Classification"

    def validate(self):
        """
        NAME: validate
        """
        supported_impurities = ("gini", "entropy")
        if self.criterion not in supported_impurities:
            raise ValueError("Invalid criterion for RandomForestClassifier:"
                             + "'{}'".format(self.criterion))

        if self.n_estimators <= 0:
            raise ValueError("n_estimators can not be negative !")

        if self.max_depth is None:
            self.max_depth = 4 # default as in frovedis random_forest
        elif self.max_depth < 0:
            raise ValueError("max depth can not be negative !")

        if self.min_impurity_decrease < 0:
            raise ValueError(\
            "Value of min_impurity_decrease should be greater than 0")

        if self.max_bins < 0:
            raise ValueError("Value of max_bin should be greater than 0")

        if self.min_samples_leaf < 0:
            raise ValueError(\
            "Value of min_samples_leaf should be greater than 0!")

        if self.random_state is None:
            self.random_state = -1

        if(isinstance(self.min_samples_leaf, float)):
            self.min_samples_leaf = int(np.ceil(self.min_samples_split * self.n_samples_))

        if(isinstance(self.max_features, int)):
            self.feature_subset_strategy = "customrate"
            self.feature_subset_rate = (self.max_features*1.0)/self.n_features_
        elif(isinstance(self.max_features, float)):
            self.feature_subset_strategy = "customrate"
            self.feature_subset_rate = self.max_features
        elif(self.max_features is None):
            self.feature_subset_strategy = "all"
            self.feature_subset_rate = self.n_features_
        elif(self.max_features == "auto"):
            self.feature_subset_strategy = "auto"
            self.feature_subset_rate = np.sqrt(self.n_features_)
        elif(self.max_features == "sqrt"):
            self.feature_subset_strategy = "sqrt"
            self.feature_subset_rate = np.sqrt(self.n_features_)
        elif(self.max_features == "log2"):
            self.feature_subset_strategy = "log2"
            self.feature_subset_rate = np.log2(self.n_features_)
        else:
            raise ValueError("validate: unsupported max_features is encountered!")

    # Fit Random Forest classifier according to X (input data), y (Label)
    def fit(self, X, y):
        """
        NAME: fit
        """
        self.release()
        inp_data = FrovedisLabeledPoint(X, y, \
                   caller = "[" + self.__class__.__name__ + "] fit: ",\
                   encode_label = True, binary_encoder=[0, 1], \
                   dense_kind = 'colmajor', densify=True)
        X, y, logic = inp_data.get()
        self._classes = inp_data.get_distinct_labels()
        self.n_classes_ = len(self._classes)
        self.n_features_ = inp_data.numCols()
        self.label_map = logic
        dtype = inp_data.get_dtype()
        itype = inp_data.get_itype()
        dense = inp_data.is_dense()
        self.validate()

        self.__mdtype = dtype
        self.__mid = ModelID.get()
        (host, port) = FrovedisServer.getServerInstance()
        rpclib.rf_train(host, port, X.get(), y.get(), 
                        self.algo.encode('ascii'), 
                        self.criterion.encode('ascii'),
                        self.n_estimators, self.max_depth,
                        self.n_classes_, 
                        self.feature_subset_strategy.encode('ascii'),
                        self.feature_subset_rate,
                        self.max_bins, self.min_samples_leaf,
                        self.min_impurity_decrease, self.random_state,
                        self.verbose, self.__mid, dtype, itype, dense)
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        return self

    # Perform classification on an array of test vectors X.
    def predict(self, X):
        """
        NAME: predict
        """
        if self.__mid is not None:
            frov_pred = GLM.predict(X, self.__mid, self.__mkind, \
                                    self.__mdtype, False)
            return np.asarray([self.label_map[frov_pred[i]] \
                              for i in range(0, len(frov_pred))])
        else:
            raise ValueError(\
            "predict is called before calling fit, or the model is released.")

    @property
    def classes_(self):
        """classes_ getter"""
        if self.__mid is not None:
            if self._classes is None:
                self._classes = np.sort(list(self.label_map.values()))
            return self._classes
        else:
            raise AttributeError("attribute 'classes_' " \
               "might have been released or called before fit")

    @classes_.setter
    def classes_(self, val):
        """classes_ setter"""
        raise AttributeError(\
            "attribute 'classes_' of RandomForestClassifier "
            "object is not writable")

    # Load the model from a file
    def load(self, fname, dtype=None):
        """
        NAME: load
        """
        if not os.path.exists(fname):
            raise ValueError(\
                "the model with name %s does not exist!" % fname)
        self.release()
        target = open(fname+"/label_map", "rb")
        self.label_map = pickle.load(target)
        target.close()
        self._classes = np.sort(list(self.label_map.values()))
        metadata = open(fname+"/metadata", "rb")
        self.n_classes_, self.__mkind, self.__mdtype = pickle.load(metadata)
        metadata.close()
        if dtype is not None:
            mdt = TypeUtil.to_numpy_dtype(self.__mdtype)
            if dtype != mdt:
                raise ValueError("load: type mismatches detected!" + \
                                 "expected type: " + str(mdt) + \
                                 "; given type: " + str(dtype))
        self.__mid = ModelID.get()
        GLM.load(self.__mid, self.__mkind, self.__mdtype, fname+"/model")
        return self

    # calculate the mean accuracy on the given test data and labels.
    def score(self, X, y):
        """
        NAME: score
        """
        if self.__mid is not None:
            return accuracy_score(y, self.predict(X))


    # Save model to a file
    def save(self, fname):
        """
        NAME: save
        """
        if self.__mid is not None:
            if os.path.exists(fname):
                raise ValueError(\
                    "another model with %s name already exists!" % fname)
            else:
                os.makedirs(fname)
            GLM.save(self.__mid, self.__mkind, self.__mdtype, fname+"/model")
            target = open(fname+"/label_map", "wb")
            pickle.dump(self.label_map, target)
            target.close()
            metadata = open(fname+"/metadata", "wb")
            pickle.dump((self.n_classes_, self.__mkind, \
                self.__mdtype), metadata)
            metadata.close()
        else:
            raise AttributeError(\
                "save: requested model might have been released!")

    # Show the model
    def debug_print(self):
        """
        NAME: debug_print
        """
        if self.__mid is not None:
            GLM.debug_print(self.__mid, self.__mkind, self.__mdtype)

    # Release the model-id to generate new model-id
    def release(self):
        """
        NAME: release
        """
        if self.__mid is not None:
            GLM.release(self.__mid, self.__mkind, self.__mdtype)
            self.__mid = None
            self.__mdtype = None
            self.label_map = None
            self._classes = None
            self.n_classes_ = None


    # Check FrovedisServer is up then release
    def __del__(self):
        """
        NAME: __del__
        """
        if FrovedisServer.isUP():
            self.release()

# Random Forest Regressor Class
class RandomForestRegressor(BaseEstimator):
    """A python wrapper of Frovedis Random Forest Regressor
    parameter   		:   default value
    n_estimators	   	:   100
    criterion or impurity   	:   'mse'
    min_samples_split  		:   2
    min_samples_leaf   		:   1
    min_weight_fraction_leaf 	:   0.0
    min_impurity_decrease	:   0.0
    bootstrap     		:   False
    verbose      		:   0
    """
    # defaults are as per Frovedis/scikit-learn
    # Random Forest Regressor constructor
    def __init__(self, n_estimators=100, criterion='mse', max_depth=None,
                 min_samples_split=2, min_samples_leaf=1,
                 min_weight_fraction_leaf=0.0, max_features='auto',
                 max_leaf_nodes=None, min_impurity_decrease=0.0,
                 min_impurity_split=None, bootstrap=True, oob_score=False,
                 n_jobs=None, random_state=None, verbose=0, warm_start=False,
                 ccp_alpha=0.0, max_samples=None, 
                 max_bins=32):
        self.n_estimators = n_estimators
        self.criterion = criterion
        self.max_depth = max_depth
        self.min_samples_split = min_samples_split
        self.min_samples_leaf = min_samples_leaf
        self.min_weight_fraction_leaf = min_weight_fraction_leaf
        self.max_features = max_features
        self.max_leaf_nodes = max_leaf_nodes
        self.min_impurity_decrease = min_impurity_decrease
        self.min_impurity_split = min_impurity_split
        self.bootstrap = bootstrap
        self.oob_score = oob_score
        self.n_jobs = n_jobs
        self.random_state = random_state
        self.verbose = verbose
        self.warm_start = warm_start
        self.ccp_alpha = ccp_alpha
        self.max_samples = max_samples
        # extra
        self.__mid = None
        self.__mdtype = None
        self.__mkind = M_KIND.RFM
        # Frovedis side parameters
        self.max_bins = max_bins
        self.algo = "Regression"

    def validate(self):
        """
        NAME: validate
        """
        self.n_classes_ = -1 # regressor doesn't have n_classes_ parameter

        if self.n_estimators < 0:
            raise ValueError("n_estimators can not be negative !")

        supported_impurities = ("mse", "mae")
        if self.criterion not in supported_impurities:
            raise ValueError("Invalid criterion for RandomForestRegressor:"
                             + "'{}'".format(self.criterion))

        if self.max_depth is None:
            self.max_depth = 4 #default as in frovedis random_forest
        elif self.max_depth < 0:
            raise ValueError("max depth can not be negative !")

        if self.min_impurity_decrease < 0:
            raise ValueError(\
            "Value of min_impurity_decrease should be greater than 0!")
        
        if self.max_bins < 0:
            raise ValueError("Value of max_bin should be greater than 0!")

        if self.min_samples_leaf < 0:
            raise ValueError(\
            "Value of min_samples_leaf should be greater than 0!")
        if(isinstance(self.min_samples_leaf, float)):
            self.min_samples_leaf = \
            int(np.ceil(self.min_samples_split * self.n_samples_))

        if self.random_state is None:
            self.random_state = -1

        if(isinstance(self.max_features, int)):
            self.feature_subset_strategy = "customrate"
            self.feature_subset_rate = (self.max_features*1.0)/self.n_features_
        elif(isinstance(self.max_features, float)):
            self.feature_subset_strategy = "customrate"
            self.feature_subset_rate = self.max_features
        elif(self.max_features is None):
            self.feature_subset_strategy = "all"
            self.feature_subset_rate = self.n_features_
        elif(self.max_features == "auto"):
            self.feature_subset_strategy = "auto"
            self.feature_subset_rate = np.sqrt(self.n_features_)
        elif(self.max_features == "sqrt"):
            self.feature_subset_strategy = "sqrt"
            self.feature_subset_rate = np.sqrt(self.n_features_)
        elif(self.max_features == "log2"):
            self.feature_subset_strategy = "log2"
            self.feature_subset_rate = np.log2(self.n_features_)
        else:
            raise ValueError("validate: unsupported max_features is encountered!")

    # Fit Random Forest classifier according to X (input data), y (Label)
    def fit(self, X, y):
        """
        NAME: fit
        """
        self.release()
        inp_data = FrovedisLabeledPoint(X, y, \
                   caller = "[" + self.__class__.__name__ + "] fit: ",\
                   dense_kind = 'colmajor', densify=True)
        (X, y) = inp_data.get()
        dtype = inp_data.get_dtype()
        itype = inp_data.get_itype()
        dense = inp_data.is_dense()
        self.n_features_ = inp_data.numCols()
        self.validate()
        self.__mdtype = dtype
        self.__mid = ModelID.get()
        (host, port) = FrovedisServer.getServerInstance()
        rpclib.rf_train(host, port, X.get(), y.get(),
                        self.algo.encode('ascii'),
                        self.criterion.encode('ascii'),
                        self.n_estimators, self.max_depth,
                        self.n_classes_,
                        self.feature_subset_strategy.encode('ascii'),
                        self.feature_subset_rate,
                        self.max_bins, self.min_samples_leaf,
                        self.min_impurity_decrease, self.random_state,
                        self.verbose, self.__mid, dtype, itype, dense)
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        return self

    # Perform prediction on an array of test vectors X.
    def predict(self, X):
        """
        NAME: predict
        """
        if self.__mid is not None:
            ret = GLM.predict(X, self.__mid, self.__mkind, \
                              self.__mdtype, False)
            return np.asarray(ret, dtype=np.float64)
        else:
            raise ValueError(\
            "predict is called before calling fit, or the model is released.")

    # Load the model from a file
    def load(self, fname, dtype=None):
        """
        NAME: load
        """
        if not os.path.exists(fname):
            raise ValueError(\
                "the model with name %s does not exist!" % fname)
        self.release()
        metadata = open(fname+"/metadata", "rb")
        self.__mkind, self.__mdtype = pickle.load(metadata)
        metadata.close()
        if dtype is not None:
            mdt = TypeUtil.to_numpy_dtype(self.__mdtype)
            if dtype != mdt:
                raise ValueError("load: type mismatches detected!" + \
                                 "expected type: " + str(mdt) + \
                                 "; given type: " + str(dtype))
        self.__mid = ModelID.get()
        GLM.load(self.__mid, self.__mkind, self.__mdtype, fname+"/model")
        return self

    # calculate the root mean square value on the given test data and labels.
    def score(self, X, y):
        """
        NAME: score
        """
        if self.__mid is not None:
            return r2_score(y, self.predict(X))

    # Save model to a file
    def save(self, fname):
        """
        NAME: save
        """
        if self.__mid is not None:
            if os.path.exists(fname):
                raise ValueError(\
                    "another model with %s name already exists!" % fname)
            else:
                os.makedirs(fname)
            GLM.save(self.__mid, self.__mkind, self.__mdtype, fname+"/model")
            metadata = open(fname+"/metadata", "wb")
            pickle.dump((self.__mkind, self.__mdtype), metadata)
            metadata.close()
        else:
            raise AttributeError(\
                "save: requested model might have been released!")

    # Show the model
    def debug_print(self):
        """
        NAME: debug_print
        """
        if self.__mid is not None:
            GLM.debug_print(self.__mid, self.__mkind, self.__mdtype)

    # Release the model-id to generate new model-id
    def release(self):
        """
        NAME: release
        """
        if self.__mid is not None:
            GLM.release(self.__mid, self.__mkind, self.__mdtype)
            self.__mid = None
            self.__mdtype = None

    # Check FrovedisServer is up then release
    def __del__(self):
        """
        NAME: __del__
        """
        if FrovedisServer.isUP():
            self.release()

