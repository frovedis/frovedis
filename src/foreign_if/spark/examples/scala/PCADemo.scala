package test.scala;

import com.nec.frovedis.Jexrpc.FrovedisServer
import com.nec.frovedis.matrix.RowMatrixUtils._
import org.apache.spark.{SparkConf, SparkContext}
import org.apache.spark.rdd.RDD
import org.apache.spark.mllib.linalg.{Vector, Vectors}
import org.apache.spark.mllib.linalg.Matrix
import org.apache.spark.mllib.linalg.distributed.RowMatrix

object PCADemo {
  def main(args: Array[String]): Unit = {

    // -------- configurations --------
    val conf = new SparkConf().setAppName("PCADemo").setMaster("local[2]")
    val sc = new SparkContext(conf)

    // initializing Frovedis server with "personalized command", if provided in command line
    if(args.length != 0) FrovedisServer.initialize(args(0))

    val data = Array(Vectors.dense(1.0, 0.0, 7.0, 0.0, 0.0),
                     Vectors.dense(2.0, 0.0, 3.0, 4.0, 5.0),
                     Vectors.dense(4.0, 0.0, 0.0, 6.0, 7.0))
    val rows = sc.parallelize(data)
    val mat: RowMatrix = new RowMatrix(rows)

    // (SPARK WAY) Compute the top 2 principal components.
    println("\nComputation using Spark native APIs:")
    val s_pc1 = mat.computePrincipalComponents(2)
    println("Principal Components: ")
    println(s_pc1.toString)

    // with variance
    println("\nWith variance: ")
    val (s_pc2,s_var) = mat.computePrincipalComponentsAndExplainedVariance(2) 
    println("Principal Components: ")
    println(s_pc2.toString)
    println("Variance: ")
    println(s_var.toString)

    // (FROVEDIS WAY) Compute the top 2 principal components.
    println("\n\nComputation using Frovedis APIs getting called from Spark client:")
    // API as a member method of Spark RowMatrix
    val res1 = mat.computePrincipalComponentsUsingFrovedis(2) // res: Frovedis side result pointer
    // can also be called as a method of RowMatrixUtils
    // import com.nec.frovedis.matrix.RowMatrixUtils
    // val res1 = RowMatrixUtils.computePrincipalComponents(mat,2) // res: Frovedis side result pointer
    val f_pc1 = res1.to_spark_result()._1
    println("Principal Components: ")
    println(f_pc1.toString)

    // with variance
    println("\nWith variance: ")
    val res2 = mat.computePrincipalComponentsAndExplainedVarianceUsingFrovedis(2) 
    //val res2 = RowMatrixUtils.computePrincipalComponentsAndExplainedVariance(mat,2) 
    val (f_pc2, f_var) = res2.to_spark_result()
    println("Principal Components: ")
    println(f_pc2.toString)
    println("Variance: ")
    println(f_var.toString)

    FrovedisServer.shut_down()
    sc.stop()
  }
}
