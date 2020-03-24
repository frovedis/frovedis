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
import org.apache.spark.sql.SparkSession
import org.apache.spark.mllib.linalg.{Vectors, Vector, Matrix}
import org.apache.spark.util.Utils
import org.apache.spark.mllib.linalg.distributed.RowMatrix

/*
 * References:
 *  - This implementation is a wrapper of Spark LDA,
 *  - Ref: https://github.com/apache/spark/blob/v2.4.4/mllib/src/main/scala/org/apache/spark/ml/clustering/LDA.scala
 *  - Spark has different implementations of LDA in spark/ml, and spark/mllib. 
 *  - We are referring spark/mllib in this implementation, 
 *    and have additionally added support for transform(). 
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
  /* save the document ids in the model */
    var orig_doc_ids = documents.map(x => x._1).collect
    var doc_vec = documents.map(x => x._2)
    val fdata = new FrovedisSparseData(doc_vec)
    val save_doc_id = true 
    val model =  run(orig_doc_ids, fdata, save_doc_id)
    fdata.release()
    return model
  }
  def run(data: FrovedisSparseData): LDAModel = { 
    val orig_doc_ids = new Array[Long](0)
    val save_doc_id = false // ordered doc_ids 
    return run(orig_doc_ids, data, save_doc_id) 
  }

  def run(orig_doc_ids: Array[Long], data: FrovedisSparseData, 
                                 save_doc_id: Boolean = false): LDAModel = {
    val mid = ModelID.get()
    val num_docs = orig_doc_ids.length
    val fs = FrovedisServer.getServerInstance()
    val ret = JNISupport.callFrovedisLDA(fs.master_node,
                                data.get(),orig_doc_ids,num_docs,save_doc_id,mid,k,maxIterations,
                                getAlpha,getBeta,num_explore_iter,
                                num_eval_cycle,algorithm)
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    require(ret.vocabsz > 0, s"run: Incorrect vocabSize : ${ret.vocabsz}")
    return new LDAModel(mid, ret.num_topics, ret.vocabsz, ret.num_docs, 
                        maxIterations, getAlpha, getBeta, num_explore_iter, 
			algorithm,save_doc_id)
  }
}

class LDAModel(private var model_Id: Int, 
               private var k: Int, // num_topics
               private var vocabsz: Int, 
               private var num_docs: Int, 
	       private var maxIterations: Int = 20,
               private var alpha: Double = 0.1, 
	       private var beta: Double = 0.1, 
	       private var num_explore_iter: Int = 0, 
	       private var algorithm: String = "original",
               /*private var orig_doc_ids: Array[Long],*/
               private var save_doc_id: Boolean = false) // assume ordered doc_ids
      extends GenericModel(model_Id, M_KIND.LDASP) {
  /* show/save are handled in GenericModel */
  private var ppl: Double = 0.0
  private var llh: Double = 0.0
  /* performance parameter to reduce server side redundant tasks */
  private var sorted_topic_word_distribution: FrovedisRowmajorMatrix = null
  private var sorted_doc_topic_distribution: FrovedisRowmajorMatrix = null
  private var sorted_topic_doc_distribution: FrovedisRowmajorMatrix = null
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
    require(maxTermsPerTopic > 0 && maxTermsPerTopic <= vocabSize, 
    s"describeTopics: Input maxTermsPerTopic " +
    "must be less than or equal to vocabsize: ${vocabsz}, but got ${maxTermsPerTopic}")
    val fs = FrovedisServer.getServerInstance()
    val word_id = new Array[Int](k * maxTermsPerTopic)
    val topic_word_dist = new Array[Double](k * maxTermsPerTopic)
    if (this.sorted_topic_word_distribution == null) {
      val dmat = JNISupport.getTopicWordDistribution(
                                       fs.master_node, model_Id)
      val info = JNISupport.checkServerException()
      if (info != "") throw new java.rmi.ServerException(info)
      this.sorted_topic_word_distribution = new FrovedisRowmajorMatrix(dmat)
    }
    JNISupport.extractTopWordsPerTopic(fs.master_node,
                                  sorted_topic_word_distribution.get(), 
                                  k, maxTermsPerTopic,
                                  word_id, topic_word_dist)
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    val arry = word_id.grouped(maxTermsPerTopic).toArray
    val arrz = topic_word_dist.grouped(maxTermsPerTopic).toArray
    return arry.zip(arrz)
  }
  def describeTopics(): Array[(Array[Int], Array[Double])] = {
    describeTopics(vocabSize)
  }
  def topicsMatrix: Matrix = {
    val fs = FrovedisServer.getServerInstance()
    val dummyres = JNISupport.getTopicsMatrix(fs.master_node, model_Id)
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    val dummy_dist = new DummyMatrix(dummyres.mptr, 
                                     dummyres.nrow, // vocabSize 
                                     dummyres.ncol, // k 
                                     MAT_KIND.RMJR)
    require(dummyres.nrow == vocabSize && dummyres.ncol == k, 
            "Internal issue in getting topics matrix: REPORT BUG!")
    return new FrovedisRowmajorMatrix(dummy_dist).to_spark_Matrix()
  }
  def transform(documents: RDD[(Long, Vector)],
                need_distribution: Boolean = true):  RDD[(Long, Vector)] = {
    val ctxt = documents.context
    val test_doc_id = documents.map(x => x._1) // RDD[Long]
    val doc_vec = documents.map(x => x._2)
    val save_doc_id = true
    val fdata = new FrovedisSparseData(doc_vec)
    val transformRes = transform(fdata)
    fdata.release()
    var ret: RDD[(Long, Vector)] = null
    if (need_distribution) {
      val spark_rows = transformRes.to_spark_RowMatrix(ctxt).rows // RDD[Vector]
      ret = test_doc_id.zipWithIndex.map(_.swap).join(spark_rows.zipWithIndex.map(_.swap)).values
    } 
    transformRes.release()
    return ret
  }
  def transform(data: FrovedisSparseData): FrovedisRowmajorMatrix = {
    val fs = FrovedisServer.getServerInstance()
    val dummyres = JNISupport.callFrovedisLDATransform(fs.master_node,
                                            data.get(), model_Id,
					    maxIterations,
                                            getAlpha, getBeta,
					    num_explore_iter,
                                            algorithm)
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    val dummy_dist = new DummyMatrix(dummyres.mptr, 
                                     dummyres.nrow, // num_docs 
                                     dummyres.ncol, // k: num_topics 
                                     MAT_KIND.RMJR)
    val doc_topic_dist_mat = new FrovedisRowmajorMatrix(dummy_dist)
    this.llh = dummyres.likelihood
    this.ppl = dummyres.perplexity
    return doc_topic_dist_mat
  }
  def topicDistributions(documents: RDD[(Long, Vector)]): RDD[(Long, Vector)] = {
    return transform(documents)
  } 
  def topicDistributions(documents: FrovedisSparseData): FrovedisRowmajorMatrix = {
    return transform(documents)
  }  

  def logLikelihood(documents: RDD[(Long, Vector)]): Double = {
    transform(documents, false)
    return this.llh
  }  
  def logLikelihood(documents: FrovedisSparseData): Double = {
    val res = transform(documents)
    res.release()
    return this.llh
  }  
  def logPerplexity(documents: RDD[(Long, Vector)]): Double = {
    transform(documents, false) 
    return this.ppl
  }  
  def logPerplexity(documents: FrovedisSparseData): Double = {
    val res = transform(documents)
    res.release()
    return this.ppl
  }
  def topDocumentsPerTopic(maxDocumentsPerTopic: Int):
                           Array[(Array[Long], Array[Double])]= { // for training document
    require(maxDocumentsPerTopic > 0 && maxDocumentsPerTopic <= num_docs, 
    s"topDocumentsPerTopic: Input maxDocumentsPerTopic " +
    "must be less than or equal to num_docs: ${num_docs}, but got ${maxDocumentsPerTopic}")
    val fs = FrovedisServer.getServerInstance()
    val doc_id = new Array[Long](k * maxDocumentsPerTopic)
    val topic_doc_dist = new Array[Double](k * maxDocumentsPerTopic)
    if (this.sorted_topic_doc_distribution == null) {
      val dmat = JNISupport.getTopicDocDistribution(
                                       fs.master_node, model_Id)
      val info = JNISupport.checkServerException()
      if (info != "") throw new java.rmi.ServerException(info)
      this.sorted_topic_doc_distribution = new FrovedisRowmajorMatrix(dmat)
    }
    JNISupport.extractTopDocsPerTopic(fs.master_node, model_Id,
                                  sorted_topic_doc_distribution.get(),
                                  k, maxDocumentsPerTopic,
                                  doc_id, topic_doc_dist)
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    val arry = doc_id.grouped(maxDocumentsPerTopic).toArray
    val arrz = topic_doc_dist.grouped(maxDocumentsPerTopic).toArray
    return arry.zip(arrz)
  }
  def topDocumentsPerTopic(documents: RDD[(Long, Vector)],
                           maxDocumentsPerTopic: Int):
                           Array[(Array[Long], Array[Double])] = { // for test document
    val doc_id = documents.map(x => x._1).collect
    val doc_vec = documents.map(x => x._2)
    val save_doc_id = true
    val fdata = new FrovedisSparseData(doc_vec)
    val ret = topDocumentsPerTopic(doc_id, fdata, maxDocumentsPerTopic, 
                                   save_doc_id)
    fdata.release()
    return ret
  }
  def topDocumentsPerTopic(data: FrovedisSparseData, 
                           maxDocumentsPerTopic: Int): 
                           Array[(Array[Long], Array[Double])] = {
    val orig_doc_id = new Array[Long](0)
    val save_doc_id = false
    return topDocumentsPerTopic(orig_doc_id, data, maxDocumentsPerTopic,
                                save_doc_id)
  }
  private def topDocumentsPerTopic(orig_doc_id: Array[Long], data: FrovedisSparseData,
                           maxDocumentsPerTopic: Int, save_doc_id: Boolean=false):
                           Array[(Array[Long], Array[Double])] = { 
    require(maxDocumentsPerTopic > 0 && maxDocumentsPerTopic <= num_docs, 
    s"topDocumentsPerTopic: Input maxDocumentsPerTopic " +
    "must be less than or equal to num_docs: ${num_docs}, but got ${maxDocumentsPerTopic}")
    val fs = FrovedisServer.getServerInstance()
    val doc_id = new Array[Long](k * maxDocumentsPerTopic)
    val topic_doc_dist = new Array[Double](k * maxDocumentsPerTopic)
    JNISupport.transformAndExtractTopDocsPerTopic(fs.master_node,
                                            data.get(),
                                            orig_doc_id,
                                            save_doc_id,
                                            num_docs,
                                            model_Id,
					    maxIterations,
                                            getAlpha, getBeta,
					    num_explore_iter,
                                            algorithm,
                                            k, maxDocumentsPerTopic,
                                            doc_id, topic_doc_dist)
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    val arry = doc_id.grouped(maxDocumentsPerTopic).toArray
    val arrz = topic_doc_dist.grouped(maxDocumentsPerTopic).toArray
    return arry.zip(arrz)
  }
  def topTopicsPerDocument(maxTopics: Int):
                           RDD[(Long, Array[Int], Array[Double])]= { // for training document
    require(maxTopics > 0 && maxTopics <= k, 
    s"topTopicsPerDocument: Input maxTopics " +
    "must be less than or equal to num_topics: ${k}, but got ${maxTopics}")
    val fs = FrovedisServer.getServerInstance()
    val test_doc_id = new Array[Long](num_docs) 
    val topic_id = new Array[Int](num_docs * maxTopics)
    val doc_topic_dist = new Array[Double](num_docs * maxTopics)
    if (this.sorted_doc_topic_distribution == null) {
      val dmat = JNISupport.getDocTopicDistribution(
                                       fs.master_node, 
                                       model_Id, test_doc_id, 
                                       num_docs)
      val info = JNISupport.checkServerException()
      if (info != "") throw new java.rmi.ServerException(info)
      this.sorted_doc_topic_distribution = new FrovedisRowmajorMatrix(dmat)
    }
    JNISupport.extractTopTopicsPerDoc(fs.master_node,
                                  sorted_doc_topic_distribution.get(),
                                  num_docs, maxTopics,
                                  topic_id, doc_topic_dist)
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    val arry = topic_id.grouped(maxTopics).toArray
    val arrz = doc_topic_dist.grouped(maxTopics).toArray
    val zip_pair = arry.zip(arrz)
    val sc = SparkContext.getOrCreate()
    val rdd_pair = sc.parallelize(zip_pair)
    val rdd_test_doc_id = sc.parallelize(test_doc_id)
    var pair: RDD[(Long, Array[Int], Array[Double])] = null
    if (save_doc_id) {
      pair = rdd_pair.zip(rdd_test_doc_id).map { case ((a, b), index) => (index, a, b) } 
    }
    else {
      pair = rdd_pair.zipWithIndex().map { case ((a, b), index) => (index, a, b) } // BUG
    }
    return pair
  }
  def topTopicsPerDocument(documents: RDD[(Long, Vector)],
                           maxTopics: Int):
                           RDD[(Long, Array[Int], Array[Double])]= { // for test document
    val test_doc_id = documents.map(x => x._1).collect
    val doc_vec = documents.map(x => x._2)
    val save_doc_id = true
    val fdata = new FrovedisSparseData(doc_vec)
    val ret = topTopicsPerDocument(test_doc_id, fdata, maxTopics, save_doc_id)
    fdata.release()
    return ret
  }
  def topTopicsPerDocument(data: FrovedisSparseData,maxTopics: Int): 
                                        RDD[(Long, Array[Int], Array[Double])]= {
    val test_doc_id = new Array[Long](0)
    val save_doc_id = false
    return topTopicsPerDocument(test_doc_id,data,maxTopics,save_doc_id)
  }
  private def topTopicsPerDocument(test_doc_id: Array[Long], data: FrovedisSparseData,
                           maxTopics: Int, save_doc_id: Boolean=false):
                           RDD[(Long, Array[Int], Array[Double])]= {
    require(maxTopics > 0 && maxTopics <= k,
    s"topTopicsPerDocument: Input maxTopics " +
    "must be less than or equal to num_topics: ${k}, but got ${maxTopics}")
    val fs = FrovedisServer.getServerInstance()
    val topic_id = new Array[Int](num_docs * maxTopics)
    val doc_topic_dist = new Array[Double](num_docs * maxTopics)
    JNISupport.transformAndExtractTopTopicsPerDoc(fs.master_node,
                                            data.get(), model_Id,
					    maxIterations,
                                            getAlpha, getBeta,
					    num_explore_iter,
                                            algorithm,
                                            num_docs, maxTopics,
                                            topic_id, doc_topic_dist)
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    val arry = topic_id.grouped(maxTopics).toArray
    val arrz = doc_topic_dist.grouped(maxTopics).toArray
    val zip_pair = arry.zip(arrz)
    val sc = SparkContext.getOrCreate()
    val rdd_pair = sc.parallelize(zip_pair)
    val rdd_test_doc_id = sc.parallelize(test_doc_id)
    var pair: RDD[(Long, Array[Int], Array[Double])] = null
    if (save_doc_id) {
      pair = rdd_pair.zip(rdd_test_doc_id).map { case ((a, b), index) => (index, a, b) } 
    }
    else {
      pair = rdd_pair.zipWithIndex().map { case ((a, b), index) => (index, a, b) } // BUG
    }
    return pair
  }
  override def release(): Unit = {
    super.release()
    // --- resetting metadata ---
    ppl = 0.0
    llh = 0.0
    if (sorted_topic_word_distribution != null) {
      sorted_topic_word_distribution.release()
      sorted_topic_word_distribution = null
    }
    if (sorted_doc_topic_distribution != null) {
      sorted_doc_topic_distribution.release()
      sorted_doc_topic_distribution = null
    }
    if (sorted_topic_doc_distribution != null) {
      sorted_topic_doc_distribution.release()
      sorted_topic_doc_distribution = null    
    }
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
    return new LDAModel(model_id, ret.num_topics, ret.vocabsz, ret.num_docs)
  }
}
