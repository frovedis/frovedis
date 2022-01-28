% FPGrowth

# NAME

FPGrowth - A frequent pattern mining algorithm supported by Frovedis.  

# SYNOPSIS

    class frovedis.mllib.fpm.FPGrowth(minSupport=0.3, minConfidence=0.8, itemsCol='items',  
                                      predictionCol='prediction', numPartitions=None,  
                                      tree_depth=None, compression_point=4, mem_opt_level=0,  
                                      verbose=0, encode_string_input=False)  

## Public Member Functions

fit(data)  
generate_rules(confidence = None)  
transform(data)  
load(fname)  
save(fname)  
debug_print()  
release()  
is_fitted()  

# DESCRIPTION
FPGrowth is an algorithm for discovering frequent itemsets in a transaction database. 
The input of FPGrowth is a set of transactions called transaction database. Each transaction 
is a set of items. **Frovedis supports numeric and non-numeric values for transaction data.**  

For example, consider the following transaction database. It contains 4 
transactions (t1, t2, t3, t4) and 4 items (1, 2, 3, 4). The first transaction represents 
the set of items 1, 2 , 3 and 4.  

    Transaction id      Items  
    t1                  {1, 2, 3, 4}  
    t2                  {1, 2, 3}  
    t3                  {1, 2}  
    t4                  {1}  

It is important to note that repetition of item in any given transaction needs to be avoided. It 
would raise exception in that case.  

Now, if FPGrowth is run on the above transaction database with a minSupport of 40% and a 
tree_depth of 5 levels,  

FPGrowth produces the following result:  

    items             freq
    [1]               4
    [2]               3
    [3]               2
    [2, 1]            3
    [3, 1]            2
    [3, 2]            2
    [3, 2, 1]         2

In the results, each itemset is annotated with its corresponding frequency.   

This module provides a client-server implementation, where the client application 
is a normal python program. In this implementation, a python client can interact 
with a frovedis server by sending the required python data for training at frovedis 
side. Python data is converted into frovedis compatible data internally and the 
python ML call is linked with the respective frovedis ML call to get the job done 
at frovedis server. 

Python side calls for FPGrowth on the frovedis server. Once the training is completed 
with the input data at the frovedis server, it returns an abstract model with a 
unique model ID to the client python program.  

When prediction-like request would be made on the trained model, python program 
will send the same request to the frovedis server. After the request is served at 
the frovedis server, the output would be sent back to the python client.  

## Detailed Description

### 1. FPGrowth()

__Parameters__  
**_minSupport_**: A positive double (float64) type value that specifies the minimum 
support level of frequent itemsets. Its value must be within 0 to 1. (Default: 0.3)  
**_minConfidence_**: A positive double (float64) type value that specifies the minimal 
confidence for generating association rules. It will not affect the mining for frequent 
itemsets, but will affect the association rules generation. Its value must be 
within 0 to 1. (Default: 0.8)  
**_itemsCol_**: An unsed parameter. (Default: 'items')  
**_predictionCol_**: An unsed parameter. (Default: 'prediction')  
**_numPartitions_**: An unsed parameter. (Default: None)  
**_tree\_depth_**: A positive integer parameter specifying the maximum number of levels 
for tree construction. Its value must be greater than 1. (Default: None)  
When it is None (not specified explicitly), the tree is constructed to its maximum 
depth according to the data. Since transaction databases tend to be very large, there may be a 
scenario wherein entire FP tree cannot be contained in memory. In those cases, the size of 
FP tree may be limited by using this parameter.  
**_compression\_point_**: A positive integer parameter. This is an internal memory optimisation 
strategy which helps when working with large transaction databases. Its value must be greater 
than or equal to 2. No compression will be performed till the level specified by this parameter 
is reached. (Default: 4)  
**_mem\_opt\_level_**: An integer value which must be either 0 (memory optimisation OFF) or 
1 (memory optimisation ON). If switched On, it will lead to removal of redundant tree data residing 
in memory at server side. It should only be used where systems have memory constraints. 
By default, it is 0, but in case of memory constraints, execution should be attempted 
keeping this value as 1, it will help in reducing memory footprint. However, when it is 1, it 
might still cause memory issue in case data is too big. In this case, data may be spilled onto disk 
if this environment variable has been set. This will degrade performance (execution time).  
**_verbose_**: An integer parameter specifying the log level to use. Its value is 0 by 
default (INFO level). But it can be set to 1 (DEBUG level) or 2 (TRACE level) for getting 
training time logs from frovedis server.  
**_encode\_string\_input_**: A boolean parameter when set to True, encodes the non-numeric 
(like strings) itemset values. It first internally encodes the named-items to an encoded 
numbered-items and the encoded dataframe is used for further training at frovedis server.
It helps to train the encoded fpgrowth model faster with non-numeric itemsets since data present 
with server is two column dataframe, operations like join(), etc are little on slower on non-numeric 
columns than the numeric columns. (Default: False)  

For example,  

    # let the data be some non-numeric transaction database 
    data = [['banana','apple','mango','cake'],
            ['cake','banana','apple'],
            ['bread','banana'],
            ['banana']]
    # Using FPGrowth object with memory optimization parameters for training 
    # Here, disabling the parameter encode_string_input = False by default
    from frovedis.mllib.fpm import FPGrowth
    fpm = FPGrowth(minSupport = 0.01, minConfidence = 0.5, compression_point = 4, 
                   mem_opt_level = 1) 
    fpm.fit(data)

**Frequent itemsets generation time: 0.0361 sec**  

And, when enabling 'encode_string_input' with the same non-numeric data,  

    # Using FPGrowth object with memory optimization parameters for training 
    # Here, parameter encode_string_input = True 
    from frovedis.mllib.fpm import FPGrowth
    fpm = FPGrowth(minSupport = 0.01, minConfidence = 0.5, compression_point = 4, 
                   mem_opt_level = 1, encode_string_input = True) 
    fpm.fit(data)

**Frequent itemsets generation time: 0.0315 sec**  

__Attributes__  
**_freqItemsets_**: A pandas dataframe having two fields, 'items' and 'freq', where 'items' is
an array whereas 'freq' is double (float64) type value. It contains the itemsets along with their 
frequency values. Here, the frequency of an itemset signifies as to how many times the itemset 
appears in the transaction database.  
**_associationRules_**: A pandas dataframe having six fields, 'antecedent', 'consequent', 
'confidence', 'lift', 'support' and 'conviction'.  
Every association rule is composed of two parts: an antecedent (if) and a consequent (then).  
An 'antecedent' is an item found within the data. A 'consequent' is an item found in combination with 
the 'antecedent'. Both are itemsets (arrays).  
For measuring the effectiveness of association rule, 'confidence', 'lift', 'support' and 'conviction' 
are used. All are double (float64) type values.  
'confidence' refers to the amount of times a given rule turns out to be true in practice.  
'support' is an indication of how frequently the itemset appears in the dataset.  
'lift' is the ratio of confidence to support. If the rule has a lift of 1, it would imply that 
the probability of occurrence of the 'antecedent' and that of the 'consequent' are independent 
of each other. When two events are independent of each other, no rule can be drawn involving those 
two events. If the lift is greater than 1, that lets us know the degree to which those two occurrences 
are dependent on one another, and makes those rules potentially useful for predicting the consequent 
in future data sets. If the lift is less than 1, that lets us know the items are substitute to each 
other. This means that presence of one item has negative effect on presence of other item and vice versa.  
'conviction' compares the probability that X appears without Y if they were dependent with the 
actual frequency of the appearance of X without Y. If it equals 1, then they are completely unrelated.  

**_count_**: A positive integer value which specifies the frequent itemsets count.  
**_encode\_logic_**: A python dictionary having transaction items with a corresponding encoded 
number as key-value pairs. It is only available when transaction items are have string values 
and 'encode_string_input' parameter is set to True. Otherwise it is None. This is used internally to 
perform auto decoding of items during 'freqItemsets' construction.  

For example,
    
    # let data be some non-numeric transaction database 
    data = [['banana','apple','mango','cake'],
            ['cake','banana','apple'],
            ['bread','banana'],
            ['banana']]
    
    # creating a pandas dataframe
    import pandas as pd
    dataDF = pd.DataFrame(data)
    
    # Using FPGrowth object with memory optimization parameters and enabling encoding on non-numeric 
    # inputs and training 
    from frovedis.mllib.fpm import FPGrowth
    fpm = FPGrowth(minSupport = 0.01, minConfidence = 0.5, compression_point = 4, 
                   mem_opt_level = 1, encode_string_input = True) 
    fpm.fit(data)
    print("logic: ", fpm.encode_logic)

Output, 
    
    logic:  {1: 'apple', 2: 'banana', 3: 'bread', 4: 'cake', 5: 'mango'}

__Purpose__  
It initializes an FPGrowth object with the given parameters.  

The parameters: "itemsCol", "predictionCol" and "numPartitions" are simply kept in to 
make the interface uniform to the PySpark FPGrowth module. They are not used anywhere 
within the frovedis implementation.  

__Return Value__  
It simply returns "self" reference.  

### 2. fit(data)  
__Parameters__  
**_data_**: A python iterable or a pandas dataframe (manually constructed or loaded from file) 
or frovedis-two column dataframe containing the transaction data. Frovedis supports numeric and 
non-numeric values in transaction dataset.  

__Purpose__  
It accepts the training data and trains the fpgrowth model with specified minimum support 
value and tree depth value for construction of tree.  

For pandas dataframe, if it has 2 columns, the second column would be treated as items and needs 
to be an array-like input.  

For example,  

    # creating a pandas dataframe 
    import pandas as pd
    data = [['banana','apple','mango','cake'], 
            ['cake','banana','apple'], 
            ['bread','banana'], 
            ['banana']]
    dataDF = pd.DataFrame(data)
    
    # fitting input dataframe on FPGrowth object  
    fpm = FPGrowth(minSupport = 0.01, minConfidence = 0.5, compression_point = 4, 
                   mem_opt_level = 1).fit(dataDF)   
    # Now, to print the frequent itemsets table after training is completed
    print(fpm.freqItemsets)   
    # to print table with all the association rules 
    print(fpm.associationRules)
  
Output,

    frequent itemsets:
                               items  freq
    0                       [banana]   4.0
    1                        [apple]   2.0
    2                         [cake]   2.0
    3                        [bread]   1.0
    4                        [mango]   1.0
    5                  [cake, apple]   2.0
    6                 [mango, apple]   1.0
    7                [apple, banana]   2.0
    8                [bread, banana]   1.0
    9                 [cake, banana]   2.0
    10               [mango, banana]   1.0
    ...
    16  [mango, cake, apple, banana]   1.0
    association rules:
                    antecedent consequent  confidence  lift  support  conviction
    0                 [banana]    [apple]         0.5   1.0     0.50         1.0
    1                  [apple]     [cake]         1.0   2.0     0.50         NaN
    2                 [banana]     [cake]         0.5   1.0     0.50         1.0
    3                  [apple]    [mango]         0.5   2.0     0.25         1.5
    4                   [cake]    [mango]         0.5   2.0     0.25         1.5
    5                  [apple]   [banana]         1.0   1.0     0.50         NaN
    6                  [bread]   [banana]         1.0   1.0     0.25         NaN
    7                   [cake]    [apple]         1.0   2.0     0.50         NaN
    8                   [cake]   [banana]         1.0   1.0     0.50         NaN
    9                  [mango]    [apple]         1.0   2.0     0.25         NaN
    10                 [mango]   [banana]         1.0   1.0     0.25         NaN
    ...
    27    [mango, cake, apple]   [banana]         1.0   1.0     0.25         NaN

If it has more than 2 columns, all columns would be treated as individual 
item and missing items in any given transaction needs to be NaN.  

For example,  

FILE: groceries.csv

Item(s),Item 1,Item 2,Item 3,Item 4,Item 5,Item 6,Item 7, .... , Item 30,Item 31,Item 32  
4,citrus fruit,semi-finished bread,margarine,ready soups,,,,,,,,,,,,,,,,,,,,,,,,,,,,  
3,tropical fruit,yogurt,coffee,,,,,,,,,,,,,,,,,,,,,,,,,,,,,  
1,whole milk,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,  
4,pip fruit,yogurt,cream cheese,meat spreads,,,,,,,,,,,,,,,,,,,,,,,,,,,,  
...  
10,chicken,citrus fruit,other vegetables,butter,yogurt, ..., cling film/bags,,,,,,,,,,,,,,,,,,,,,,  
4,semi-finished bread,bottled water,soda,bottled beer,,,,,,,,,,,,,,,,,,,,,,,,,,,,  
5,chicken,tropical fruit,other vegetables,vinegar,shopping bags,,,,,,,,,,,,,,,,,,,,,,,,,,,  

    # reading data from csv file 
    import pandas as pd
    dataDF = pd.read_csv("./input/groceries.csv", dtype=str).drop(['Item(s)'], axis=1)
    
    # fitting input dataframe on FPGrowth object  
    fpm = FPGrowth(minSupport = 0.01, minConfidence = 0.5, compression_point = 4, 
                   mem_opt_level = 1).fit(dataDF)   
    # Now, to print the frequent itemsets table after training is completed
    print(fpm.freqItemsets)   
    # to print table with all the association rules 
    print(fpm.associationRules)

Output
    
    frequent itemsets:
                                            items    freq
    0                                [whole milk]  2513.0
    1                          [other vegetables]  1903.0
    2                                [rolls/buns]  1809.0
    3                                      [soda]  1715.0
    4                                    [yogurt]  1372.0
    ..                                        ...     ...
    328  [whipped/sour cream, yogurt, whole milk]   107.0
    329    [yogurt, other vegetables, whole milk]   219.0
    330    [yogurt, rolls/buns, other vegetables]   113.0
    331          [yogurt, rolls/buns, whole milk]   153.0
    332                [yogurt, soda, whole milk]   103.0
    association rules:
                              antecedent         consequent    confidence    lift   support  conviction
    0             [yogurt, other vegetables]    [whole milk]     0.512881  2.007235 0.022267 1.528340
    1           [whipped/sour cream, yogurt]    [whole milk]     0.524510  2.052747 0.010880 1.565719
    2 [whipped/sour cream, other vegetables]    [whole milk]     0.507042  1.984385 0.014642 1.510239
    3               [tropical fruit, yogurt]    [whole milk]     0.517361  2.024770 0.015150 1.542528
    4      [tropical fruit, root vegetables]    [whole milk]     0.570048  2.230969 0.011998 1.731553
    5              [root vegetables, yogurt]    [whole milk]     0.562992  2.203354 0.014540 1.703594
    6          [root vegetables, rolls/buns]    [whole milk]     0.523013  2.046888 0.012710 1.560804
    7          [pip fruit, other vegetables]    [whole milk]     0.517510  2.025351 0.013523 1.543003
    8      [domestic eggs, other vegetables]    [whole milk]     0.552511  2.162336 0.012303 1.663694
    9                         [curd, yogurt]    [whole milk]     0.582353  2.279125 0.010066 1.782567
    10            [butter, other vegetables]    [whole milk]     0.573604  2.244885 0.011490 1.745992
    11     [tropical fruit, root vegetables] [other vegetables]  0.584541  3.020999 0.012303 1.941244
    12             [root vegetables, yogurt] [other vegetables]  0.500000  2.584078 0.012913 1.613015
    13         [root vegetables, rolls/buns] [other vegetables]  0.502092  2.594890 0.012201 1.619792
    14       [citrus fruit, root vegetables] [other vegetables]  0.586207  3.029608 0.010371 1.949059
    
When native python iterable or a pandas dataframe is provided, it is converted to frovedis 
dataframe and sent to frovedis server which consumes some data transfer time. Pre-constructed 
frovedis-like inputs can be used to speed up the training time, especially when same data 
would be used for multiple executions. The storage representation when creating frovedis 
dataframe is slightly different than pandas dataframe as shown below :  

    # creating a pandas dataframe
    import pandas as pd
    data = [['banana','apple','mango','cake'], 
            ['cake','banana','apple'], 
            ['bread','banana'], 
            ['banana']]
    dataDF = pd.DataFrame(data)
    
    # Creating a two column frovedis dataframe
    import frovedis.dataframe as fpd
    frovDF = fpd.DataFrame(dataDF)
    
    # fitting frovedis-like input dataframe on FPGrowth object  
    fpm = FPGrowth(minSupport = 0.01, minConfidence = 0.5, compression_point = 4, 
                   mem_opt_level = 1).fit(frovDF)  
    # Now, to print the frequent itemsets table after training is completed
    print(fpm.freqItemsets)   
    # to print table with all the association rules 
    print(fpm.associationRules)

Output,

    frequent itemsets:
                               items  freq
    0                       [banana]   4.0
    1                        [apple]   2.0
    2                         [cake]   2.0
    3                        [bread]   1.0
    4                        [mango]   1.0
    5                  [cake, apple]   2.0
    6                 [mango, apple]   1.0
    7                [apple, banana]   2.0
    8                [bread, banana]   1.0
    9                 [cake, banana]   2.0
    10               [mango, banana]   1.0
    ...
    16  [mango, cake, apple, banana]   1.0
    association rules:
                    antecedent consequent  confidence  lift  support  conviction
    0                 [banana]    [apple]         0.5   1.0     0.50         1.0
    1                  [apple]     [cake]         1.0   2.0     0.50         NaN
    2                 [banana]     [cake]         0.5   1.0     0.50         1.0
    3                  [apple]    [mango]         0.5   2.0     0.25         1.5
    4                   [cake]    [mango]         0.5   2.0     0.25         1.5
    5                  [apple]   [banana]         1.0   1.0     0.50         NaN
    6                  [bread]   [banana]         1.0   1.0     0.25         NaN
    7                   [cake]    [apple]         1.0   2.0     0.50         NaN
    8                   [cake]   [banana]         1.0   1.0     0.50         NaN
    9                  [mango]    [apple]         1.0   2.0     0.25         NaN
    10                 [mango]   [banana]         1.0   1.0     0.25         NaN
    ...
    27    [mango, cake, apple]   [banana]         1.0   1.0     0.25         NaN

Another way to work with frovedis dataframe is by loading it from a file. Since in real world, 
large transaction database will be used for rule mining, such files needs to have two columns 
named “trans_id” and “item”, where “trans_id” column will have respective transaction id and “item” 
column will have the individual items in that transaction. If other names are present for the columns 
in the file, then it would raise exception.  

For example:

FILE: trans.csv

trans_id,item
1,1  
1,2  
1,3  
1,4  
2,1  
2,2  
2,3  
3,1  
3,2  
4,1  

The above data can be loaded and passed to frovedis FPGrowth as follows:

    # loadinf frovedis dataframe from a csv file
    import frovedis.dataframe as fdf
    frovDF = fdf.read_csv(“trans.csv”)
    
    # fitting frovedis-like input dataframe on FPGrowth object  
    fpm = FPGrowth(minSupport = 0.01, minConfidence = 0.5, compression_point = 4, 
                   mem_opt_level = 1).fit(frovDF)  

__Return Value__  
It simply returns "self" reference.  

### 3. generate_rules(confidence = None)
__Parameters__  
**_confidence_**: A double (float64) type parameter indicating the minimum confidence 
value. (Default: None)  
When it is None (not specified explicitly), then it will use confidence value used during 
FPGrowth object creation.  

__Purpose__  
It accepts the minimum confidence value to trim the rules during the generation 
of association rules.  

For example,  

    # generating rules with minimum confidence value of 0.2  
    fp_rules = fpm.generate_rules(0.2)  

This will generate tables containing rules at server side.  
To print these generated rules, debug_print() may be used as show below:

    fp_rules.debug_print()

This will show all tables of different antecedent length at server side. Here, encoding 
was disabled during rule generation.  

Output,

    --- rule[0] ---
    antecedent1   consequent   confidence   lift   support  conviction
          apple   cake         1            2      0.5      NULL
          apple   mango        0.5          2      0.25     1.5
         banana   apple        0.5          1      0.5      1
         banana   cake         0.5          1      0.5      1
           cake   mango        0.5          2      0.25     1.5

    --- rule[1] ---
    antecedent1   consequent   confidence   lift   support  conviction
           cake   apple        1            2      0.5      NULL
          mango   apple        1            2      0.25     NULL
          apple   banana       1            1      0.5      NULL
          bread   banana       1            1      0.25     NULL
           cake   banana       1            1      0.5      NULL
          mango   banana       1            1      0.25     NULL
          mango   cake         1            2      0.25     NULL

    --- rule[2] ---
    antecedent1   antecedent2  consequent  confidence  lift  support  conviction
          apple   banana       cake        1           2     0.5      NULL
           cake   banana       mango       0.5         2     0.25     1.5
          apple   banana       mango       0.5         2     0.25     1.5
           cake   apple        mango       0.5         2     0.25     1.5

    --- rule[3] ---
    antecedent1   antecedent2  consequent  confidence  lift  support  conviction
          mango   banana       apple       1           2     0.25     NULL
           cake   banana       apple       1           2     0.5      NULL
          mango   banana       cake        1           2     0.25     NULL
          mango   apple        cake        1           2     0.25     NULL

    --- rule[4] ---
    antecedent1   antecedent2  consequent  confidence  lift  support  conviction
          mango   cake         banana      1           1     0.25     NULL
          mango   apple        banana      1           1     0.25     NULL
           cake   apple        banana      1           1     0.5      NULL
          mango   cake         apple       1           2     0.25     NULL

    --- rule[5] ---
    antecedent1   antecedent2  antecedent3 consequent  confidence  lift  support  conviction
           cake   apple        banana           mango  0.5         2     0.25     1.5

    --- rule[6] ---
    antecedent1   antecedent2  antecedent3 consequent  confidence  lift  support  conviction
          mango   apple        banana          cake    1           2     0.25     NULL

    --- rule[7] ---
    antecedent1   antecedent2  antecedent3 consequent  confidence  lift  support  conviction
          mango   cake         banana          apple   1           2     0.25     NULL

    --- rule[8] ---
    antecedent1   antecedent2  antecedent3 consequent  confidence  lift  support  conviction
          mango   cake         apple           banana  1           1     0.25     NULL


**This output will be visible on server side. No such output will be visible on client side.**  

These above generated rules can also be saved and loaded separately as shown below:  

    rule.save("./out/FPRule")

It saves the rules in 'FPRule' directory.  
It would raise exception if the directory already exists with same name.  

The 'FPRule' directory has  

**FPRule**  
|-------- encode_logic  
|-------- **rule_0**  
|-------- **rule_1**  
|-------- **rule_2**  
|-------- **rule_3**  
|-------- **rule_4**  
|-------- **rule_5**  
|-------- **rule_6**  
|-------- **rule_7**  
|-------- **rule_8**  

The 'encode_logic' file is created only when 'encode_string_input = True' while training.  
Other directories are created according to number of rules created which were constructed during training.  
Each rule based directory contains information about antecedent, consequent, confidence, lift, 
support, conviction.  

For loading the already saved rules, following should be done:

    rule.load("./out/FPRule")

__Return Value__  
It returns an FPRules object.  

### 4. transform(data)
__Parameters__  
**_data_**: A python iterable or a pandas dataframe or frovedis-like dataframe containing the 
transaction data. Frovedis supports numeric and non-numeric values as for transaction data.  

__Purpose__  
It gives the prediction of list of items for each of the corresponding transaction items.  

For example,   

    print(fpm.transform(data))

Output

                              items     prediction
    0  [banana, apple, mango, cake]             []
    1         [cake, banana, apple]        [mango]
    2               [bread, banana]  [apple, cake]
    3                      [banana]  [apple, cake]

In case no prediction is made for any list of items, then it will give an empty list, just 
like in pandas dataframe as well.  

__Return Value__  
It returns a pandas dataframe having two fields 'items' and 'prediction'. Here 'items' is 
a list of transaction items and 'prediction' is a corresponding list of predictions for 
these transaction items.  

### 5. load(fname)
__Parameters__  
**_fname_**: A string object containing the name of the file having model 
information to be loaded.  

__Purpose__  
It loads the model from the specified file path.

For example,   

    fpm.load("./out/FPModel")  

__Return Value__  
It simply returns "self" reference.  

### 6. save(fname)
__Parameters__  
**_fname_**: A string object containing the name of the file on which the target 
model is to be saved.  

__Purpose__  
On success, it writes the model information in the specified file path. Otherwise, 
it throws an exception.  

For example,  

    # To save the FPGrowth model
    fpm.save("./out/FPModel")  

This will save the fp-growth model on the path '/out/FPModel'.  
It would raise exception if the directory already exists with same name.  

The 'FPModel' directory has  

**FPModel**  
|-------- encode_logic  
|-------- metadata  
|-------- **tree_0**  
|-------- **tree_1**  
|-------- **tree_2**  
|-------- **tree_3**  

The 'encode_logic' file is created only when 'encode_string_input = True' while training.  
The metadata file contains the number of transactions and rule frequent itemsets count.  
Rest of the directories are created according to tree levels which were constructed during training.  
Each tree based directory contains information about items and their frequency count on each tree level.  

__Return Value__  
It returns nothing  

### 7. debug_print()  

__Purpose__  
It shows the target model information (frequent itemsets, generated fprules, frequent itemsets count) 
on the server side user terminal. It is mainly used for debugging purpose.  

For example,  
 
    fpm.debug_print()  

Output  

    --- item_support ---
    item    item_support
    banana  1
    apple   0.5
    cake    0.5
    bread   0.25
    mango   0.25

    --- tree[0] ---
    item    count
    banana  4
    apple   2
    cake    2
    bread   1
    mango   1

    --- tree_info[0] ---

    --- tree[1] ---
    item    item1   count
    cake    apple   2
    mango   apple   1
    apple   banana  2
    bread   banana  1
    cake    banana  2
    mango   banana  1
    mango   cake    1

    --- tree_info[1] ---

    --- tree[2] ---
    item    item1   item2   count
    mango   cake    apple   1
    cake    apple   banana  2
    mango   apple   banana  1
    mango   cake    banana  1

    --- tree_info[2] ---

    --- tree[3] ---
    item    item1   item2   item3   count
    mango   cake    apple   banana  1

    --- tree_info[3] ---

    total #FIS: 17

This output will be visible on server side. It displays the in memory frequent itemsets, 
generated fprules, frequent itemsets count which is currently present on the server.

**No such output will be visible on client side.**  

__Return Value__  
It returns nothing

### 8. release()

__Purpose__  
It can be used to release the in-memory model at frovedis server.  

For example,
 
    fpm.release()  

This will reset the after-fit populated attributes to None, along with releasing 
server side memory.  

__Return Value__  
It returns nothing

### 9. is_fitted()  

__Purpose__  
It can be used to confirm if the model is already fitted or not.  

__Return Value__  
It returns 'True', if the model is already fitted, otherwise, it returns 'False'.  

# SEE ALSO  
dataframe