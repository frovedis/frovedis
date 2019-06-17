import com.nec.frovedis.Jexrpc.FrovedisServer
import com.nec.frovedis.exrpc.FrovedisSparseData
import org.apache.spark.{SparkConf, SparkContext}
import org.apache.spark.mllib.util.MLUtils

object DataTransfer extends Serializable {
  def main(args: Array[String]): Unit = {
    val conf = new SparkConf().setAppName("DataTransfer").setMaster("local[2]")
    val sc = new SparkContext(conf)
    FrovedisServer.initialize("mpirun -np 2 " + sys.env("FROVEDIS_SERVER"))
    val data = MLUtils.loadLibSVMFile(sc, "file://" + sys.env("INSTALLPATH") + "/x86/doc/tutorial_spark/src/tut5-1/libSVMFile.txt")
    val fdata = new FrovedisLabeledPoint(data)
    fdata.debug_print()
    fdata.release()
    val mat = data.map(_.features)
    val fmat = new FrovedisSparseData(mat)
    fmat.debug_print() 
    fmat.release()
    FrovedisServer.shut_down()
  }
}
