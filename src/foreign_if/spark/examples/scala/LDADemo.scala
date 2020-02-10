package test.scala;

import com.nec.frovedis.Jexrpc.FrovedisServer
import com.nec.frovedis.mllib.clustering.{LDA,LDAModel}
import org.apache.spark.{SparkConf, SparkContext}
import org.apache.spark.mllib.linalg.Vectors
import com.nec.frovedis.matrix.FrovedisRowmajorMatrix
import com.nec.frovedis.exrpc.FrovedisSparseData

object LDADemo {
  def main(args: Array[String]): Unit = {
    // -------- configurations --------
    val conf = new SparkConf().setAppName("SPAExample").setMaster("local[2]")
    val sc = new SparkContext(conf)

    // initializing Frovedis server with "personalized command", if provided in command line
    if(args.length != 0) FrovedisServer.initialize(args(0))

    // Load and parse the data
    val data = sc.textFile("./input/sample_lda_data.txt")
    val parsedData = data.map(s => Vectors.dense(s.trim.split(' ').map(_.toDouble)))
    // Index documents with unique IDs
    val doc = parsedData.zipWithIndex.map(_.swap).cache()
    val corp = doc.map(x => x._2)
    val corpus = new FrovedisSparseData(corp)
    
    //---LDA train---
    val vec = Vectors.dense(1.1,1.1,1.1) 
    // Cluster the documents into three topics using LDA
    val lda = new LDA()
    println("getDocConcentration: " + lda.getDocConcentration)
    println("lda.getK: " + lda.getK)
    println("lda.getMaxIterations: " + lda.getMaxIterations)
    println("lda.getDocConcentration: " + lda.getDocConcentration)
    println("lda.getTopicConcentration: " + lda.getTopicConcentration)
    println("lda.getCheckpointInterval: " + lda.getCheckpointInterval)
    println("lda.getSeed: " + lda.getSeed)
    println("lda.getOptimizer: " + lda.getOptimizer)
    println("lda.getNumevalcycle: " + lda.getNumevalcycle)
    println("lda.getNumexploreiter: " + lda.getNumexploreiter)
    println("lda.getAlgorithm: " + lda.getAlgorithm)

    val ldaModel = lda.setK(3)
                      .setMaxIterations(10)
                      .setDocConcentration(vec)
                      .setTopicConcentration(0.1)
                      .setCheckpointInterval(100)
                      .setSeed(10)
                      .setOptimizer("em")
                      .setNumevalcycle(1)
                      .setNumexploreiter(1)
                      .setAlgorithm("original")
                      .run(corpus)

    println("ldaModel.vocabSize: " + ldaModel.vocabSize)
    println("topicMatrix: " + ldaModel.topicsMatrix)
    println("ldaModel.transform(corpus): " + ldaModel.transform(corpus))
    println("ldaModel.logLikelihood is: " + ldaModel.logLikelihood(corpus))
    println("ldaModel.logPerplexity is: " + ldaModel.logPerplexity(corpus))
    println("ldaModel.describeTopics")
    val mm = ldaModel.describeTopics
    for(e<-mm) { for(a<-e._1) print(a+" "); println(); for(b<-e._2) print(b+" "); println();  }
    ldaModel.save(sc, "./out/LDAModel")
    val mm1 = ldaModel.describeTopics(3)
    for(e<-mm1) { for(a<-e._1) print(a+" "); println(); for(b<-e._2) print(b+" "); println();  }
    ldaModel.release() 
    LDAModel.load(sc, "./out/LDAModel") 

    FrovedisServer.shut_down()
    sc.stop()
  }
}
