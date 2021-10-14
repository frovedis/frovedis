"""
fpm.py
"""

from collections import Iterable
import pickle
import numpy as np
import pandas as pd
import frovedis.dataframe as fpd
from os.path import exists as file_exists
from ..exrpc import rpclib
from ..exrpc.server import FrovedisServer, set_association, \
                           check_association, do_if_active_association
from ..matrix.dtype import DTYPE
from .model_util import *

def merge_and_append_fis(append_buf, merge_target, encode_logic):
    """Merge fis"""
    m_lst = []
    for each in merge_target.values.tolist():
        items = each[:-1]
        if encode_logic is not None:
            items = [encode_logic[i] for i in items]
        freq = each[-1]
        m_lst.append([items, freq])
    m_df = pd.DataFrame(m_lst, columns=['items', 'freq'])
    return append_buf.append(m_df)

def merge_and_append_rules(append_buf, merge_target, encode_logic):
    """Merge rules"""
    m_lst = []
    for each in merge_target.values.tolist():
        antecedents = each[:-5]
        consequent = [each[-5]]
        if encode_logic is not None:
            antecedents = [encode_logic[i] for i in antecedents]
            consequent = [encode_logic[i] for i in consequent]
        confidence = each[-4]
        lift = each[-3]
        support = each[-2]
        conviction = each[-1]
        m_lst.append([antecedents, consequent, confidence,
                      lift, support, conviction])
    m_df = pd.DataFrame(m_lst, columns=['antecedent', 'consequent', \
                                        'confidence', 'lift', 'support', \
                                        'conviction'])
    return append_buf.append(m_df)

def adjust_transform_output(transform_df, data):
    """Adjust transform output"""
    df = transform_df.groupby("trans_id").agg(lambda x:[i for i in x])
    data1 = data.groupby("trans_id").agg(lambda x:[i for i in x])
    df1 = data1.join(df).reset_index(drop=True)
    df1.columns = ['items', 'prediction']
    df1["prediction"] = df1["prediction"].apply(lambda d: d if isinstance(d, list) else [])
    return df1

class FPGrowth(object):
    """A python wrapper of Frovedis FP-Growth"""
    def __init__(self, minSupport=0.3, minConfidence=0.8,
                 itemsCol='items', predictionCol='prediction', #not used in frovedis
                 numPartitions=None, # not used in frovedis
                 tree_depth=None, #added parameter to limit tree construction till a certain depth
                 compression_point=4, mem_opt_level=0, # for memory optimizations
                 verbose=0,
                 encode_string_input=False): #whether to encode non-numeric items
        self.minSupport = minSupport
        self.minConfidence = minConfidence
        self.itemsCol = itemsCol
        self.predictionCol = predictionCol
        self.numPartitions = numPartitions
        # --- added parameters in frovedis ---
        self.tree_depth = tree_depth
        self.compression_point = compression_point
        self.mem_opt_level = mem_opt_level
        self.verbose = verbose
        self.encode_string_input = encode_string_input
        self.__mkind = M_KIND.FPM
        # --- post training available attributes ---
        self.__mid = None
        self.__fis = None
        self.__rule = None
        self.count = None
        self.encode_logic = None

    def __encode_input(self, data):
        """Encode input"""
        import itertools
        unq = np.unique(list(itertools.chain.from_iterable(data)))
        int_id = np.arange(1, len(unq) + 1, 1, dtype=np.int32)
        transmap = dict(zip(unq, int_id))
        ret = []
        for e in data:
            enc = [transmap[i] for i in e]
            ret.append(enc)
        self.encode_logic = dict(zip(int_id, unq))
        return ret

    def __convert_iterable(self, data):
        """
        converts an iterable to frovedis DataFrame
        """
        tid = []
        item = []
        cur_id = 1
        #print("### iterable data: ")
        #print(data)
        if self.encode_string_input:
            data = self.__encode_input(data)
            #print("### encoded iterable data: ")
            #print(data)
        for trans in data: # for each transaction in data
            for trans_it in trans: # for each item in each transaction
                tid.append(cur_id)
                item.append(trans_it)
            cur_id = cur_id + 1
        df_t = pd.DataFrame({'trans_id': tid, 'item': item}, \
                            columns=['trans_id', 'item'])
        #print("### converted pandas dataframe: ")
        #print(df_t)
        return fpd.DataFrame(df_t)

    def __convert_pandas_df(self, data):
        """
        converts a pandas DataFrame (having multiple columns containing items)
        to frovedis DataFrame
        """
        item_list = []
        if len(data.columns) > 2:
            for ilist in data.values.tolist():
                item = [itm for itm in ilist if str(itm) != 'nan']
                item_list.append(item)
        elif len(data.columns) == 2:
            item_list = data[data.columns[1]].tolist()
        else: #unsupported pandas format
            raise ValueError("fit: unsupported pandas dataframe received!\n")
        return self.__convert_iterable(item_list)

    def __check_input(self, data):
        """Check input"""
        if self.minSupport < 0 or self.minSupport > 1:
            raise ValueError("fit: minSupport value must be within 0 to 1.\n")
        if self.minConfidence < 0 or self.minConfidence > 1:
            raise ValueError("fit: minConfidence value must be within 0 to 1.\n")
        if self.tree_depth is None:
            self.tree_depth = np.iinfo(np.int32).max
        elif self.tree_depth < 1:
            raise ValueError("fit: tree_depth value must be >= 1.\n")
        if self.compression_point < 2:
            raise ValueError("fit: compression_point value must be >= 2.\n")
        if self.mem_opt_level != 0 and self.mem_opt_level != 1:
            raise ValueError("fit: mem_opt_level value must be 0 or 1.\n")
        if isinstance(data, pd.DataFrame):
            f_df = self.__convert_pandas_df(data)
        elif isinstance(data, fpd.DataFrame):
            f_df = data
        elif isinstance(data, Iterable):
            f_df = self.__convert_iterable(data)
        else:
            raise ValueError("fit: only dataframe and iterable " +
                             "inputs are supported!\n")
        return f_df

    @set_association
    def fit(self, data):
        """
        NAME: fit
        """
        self.release()
        f_df = self.__check_input(data)
        '''
        #
        # debug code to confirm if encoding non-numeric data is correct
        # 
        p_df = f_df.to_pandas_dataframe()
        if self.encode_logic is not None:
            p_df['rowid'] = np.arange(len(p_df))
            logic = pd.DataFrame()
            logic[["item_name", "item_id"]] = \
                pd.DataFrame.from_dict(self.encode_logic, orient='index')\
                            .reset_index()
            ret = p_df.merge(logic, left_on='item', right_on='item_id')\
                      .sort('rowid')[['trans_id', 'item_name']]
            ret.to_csv("dump.csv", header=False, index=False)
        else:
            p_df.to_csv("dump.csv", header=False, index=False)
        ''' 
        (host, port) = FrovedisServer.getServerInstance()
        self.__mid = ModelID.get()
        self.count = rpclib.fpgrowth_generate_fis(host, port, \
            f_df.get(), self.__mid, \
            self.minSupport, self.tree_depth, self.compression_point, \
            self.mem_opt_level, self.verbose)
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        return self

    @check_association
    def generate_rules(self, confidence=None):
        """
        NAME: generate_rules
        """
        if self.__rule is None:
            rule_mid = ModelID.get()
            if confidence is None:  # to use confidence provided during init()
                confidence = self.minConfidence
            (host, port) = FrovedisServer.getServerInstance()
            rule_count = rpclib.fpgrowth_generate_rules(host, port, \
                                              self.__mid, rule_mid, confidence)
            excpt = rpclib.check_server_exception()
            if excpt["status"]:
                raise RuntimeError(excpt["info"])
            self.__rule = FPRules(rule_mid, rule_count, self.encode_logic)
        return self.__rule

    @property
    @check_association
    def freqItemsets(self):
        """freqItemsets getter"""
        if self.__fis is None:
            (host, port) = FrovedisServer.getServerInstance()
            ldf = rpclib.get_fpgrowth_fis(host, port, self.__mid)
            excpt = rpclib.check_server_exception()
            if excpt["status"]:
                raise RuntimeError(excpt["info"])
            fis = pd.DataFrame()
            for each in ldf:
                res = fpd.DataFrame().load_dummy(each["dfptr"], \
                                       each["names"], each["types"])
                fis = merge_and_append_fis(fis, res.to_pandas_dataframe(), \
                                           self.encode_logic)
            self.__fis = fis.reset_index(drop=True)
        return self.__fis

    @freqItemsets.setter
    def freqItemsets(self, val):
        """freqItemsets setter"""
        raise AttributeError(\
            "attribute 'freqItemsets' of FPGrowth object is not writable")

    @property
    @check_association
    def associationRules(self):
        """associationRules getter"""
        return self.generate_rules().get_association_rules()

    @associationRules.setter
    def associationRules(self, val):
        """associationRules setter"""
        raise AttributeError(\
            "attribute 'associationRules' of FPGrowth object is not writable")
    
    @check_association
    def transform(self, data):
        """transform"""
        f_df = self.__check_input(data)
        self.generate_rules() # uses confidence provided during init()
        (host, port) = FrovedisServer.getServerInstance()
        pred = rpclib.fp_transform(host, port, f_df.get(), self.__mid)
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        res = fpd.DataFrame().load_dummy(pred["dfptr"], \
                                   pred["names"], pred["types"])
        if len(res) >  0:
            pred = adjust_transform_output(
                                    res.to_pandas_dataframe(), 
                                    f_df.to_pandas_dataframe())
        else:
            res = pd.DataFrame({'trans_id':[], 'consequent':[]})
            pred = adjust_transform_output(
                                    res, 
                                    f_df.to_pandas_dataframe())
        return pred

    @set_association
    def load(self, fname):
        """
        NAME: load
        """
        self.release()
        (host, port) = FrovedisServer.getServerInstance()
        self.__mid = ModelID.get()
        self.count = rpclib.load_fp_model(host, port, self.__mid, \
                         self.__mkind, fname.encode('ascii'))
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        if file_exists(fname + "/encode_logic"):
            target = open(fname + "/encode_logic", "rb")
            self.encode_logic = pickle.load(target)
            target.close()
        return self

    @check_association
    def save(self, fname):
        """
        NAME: save
        """
        GLM.save(self.__mid, self.__mkind, DTYPE.DOUBLE, fname)
        if self.encode_logic:
            target = open(fname + "/encode_logic", "wb")
            pickle.dump(self.encode_logic, target)
            target.close()

    @check_association
    def debug_print(self):
        """
        NAME: debug_print
        """
        GLM.debug_print(self.__mid, self.__mkind, DTYPE.DOUBLE)

    def release(self):
        """
        resets after-fit populated attributes to None
        """
        self.__release_server_heap()
        self.__mid = None
        self.__fis = None
        self.__rule = None
        self.count = None
        self.encode_logic = None

    @do_if_active_association
    def __release_server_heap(self):
        """
        to release model pointer from server heap
        """
        GLM.release(self.__mid, self.__mkind, DTYPE.DOUBLE)

    def __del__(self):
        """
        NAME: __del__
        """
        self.release()

    def is_fitted(self):
        """ function to confirm if the model is already fitted """
        return self.__mid is not None

class FPRules(object):
    """
    FPRules
    """
    @set_association
    def __init__(self, mid=None, count=None, encode_logic=None):
        self.__mid = mid
        self.count = count
        self.encode_logic = encode_logic
        self.__mkind = M_KIND.FPR
        self.__rule = None

    @set_association
    def load(self, fname):
        """
        NAME: load
        """
        self.release()
        self.__mid = ModelID.get()
        (host, port) = FrovedisServer.getServerInstance()
        self.count = rpclib.load_fp_model(host, port, self.__mid, \
                              self.__mkind, fname.encode('ascii'))
        excpt = rpclib.check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        if file_exists(fname + "/encode_logic"):
            target = open(fname + "/encode_logic", "rb")
            self.encode_logic = pickle.load(target)
            target.close()
        return self

    @check_association
    def save(self, fname):
        """
        NAME: save
        """
        GLM.save(self.__mid, self.__mkind, DTYPE.DOUBLE, fname)
        if self.encode_logic:
            target = open(fname + "/encode_logic", "wb")
            pickle.dump(self.encode_logic, target)
            target.close()

    @check_association
    def get_association_rules(self):
        """associationRules getter helper"""
        if self.__rule is None:
            (host, port) = FrovedisServer.getServerInstance()
            ldf = rpclib.get_association_rules(host, port, self.__mid)
            excpt = rpclib.check_server_exception()
            if excpt["status"]:
                raise RuntimeError(excpt["info"])
            rules = pd.DataFrame()
            for each in ldf:
                res = fpd.DataFrame().load_dummy(each["dfptr"], \
                                       each["names"], each["types"])
                rules = merge_and_append_rules(rules, \
                        res.to_pandas_dataframe(), self.encode_logic)
            self.__rule = rules.reset_index(drop=True)
        return self.__rule

    @check_association
    def debug_print(self):
        """
        NAME: debug_print
        """
        GLM.debug_print(self.__mid, self.__mkind, DTYPE.DOUBLE)

    def release(self):
        """
        resets after-fit populated attributes to None
        """
        self.__release_server_heap()
        self.__mid = None
        self.__rule = None
        self.count = None
        self.encode_logic = None

    @do_if_active_association
    def __release_server_heap(self):
        """
        to release model pointer from server heap
        """
        GLM.release(self.__mid, self.__mkind, DTYPE.DOUBLE)

    def __del__(self):
        """
        NAME: __del__
        """
        self.release()

    def is_fitted(self):
        """ function to confirm if the model is already fitted """
        return self.__mid is not None

