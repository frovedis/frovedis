package test.scala;

import com.nec.frovedis.Jexrpc.FrovedisServer
import com.nec.frovedis.mllib.regression.{LinearRegressionWithLBFGS,LinearRegressionModel}
import org.apache.spark.{SparkConf, SparkContext}
import org.apache.spark.mllib.util.MLUtils
import org.apache.spark.mllib.linalg.Vectors
import org.apache.spark.mllib.regression.LabeledPoint

// OBJECTIVE: To test Frovedis scala wrapper of Linear regression with lbfgs optimizer for spase and dense data

object GenericTest {

  def main(args: Array[String]): Unit = {

    // -------- configurations --------
    val conf = new SparkConf().setAppName("GenericTest").setMaster("local[2]")
    val sc = new SparkContext(conf)

    // initializing Frovedis server with "personalized command", if provided in command line
    if(args.length != 0) FrovedisServer.initialize(args(0))
    // -------- create dense and sparse data--------
    val vec1 = Vectors.dense(1.0,0.0,0.0,1.0,1.0,0.0)
    val vec2 = Vectors.dense(1.0,0.0,1.0,0.0,1.0,0.0)
    val vec3 = Vectors.dense(1.0,0.0,0.0,1.0,1.0,0.0)
    val vec4 = Vectors.dense(1.0,0.0,1.0,0.0,1.0,0.0)
    val dense_data = sc.parallelize(Array(LabeledPoint(1,vec1),
                                          LabeledPoint(0,vec2),
                                          LabeledPoint(1,vec3),
                                          LabeledPoint(0,vec4)))

    val sparse_data = sc.parallelize(Array(LabeledPoint(1,vec1.toSparse),
                                           LabeledPoint(0,vec2.toSparse),
                                           LabeledPoint(1,vec3.toSparse),
                                           LabeledPoint(0,vec4.toSparse)))
    // -------- training --------
    val m1 = LinearRegressionWithLBFGS.train(dense_data)
    val m2 = LinearRegressionWithLBFGS.train(sparse_data)
  
    // -------- prediction --------
    val tdata = sparse_data.map(_.features) 
    val pd1 = m1.predict(tdata).collect
    val pd2 = m2.predict(tdata).collect

    if (pd1.deep == pd2.deep) println("Status: Passed")
    else println("Status: Failed")
  
    // -------- clean-up --------
    m1.release() 
    m2.release() 

    FrovedisServer.shut_down()
    sc.stop()
  }
}



