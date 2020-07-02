package com.nec.frovedis.io;

import java.io.File

object FrovedisIO {
  def createDir(path: String): Boolean = {
    return new File(path).mkdir()
  }
  def createFile(path: String): Boolean = {
    return new File(path).createNewFile()
  }
  def checkExists(path: String): Boolean = {
    return new File(path).exists
  }
}
