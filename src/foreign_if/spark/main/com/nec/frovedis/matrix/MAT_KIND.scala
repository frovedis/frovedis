package com.nec.frovedis.matrix;

// assumed total no. of matrix can be 20

object DMAT_KIND {
  val RMJR: Short = 1
  val CMJR: Short = 2
  val BCLC: Short = 3
  val RMJR_L: Short = (20 + RMJR).shortValue
  val CMJR_L: Short = (20 + CMJR).shortValue
  val BCLC_L: Short = (20 + BCLC).shortValue
}

object SMAT_KIND {
  val CRS: Short = 1
  val CCS: Short = 2
  val ELL: Short = 3
  val HYBRID: Short = 4
  val CRS_L: Short = (20 + CRS).shortValue
  val CCS_L: Short = (20 + CCS).shortValue
  val ELL_L: Short = (20 + ELL).shortValue
  val HYBRID_L: Short = (20 + HYBRID).shortValue
}
