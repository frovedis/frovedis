"""
fm.py: wrapper of frovedis Factorization Machine
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

# Factorization Machine Classifier class
class FactorizationMachineClassifier(BaseEstimator):
    """
    A python wrapper of Frovedis Factorization Machine Classifier
    parameter               :    default value
    iteration               :    100
    init_stdev              :    0.1
    init_learn_rate         :    0.01
    optimizer               :    SGD
    global_bias             :    True
    dim_one_interactions    :    True
    dim_factors_no          :    8
    reg_intercept           :    0
    reg_one_interactions    :    0
    reg_factors_no          :    0
    verbose                 :    0
    batch_size_pernode      :    100
    """

    # defaults are as per Frovedis/scikit-learn
    # Factorization Machine Classifier constructor
    def __init__(self, iteration=100, init_stdev=0.1,
                 init_learn_rate=0.01, optimizer="SGD",
                 dim=(True, True, 8),
                 reg=(0, 0, 0), batch_size_pernode=100, verbose=0):
        self.iteration = iteration
        self.init_stdev = init_stdev
        self.init_learn_rate = init_learn_rate
        self.optimizer = optimizer
        self.batch_size_pernode = batch_size_pernode
        self.dim = dim
        self.reg = reg
        self.global_bias = dim[0]           #global bias term
        self.dim_one_interactions = dim[1]  #one-way interactions
        #number of factors that are used for pairwise interactions
        self.dim_factors_no = dim[2]
        #regularization parameters of intercept
        self.reg_intercept = reg[0]
        self.reg_one_interactions = reg[1]  #one-way interactions
        #number of factors that are used for pairwise interactions
        self.reg_factors_no = reg[2]
        self.verbose = verbose
        # extra
        self.__mid = None
        self.__mdtype = None
        self.__mkind = M_KIND.FMM
        self.isregressor = False

    def validate(self):
        """
        NAME: validate
        """
        support = ['SGD', 'SGDA', 'ALS', 'MCMC']
        if self.init_stdev < 0:
            raise ValueError("Initial standard deviation should be positive")
        elif self.init_learn_rate < 0.00001 or self.init_learn_rate > 1.0:
            raise ValueError( \
                "init_learn_rate should be in range [0.00001,1.0]")
        elif self.iteration < 0:
            raise ValueError("iteration should be positive")
        elif self.batch_size_pernode < 0:
            raise ValueError("BatchSize should be positive")
        elif self.reg_intercept < 0 and self.reg_one_interactions < 0 \
                and self.reg_factors_no < 0:
            raise ValueError("Reg values should be positive")
        elif self.dim_factors_no < 1:
            raise ValueError("Factor should be greater than 0")
        elif self.optimizer not in support:
            raise ValueError("Unknown optimizer ", self.optimizer, "!")
        elif (self.global_bias is None) or (self.dim_one_interactions is None) \
                or (self.dim_factors_no is None):
            raise ValueError("Dimension parameters can not be None !")
        elif (self.reg_intercept is None) \
                or (self.reg_one_interactions is None) \
                or (self.reg_factors_no is None):
            raise ValueError("Regularization parameters can not be None")
        elif self.verbose < 0:
            raise ValueError("verbose should be positive")

    # Fit Factorization Machine classifier according to
    #  X (input data), y (Label)
    def fit(self, X, y, sample_weight=None):
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
        if dense:
            raise TypeError("Expected sparse matrix data for training!")
        (host, port) = FrovedisServer.getServerInstance()
        rpclib.fm_train(host, port, X.get(),
                        y.get(), self.init_stdev, self.iteration,
                        self.init_learn_rate, self.optimizer.encode('ascii'),
                        self.global_bias,
                        self.dim_one_interactions, self.dim_factors_no,
                        self.reg_intercept,
                        self.reg_one_interactions, self.reg_factors_no,
                        self.batch_size_pernode,
                        self.__mid, self.isregressor, self.verbose,
                        dtype, itype)
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
            return GLM.predict(X, self.__mid, self.__mkind, \
                self.__mdtype, False)
        else:
            raise ValueError( \
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
            metadata = open(fname+"/metadata", "wb")
            pickle.dump((self.__mkind, self.__mdtype), metadata)
            metadata.close()
        else:
            raise ValueError("save: the requested model might have \
                been released!")

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

# Factorization Machine Regressor class
class FactorizationMachineRegressor(BaseEstimator):
    """
    A python wrapper of Frovedis Factorization Machine Regressor
    parameter               :    default value
    iteration               :    100
    init_stdev              :    0.1
    init_learn_rate         :    0.01
    optimizer               :    SGD
    global_bias             :    True
    dim_one_interactions    :    True
    dim_factors_no          :    8
    reg_intercept           :    0
    reg_one_interactions    :    0
    reg_factors_no          :    0
    verbose                 :    0
    batch_size_pernode      :    100
    """

    # defaults are as per Frovedis/scikit-learn
    # Factorization Machine Regressor constructor
    def __init__(self, iteration=100, init_stdev=0.1,
                 init_learn_rate=0.01, optimizer="SGD", dim=(True, True, 8),
                 reg=(0, 0, 0), batch_size_pernode=100, verbose=0):
        self.iteration = iteration
        self.init_stdev = init_stdev
        self.init_learn_rate = init_learn_rate
        self.optimizer = optimizer
        self.batch_size_pernode = batch_size_pernode
        self.dim = dim
        self.reg = reg
        self.global_bias = dim[0]                       #global bias term
        self.dim_one_interactions = dim[1]              #one-way interactions
        #number of factors that are used for pairwise interactions
        self.dim_factors_no = dim[2]
        #regularization parameters of intercept
        self.reg_intercept = reg[0]
        self.reg_one_interactions = reg[1]              #one-way interactions
        #number of factors that are used for pairwise interactions
        self.reg_factors_no = reg[2]
        self.verbose = verbose
        # extra
        self.__mid = None
        self.__mdtype = None
        self.__mkind = M_KIND.FMM
        self.isregressor = True

    def validate(self):
        """
        NAME: validate
        """
        support = ['SGD', 'SGDA', 'ALS', 'MCMC']
        if self.init_stdev < 0:
            raise ValueError("Initial standard deviation should be positive")
        elif self.init_learn_rate < 0.00001 or self.init_learn_rate > 1.0:
            raise ValueError( \
                "init_learn_rate should be in range [0.00001,1.0]")
        elif self.iteration < 0:
            raise ValueError("iteration should be positive")
        elif self.batch_size_pernode < 0:
            raise ValueError("BatchSize should be positive")
        elif self.reg_intercept < 0 and self.reg_one_interactions < 0 \
                        and self.reg_factors_no < 0:
            raise ValueError("Reg values should be positive")
        elif self.dim_factors_no < 1:
            raise ValueError("Factor should be greater than 0")
        elif self.optimizer not in support:
            raise ValueError("Unknown optimizer ", self.optimizer, "!")
        elif self.global_bias is None or self.dim_one_interactions is None \
                    or self.dim_factors_no is None:
            raise ValueError("Dimension parameters can not be None !")
        elif self.reg_intercept is None or self.reg_one_interactions is None \
                 or self.reg_factors_no is None:
            raise ValueError("Regularization parameters can not be None")
        elif self.verbose < 0:
            raise ValueError("verbose should be positive")

    # Fit Factorization Machine Regressor according to
    #  X (input data), y (Label)
    def fit(self, X, y, sample_weight=None):
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
        if dense:
            raise TypeError("Expected Sparse matrix data for training!")
        (host, port) = FrovedisServer.getServerInstance()
        rpclib.fm_train(host, port, X.get(),
                        y.get(), self.init_stdev, self.iteration,
                        self.init_learn_rate, self.optimizer.encode('ascii'),
                        self.global_bias, self.dim_one_interactions,
                        self.dim_factors_no, self.reg_intercept,
                        self.reg_one_interactions, self.reg_factors_no,
                        self.batch_size_pernode,
                        self.__mid, self.isregressor, self.verbose,
                        dtype, itype)
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        return self

    # Perform regression on an array of test vectors X.
    def predict(self, X):
        """
        NAME: predict
        """
        if self.__mid is not None:
            return GLM.predict(X, self.__mid, self.__mkind, \
                self.__mdtype, False)
        else:
            raise ValueError( \
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
            raise ValueError("save: the requested model might have been \
                released!")

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
