package com.nec.frovedis.mllib.feature;

import scala.collection.mutable
import java.util.Arrays

import com.nec.frovedis.Jexrpc.{FrovedisServer,JNISupport}
import com.nec.frovedis.matrix.{IntDvector, FrovedisRowmajorMatrix}
import com.nec.frovedis.mllib.{GenericModel,ModelID,M_KIND}

import org.apache.spark.{SparkConf, SparkContext}
import org.apache.spark.rdd.RDD

// This implementation is a wrapper of Spark Word2Vector,
// Vocabulary creation part is done at Spark client side,
// The Hash values along with the count values of the created vocabs are passed
// to Frovedis server for the training to be performed on VE
// It is possible to convert the in memory Frovedis Word2Vec model 
// to corresponding Spark model.
// Ref: https://github.com/apache/spark/blob/master/mllib/src/main/scala/org/apache/spark/mllib/feature/Word2Vec.scala

// For making entry in Vocabulary
private case class VocabWord(
  var word: String,
  var cn: Int
)

class Word2Vec extends java.io.Serializable {

  // data members in native spark implementation 
  private var vectorSize = 100         // hidden_size: size of word vectors
  private var learningRate = 0.025     // alpha: starting learning rate
  private var numPartitions = 1        // not supported in frovedis: internally skipped
  private var numIterations = 1        // iter: Number of training iterations
  private var minCount = 5             // min_count: discard words that appear less than the times
  private var maxSentenceLength = 1000 // not supported in frovedis: internally skipped
  private var window = 5               // window: max skip length between words
  private var seed = 0L                // not supported in frovedis: internally skipped

  // data members in frovedis specific implementation
  private var threshold: Float = 1e-3f // threshold for occurrence of words [useful range is (0, 1e-5)]
  private var negative: Int = 5        // number of negative examples
  private var modelSyncPeriod = 0.1f   // synchronize model every specified seconds
  private var minSyncWords: Int = 1024 // minimal number of words to be synced at each model sync
  private var fullSyncTimes: Int = 0   // enforced full model sync-up times during training
  private var messageSize: Int = 1024  // MPI message chunk size in MB
  private var numThreads: Int = 0      // Number of threads, 0 is to be used for all available cores

  // member setter methods in native spark implementation
  def setMaxSentenceLength(maxSentenceLength: Int): this.type = {
    require(maxSentenceLength > 0,
      s"Maximum length of sentences must be positive but got ${maxSentenceLength}")
    this.maxSentenceLength = maxSentenceLength
    this
  }
  def setVectorSize(vectorSize: Int): this.type = {
    require(vectorSize > 0,
      s"vector size must be positive but got ${vectorSize}")
    this.vectorSize = vectorSize
    this
  }
  def setLearningRate(learningRate: Double): this.type = {
    require(learningRate > 0,
      s"Initial learning rate must be positive but got ${learningRate}")
    this.learningRate = learningRate
    this
  }
  def setNumPartitions(numPartitions: Int): this.type = {
    require(numPartitions > 0,
      s"Number of partitions must be positive but got ${numPartitions}")
    this.numPartitions = numPartitions
    this
  }
  def setNumIterations(numIterations: Int): this.type = {
    require(numIterations >= 0,
      s"Number of iterations must be nonnegative but got ${numIterations}")
    this.numIterations = numIterations
    this
  }
  def setSeed(seed: Long): this.type = {
    this.seed = seed
    this
  }
  def setWindowSize(window: Int): this.type = {
    require(window > 0,
      s"Window of words must be positive but got ${window}")
    this.window = window
    this
  }
  def setMinCount(minCount: Int): this.type = {
    require(minCount >= 0,
      s"Minimum number of times must be nonnegative but got ${minCount}")
    this.minCount = minCount
    this
  }

  // member setter methods for frovedis specific implementation
  def setThreshold(threshold: Float): this.type = {
    this.threshold = threshold // can be any value [useful range is (0, 1e-5)]
    this
  }
  def setNegative(negative: Int): this.type = {
    require(negative >= 0,
      s"Number of negative examples must be nonnegative but got ${negative}")
    this.negative = negative
    this
  }
  def setModelSyncPeriod(modelSyncPeriod: Float): this.type = {
    require(modelSyncPeriod > 0,
      s"Specified seconds for model sync must be positive but got ${modelSyncPeriod}")
    this.modelSyncPeriod = modelSyncPeriod 
    this
  }
  def setMinSyncWords(minSyncWords: Int): this.type = {
    require(minSyncWords >= 0,
      s"Minimum number of words to be synced must be nonnegative but got ${minSyncWords}")
    this.minSyncWords = minSyncWords 
    this
  }
  def setFullSyncTimes(fullSyncTimes: Int): this.type = {
    require(fullSyncTimes >= 0,
      s"Enforced full model sync-up times during training must be nonnegative but got ${fullSyncTimes}")
    this.fullSyncTimes = fullSyncTimes
    this
  }
  def setMessageSize(messageSize: Int): this.type = {
    require(messageSize > 0,
      s"MPI message chunk size (in MB) must be positive but got ${messageSize}")
    this.messageSize = messageSize
    this
  }
  def setNumThreads(numThreads: Int): this.type = {
    require(numThreads > 0,
      s"Number of threads must be nonnegative but got ${numThreads}")
    this.numThreads = numThreads
    this
  }

  private def learnVocab(words: RDD[String]): Array[VocabWord] = { 
    val sorted_vocab = words.map(w => (w, 1))
      .reduceByKey(_ + _)
      .filter(_._2 >= minCount)
      .map(x => VocabWord(x._1, x._2))
      .collect()
      .sortWith((a, b) => a.cn > b.cn)
    val vocab = VocabWord("</s>",0) +: sorted_vocab // addToVocab as in cpp implementation
    return vocab
  }

  private def getStream(vocabHash: mutable.HashMap[String,Int],
                        word: String): Int = {
    if (vocabHash.contains(word)) vocabHash(word) else -1
  }

  def fit(dataset: RDD[Array[String]]): Word2VecModel = {
    val fs = FrovedisServer.getServerInstance()
    setNumPartitions(fs.worker_size)

    val words = dataset.flatMap(x => x)
    val vocabWordsArr = learnVocab(words)

    val vocabSize = vocabWordsArr.size.intValue
    require(vocabSize > 0, "The vocabulary size should be > 0. You may need to check " +
      "the setting of minCount, which could be large enough to remove all your words in sentences.")

    var vocabHash = mutable.HashMap.empty[String,Int]
    var vocab_count = new Array[Int](vocabSize)
    var vocab_word = new Array[String](vocabSize)
    //println ("--- created vocab/count pair ---")
    for (i <- 0 to (vocabSize - 1)) {
      vocabHash += vocabWordsArr(i).word -> i
      vocab_word(i) = vocabWordsArr(i).word
      vocab_count(i) = vocabWordsArr(i).cn
      //println("(" + vocabWordsArr(i).word + "," + vocabWordsArr(i).cn + ")" )
    }
    //println ("--- created vocab/hash map ---")
    //vocabHash.foreach(println)
    val vocab_stream = words.map(w => getStream(vocabHash,w)).filter(w => w != -1)
    val extra = words.context.parallelize(Array(vocabHash("</s>"))) // as in cpp implementation
    val vocab_stream_with_extra = vocab_stream.union(extra)
    //println("--- stream ---")
    //vocab_stream_with_extra.collect().foreach(println)
    // spark RDD[Int] -> frovedis::dvector<int>     
    val vocab_stream_dv = IntDvector.get(vocab_stream_with_extra.repartition(numPartitions))
    // --- training at frovedis server ---
    val mid = ModelID.get()
    JNISupport.callFrovedisW2V(fs.master_node, 
                               vocab_stream_dv, vocab_count, vocabSize,
                               vectorSize, window, threshold,
                               negative, numIterations, learningRate,
                               modelSyncPeriod, minSyncWords, fullSyncTimes, 
                               messageSize, numThreads, mid)
    val info = JNISupport.checkServerException();
    if (info != "") throw new java.rmi.ServerException(info);
    return new Word2VecModel(mid).setVocabWords(vocab_word)
  }
}

class Word2VecModel(model_Id: Int) 
  extends GenericModel(model_Id, M_KIND.W2V) {
  private var vocab: Array[String] = null
  def setVocabWords(vocab: Array[String]) = {
    this.vocab = vocab
    this
  }
  override def debug_print(): Unit = {
    if(vocab != null) {
      println("vocab: ")
      for(v <- vocab) println(v)
      val fs = FrovedisServer.getServerInstance()
      println("weight: ")
      JNISupport.showW2VWeight(fs.master_node,mid)
      val info = JNISupport.checkServerException()
      if (info != "") throw new java.rmi.ServerException(info)
    }
    else throw new NullPointerException("model instance is empty!")
  }
  override def save(sc: SparkContext, path: String) : Unit = save(path)
  override def save(path: String) : Unit = {
    if(vocab != null) {
      val fs = FrovedisServer.getServerInstance()
      val vocab_size = vocab.size.intValue
      JNISupport.saveW2VModel(fs.master_node,mid,vocab,vocab_size,path) 
      val info = JNISupport.checkServerException()
      if (info != "") throw new java.rmi.ServerException(info)
    }
    else throw new NullPointerException("model instance is empty!")
  }
  def get_weight_matrix(): FrovedisRowmajorMatrix = {
    if(vocab != null) {
      val fs = FrovedisServer.getServerInstance()
      val dmat = JNISupport.getW2VWeightPointer(fs.master_node, mid);
      val info = JNISupport.checkServerException()
      if (info != "") throw new java.rmi.ServerException(info)
      return new FrovedisRowmajorMatrix(dmat)
    }
    else throw new NullPointerException("model instance is empty!")
  }
  def to_spark_model(sc: SparkContext): org.apache.spark.mllib.feature.Word2VecModel = {
    if(vocab != null) {
      val fs = FrovedisServer.getServerInstance()
      val weight_vec = JNISupport.getW2VWeight(fs.master_node, mid)
      val info = JNISupport.checkServerException()
      if (info != "") throw new java.rmi.ServerException(info)
      val vocab_size = vocab.size.intValue
      val weight_size = weight_vec.size.intValue
      require(vocab_size > 0, "Word2VecModel vocab size should be positive, but got ${vocab_size}")
      require(weight_size > 0, "Word2VecModel weight size should be positive, but got ${weight_size}")
      val hidden_size = weight_size / vocab_size
      //println("vocab_size: " + vocab_size)
      //println("hidden_size: " + hidden_size)
      //println("weight_vec_size: " + weight_vec.size)
      val weight = new Array[Array[Float]](vocab_size)
      for(i <- 0 to (vocab_size - 1)) {
        var st = i * hidden_size
        var end = st + hidden_size
        weight(i) = Arrays.copyOfRange(weight_vec, st, end)
      }
      val model_map = vocab.zip(weight).toMap
      return new org.apache.spark.mllib.feature.Word2VecModel(model_map)
    }
    else throw new NullPointerException("model instance is empty!")
  }
}
