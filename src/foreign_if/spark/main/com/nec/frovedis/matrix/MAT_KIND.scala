package com.nec.frovedis.matrix;

// assumed total no. of matrix can be 100

object MAT_KIND {
  val RMJR: Short = 1
  val CMJR: Short = 2
  val BCLC: Short = 3
  val SCRS: Short = 4
  val SCCS: Short = 5
  val SELL: Short = 6
  val SHYBRID: Short = 7
  val RMJR_L: Short = (100 + RMJR).shortValue
  val CMJR_L: Short = (100 + CMJR).shortValue
  val BCLC_L: Short = (100 + BCLC).shortValue
  val SCRS_L: Short = (100 + SCRS).shortValue
  val SCCS_L: Short = (100 + SCCS).shortValue
  val SELL_L: Short = (100 + SELL).shortValue
  val SHYBRID_L: Short = (100 + SHYBRID).shortValue
}
