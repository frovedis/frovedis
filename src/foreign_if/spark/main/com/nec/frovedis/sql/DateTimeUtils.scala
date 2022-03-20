package com.nec.frovedis.sql;

import java.time.format.DateTimeFormatter

object DateTimeUtils extends java.io.Serializable {

  def get_symbol(item: String): String = { 
    //println(item)
    if ((item contains "WeekBasedYear") || (item contains "YearOfEra")) {
      val tmp = item.split(",") 
      if (tmp.length > 1 && tmp(1) != "4") throw new IllegalArgumentException(
        "Unsupported year format is specified!")
      return "%Y"
    } else if (item contains "DayOfMonth") {
      val tmp = item.split(",")
      if (tmp.length == 1) throw new IllegalArgumentException(
        "Unsupported day format is specified!")
      return "%d"
    } else if (item contains "MonthOfYear") {
      val tmp = item.split(",")
      if (tmp.length > 1) {
        if (tmp(1) == "2)") return "%m"
        else if (tmp(1) == "SHORT)") return "%b"
        else throw new IllegalArgumentException(
          "Unsupported month format is specified!")
      }
      else return "%m"
    } else if (item contains "HourOfDay") {
      val tmp = item.split(",")
      if (tmp.length == 1) throw new IllegalArgumentException(
        "Unsupported hour format is specified!")
      return "%H"
    } else if (item contains "MinuteOfHour") {
      val tmp = item.split(",")
      if (tmp.length == 1) throw new IllegalArgumentException(
        "Unsupported minute format is specified!")
      return "%M"
    } else if (item contains "SecondOfMinute") {
      val tmp = item.split(",")
      if (tmp.length == 1) throw new IllegalArgumentException(
        "Unsupported second format is specified!")
      return "%S"
    } 
    else throw new IllegalArgumentException("Unsupported date format is specified!")
}

def parse_format(format: String): String = {
  val pattern = DateTimeFormatter.ofPattern(format).toString
  val items = pattern.split("'")
  if (items.length > 11) throw new IllegalArgumentException(
    "Unsupported date format is specified!")

  var ret: String = ""
  for (i <- 0 until items.length) {
    val e = items(i)
    ret += (if (e.length < 2) e else get_symbol(e))
  }
  return ret  
}

}
