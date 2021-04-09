"""
metrics.py
"""

#!/usr/bin/env python

import warnings
import numbers
import numpy as np

# simple implementation of r2_score and accuracy_score for the systems
# without scikit-learn installed support
def check_targets(y_true, y_pred, sample_weight=None):
    y_true = np.asarray(y_true)
    y_pred = np.asarray(y_pred)
    if len(y_true) != len(y_pred):
        raise ValueError("input lengths are not matched!")
    if sample_weight is not None:
        if isinstance(sample_weight, numbers.Number):
            sample_weight = np.full(len(y_true), sample_weight, dtype=np.float64)
        else: # array-like
            sample_weight = np.asarray(sample_weight)
        if len(y_true) != len(sample_weight):
            raise ValueError(\
            "sample_weight length is different than input labels!")
    return y_true, y_pred, sample_weight

def weighted_sum(score, sample_weight, normalize=False):
    if normalize:
        return np.average(score, weights=sample_weight)
    elif sample_weight is not None:
        return np.dot(sample_score, sample_weight)
    else:
        return sample_score.sum()

def accuracy_score(y_true, y_pred, normalize=True, sample_weight=None):
    """
    NAME: accuracy_score
    """
    if len(y_true) == 0:
        return 0.
    y_true, y_pred, sample_weight = \
    check_targets(y_true, y_pred, sample_weight)
    score = (y_true == y_pred)
    return weighted_sum(score, sample_weight, normalize)

def r2_score(y_true, y_pred,
             sample_weight=None,
             multioutput="uniform_average"):
    """
    NAME: r2_score
    """
    y_true, y_pred, sample_weight = \
    check_targets(y_true, y_pred, sample_weight)

    if len(y_true) < 2:
        msg = "R^2 score is not well-defined with less than two samples."
        warnings.warn(msg, RuntimeWarning)
        return float('nan')

    weight = 1. if sample_weight is None else np.asarray(sample_weight)
    numerator = (weight * (y_true - y_pred) ** 2).sum(axis=0, \
                                                      dtype=np.float64)
    denominator = (weight * (y_true - np.average( \
        y_true, axis=0, weights=sample_weight)) ** 2).sum(axis=0, \
                                                          dtype=np.float64)
   
    if denominator == 0.0: # to avoid inf
        return 0.0
    else:
        return 1.0 - (float(numerator) / denominator)
