"""
metrics.py
"""

#!/usr/bin/env python

import numpy as np

# simple implementation of r2_score and accuracy_score for the systems
# without scikit-learn installed support
def r2_score(y_true, y_pred,
             sample_weight=None,
             multioutput="uniform_average"):
    """
    NAME: r2_score
    """
    y_true = np.asarray(y_true)
    y_pred = np.asarray(y_pred)
    if len(y_true) == 0:
        return 0.
    if len(y_true) != len(y_pred):
        raise ValueError("input lengths are not matched!")
    tot = numerator = denominator = 0.
    for i in range(0, len(y_true)):
        numerator = numerator + ((y_true[i] - y_pred[i]) ** 2)
        tot = tot + y_true[i]
    avg = tot / len(y_true)
    for i in range(0, len(y_true)):
        denominator = denominator + ((y_true[i] - avg) ** 2)

    if numerator != 0. and denominator != 0.:
        return 1 - (float(numerator) / denominator)
    else:
        return 0.

def accuracy_score(y_true, y_pred, normalize=True, sample_weight=None):
    """
    NAME: accuracy_score
    """
    y_true = np.asarray(y_true)
    y_pred = np.asarray(y_pred)
    if len(y_true) == 0:
        return 0.
    if len(y_true) != len(y_pred):
        raise ValueError("input lengths are not matched!")
    accuracy = 0
    for i in range(0, len(y_true)):
        accuracy = accuracy + (y_true[i] == y_pred[i])
    return float(accuracy) / len(y_true)
