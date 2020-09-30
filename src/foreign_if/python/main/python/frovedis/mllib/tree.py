"""
tree.py: wrapper of frovedis decistion tree (classifier and regressor)
"""

#!/usr/bin/env python
import os.path
import pickle
from .model_util import *
from ..base import *
from ..exrpc import rpclib
from ..exrpc.server import FrovedisServer
from ..matrix.ml_data import FrovedisLabeledPoint
from ..matrix.dtype import TypeUtil
from .metrics import *
import numpy as np

# Decision Tree Regressor Class
class DecisionTreeRegressor(BaseEstimator):
    """A python wrapper of Frovedis Decision Tree Regressor
    parameter                   :   default value
    criterion or impurity       :   'mse'
    splitter    		:   'best'
    in_impurity_decrease	:   0.0
    in_samples_split  		:   2
    min_samples_leaf   		:   1
    in_weight_fraction_leaf 	:   0.0
    presort     		:   False
    verbose      		:   0
    """

    # defaults are as per Frovedis/scikit-learn
    # Decision Tree Regressor constructor
    def __init__(self, criterion='mse', splitter='best',
                 max_depth=None, min_samples_split=2, min_samples_leaf=1,
                 min_weight_fraction_leaf=0.0, max_features=None,
                 random_state=None,
                 max_leaf_nodes=1, min_impurity_decrease=0.0,
                 min_impurity_split=None,
                 class_weight=None, presort=False,
                 min_info_gain=0.0, max_bins=32, verbose=0):
        self.splitter = splitter
        self.max_depth = max_depth
        self.criterion = criterion
        self.min_samples_split = min_samples_split
        self.min_samples_leaf = min_samples_leaf
        self.min_weight_fraction_leaf = min_weight_fraction_leaf
        self.max_features = max_features
        self.random_state = random_state
        self.max_leaf_nodes = max_leaf_nodes
        self.min_impurity_decrease = min_impurity_decrease
        self.min_impurity_split = min_impurity_split
        self.class_weight = class_weight
        self.presort = presort
        self.verbose = verbose
        # extra
        self.__mid = None
        self.__mdtype = None
        self.__mkind = M_KIND.DTM
        # Frovedis side parameters
        self.min_info_gain = min_info_gain
        self.max_bins = max_bins
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
            raise ValueError("max depth can not be negative !")

        if self.min_info_gain < 0:
            raise ValueError("Value of min_info_gain should be greater than 0")
        if self.max_bins < 0:
            raise ValueError("Value of max_bin should be greater than 0")
        if self.n_classes_ < 0:
            raise ValueError(\
            "Value of number of classes should be +ve integer or zero!")
        if self.min_samples_leaf < 0:
            raise ValueError(\
            "Value of min_samples_leaf should be greater than 0!")

    # Fit Decision Tree Regressor according to X (input data), y (Label)
    def fit(self, X, y):
        """
        NAME: fit
        """
        self.validate()
        self.release()
        self.__mid = ModelID.get()
        inp_data = FrovedisLabeledPoint(X, y)
        (X, y) = inp_data.get()
        dtype = inp_data.get_dtype()
        itype = inp_data.get_itype()
        dense = inp_data.is_dense()
        self.__mdtype = dtype
        (host, port) = FrovedisServer.getServerInstance()
        rpclib.dt_train(host, port, X.get(), y.get(), \
                        self.algo.encode('ascii'), \
                        self.criterion.encode('ascii'),\
                        self.max_depth, self.n_classes_, \
                        self.max_bins, self.min_samples_leaf, \
                        self.min_info_gain, self.verbose, self.__mid, \
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
        if self.__mid is not None:
            return GLM.predict(X, self.__mid, self.__mkind,\
                               self.__mdtype, False)
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
            raise ValueError(\
                "save: the requested model might have been released!")

    # calculate the root mean square value on the given test data and labels.
    def score(self, X, y):
        """
        NAME: score
        """
        if self.__mid is not None:
            return r2_score(y, self.predict(X))

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

    # Check FrovedisServer is up then release
    def __del__(self):
        """
        NAME: __del__
        """
        if FrovedisServer.isUP():
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
    presort     		:   False
    verbose      		:   0
    """
    # defaults are as per Frovedis/scikit-learn
    # Decision Tree Classifier constructor
    def __init__(self, criterion='gini', splitter='best',
                 max_depth=None, min_samples_split=2, min_samples_leaf=1,
                 min_weight_fraction_leaf=0.0, max_features=None,
                 random_state=None,
                 max_leaf_nodes=1, min_impurity_decrease=0.0,
                 min_impurity_split=None,
                 class_weight=None, presort=False, min_info_gain=0.0,
                 max_bins=32, verbose=0):
        self.criterion = criterion
        self.splitter = splitter
        self.max_depth = max_depth
        self.min_samples_split = min_samples_split
        self.min_samples_leaf = min_samples_leaf
        self.min_weight_fraction_leaf = min_weight_fraction_leaf
        self.max_features = max_features
        self.random_state = random_state
        self.min_impurity_decrease = min_impurity_decrease
        self.min_impurity_split = min_impurity_split
        self.class_weight = class_weight
        self.max_leaf_nodes = max_leaf_nodes
        self.presort = presort
        self.verbose = verbose
        # extra
        self.__mid = None
        self.__mdtype = None
        self.__mkind = M_KIND.DTM
        self.label_map = None
        self.n_classes_ = None
        # Frovedis side parameters
        self.min_info_gain = min_info_gain
        self.max_bins = max_bins
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

        if self.min_info_gain < 0:
            raise ValueError(\
            "Value of min_info_gain should be greater than 0")
        if self.max_bins < 0:
            raise ValueError("Value of max_bin should be greater than 0")
        if self.min_samples_leaf < 0:
            raise ValueError(\
            "Value of min_samples_leaf should be greater than 0!")
        if self.n_classes_ < 0:
            raise ValueError(\
            "Value of number of classes should be +ve integer or zero!")

    # Fit Decision Tree classifier according to X (input data), y (Label)
    def fit(self, X, y):
        """
        NAME: fit
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
        self.n_classes_ = unique_labels.size
        self.__mdtype = dtype
        # validate hyper-parameters
        self.validate()
        if self.n_classes_ > 2:
            encoded_y, logic = y.encode(need_logic=True)
        elif self.n_classes_ == 2:
            target = [0, 1]
            encoded_y, logic = y.encode(unique_labels, target, need_logic=True)
        else:
            raise ValueError(\
                "fit: number of unique labels in y are less than 2")
        self.label_map = logic
        (host, port) = FrovedisServer.getServerInstance()
        rpclib.dt_train(host, port, X.get(), encoded_y.get(), \
                        self.algo.encode('ascii'), \
                        self.criterion.encode('ascii'),\
                        self.max_depth, self.n_classes_, \
                        self.max_bins, self.min_samples_leaf, \
                        self.min_info_gain, self.verbose, self.__mid,\
                        dtype, itype, dense)
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
            new_pred = \
            [self.label_map[frov_pred[i]] for i in range(0, len(frov_pred))]
            return np.asarray(new_pred, dtype=np.int64)
        else:
            raise ValueError(\
            "predict is called before calling fit, or the model is released.")

    # Perform classification on an array and return probability
    # estimates for the test vector X.
    def predict_proba(self, X):
        """
        NAME: predict_proba
        """
        if self.__mid is not None:
            if(self.n_classes_ > 2):
                raise AttributeError("Frovedis DecisionTreeClassifier doesn't support" \
                                     + " predict_proba() for multinomial classification!")
            pred = GLM.predict(X, self.__mid, self.__mkind, \
                               self.__mdtype, True)
            # TODO: Perform (1.0 - pred) from within frovedis library
            return np.asarray(pred).reshape((len(pred), 1)) 
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
        target = open(fname+"/label_map", "rb")
        self.label_map = pickle.load(target)
        target.close()
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

    # Check FrovedisServer is up then release
    def __del__(self):
        """
        NAME: __del__
        """
        if FrovedisServer.isUP():
            self.release()

