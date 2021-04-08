"""
tree.py: wrapper of frovedis decistion tree (classifier and regressor)
"""

#!/usr/bin/env python
import os.path
import pickle
import numpy as np
import numbers
from ..base import *
from ..exrpc import rpclib
from ..exrpc.server import FrovedisServer, check_server_state
from ..matrix.ml_data import FrovedisLabeledPoint
from ..matrix.dtype import TypeUtil
from .metrics import *
from .model_util import *

# Decision Tree Regressor Class
class DecisionTreeRegressor(BaseEstimator):
    """A python wrapper of Frovedis Decision Tree Regressor
    parameter                   :   default value
    criterion or impurity       :   'mse'
    splitter    		:   'best'
    min_impurity_decrease	:   0.0
    min_samples_split  		:   2
    min_samples_leaf   		:   1
    min_weight_fraction_leaf 	:   0.0
    presort     		:   deprecated
    verbose      		:   0
    """

    # defaults are as per Frovedis/scikit-learn
    # Decision Tree Regressor constructor
    def __init__(self, criterion='mse', splitter='best',
                 max_depth=None, min_samples_split=2, min_samples_leaf=1,
                 min_weight_fraction_leaf=0.0, max_features=None,
                 random_state=None,
                 max_leaf_nodes=None, min_impurity_decrease=0.0,
                 presort='deprecated', ccp_alpha=0.0,
                 max_bins=32, verbose=0, categorical_info={}):
        self.criterion = criterion
        self.splitter = splitter
        self.max_depth = max_depth
        self.min_samples_split = min_samples_split
        self.min_samples_leaf = min_samples_leaf
        self.min_weight_fraction_leaf = min_weight_fraction_leaf
        self.max_features = max_features
        self.random_state = random_state
        self.max_leaf_nodes = max_leaf_nodes
        self.min_impurity_decrease = min_impurity_decrease
        self.presort = presort
        self.ccp_alpha = ccp_alpha
        # extra
        self.__mid = None
        self.__sid = None
        self.__mdtype = None
        self.__mkind = M_KIND.DTM
        # Frovedis side parameters
        self.max_bins = max_bins
        self.verbose = verbose
        self.categorical_info = categorical_info
        self.algo = "Regression"
        self.n_classes_ = 0 

    #To validate the input parameters
    def validate(self):
        """
        NAME: validate
        """
        self.criterion = self.criterion.upper()
        if self.criterion != "MSE":
            raise ValueError("Invalid criterion for Decision Tree Regressor!")

        if self.max_depth is None:
            self.max_depth = 5 # as in Frovedis DecisionTree
        elif self.max_depth < 0:
            raise ValueError("max depth can not be negative!")

        if self.min_impurity_decrease < 0:
            raise ValueError(\
            "Value of min_impurity_decrease should be greater than 0!")

        if self.max_bins < 0:
            raise ValueError("Value of max_bin should be greater than 0!")

        if isinstance(self.min_samples_leaf, numbers.Integral):
            if not 1 <= self.min_samples_leaf:
                raise ValueError("min_samples_leaf must be at least 1 "
                                 "or in (0, 0.5], got %s!"
                                 % self.min_samples_leaf)
        else:  # float
            if not 0. < self.min_samples_leaf <= 0.5:
                raise ValueError("min_samples_leaf must be at least 1 "
                                 "or in (0, 0.5], got %s!"
                                 % self.min_samples_leaf)
            self.min_samples_leaf = \
            int(ceil(self.min_samples_leaf * self.n_samples_))

    # Fit Decision Tree Regressor according to X (input data), y (Label)
    def fit(self, X, y):
        """
        NAME: fit
        """
        self.release()
        inp_data = FrovedisLabeledPoint(X, y, \
                   caller = "[" + self.__class__.__name__ + "] fit: ", \
                   dense_kind='colmajor', densify=True)
        X, y = inp_data.get()
        dtype = inp_data.get_dtype()
        itype = inp_data.get_itype()
        dense = inp_data.is_dense()
        self.n_samples_ = inp_data.numRows()
        self.n_features_ = inp_data.numCols()
        self.__mdtype = dtype
        self.validate()
        info_keys = np.asarray(list(self.categorical_info.keys()), \
                               dtype=np.int32)
        info_vals = np.asarray(list(self.categorical_info.values()), \
                               dtype=np.int32)
        (host, port) = FrovedisServer.getServerInstance()
        self.__mid = ModelID.get()
        self.__sid = FrovedisServer.getID()
        rpclib.dt_train(host, port, X.get(), y.get(), \
                        self.algo.encode('ascii'), \
                        self.criterion.encode('ascii'),\
                        self.max_depth, self.n_classes_, \
                        self.max_bins, self.min_samples_leaf, \
                        self.min_impurity_decrease, self.verbose, \
                        self.__mid, info_keys, info_vals, len(info_keys), \
                        dtype, itype, dense)
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        return self

    # Perform prediction on an array of test vectors X.
    def predict(self, X):
        """
        NAME: predict
        """
        if self.__mid is None:
            raise ValueError(\
            "predict is called before calling fit, or the model is released.")
        check_server_state(self.__sid, self.__class__.__name__)
        ret = GLM.predict(X, self.__mid, self.__mkind, self.__mdtype, False)
        return np.asarray(ret, dtype=np.float64)

    # Load the model from a file
    def load(self, fname, dtype=None):
        """
        NAME: load
        """
        if not os.path.exists(fname):
            raise ValueError(\
                "the model with name %s does not exist!" % fname)
        self.release()
        metadata = open(fname + "/metadata", "rb")
        self.__mkind, self.__mdtype = pickle.load(metadata)
        metadata.close()
        if dtype is not None:
            mdt = TypeUtil.to_numpy_dtype(self.__mdtype)
            if dtype != mdt:
                raise ValueError("load: type mismatches detected! " + \
                                 "expected type: " + str(mdt) + \
                                 "; given type: " + str(dtype))
        self.__mid = ModelID.get()
        GLM.load(self.__mid, self.__mkind, self.__mdtype, fname + "/model")
        self.__sid = FrovedisServer.getID()
        return self

    # Save model to a file
    def save(self, fname):
        """
        NAME: save
        """
        if self.__mid is None:
            raise ValueError(\
                "save: the requested model might have been released!")
        if os.path.exists(fname):
            raise ValueError(\
                "another model with %s name already exists!" % fname)
        else:
            os.makedirs(fname)
        check_server_state(self.__sid, self.__class__.__name__)
        GLM.save(self.__mid, self.__mkind, self.__mdtype, fname + "/model")
        metadata = open(fname + "/metadata", "wb")
        pickle.dump((self.__mkind, self.__mdtype), metadata)
        metadata.close()

    # calculate the root mean square value on the given test data and labels.
    def score(self, X, y, sample_weight=None):
        """
        NAME: score
        """
        return r2_score(y, self.predict(X), sample_weight=sample_weight)

    # Show the model
    def debug_print(self):
        """
        NAME: debug_print
        """
        if self.__mid is not None:
            check_server_state(self.__sid, self.__class__.__name__)
            GLM.debug_print(self.__mid, self.__mkind, self.__mdtype)

    # Release the model-id to generate new model-id
    def release(self):
        """
        NAME: release
        """
        if self.__mid is not None:
            if FrovedisServer.isUP(self.__sid):
                GLM.release(self.__mid, self.__mkind, self.__mdtype)
        self.__mid = None
        self.__sid = None
        self.n_samples_ = self.n_features_ = None

    # Check FrovedisServer is up then release
    def __del__(self):
        """
        NAME: __del__
        """
        self.release()

# Decision Tree Classifier Class
class DecisionTreeClassifier(BaseEstimator):
    """A python wrapper of Frovedis Decision Tree Classifier
    parameter   		:   default value
    criterion or impurity   	:   'gini'
    splitter    		:   'best'
    min_impurity_decrease	:   0.0
    min_samples_split  		:   2
    min_samples_leaf   		:   1
    min_weight_fraction_leaf 	:   0.0
    presort     		:   deprecated
    verbose      		:   0
    """
    # defaults are as per Frovedis/scikit-learn
    # Decision Tree Classifier constructor
    def __init__(self, criterion='gini', splitter='best',
                 max_depth=None, min_samples_split=2, min_samples_leaf=1,
                 min_weight_fraction_leaf=0.0, max_features=None,
                 random_state=None,
                 max_leaf_nodes=None, min_impurity_decrease=0.0,
                 class_weight=None, presort='deprecated', ccp_alpha=0.0,
                 max_bins=32, verbose=0, categorical_info={}):
        self.criterion = criterion
        self.splitter = splitter
        self.max_depth = max_depth
        self.min_samples_split = min_samples_split
        self.min_samples_leaf = min_samples_leaf
        self.min_weight_fraction_leaf = min_weight_fraction_leaf
        self.max_features = max_features
        self.random_state = random_state
        self.min_impurity_decrease = min_impurity_decrease
        self.class_weight = class_weight
        self.max_leaf_nodes = max_leaf_nodes
        self.presort = presort
        self.ccp_alpha = ccp_alpha
        # extra
        self.__mid = None
        self.__sid = None
        self.__mdtype = None
        self.__mkind = M_KIND.DTM
        self.label_map = None
        self._classes = None
        self.n_classes_ = None
        self.n_features_ = None
        # Frovedis side parameters
        self.max_bins = max_bins
        self.verbose = verbose
        self.categorical_info = categorical_info
        self.algo = "Classification"

    def validate(self):
        """
        NAME: validate
        """
        self.criterion = self.criterion.upper()
        if self.criterion != "GINI" and self.criterion != "ENTROPY":
            raise ValueError("Invalid criterion for Decision Tree Classifier!")

        if self.max_depth is None:
            self.max_depth = 5 # as in Frovedis DecisionTree
        elif self.max_depth < 0:
            raise ValueError("max depth can not be negative !")

        if self.min_impurity_decrease < 0:
            raise ValueError(\
            "Value of min_impurity_decrease should be greater than 0!")

        if self.max_bins < 0:
            raise ValueError("Value of max_bin should be greater than 0!")

        if isinstance(self.min_samples_leaf, numbers.Integral):
            if not 1 <= self.min_samples_leaf:
                raise ValueError("min_samples_leaf must be at least 1 "
                                 "or in (0, 0.5], got %s!"
                                 % self.min_samples_leaf)
        else:  # float
            if not 0. < self.min_samples_leaf <= 0.5:
                raise ValueError("min_samples_leaf must be at least 1 "
                                 "or in (0, 0.5], got %s!"
                                 % self.min_samples_leaf)
            self.min_samples_leaf = \
            int(ceil(self.min_samples_leaf * self.n_samples_))

    # Fit Decision Tree classifier according to X (input data), y (Label)
    def fit(self, X, y):
        """
        NAME: fit
        """
        self.release()
        # for binary case: frovedis supports 0 and 1
        inp_data = FrovedisLabeledPoint(X, y, \
                   caller = "[" + self.__class__.__name__ + "] fit: ",\
                   encode_label = True, binary_encoder=[0, 1], \
                   dense_kind = 'colmajor', densify=True)
        X, y, logic = inp_data.get()
        self._classes = inp_data.get_distinct_labels()
        self.n_classes_ = len(self._classes)
        self.n_samples_ = inp_data.numRows()
        self.n_features_ = inp_data.numCols()
        self.label_map = logic
        dtype = inp_data.get_dtype()
        itype = inp_data.get_itype()
        dense = inp_data.is_dense()
        self.__mdtype = dtype
        self.validate()
        info_keys = np.asarray(list(self.categorical_info.keys()), \
                               dtype=np.int32)
        info_vals = np.asarray(list(self.categorical_info.values()), \
                               dtype=np.int32)
        (host, port) = FrovedisServer.getServerInstance()
        self.__mid = ModelID.get()
        self.__sid = FrovedisServer.getID()
        rpclib.dt_train(host, port, X.get(), y.get(), \
                        self.algo.encode('ascii'), \
                        self.criterion.encode('ascii'),\
                        self.max_depth, self.n_classes_, \
                        self.max_bins, self.min_samples_leaf, \
                        self.min_impurity_decrease, self.verbose, \
                        self.__mid, info_keys, info_vals, len(info_keys), \
                        dtype, itype, dense)
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        return self

    @property
    def classes_(self):
        """classes_ getter"""
        if self.__mid is None:
            raise AttributeError("attribute 'classes_' \
               might have been released or called before fit")
        if self._classes is None:
            self._classes = np.sort(list(self.label_map.values()))
        return self._classes

    @classes_.setter
    def classes_(self, val):
        """classes_ setter"""
        raise AttributeError(\
            "attribute 'classes_' of DecisionTreeClassifier object is not writable")

    # Perform classification on an array of test vectors X.
    def predict(self, X):
        """
        NAME: predict
        """
        if self.__mid is None:
            raise ValueError(\
            "predict is called before calling fit, or the model is released.")
        check_server_state(self.__sid, self.__class__.__name__)
        frov_pred = GLM.predict(X, self.__mid, self.__mkind, \
                           self.__mdtype, False)
        return np.asarray([self.label_map[frov_pred[i]] \
                   for i in range(0, len(frov_pred))])

    # Perform classification on an array and return probability
    # estimates for the test vector X.
    def predict_proba(self, X):
        """
        NAME: predict_proba
        """
        if self.__mid is None:
            raise ValueError("predict_proba is called before calling fit," \
                             " or the model is released!")
        if(self.n_classes_ > 2):
            raise AttributeError("Frovedis DecisionTreeClassifier currently" \
            " does not support predict_proba() for" 
            " multinomial classification!")
        check_server_state(self.__sid, self.__class__.__name__)
        proba = GLM.predict(X, self.__mid, self.__mkind, \
                            self.__mdtype, True, self.n_classes_)
        n_samples = len(proba) // self.n_classes_
        shape = (n_samples, self.n_classes_)
        return np.asarray(proba).reshape(shape)

    # Load the model from a file
    def load(self, fname, dtype=None):
        """
        NAME: load
        """
        if not os.path.exists(fname):
            raise ValueError(\
                "the model with name %s does not exist!" % fname)
        self.release()
        target = open(fname + "/label_map", "rb")
        self.label_map = pickle.load(target)
        target.close()
        self._classes = np.sort(list(self.label_map.values()))
        metadata = open(fname + "/metadata", "rb")
        self.n_classes_, self.__mkind, self.__mdtype = pickle.load(metadata)
        metadata.close()
        if dtype is not None:
            mdt = TypeUtil.to_numpy_dtype(self.__mdtype)
            if dtype != mdt:
                raise ValueError("load: type mismatches detected! " + \
                                 "expected type: " + str(mdt) + \
                                 "; given type: " + str(dtype))
        self.__mid = ModelID.get()
        GLM.load(self.__mid, self.__mkind, self.__mdtype, fname + "/model")
        self.__sid = FrovedisServer.getID()
        return self

    # calculate the mean accuracy on the given test data and labels.
    def score(self, X, y, sample_weight=None):
        """
        NAME: score
        """
        return accuracy_score(y, self.predict(X), sample_weight=sample_weight)

    # Save model to a file
    def save(self, fname):
        """
        NAME: save
        """
        if self.__mid is None:
            raise AttributeError(\
                "save: requested model might have been released!")
        if os.path.exists(fname):
            raise ValueError(\
                "another model with %s name already exists!" % fname)
        else:
            os.makedirs(fname)
        check_server_state(self.__sid, self.__class__.__name__)
        GLM.save(self.__mid, self.__mkind, self.__mdtype, fname + "/model")
        target = open(fname + "/label_map", "wb")
        pickle.dump(self.label_map, target)
        target.close()
        metadata = open(fname + "/metadata", "wb")
        pickle.dump((self.n_classes_, self.__mkind, \
            self.__mdtype), metadata)
        metadata.close()

    # Show the model
    def debug_print(self):
        """
        NAME: debug_print
        """
        if self.__mid is not None:
            check_server_state(self.__sid, self.__class__.__name__)
            GLM.debug_print(self.__mid, self.__mkind, self.__mdtype)

    # Release the model-id to generate new model-id
    def release(self):
        """
        NAME: release
        """
        if self.__mid is not None:
            if FrovedisServer.isUP(self.__sid):
                GLM.release(self.__mid, self.__mkind, self.__mdtype)
        self.__mid = None
        self.__sid = None
        self.label_map = None
        self._classes = None
        self.n_classes_ = self.n_samples_ = self.n_features_ = None

    # Check FrovedisServer is up then release
    def __del__(self):
        """
        NAME: __del__
        """
        self.release()

