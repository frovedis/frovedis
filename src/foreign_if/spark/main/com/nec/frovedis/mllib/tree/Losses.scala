package com.nec.frovedis.mllib.tree

import org.apache.spark.mllib.tree.loss._

object Losses {
  def toString(x: Loss): String = x match {
    case  SquaredError => "leastsquareserror"
    case  AbsoluteError => "leastabsoluteerror"
    case  LogLoss => "logloss"
    case _ => throw new IllegalArgumentException(s"Did not recognize Loss type: $x")
  }
}
