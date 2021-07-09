package com.nec.frovedis.matrix

import org.apache.spark.mllib.linalg.{Vectors, Vector}

class ScalaCRS extends java.io.Serializable {
  var nrows: Long = 0
  var ncols: Long = 0
  var off: Array[Int] = null
  var idx: Array[Int] = null
  var data: Array[Double] = null
 
  def this(input_arr: Array[Vector],
           nc: Long) = {
    this()
    val input = input_arr.map(p => p.toSparse)
    nrows = input.length
    ncols = nc
    val nr = nrows.intValue
    var nnz = 0
    for (i <- 0 until nr) {
      nnz = nnz + input(i).indices.size
    }
    off = new Array[Int](nr + 1)
    idx = new Array[Int](nnz)
    data = new Array[Double](nnz)
    off(0) = 0
    var count = 0
    for (i <- 0 until nr) {
      val indx = input(i).indices
      val vals = input(i).values
      if (indx.length != vals.length) println ("Input error in ScalaCRS")
      for(j <- 0 until indx.length) { 
        idx(count + j) = indx(j)
        data(count + j) = vals(j)
      }
      count = count + indx.length
      off(i + 1) = count
    }
  }

  def to_vector_array(): Array[Vector] = {
    val local_row = nrows.intValue
    val local_col = ncols.intValue
    val ret = new Array[Vector](local_row)
    for (i <- 0 until local_row) {
      val nzero = off(i+1) - off(i)
      val indices = new Array[Int](nzero)
      val values = new Array[Double](nzero)
      var k = 0
      for (j <- off(i) until off(i+1)) {
        indices(k) = idx(j)
        values(k) = data(j) 
        k = k + 1
      }
      ret(i) = Vectors.sparse(local_col, indices, values)
    } 
    return ret 
  }

  def debug_print() : Unit = {
    println("nrows: " + nrows + " ncols: " + ncols)
    for (o <- off)  print(o + " "); println
    for (i <- idx)  print(i + " "); println
    for (d <- data) print(d + " "); println
  }
}
