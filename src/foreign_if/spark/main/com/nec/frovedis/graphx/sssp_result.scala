package com.nec.frovedis.graphx;

class sssp_result extends java.io.Serializable {
  var nodes_dist: Array[Int]  = Array()
  var nodesPred: Array[Long] = Array()
  var num_nodes: Long = -1
  var sourceVertex: Long = -1

  def this(pred: Array[Long], dist: Array[Int], num_nodes: Long, 
          sourceVertex: Long){
    this()
    this.nodesPred = pred
    this.nodes_dist = dist
    this.num_nodes = num_nodes
    this.sourceVertex = sourceVertex
  }
  def sssp_query(dest: Long): (Int, String) = {
    if (dest < 0 || dest > this.num_nodes - 1) 
      return (-1, "ERROR: Node does not exist!")
    var path: String = dest.toString + " <= "
    var node_pred: Long = this.nodesPred(dest.toInt);
    while(node_pred != this.sourceVertex && node_pred != 
                      this.nodesPred(node_pred.toInt)){
      path = path + node_pred.toString + " <= "
      node_pred = this.nodesPred(node_pred.toInt);
    }
    path = path + this.sourceVertex.toString
    return (this.nodes_dist(dest.toInt), path)
  }
  def sssp_query(dest_arr: Array[Int]): Array[(Int, String)] = {
    var res_arr: Array[(Int, String)] = new Array(dest_arr.size.toInt)
    for (i <- 0 until dest_arr.size.toInt)
      res_arr(i) = sssp_query(dest_arr(i))
    return res_arr
  }
}
