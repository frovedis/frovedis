import org.apache.spark.{SparkConf, SparkContext}
import org.apache.spark.mllib.util.MLUtils
import org.apache.spark.mllib.linalg.Vectors
import org.apache.spark.mllib.regression.LabeledPoint
import com.nec.frovedis.Jexrpc.FrovedisServer
//import org.apache.spark.mllib.classification.LogisticRegressionWithSGD
import com.nec.frovedis.mllib.classification.LogisticRegressionWithSGD
import org.apache.spark.mllib.evaluation.MulticlassMetrics

object LR extends Serializable {

  def createVector(line: String) : LabeledPoint = {
    val item = line.split(",")
    val vlen = item.length
    var point = Array.ofDim[Double](vlen-2)
    var label = 1.0
    if(item(1) == "B") label = -1.0
    for(i <- 2 until vlen) {
      point(i-2) = item(i).toDouble
    }
    LabeledPoint(label,Vectors.dense(point))
  }

  def main(args: Array[String]): Unit = {
    val conf = new SparkConf().setAppName("LR")
    val sc = new SparkContext(conf)
    val data=sc.textFile("file://" + sys.env("INSTALLPATH") + "/x86/doc/tutorial_spark/src/tut3/wdbc.data").map(createVector)
    FrovedisServer.initialize("mpirun -np 4 " + sys.env("FROVEDIS_SERVER"))
    val model = LogisticRegressionWithSGD.train(data,1000)
    val points = data.map(_.features)
    val predicted = model.predict(points).collect()
    val lbl = data.map(_.label).collect()
    val predictionAndLabels = predicted.zip(lbl)
    val metrics = new MulticlassMetrics(sc.parallelize(predictionAndLabels,1))
    val accuracy = metrics.accuracy
    printf("Accuracy = %.4f\n", accuracy)
    FrovedisServer.shut_down()
  }
}
