package test.scala;

import com.nec.frovedis.Jexrpc.FrovedisServer
import com.nec.frovedis.matrix._
import org.apache.spark.{SparkConf, SparkContext}
import org.apache.spark.rdd.RDD
import org.apache.spark.mllib.linalg.{Vector, Vectors}

object FrovedisMatrixDemo {
  def main(args: Array[String]): Unit = {

    // -------- configurations --------
    val conf = new SparkConf().setAppName("FrovedisMatrixDemo").setMaster("local[2]")
    val sc = new SparkContext(conf)

    // initializing Frovedis server with "personalized command", if provided in command line
    if(args.length != 0) FrovedisServer.initialize(args(0))

    // -------- data loading from sample text file at Spark side (as RDD[Vector]) --------
    val rdd_vec = sc.textFile("./input/mat_4x4")
                    .map(x => Vectors.dense(x.split(' ').map(_.toDouble)))

    val rmat = new FrovedisRowmajorMatrix(rdd_vec)
    println("\nprinting loaded rowmajor matrix: ")
    rmat.debug_print()
    val t_rmat = rmat.transpose()
    println("\nprinting transposed rowmajor matrix: ")
    t_rmat.debug_print()
    rmat.release()
    t_rmat.release()
    
    val cmat = new FrovedisColmajorMatrix(rdd_vec)
    println("\nprinting loaded colmajor matrix: ")
    cmat.debug_print()
    // cmat.transpose() is not yet supported at server side
    cmat.release()
    
    val bmat = new FrovedisBlockcyclicMatrix(rdd_vec)
    println("\nprinting loaded blockcyclic matrix: ")
    bmat.debug_print()
    val t_bmat = bmat.transpose()
    println("\nprinting transposed blockcyclic matrix: ")
    t_bmat.debug_print()
    bmat.release()
    t_bmat.release()
    
    FrovedisServer.shut_down()
    sc.stop()
  }
}
