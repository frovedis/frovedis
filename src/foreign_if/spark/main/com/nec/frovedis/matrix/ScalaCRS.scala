package com.nec.frovedis.matrix

import scala.collection.mutable.ArrayBuffer
import org.apache.spark.mllib.linalg.SparseVector

class ScalaCRS extends java.io.Serializable {
  var nrows: Long = 0
  var ncols: Long = 0
  var off  = new ArrayBuffer[Int]()
  var idx  = new ArrayBuffer[Int]()
  var data = new ArrayBuffer[Double]()
  off += 0 // 0 based offset
 
  def this(input: Array[SparseVector]) = {
    this()
    nrows = input.length
    ncols = if (nrows > 0) input(0).size else 0
    for (i <- 0 to (input.length-1)) {
      val indx = input(i).indices
      val vals = input(i).values
      if (indx.length != vals.length) println ("Input error in ScalaCRS")
      // Note: Spark reduces the index count by 1, while parsing libSVMFile
      // Whereas, Frovedis read the file as it is...
      // This issue needs to be fixed in Frovedis side!!
      for(j <- 0 to (indx.length-1)) idx  += indx(j)
      for(j <- 0 to (vals.length-1)) data += vals(j)
      val tmp = off(i) + indx.length
      off += tmp
    }
  }

  def debug_print() : Unit = {
    println("nrows: " + nrows + " ncols: " + ncols)
    for (o <- off)  print(o + " "); println
    for (i <- idx)  print(i + " "); println
    for (d <- data) print(d + " "); println
  }
}
