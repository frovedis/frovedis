package test.scala;

import com.nec.frovedis.Jexrpc.FrovedisServer
import com.nec.frovedis.mllib.clustering.{LDA,LDAModel}
import org.apache.spark.{SparkConf, SparkContext}
import org.apache.spark.mllib.linalg.Vectors
import com.nec.frovedis.matrix.FrovedisRowmajorMatrix
import com.nec.frovedis.exrpc.FrovedisSparseData
import org.apache.log4j.{Level, Logger}

object LDADemo {
  def main(args: Array[String]): Unit = {
    Logger.getLogger("org").setLevel(Level.ERROR)

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
    val vec = Vectors.dense(1.1,1.1,1.1,1.1,1.1) 
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

    val ldaModel = lda.setK(5)
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
    println("ldaModel.describeTopics(3)")
    val mm1 = ldaModel.describeTopics(3)
    for(e<-mm1) { for(a<-e._1) print(a+" "); println(); for(b<-e._2) print(b+" "); println();  }
    println("ldaModel.topDocumentsPerTopic(4):")
    val mm2 = ldaModel.topDocumentsPerTopic(4)
    for(e<-mm2) { for(a<-e._1) print(a+" "); println(); for(b<-e._2) print(b+" "); println();  }
    println("ldaModel.topTopicsPerDocument(3):")

    val mm3 = ldaModel.topTopicsPerDocument(3)
    for(e<-mm3) { for(b<-e._2) print(b+" "); println(); for(c<-e._3) print(c+" "); println();}
    println("ldaModel.topDocumentsPerTopic(corpus,2):")
    val mm4 = ldaModel.topDocumentsPerTopic(corpus,2)
    for(e<-mm4) { for(a<-e._1) print(a+" "); println(); for(b<-e._2) print(b+" "); println();  }
    println("ldaModel.topTopicsPerDocument(corpus,4):")
    val mm5 = ldaModel.topTopicsPerDocument(corpus,4)
    for(e<-mm5) { for(b<-e._2) print(b+" "); println(); for(c<-e._3) print(c+" "); println();}
    println("ldaModel.topicDistributions(corpus).save(rmm_topicDistributions): " 
                          + ldaModel.topicDistributions(corpus).save("rmm_topicDistributions"))
    ldaModel.save(sc, "./out/LDAModel")
    ldaModel.release() 
    LDAModel.load(sc, "./out/LDAModel") 

    FrovedisServer.shut_down()
    sc.stop()
  }
}
