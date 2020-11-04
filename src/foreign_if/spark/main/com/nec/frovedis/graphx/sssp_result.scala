package com.nec.frovedis.graphx;

class sssp_result extends java.io.Serializable {
  var nodesDist: Array[Double]  = Array()
  var nodesPred: Array[Long] = Array()
  var num_nodes: Long = -1
  var sourceVertex: Long = -1

  def this(pred: Array[Long], 
           dist: Array[Double], 
           num_nodes: Long, 
           sourceVertex: Long){
    this()
    this.nodesPred = pred            // 1-based
    this.nodesDist = dist
    this.num_nodes = num_nodes
    this.sourceVertex = sourceVertex // 1-based
  }
  def sssp_query(dest: Long): (Double, String) = {
    if(dest < 1 || dest > this.num_nodes) 
      return (-1, "ERROR: node does not exist!")
    var pred: Long = this.nodesPred(dest.toInt - 1) // dest/pred is 1-based
    var ret: (Double, String) = null
    if (pred == this.sourceVertex) ret = (0.0, this.sourceVertex.toString)
    else if (pred == dest) ret = (this.nodesDist(dest.toInt - 1), 
                                  "not reachable")
    else {
      var path: String = dest.toString + " <= "
      while(pred != this.sourceVertex) {
        path = path + pred.toString + " <= "
        pred = this.nodesPred(pred.toInt - 1)
      } 
      path = path + this.sourceVertex.toString
      ret = (this.nodesDist(dest.toInt - 1), path)
    }
    return ret
  }
  def sssp_query(dest_arr: Array[Long]): Array[(Double, String)] = {
    var res_arr: Array[(Double, String)] = new Array(dest_arr.size.toInt)
    for (i <- 0 until dest_arr.size.toInt)
      res_arr(i) = sssp_query(dest_arr(i))
    return res_arr
  }
}
