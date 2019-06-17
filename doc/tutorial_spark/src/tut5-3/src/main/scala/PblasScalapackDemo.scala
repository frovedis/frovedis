import com.nec.frovedis.Jexrpc.FrovedisServer
import com.nec.frovedis.matrix._
import org.apache.spark.{SparkConf, SparkContext}
import org.apache.spark.rdd.RDD
import org.apache.spark.mllib.linalg.{Vector, Vectors}

object PblasScalapackDemo extends Serializable {
  def main(args: Array[String]): Unit = {

    val conf = new SparkConf().setAppName("PblasScalapackDemo").setMaster("local[2]")
    val sc = new SparkContext(conf)

    FrovedisServer.initialize("mpirun -np 2 " + sys.env("FROVEDIS_SERVER"))
    val input = "file://" + sys.env("INSTALLPATH") + "/x86/doc/tutorial_spark/src/tut5-3/"
    val rdd_mat1 = sc.textFile(input + "mat_4x4").map(x => Vectors.dense(x.split(' ').map(_.toDouble)))
    val rdd_mat2 = sc.textFile(input + "mat_4x4").map(x => Vectors.dense(x.split(' ').map(_.toDouble)))

    // --- gemm ---
    val bmat1 = new FrovedisBlockcyclicMatrix(rdd_mat1)
    val bmat2 = new FrovedisBlockcyclicMatrix(rdd_mat2)
    println("\ncalling gemm for input matrices: ")
    println("mat1: ")
    bmat1.get_rowmajor_view()
    println("mat2: ")
    bmat2.get_rowmajor_view()
    val gemm_ret = PBLAS.gemm(bmat1,bmat2)
    gemm_ret.get_rowmajor_view()
    println("saving result to text file: ./mm_4x4")
    gemm_ret.save("./out_mm_4x4") // result can be saved in text file
    bmat1.release()
    bmat2.release()
    gemm_ret.release()

    // --- dot ---
    val rdd_vec1 = sc.textFile(input + "mat_4x1").map(x => Vectors.dense(x.split(' ').map(_.toDouble))) // RDD[Vector]
    val rdd_vec2 = sc.textFile(input + "mat_4x1").map(x => Vectors.dense(x.split(' ').map(_.toDouble))) // RDD[Vector]
    val bvec1 = new FrovedisBlockcyclicMatrix(rdd_vec1)
    val bvec2 = new FrovedisBlockcyclicMatrix(rdd_vec2)
    println("\ncalling dot for input vectors: ")
    println("vec1: ")
    bvec1.to_spark_Vector().toArray.foreach(println)
    println("vec2: ")
    bvec2.to_spark_Vector().toArray.foreach(println)
    val dot_ret = PBLAS.dot(bvec1,bvec2)
    println("dot: " + dot_ret)

    // --- scal ---
    println("\nbefore scaling: ")
    bvec1.get_rowmajor_view()
    PBLAS.scal(bvec1,2)
    println("after scaling with 2 (rowmajor view): ")
    bvec1.get_rowmajor_view()
    println("after scaling with 2 (spark vector view): ")
    bvec1.to_spark_Vector().toArray.foreach(println)  // converting to spark Vector and then printing
    bvec1.release()

    // --- nrm2 ---
    println("\nnrm2 input vector: ")
    bvec2.get_rowmajor_view()
    val norm = PBLAS.nrm2(bvec2)
    println("norm: " + norm)
    bvec2.release()
   
    // --- loading matrix from file --- 
    println("\nloading blockcyclic matrix from file: ./input/mat_3x3")
    bmat2.load(input + "mat_3x3") // matrix can directly be loaded from text file
    println("printing loaded blockcyclic matrix:")
    bmat2.debug_print() 
    println("printing loaded blockcyclic matrix in rowmajor order:")
    bmat2.get_rowmajor_view()

    // --- getrf, getri ---
    val rf = ScaLAPACK.getrf(bmat2)
    println("\ngetrf info: " + rf.stat())
    val stat = ScaLAPACK.getri(bmat2,rf.ipiv())
    println("getri info: " + stat)
    println("saving result to text file: ./inv_mat_3x3")
    bmat2.save("./out_inv_3x3") // result can be saved in text file
    rf.release()
    bmat2.release()

    // --- gesvd ---
    println("\nloading blockcyclic matrix from file: ./input/svd_input_4x4")
    bmat2.load(input + "/svd_input_4x4") 
    println("printing loaded blockcyclic matrix in rowmajor order:")
    bmat2.get_rowmajor_view()
    val svd_ret = ScaLAPACK.gesvd(bmat2) // want both U and V, along with s

    println("\ngesvd info: " + svd_ret.stat())
    println("saving svd results: ")
    // sfile name is mandatory... ufile/vfile can be null if not required to be saved
    svd_ret.save("./out_svd_sfile", "./out_svd_ufile",
                 "./out_svd_vfile")

    println("\nreleasing frovedis side svd input and result data")
    svd_ret.release()
    bmat2.release()

    println("\nloading the same svd results from saved files as blockcyclic matrix (umat/mat): ")
    // sfile name is mandatory... ufile/vfile can be null if not required to be loaded
    svd_ret.load_as_blockcyclic("./out_svd_sfile","./out_svd_ufile",
                                "./out_svd_vfile")
    println("printing the loaded svd results: ")
    svd_ret.debug_print()

    println("\nconverting to spark result: ")
    val sp = svd_ret.to_spark_result(sc)
    println("printing converted spark results: ")
    println("svec: ")
    sp.s.toArray.foreach(println)
    println("umat: ")
    sp.U.rows.collect.foreach(println)
    println("vmat: ")
    sp.V.toArray.foreach(println)

    // once released printing/saving results will be no-operation
    svd_ret.release()
    svd_ret.debug_print()
    svd_ret.save("nothing",null,null)

    FrovedisServer.shut_down()
    sc.stop()
  }
}
