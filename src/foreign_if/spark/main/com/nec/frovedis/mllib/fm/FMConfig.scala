package com.nec.frovedis.mllib.fm
/**
  * Class for configuring Factorization Machine Hyper-parameters 
  * initStdev    --- stdev of initialization of 2-way factors Double Type
  * learnRate    --- learning rate of algorithm default =0.1
  * iter         ---  Number of Iterations to be performed
  * optimizer    --- type of learning method in SGD, SGDA, ALS, MCMC
  * isRegression --- a bool value to specify regression or classification
  * batchsize    --- minimum batchsize per node
  * dim          --- (Boolean,Boolean,Int) 3-Tuple stands for whether the
  *                  global bias term should be used, whether the
  *                  one-way interactions should be used, and the number of factors 
  *                  that are used for pair-wise interactions, respectively.
  * regParam     --- (Double,Double,Double) 3-Tuple stands for the 
  *                  regularization parameters of intercept, one-way  
*/

class FMConfig(var initStdev: Double = 0.1, 
               var iter: Int = 100,
               var learnRate: Double = 0.01,
               var optimizer: String = "SGD", 
               var isRegression: Boolean = true,
               var dim: (Boolean,Boolean,Int) = (true,true,8),
               var regParam: (Double,Double,Double) = (0, 1e-3, 1e-4),
               var batchsize: Int = 100) {
  private[frovedis] def assertValid(): Unit = {
    if(initStdev < 0)
      throw new IllegalStateException("Invalid initDev: " + initStdev)
    if(learnRate < 0.0001 || learnRate > 1.0) 
      throw new IllegalStateException("Invalid learning-rate: " + learnRate)
    if(iter < 0) throw new IllegalStateException("Invalid iterations: " + iter)
    val supported_opt = Array("SGD", "SGDA", "ALS", "MCMC")
    if (!(supported_opt contains optimizer))
      throw new IllegalStateException("Invalid optimizer: " + optimizer)
    if(batchsize < 0)
      throw new IllegalStateException("Invalid batch size: " + batchsize)
    require(regParam._1 >= 0 && regParam._2 >= 0 && regParam._3 >= 0)
    require(dim._3 > 0)
  }
  def setInitStdev(initDev: Double): FMConfig = {
    if(initDev < 0) 
      throw new IllegalStateException("Invalid initDev: " + initDev) 
    this.initStdev = initDev
    this
  }
    
  def getInitStdev(): Double =  this.initStdev
  
  def setLearnRate(learnRate: Double): FMConfig = {
    if(learnRate < 0.0001 || learnRate > 1.0) {
      throw new IllegalStateException("Invalid learning-rate: " + learnRate)
    }
    this.learnRate = learnRate
    this
  }

  def getLearnRate(): Double = this.learnRate

  def setIterations(iterations:Int): FMConfig = {
    if(iterations < 0) 
      throw new IllegalStateException("Invalid iterations: " + iterations)
    this.iter = iterations
    this
  }
  def getIterations(): Int = this.iter

  def setOptimizer(optimizer: String): FMConfig = {
    val supported_opt = Array("SGD", "SGDA", "ALS", "MCMC")
    if (!(supported_opt contains optimizer)) 
      throw new IllegalStateException("Invalid optimizer: " + optimizer)
    this.optimizer = optimizer
    this
  }

  def getOptimizer(): String = this.optimizer

  def setIsRegression(reg: Boolean): FMConfig = {
    this.isRegression = reg
    this
  }

  def getIsRegression(): Boolean = this.isRegression

  def setBatchSize(batch: Int): FMConfig = {
    if(batch < 0) 
      throw new IllegalStateException("Invalid batch size: " + batch)
    this.batchsize = batch
    this
  }

  def getBatchSize(): Int = this.batchsize

  def setDim(dimensions: (Boolean, Boolean, Int)): FMConfig = {
    require(dimensions._3 > 0)
    this.dim.copy(_1 = dimensions._1)
    this.dim.copy(_2 = dimensions._2)
    this.dim.copy(_3 = dimensions._3)
    this
  }

  def setDim(addIntercept: Boolean = true, 
             add1Way: Boolean = true, 
             numFactors: Int = 8): FMConfig = {
    setDim((addIntercept, add1Way, numFactors))
  }

  def setRegParam(regParams: (Double, Double, Double)): FMConfig = {
    require(regParams._1 >= 0 && regParams._2 >= 0 && regParams._3 >= 0)
    this.regParam.copy(_1 = regParams._1)
    this.regParam.copy(_2 = regParams._2)
    this.regParam.copy(_3 = regParams._3)
    this
  }

  def setRegParam(regIntercept: Double = 0,
                  reg1Way: Double = 0,
	          reg2Way: Double = 0): FMConfig = {
    setRegParam((regIntercept, reg1Way, reg2Way))
  }
}

