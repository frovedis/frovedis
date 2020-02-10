package com.nec.frovedis.mllib.clustering;

import com.nec.frovedis.Jexrpc.JNISupport
import com.nec.frovedis.Jexrpc.FrovedisServer
import com.nec.frovedis.Jmatrix.DummyMatrix
import com.nec.frovedis.Jmllib.DummyLDAModel
import com.nec.frovedis.Jmllib.DummyLDAResult
import com.nec.frovedis.mllib.{M_KIND,ModelID,GenericModel}
import com.nec.frovedis.exrpc.FrovedisSparseData
import com.nec.frovedis.matrix.FrovedisRowmajorMatrix
import com.nec.frovedis.matrix.MAT_KIND
import org.apache.spark.rdd.RDD
import org.apache.spark.SparkContext
import org.apache.spark.mllib.linalg.{Vectors, Vector, Matrix}
import org.apache.spark.util.Utils
import org.apache.spark.mllib.linalg.distributed.RowMatrix

/*
 * References:
 *  - This implementation is a wrapper of Spark LDA,
 *  - Ref: https://github.com/apache/spark/blob/v2.4.4/mllib/src/main/scala/org/apache/spark/ml/clustering/LDA.scala
 *  - Spark has different implementations of LDA in spark/ml, and spark/mllib. 
 *  - We are refering spark/mllib in this implementation, 
 *    and have additionally added support for transform(). topTopicsPerDocument() 
 *    and topDocumentsPerTopic() are not supported in the current version.
 */


class LDA (
  // data members in spark specific implementation
  private var k: Int,
  private var maxIterations: Int,
  private var docConcentration: Vector,
  private var topicConcentration: Double,
  private var seed: Long,
  private var checkpointInterval: Int,
  private var ldaOptimizer: String,
  // data members in frovedis specific implementation
  private var num_explore_iter: Int,
  private var num_eval_cycle: Int,
  //private var verbose,
  private var algorithm: String) extends java.io.Serializable {

  def this() = this(k = 10, maxIterations = 20, docConcentration = Vectors.dense(0.1),
    topicConcentration = 0.1, seed = 1, checkpointInterval = 10,
    ldaOptimizer = "em", num_explore_iter = 0, num_eval_cycle = 0,
    algorithm = "original")
  /* getter setter for num_topic */
  def getK: Int = k
  def setK(k: Int): this.type = {
    require(k > 0, s"getK: LDA k (number of clusters) must be > 0, but was set to $k")
    this.k = k
    this
  }
  /* getter setter for alpha */ 
  def getDocConcentration: Double = {
    val parameter = docConcentration.toArray.sum / k
    parameter
  } 
  def setDocConcentration(docConcentration: Vector): this.type = {
    require(docConcentration.size == 1 || docConcentration.size == k,
      s"setDocConcentration: Size of docConcentration must be 1 or ${k} " +
      "but got ${docConcentration.size}")
    require(docConcentration.toArray.forall(x => x > 0),
      s"setDocConcentration: Alpha should be greater than 0")
    this.docConcentration = docConcentration
    this
  } 
  def setDocConcentration(docConcentration: Double): this.type = {
    require(docConcentration > 0,
      s"setDocConcentration: Alpha should be greater than 0")
    this.docConcentration = Vectors.dense(docConcentration)
    this
  } 
  def getAlpha: Double = getDocConcentration 
  def setAlpha(alpha: Vector): this.type = setDocConcentration(alpha)
  /* getter setter for beta */
  def getTopicConcentration: Double = this.topicConcentration
  def setTopicConcentration(topicConcentration: Double): this.type = {
    require(topicConcentration > 0,
      s"setTopicConcentration: Beta should be greater than 0")
    this.topicConcentration = topicConcentration
    this
  }
  def getBeta: Double = getTopicConcentration
  def setBeta(beta: Double): this.type = setTopicConcentration(beta)
  /* getter setter for num_iter */
  def getMaxIterations: Int = maxIterations
  def setMaxIterations(maxIterations: Int): this.type = {
    require(maxIterations >= 0,
      s"setMaxIterations: Maximum of iterations must be nonnegative but got ${maxIterations}")
    this.maxIterations = maxIterations
    this
  }
  /* getter setter for seed(not used in frovedis) */
  def getSeed: Long = seed
  def setSeed(seed: Long): this.type = {
    this.seed = seed
    this
  }
  /* getter setter for checkpointInterval(not used in frovedis) */
  def getCheckpointInterval: Int = checkpointInterval
  def setCheckpointInterval(checkpointInterval: Int): this.type = {
    require(checkpointInterval == -1 || checkpointInterval > 0,s" "+
      "getCheckpointInterval: Period between checkpoints must be" + 
      "-1 or positive but got ${checkpointInterval}")
    this.checkpointInterval = checkpointInterval
    this
  }
  /* getter setter for optimizer(not used in frovedis) */
  def getOptimizer: String = ldaOptimizer
  def setOptimizer(optimizerName: String): this.type = {
    require(optimizerName == "em" || optimizerName == "online",s"Only em," +
            "online are supported but got ${optimizerName}.")
    this.ldaOptimizer = optimizerName
    this
  }
  /* frovedis specific getter setters */
  /* getter setter for num_eval_cycle */
  def getNumevalcycle: Int = num_eval_cycle 
  def setNumevalcycle(num_eval_cycle: Int): this.type = {
    require(num_eval_cycle >= 0,
      s"setNumevalcycle: num_eval_cycle must be nonnegative but got ${num_eval_cycle}")
    this.num_eval_cycle = num_eval_cycle
    this
  }
  /* getter setter for num_explore_iter */
  def getNumexploreiter: Int = num_explore_iter
  def setNumexploreiter(num_explore_iter: Int): this.type = {
    require(num_explore_iter >= 0, s"setNumexploreiter: num_explore_iter "+
    "must be nonnegative but got ${num_explore_iter}")
    this.num_explore_iter = num_explore_iter
    this
  }
  /* getter setter for algorithm(for frovedis) */
  def getAlgorithm: String = algorithm 
  def setAlgorithm(algorithm: String): this.type = {
    val algo = ("original", "wp", "dp", "cp", "sparse")
    require(algo.productIterator.contains(algorithm),s"Specified algorithm "+ 
    "is not supported for frovedis")
    this.algorithm = algorithm
    this
  }   
  /* train lda model */
  def run(documents: RDD[(Long, Vector)]): LDAModel= {
    var doc_vec = documents.map(x => x._2)
    val fdata = new FrovedisSparseData(doc_vec)
    val ldaResult =  run(fdata)
    fdata.release()
    return ldaResult
  }
  def run(data: FrovedisSparseData): LDAModel = { 
    val mid = ModelID.get()
    val fs = FrovedisServer.getServerInstance()
    val ret = JNISupport.callFrovedisLDA(fs.master_node,
                                data.get(),mid,k,maxIterations,
                                getAlpha,getBeta,num_explore_iter,
                                num_eval_cycle,algorithm)
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    require(ret.vocabsz > 0, s"run: Incorrect vocabSize : ${ret.vocabsz}")
    return new LDAModel(mid, ret.num_topics, ret.vocabsz, maxIterations, 
                        getAlpha, getBeta, num_explore_iter, algorithm)
  }
}

class LDAModel(private var model_Id: Int, 
               private var k: Int,
               private var vocabsz: Int, 
	       private var maxIterations: Int = 20,
               private var alpha: Double = 0.1, 
	       private var beta: Double = 0.1, 
	       private var num_explore_iter: Int = 0, 
	       private var algorithm: String = "original")
      extends GenericModel(model_Id, M_KIND.LDA) {
  /* show/release/save are handled in GenericModel */
  private var ppl: Double = 0.0
  private var llh: Double = 0.0
  //for transform
  private var docConcentration: Vector = Vectors.dense(alpha)
  private var topicConcentration: Double = beta
  /* getter setter for alpha */
  def getDocConcentration: Double = {
    val parameter = docConcentration.toArray.sum / k
    parameter
  } 
  def getAlpha: Double = getDocConcentration 
  def setAlpha(alpha: Vector): this.type = setDocConcentration(alpha)
  def setDocConcentration(docConcentration: Vector): this.type = {
    require(docConcentration.size == 1 || docConcentration.size == k,
      s"setDocConcentration: Size of docConcentration must be 1 or ${k} " +
      "but got ${docConcentration.size}")
    require(docConcentration.toArray.forall(x => x > 0),
      s"setDocConcentration: Alpha should be greater than 0")
    this.docConcentration = docConcentration
    this
  } 
  def setDocConcentration(docConcentration: Double): this.type = {
    require(docConcentration > 0,
      s"setDocConcentration: Alpha should be greater than 0")
    this.docConcentration = Vectors.dense(docConcentration)
    this
  }
  /* getter setter for beta */
  def getTopicConcentration: Double = this.topicConcentration
  def getBeta: Double = getTopicConcentration
  def setBeta(beta: Double): this.type = setTopicConcentration(beta)
  def setTopicConcentration(topicConcentration: Double): this.type = {
    require(topicConcentration > 0,
      s"setTopicConcentration: Beta should be greater than 0")
    this.topicConcentration = topicConcentration
    this
  }
  def setAlgorithm(algorithm: String): this.type = {
    val algo = ("original", "wp", "dp", "cp", "sparse")
    require(algo.productIterator.contains(algorithm),s"Specified algorithm "+ 
    "is not supported for frovedis")
    this.algorithm = algorithm
    this
  }
  def setMaxIterations(maxIterations: Int): this.type = {
    require(maxIterations > 0,
      s"setMaxIterations: Maximum no. of iterations must be greater than 0 but, got ${maxIterations}")
    this.maxIterations = maxIterations
    this
  }
  def setNumexploreiter(num_explore_iter: Int): this.type = {
    require(num_explore_iter >= 0, s"setNumexploreiter: num_explore_iter "+
    "must be nonnegative but got ${num_explore_iter}")
    this.num_explore_iter = num_explore_iter
    this
  }
  def setK(k: Int): this.type = {
    require(k > 0, s"setK: LDA k (number of clusters) must be > 0, but was set to $k")
    this.k = k
    this
  }
  def vocabSize: Int = vocabsz
  /* we need to get
     word id: Array[Int](k*maxTermsPerTopic) and 
     weight of word in topics: Array[Double](k*maxTermsPerTopic) 
     result will be Array(word_id,weight of word in topics) 
   */
  def describeTopics(maxTermsPerTopic: Int): 
                     Array[(Array[Int], Array[Double])]= {
    val nr = k
    val nc = vocabSize
    require(maxTermsPerTopic <= nc, s"describeTopics: Input maxTermsPerTopic "+
    "must be less than or equal to vocabsize but got ${maxTermsPerTopic}")
    val fs = FrovedisServer.getServerInstance()
    val word_id = new Array[Int](nr * maxTermsPerTopic)
    val word_topic_dist = new Array[Double](nr * maxTermsPerTopic)
    JNISupport.getDescribeMatrix(fs.master_node, model_Id,
                                 nr, maxTermsPerTopic, 
				 word_id, word_topic_dist)
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    val arry = word_id.grouped(maxTermsPerTopic).toArray
    val arrz = word_topic_dist.grouped(maxTermsPerTopic).toArray
    return arry.zip(arrz)
  }
  def describeTopics(): Array[(Array[Int], Array[Double])] = describeTopics(vocabSize)
  def topicsMatrix: Matrix = {
    val fs = FrovedisServer.getServerInstance()
    val dummyres = JNISupport.getTopicsMatrix(fs.master_node, model_Id)
    val dummy_dist = new DummyMatrix(dummyres.mptr, 
                                     dummyres.nrow, 
                                     dummyres.ncol, MAT_KIND.RMJR)
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    return new FrovedisRowmajorMatrix(dummy_dist).to_spark_Matrix()
  }
  /* test lda model */
  def transform(documents: RDD[(Long, Vector)]): RowMatrix = {
    val doc_vec = documents.map(x => x._2)
    val fdata = new FrovedisSparseData(doc_vec)
    val transformRes = transform(fdata).to_spark_RowMatrix(documents.context)
    fdata.release()
    return transformRes
  }
  def transform(data: FrovedisSparseData): FrovedisRowmajorMatrix = {
    val fs = FrovedisServer.getServerInstance()
    val dummyres = JNISupport.callFrovedisLDATransform(fs.master_node,
                                            data.get(),model_Id,k,
					    maxIterations,
                                            getAlpha, getBeta,
					    num_explore_iter,
                                            algorithm)
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    val dummy_dist = new DummyMatrix(dummyres.mptr, 
                                     dummyres.nrow, 
                                     dummyres.ncol, MAT_KIND.RMJR)
    val doc_topic_dist_mat = new FrovedisRowmajorMatrix(dummy_dist)
    this.llh = dummyres.likelihood
    this.ppl = dummyres.perplexity
    return doc_topic_dist_mat
  }
  def logLikelihood(documents: RDD[(Long, Vector)]): Double = {
    var doc_vec = documents.map(x => x._2)
    val fdata = new FrovedisSparseData(doc_vec)
    transform(fdata)
    fdata.release()
    return this.llh
  }  
  def logLikelihood(documents: FrovedisSparseData): Double = {
    transform(documents)
    return this.llh
  }  
  def logPerplexity(documents: RDD[(Long, Vector)]): Double = {
    var doc_vec = documents.map(x => x._2)
    val fdata = new FrovedisSparseData(doc_vec)
    transform(fdata) 
    fdata.release()
    return this.ppl
  }  
  def logPerplexity(documents: FrovedisSparseData): Double = {
    transform(documents)
    return this.ppl
  }  
  def topDocumentsPerTopic(maxDocumentsPerTopic: Int): 
                           Array[(Array[Long], Array[Double])] = {
    throw new Exception("currently not supported in frovedis") 
  }
  def topTopicsPerDocument(k: Int): RDD[(Long, Array[Int], Array[Double])] = {
    throw new Exception("currently not supported in frovedis") 
  }
  def topicDistribution(document: Vector): Vector = {
    throw new Exception("currently not supported in frovedis") 
  }
  def topicDistributions(documents: RDD[(Long, Vector)]): RDD[(Long, Vector)] = {
    throw new Exception("currently not supported in frovedis") 
  }
}

object LDAModel {
  def load(sc: SparkContext, path: String): LDAModel = load(path)
  def load(path: String): LDAModel = {
    val model_id = ModelID.get()
    val fs = FrovedisServer.getServerInstance()
    val ret = JNISupport.loadFrovedisLDAModel(fs.master_node,model_id,path)
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    require(ret.vocabsz > 0, s"load: Incorrect vocabSize : ${ret.vocabsz}")
    return new LDAModel(model_id, ret.num_topics, ret.vocabsz)
  }
}
