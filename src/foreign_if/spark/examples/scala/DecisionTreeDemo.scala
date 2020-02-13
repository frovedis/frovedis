package test.scala;

import com.nec.frovedis.Jexrpc.FrovedisServer
import com.nec.frovedis.mllib.tree.{DecisionTree, DecisionTreeModel}
import org.apache.spark.{SparkConf, SparkContext}
import org.apache.spark.mllib.util.MLUtils
import org.apache.spark.mllib.regression.LabeledPoint
import org.apache.log4j.{Level, Logger}

object DecisionTreeDemo {
  def main(args: Array[String]): Unit = {
    Logger.getLogger("org").setLevel(Level.ERROR)

    // -------- configurations --------
    val conf = new SparkConf().setAppName("DecisionTreeDemo").setMaster("local[2]")
    val sc = new SparkContext(conf)

    // initializing Frovedis server with "personalized command", if provided in command line
    if(args.length != 0) FrovedisServer.initialize(args(0))

    // -------- data loading from sample libSVM file at Spark side--------
    var data = MLUtils.loadLibSVMFile(sc, "./input/libSVMFile.txt")
    data = data.map(x => LabeledPoint(x.label,x.features.toDense)) // only supports dense at this moment
    
    val numClasses = 2
    val categoricalFeaturesInfo = Map[Int, Int]()
    var impurity = "gini"
    val maxDepth = 5
    val maxBins = 32
    // -------- train classifier --------
    val model1 = DecisionTree.trainClassifier(data,
                                              numClasses, 
                                              categoricalFeaturesInfo,
                                              impurity, maxDepth, maxBins)
    model1.debug_print()
    
    // -------- prediction --------
    val predictionAndLabel = data.map(p => (model1.predict(p.features), p.label))
    val accuracy = 1.0 * predictionAndLabel.filter(x => x._1 == x._2).count() / data.count()
    println("Test accuracy: " + accuracy)

    // -------- load/save --------
    model1.save(sc, "./out/DecisionTreeClassModel")

    // -------- clean-up --------
    model1.release() 

    FrovedisServer.shut_down()
    sc.stop()
  }
}

