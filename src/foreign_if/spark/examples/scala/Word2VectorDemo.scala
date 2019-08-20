package test.scala;

import com.nec.frovedis.Jexrpc.FrovedisServer
import com.nec.frovedis.mllib.feature.Word2Vec
import org.apache.spark.{SparkConf, SparkContext}

object Word2VecDemo {
  def main(args: Array[String]): Unit = {

    // -------- configurations --------
    val conf = new SparkConf().setAppName("Word2VecExample").setMaster("local[2]")
    val sc = new SparkContext(conf)

    // initializing Frovedis server with "personalized command", if provided in command line
    if(args.length != 0) FrovedisServer.initialize(args(0))

    val input = sc.textFile("./input/text8-10k").map(line => line.split(" "))

    // --- frovedis from spark call ---
    val word2vec = new Word2Vec().setVectorSize(256)
                                 .setNumThreads(1)
                                 .setThreshold(1e-4f)
                                 .setNumIterations(5)
                                 .setLearningRate(0.05)
    
    val model = word2vec.fit(input)
    //model.debug_print()

    // wmat: FrovedisRowmajorMatrix (can be used for ML algorithms)
    val wmat = model.get_weight_matrix()
    //wmat.debug_print()
    model.save("./out/W2VModel")

    var synonyms = model.to_spark_model(sc).findSynonyms("the", 5)
    for((synonym, cosineSimilarity) <- synonyms) {
      println(s"$synonym $cosineSimilarity")
    }

    // --- native spark call ---
    val word2vec_sp = new org.apache.spark.mllib.feature
                             .Word2Vec().setVectorSize(256)
                                        .setNumIterations(5)
                                        .setLearningRate(0.05)
    val model_sp = word2vec_sp.fit(input.map(x => x.toSeq))
    synonyms = model_sp.findSynonyms("the", 5)
    for((synonym, cosineSimilarity) <- synonyms) {
      println(s"$synonym $cosineSimilarity")
    }

    FrovedisServer.shut_down()
    sc.stop()
  }
}
