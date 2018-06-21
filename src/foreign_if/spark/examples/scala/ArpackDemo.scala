package test.scala;

import com.nec.frovedis.Jexrpc.FrovedisServer
import com.nec.frovedis.matrix._
import org.apache.spark.{SparkConf, SparkContext}
import org.apache.spark.rdd.RDD
import org.apache.spark.mllib.linalg.{Vector, Vectors}
import org.apache.spark.mllib.linalg.SingularValueDecomposition
import org.apache.spark.mllib.linalg.Matrix
import org.apache.spark.mllib.linalg.distributed.RowMatrix

object ArpackDemo {
  def main(args: Array[String]): Unit = {

    // -------- configurations --------
    val conf = new SparkConf().setAppName("ArpackDemo").setMaster("local[2]")
    val sc = new SparkContext(conf)

    // initializing Frovedis server with "personalized command", if provided in command line
    if(args.length != 0) FrovedisServer.initialize(args(0))

    val data = Array(Vectors.sparse(5, Seq((1, 1.0), (3, 7.0))),
                     Vectors.dense(2.0, 0.0, 3.0, 4.0, 5.0),
                     Vectors.dense(2.0, 0.0, 3.0, 4.0, 5.0),
                     Vectors.dense(4.0, 0.0, 0.0, 6.0, 7.0))

    val rows = sc.parallelize(data)
    val mat: RowMatrix = new RowMatrix(rows)

    // (SPARK WAY) Compute the top 3 singular values and corresponding singular vectors.
    val svd1 = mat.computeSVD(3, computeU = true)
    val U1: RowMatrix = svd1.U  // The U factor is a RowMatrix.
    val s1: Vector = svd1.s     // The singular values are stored in a local dense vector.
    val V1: Matrix = svd1.V     // The V factor is a local dense matrix.
    println("svec: ")
    println(s1.toString)
    println("umat: ")
    U1.rows.collect.foreach(println)
    println("vmat: ")
    println(V1.toString)

    // (FROVEDIS WAY) Compute the top 3 singular values and corresponding singular vectors.
    val res = ARPACK.computeSVD(mat,3)
    val svd2 = res.to_spark_result(sc)
    val U2: RowMatrix = svd2.U  // The U factor is a RowMatrix.
    val s2: Vector = svd2.s     // The singular values are stored in a local dense vector.
    val V2: Matrix = svd2.V     // The V factor is a local dense matrix.
    println("svec: ")
    println(s2.toString)
    println("umat: ")
    U2.rows.collect.foreach(println)
    println("vmat: ")
    println(V2.toString)
    res.release()

    FrovedisServer.shut_down()
    sc.stop()
  }
}
