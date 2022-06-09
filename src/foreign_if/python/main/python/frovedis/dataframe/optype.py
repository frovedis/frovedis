"""
Operator types enumerator
"""
#!/usr/bin/env python

class OPT(object):
    """A python container for operator types enumerator"""
    EQ = 1
    NE = 2
    GT = 3
    GE = 4
    LT = 5
    LE = 6
    LIKE = 14
    # date
    GETYEAR = 201
    GETMONTH = 202
    GETDAYOFMONTH = 203
    GETHOUR = 204
    GETMINUTE = 205
    GETSECOND = 206
    GETQUARTER = 207
    GETDAYOFWEEK = 208
    GETDAYOFYEAR = 209
    GETWEEKOFYEAR = 210
    GETNANOSECOND = 225
