package com.nec.frovedis.graphx;

class sssp_result extends java.io.Serializable {
  var sourceVertex: Long = -1
  var dest_to_pred: Map[Long, Long] = Map()
  var dest_to_dist: Map[Long, Double] = Map()

  def this(destid: Array[Long],
           dist: Array[Double], 
           pred: Array[Long], 
           src: Long){
    this()
    sourceVertex = src    // 1-based
    dest_to_pred = (destid zip pred).toMap
    dest_to_dist = (destid zip dist).toMap
  }
  def sssp_query(dest: Long): (Double, String) = {
    if(!(dest_to_pred.contains(dest))) return (-1, "ERROR: Not rechable!")
    var ret: (Double, String) = null
    if (dest == sourceVertex) ret = (0.0, sourceVertex.toString)
    else {
      var path = dest.toString + " <= "
      var pred = dest_to_pred(dest) 
      while(pred != sourceVertex) {
        path = path + pred.toString + " <= "
        pred = dest_to_pred(pred)
      } 
      path = path + sourceVertex.toString
      ret = (dest_to_dist(dest), path)
    }
    return ret
  }
  def sssp_query(dest_arr: Array[Long]): Array[(Double, String)] = {
    val sz = dest_arr.size.toInt
    var res_arr: Array[(Double, String)] = new Array(sz)
    for (i <- 0 until sz) res_arr(i) = sssp_query(dest_arr(i))
    return res_arr
  }
}
