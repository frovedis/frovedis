package com.nec.frovedis.mllib.classification;

import com.nec.frovedis.matrix.FrovedisRowmajorMatrix
import com.nec.frovedis.exrpc.FrovedisSparseData
import com.nec.frovedis.Jexrpc.{FrovedisServer,JNISupport,MemPair}
import com.nec.frovedis.Jmllib.DummyGLM
import com.nec.frovedis.io.FrovedisIO
import com.nec.frovedis.exrpc.FrovedisLabeledPoint
import com.nec.frovedis.matrix.MAT_KIND
import com.nec.frovedis.mllib.{M_KIND,ModelID}
import com.nec.frovedis.mllib.regression.GeneralizedLinearModel
import org.apache.spark.SparkContext
import org.apache.spark.rdd.RDD
import org.apache.spark.mllib.regression.LabeledPoint
import org.apache.spark.mllib.linalg.Vector
import scala.collection.immutable.Map

class SVCModel(modelId: Int,
               modelKind: Short,
               nftr: Long,
               ncls: Int,
               logic: Map[Double, Double])
    extends GeneralizedLinearModel(modelId,modelKind,nftr,ncls, 0.0) {
    protected val enc_logic: Map[Double, Double] = logic
    protected var metaData  = Map("kernelType" -> modelKind)
    def this(m: DummyGLM,
             logic: Map[Double,Double]) = {
        this(m.mid, m.mkind, m.numFeatures, m.numClasses, logic) 
    }
    override def toString() : String = {
        val str = s"numFeatures: " + numFeatures + 
                  s", numClasses: " + numClasses
        return str
    }
    // accessing threshold related routines of this model would cause 
    // runtime exception
    override private[this] def getThreshold(): Double = 1.0
    override private[this] def setThreshold(thr: Double) : Unit = { }
    override private[this] def clearThreshold() : Unit = { }


    override def predict(data: FrovedisRowmajorMatrix) : RDD[Double] = {
        val ret = super.predict(data)
        return ret.map(x => enc_logic(x))
    }
    override def predict(data: FrovedisSparseData) : RDD[Double] = {
        val ret = super.predict(data)
        return ret.map(x => enc_logic(x))
    }
    override def save(path: String) : Unit = {
        val context = SparkContext.getOrCreate()
        save(context, path)
    }
    override def save(sc: SparkContext, path: String) : Unit = {
        val success = FrovedisIO.createDir(path)
        require(success, "Another model named " + path + " already exists!")
        super.save(path + "/model")
        sc.parallelize(logic.toSeq, 2)
          .saveAsObjectFile(path + "/label_map_spk") 
        sc.parallelize(metaData.toSeq, 2)
          .saveAsObjectFile(path + "/metadata_spk") 
    }
}

object SVCModel {  // companion object (for static members)
  def load(path: String): SVCModel = {
    val context = SparkContext.getOrCreate()
    load(context, path)
  }
  def load(sc: SparkContext, path: String) : SVCModel = {
    val exist = FrovedisIO.checkExists(path)
    require(exist, "No model named " + path + " is found!")
    val model_id = ModelID.get()
    val fs = FrovedisServer.getServerInstance()
    val context = SparkContext.getOrCreate()
    val isLinear = context.objectFile[(String, Short)](path + "/metadata_spk")
                       .collectAsMap 
                       .toMap         
    var mkind = M_KIND.KSVC
    if (isLinear("kernelType") == M_KIND.SVM) mkind = M_KIND.SVM
    val ret = JNISupport.loadFrovedisGLM(fs.master_node,model_id,
                               mkind, path + "/model")
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    val logic = context.objectFile[(Double, Double)](path + "/label_map_spk")
                       .collectAsMap // returns generic scala.collection.Map
                       .toMap        // to make it immutable Map
    return new SVCModel(ret, logic)
  }
}

// --------------------------------------------------------------------------
// NOTE: Like Spark, We have used multiple train methods instead of default
//       arguments to support Java programs.
// Default values are taken as in Frovedis
//   C = 1.0
//   kernelType = "rbf"
//   cacheSize = 128
//   maxIter = 100
// --------------------------------------------------------------------------

object SVC {
  // Kind of shortcut Spark-like interface for Spark user.
  // Spark data will get converted to Frovedis data and will be freed
  // from Frovedis side memory implicitly, once the training is completed.
  def train(data: RDD[LabeledPoint],
            C: Double,
            kernelType: String,
            degree: Int,
            gamma: Double,
            coef0: Double,
            tol: Double,
            cacheSize: Int,
            maxIter: Int) : SVCModel = { 
     return new SVC().setC(C)
                     .setKernelType(kernelType)
                     .setDegree(degree)
                     .setGamma(gamma)
                     .setCoef0(coef0)
                     .setTol(tol)
                     .setCacheSize(cacheSize)
                     .setMaxIter(maxIter)
                     .run(data)
  }

  def train(data: RDD[LabeledPoint],
            C: Double,
            kernelType: String,
            degree: Int,
            gamma: Double,
            coef0: Double,
            tol: Double,
            cacheSize: Int) : SVCModel = {
     return train(data, C, kernelType, degree, 
                  gamma, coef0, tol, cacheSize, 100)
  }

  def train(data: RDD[LabeledPoint],
            C: Double,
            kernelType: String,
            degree: Int,
            gamma: Double,
            coef0: Double,
            tol: Double) : SVCModel = {
     return train(data, C, kernelType, degree, gamma, coef0, tol, 128, 100)
  }

  def train(data: RDD[LabeledPoint],
            C: Double,
            kernelType: String,
            degree: Int,
            gamma: Double,
            coef0: Double) : SVCModel = {
     return train(data, C, kernelType, degree, gamma, coef0, 0.001, 128, 100)
  }

  def train(data: RDD[LabeledPoint],
            C: Double,
            kernelType: String,
            degree: Int,
            gamma: Double) : SVCModel = {
     return train(data, C, kernelType, degree, gamma, 0.0, 0.001, 128, 100)
  }

  def train(data: RDD[LabeledPoint],
            C: Double,
            kernelType: String,
            degree: Int) : SVCModel = {
     return train(data, C, kernelType, degree, 0.1, 0.0, 0.001, 128, 100)
  }

  def train(data: RDD[LabeledPoint],
            C: Double,
            kernelType: String) : SVCModel = {
     return train(data, C, kernelType, 3, 0.1, 0.0, 0.001, 128, 100)
  }

  def train(data: RDD[LabeledPoint],
            C: Double) : SVCModel = {
     return train(data, C, "rbf", 3, 0.1, 0.0, 0.001, 128, 100)
  }

  def train(data: RDD[LabeledPoint]) : SVCModel = {
     return train(data, 1.0, "rbf", 3, 0.1, 0.0, 0.001, 128, 100)
  }

  // User needs to convert the Spark data into Frovedis Data by himself before 
  // calling this interface. In this case, user has to explicitly free the 
  // Frovedis Data when it will no longer be needed 
  //       to avoid memory leak at Frovedis server.
  def train(data: FrovedisLabeledPoint,
            C: Double,
            kernelType: String,
            degree: Int,
            gamma: Double,
            coef0: Double,
            tol: Double,
            cacheSize: Int,
            maxIter: Int) : SVCModel = {
     return new SVC().setC(C)
                     .setKernelType(kernelType)
                     .setDegree(degree)
                     .setGamma(gamma)
                     .setCoef0(coef0)
                     .setTol(tol)
                     .setCacheSize(cacheSize)
                     .setMaxIter(maxIter)
                     .run(data)
  }

  def train(data: FrovedisLabeledPoint,
            C: Double,
            kernelType: String,
            degree: Int,
            gamma: Double,
            coef0: Double,
            tol: Double,
            cacheSize: Int) : SVCModel = {
        return train(data, C, kernelType, degree, gamma, coef0, 
                     tol, cacheSize, 100)
  }

  def train(data: FrovedisLabeledPoint,
            C: Double,
            kernelType: String,
            degree: Int,
            gamma: Double,
            coef0: Double,
            tol: Double) : SVCModel = {
        return train(data, C, kernelType, degree, gamma, coef0, 
                     tol, 128, 100)
  }

  def train(data: FrovedisLabeledPoint,
            C: Double,
            kernelType: String,
            degree: Int,
            gamma: Double,
            coef0: Double) : SVCModel = {
        return train(data, C, kernelType, degree, gamma, 
                     coef0, 0.001, 128, 100)
  }

  def train(data: FrovedisLabeledPoint,
            C: Double,
            kernelType: String,
            degree: Int,
            gamma: Double) : SVCModel = {
        return train(data, C, kernelType, degree, gamma, 
                     0.0, 0.001, 128, 100)
  }

  def train(data: FrovedisLabeledPoint,
            C: Double,
            kernelType: String,
            degree: Int) : SVCModel = {
        return train(data, C, kernelType, degree, 0.1, 
                     0.0, 0.001, 128, 100)
  }

  def train(data: FrovedisLabeledPoint,
            C: Double,
            kernelType: String) : SVCModel = {
        return train(data, C, kernelType, 3, 0.1, 0.0, 0.001, 128, 100)
  }

  def train(data: FrovedisLabeledPoint,
            C: Double) : SVCModel = {
        return train(data, C, "rbf", 3, 0.1, 0.0, 0.001, 128, 100)
  }

  def train(data: FrovedisLabeledPoint) : SVCModel = {
        return train(data, 1.0, "rbf", 3, 0.1, 0.0, 0.001, 128, 100)
  }

}

class SVC(var C: Double,
          var kernelType: String,
          var degree: Int,
          var gamma: Double, 
          var coef0: Double,
          var tol: Double,
          var cacheSize: Int,
          var maxIter: Int,
          var stepSize: Double) { //TODO: python check
    def this() = this(1.0, "rbf", 3, 0.1, 0.0, 0.001, 128, 100, 0.01)

    def setC(C: Double): this.type = {  
        require(C > 0, s"setC: expected a positive value greater than 0, "+
                s"obtained: $C.")
        this.C = C
        return this
    }
    def getC() = this.C

    def setKernelType(kernelType: String): this.type = { 
        require(Set("linear", "poly", "rbf", "sigmoid").contains(kernelType), 
                s"setKernelType: supported kernels are "+
                s"linear, poly, rbf, sigmoid, obtained: $kernelType.")
        this.kernelType = kernelType
        return this
    }
    def getKernelType() = this.kernelType

    def setDegree(degree: Int): this.type = {
        require(degree > 0, s"setDegree: expected a positive value greater "+
                s"than 0, obtained: $degree.")
        this.degree = degree
        return this
    }
    def getDegree() = this.degree

    def setGamma(gamma: Double): this.type = {
        require(gamma > 0, s"setGamma: expected a positive value greater "+
                s"than 0, obtained: $gamma.")
        this.gamma = gamma
        return this
    }
    def getGamma() = this.gamma

    def setCoef0(coef0: Double): this.type = {  
        require(coef0 >= 0, s"setCoef0: expected a positive value, "+
                s"obtained:: $coef0.")
        this.coef0 = coef0
        return this
    }
    def getCoef0() = this.coef0

    def setTol(tol: Double): this.type = {
        require(tol >= 0, s"setTol: expected a positive value, obtained: $tol.")
        this.tol = tol
        return this
    }
    def getTol() = this.tol

    def setCacheSize(cacheSize: Int): this.type = { 
        require(cacheSize > 2, s"setCacheSize: expected a value greater "+
                s"than 2, obtained: $cacheSize.")
        this.cacheSize = cacheSize
        return this
    }
    def getCacheSize() = this.cacheSize

    def setMaxIter(maxIter: Int): this.type = { 
        require(maxIter > 0, s"setMaxIter: expected a positive value greater "+
                s"than 0, obtained: $maxIter.")
        this.maxIter = maxIter
        return this
    }
    def getMaxIter() = this.maxIter
    
    def setStepSize(stepSize: Double): this.type = {
        require(stepSize > 0, s"setStepSize: expected a positive value "+
                s"greater than 0, obtained: $stepSize.")
        this.stepSize = stepSize
        return this
    }
    def getStepSize() = this.stepSize

    private[frovedis] def assertValid(): Unit = {
        val C = getC()
        require(C > 0, s"setC: expected a positive value greater than 0, "+
                s"obtained: $C.")
        val kernelType = getKernelType()
        require(Set("linear", "poly", "rbf", "sigmoid").contains(kernelType), 
                s"setKernelType: Supported kernels are linear, poly, "+
                s"rbf, sigmoid, obtained: $kernelType.")
        val degree = getDegree()
        require(degree > 0, s"setDegree: expected a positive value greater "+
                s"than 0, obtained: $degree.")
        val gamma = getGamma()
        require(gamma > 0, s"setGamma: expected a positive value greater "+
                s"than 0, obtained: $gamma.")
        val coef0 = getCoef0()
        require(coef0 >= 0, s"setCoef0: expected a positive value, "+
                s"obtained:: $coef0.")
        val tol = getTol()
        require(tol >= 0, s"setTol: expected a positive value, obtained: $tol.")
        val cacheSize = getCacheSize()
        require(cacheSize > 2, s"setCacheSize: expected a value greater "+
                s"than 2, obtained: $cacheSize.")
        val maxIter = getMaxIter()
        require(maxIter > 0, s"setMaxIter: expected a positive value greater "+
                s"than 0, obtained: $maxIter.")
    }

    def run(input: RDD[LabeledPoint]): SVCModel= {
        var need_rowMajor = false 
        if (kernelType != "linear") need_rowMajor = true
        val fdata = new FrovedisLabeledPoint(input, need_rowMajor)
        return run_impl(fdata, true)
    }

    def run(fdata: FrovedisLabeledPoint): SVCModel= {
        return run_impl(fdata, false)
    }

    private def run_impl(data: FrovedisLabeledPoint, 
                         movable: Boolean): SVCModel = {
        assertValid()
        val numClasses = data.get_distinct_label_count().intValue
        val numFeatures = data.numCols()
        val mid = ModelID.get()
        var enc_ret: (MemPair,  Map[Double, Double]) = null
        if (numClasses > 2) enc_ret = data.encode_labels()
        else enc_ret = data.encode_labels(Array(-1.0, 1.0))
        val encoded_data = enc_ret._1
        val logic = enc_ret._2
        val fs = FrovedisServer.getServerInstance()
        var m_kind = M_KIND.KSVC
        if(kernelType == "linear") {
            require (data.matType() == MAT_KIND.CMJR, 
                     s"run: please provide column major "+
                     s"points for linear SVM!\n" )
            val rparam = 1.0 / C
            val mbf = 1.0
            JNISupport.callFrovedisSVMSGD(fs.master_node,encoded_data,maxIter,
                                          stepSize,mbf,rparam,mid,movable,
                                          data.is_dense(),numClasses)
            m_kind = M_KIND.SVM
        } else {
            require (data.matType() == MAT_KIND.RMJR, 
                     s"run: please provide row major points "+
                     s"for non-linear SVM!\n" )
            JNISupport.callFrovedisKernelSVM(fs.master_node,encoded_data,C,
                                             kernelType,degree,gamma,coef0,tol,
                                             cacheSize,maxIter,mid,movable,
                                             data.is_dense(),numClasses)
        }
        val info = JNISupport.checkServerException()
        if (info != "") throw new java.rmi.ServerException(info)
        data.release_encoded_labels() // deleting encoded labels from server
        return new SVCModel(mid, m_kind, numFeatures, numClasses, logic)
    }
}
