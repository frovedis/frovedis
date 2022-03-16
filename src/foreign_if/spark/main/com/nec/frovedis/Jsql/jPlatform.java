package com.nec.frovedis.Jsql;

import org.apache.spark.unsafe.Platform;
import org.apache.spark.unsafe.types.UTF8String;
import org.apache.spark.unsafe.bitset.BitSetMethods;
import scala.collection.mutable.ArrayBuffer;
import java.util.TimeZone;

public class jPlatform implements java.io.Serializable {
  // REF: https://github.com/apache/spark/blob/95fc4c56426706546601d339067ce6e3e7f4e03f/sql/catalyst/src/main/java/org/apache/spark/sql/catalyst/expressions/UnsafeRow.java#L136
  public static long getFieldOffset(long baseOffset,
                                    int numFields,
                                    int ordinal) {
    long bitSetWidthInBytes = ((numFields + 63)/ 64) * 8;
    return baseOffset + bitSetWidthInBytes + ordinal * 8L;
  }

  private static void assertIndexIsValid(int index, int numFields) {
    assert index >= 0 : "index (" + index + ") should >= 0";
    assert index < numFields : "index (" + index + ") should < " + numFields;
  }

  public static boolean isNullAt(Object baseObject,
                                 long baseOffset,
                                 int numFields,
                                 int ordinal) {
    //assertIndexIsValid(ordinal, numFields);
    return BitSetMethods.isSet(baseObject, baseOffset, ordinal);
  }

  public static int getInt(Object baseObject,
                           long baseOffset,
                           int numFields,
                           int ordinal) {
    assertIndexIsValid(ordinal, numFields);
    if (isNullAt(baseObject, baseOffset, numFields, ordinal)) 
      return Integer.MAX_VALUE;
    else 
      return Platform.getInt(baseObject, getFieldOffset(baseOffset, numFields, ordinal));
  }

  public static long getLong(Object baseObject,
                             long baseOffset,
                             int numFields,
                             int ordinal) {
    assertIndexIsValid(ordinal, numFields);
    if (isNullAt(baseObject, baseOffset, numFields, ordinal)) 
      return Long.MAX_VALUE;
    else 
      return Platform.getLong(baseObject, getFieldOffset(baseOffset, numFields, ordinal));
  }

  public static long getDate(Object baseObject,
                             long baseOffset,
                             int numFields,
                             int ordinal) {
    long multiplier = 24L * 3600 * 1000 * 1000 * 1000;
    assertIndexIsValid(ordinal, numFields);
    if (isNullAt(baseObject, baseOffset, numFields, ordinal))
      return Long.MAX_VALUE;
    else { // (numdays * 24L * 3600 * 1000 * 1000 * 1000 -> GMT nanoseconds)
      long numdays = Platform.getLong(baseObject, getFieldOffset(baseOffset, numFields, ordinal));
      return numdays * multiplier;
    }
  }

  public static long getTime(Object baseObject,
                             long baseOffset,
                             int numFields,
                             int ordinal) {
    long ts_offset = TimeZone.getDefault().getRawOffset() * 1000L; // offset in microseconds
    assertIndexIsValid(ordinal, numFields);
    if (isNullAt(baseObject, baseOffset, numFields, ordinal))
      return Long.MAX_VALUE;
    else {
      long msec = Platform.getLong(baseObject, getFieldOffset(baseOffset, numFields, ordinal));
      long gmt_msec = msec + ts_offset; // GMT microseconds
      return gmt_msec * 1000L; // GMT nanoseconds
    }
  }

  public static float getFloat(Object baseObject,
                               long baseOffset,
                               int numFields,
                               int ordinal) {
    assertIndexIsValid(ordinal, numFields);
    if (isNullAt(baseObject, baseOffset, numFields, ordinal)) 
      return Float.MAX_VALUE;
    else 
      return Platform.getFloat(baseObject, getFieldOffset(baseOffset, numFields, ordinal));
  }

  public static double getDouble(Object baseObject,
                                 long baseOffset,
                                 int numFields,
                                 int ordinal) {
    assertIndexIsValid(ordinal, numFields);
    if (isNullAt(baseObject, baseOffset, numFields, ordinal)) 
      return Double.MAX_VALUE;
    else 
      return Platform.getDouble(baseObject, getFieldOffset(baseOffset, numFields, ordinal));
  }

  public static String getString(Object baseObject,
                                 long baseOffset,
                                 int numFields,
                                 int ordinal) {
    assertIndexIsValid(ordinal, numFields);
    if (isNullAt(baseObject, baseOffset, numFields, ordinal)) 
      return "NULL";
    else {
      long offsetAndSize = getLong(baseObject, baseOffset, numFields, ordinal);
      int offset = (int) (offsetAndSize >> 32);
      int size = (int) offsetAndSize;
      return UTF8String.fromAddress(baseObject, baseOffset + offset, size).toString();
    }
  }
 
  public static int getStringSize(Object baseObject,
                                  long baseOffset,
                                  int numFields,
                                  int ordinal) {
    assertIndexIsValid(ordinal, numFields);
    if (isNullAt(baseObject, baseOffset, numFields, ordinal)) {
      return 4; // "NULL"
    }
    else {
      long offsetAndSize = getLong(baseObject, baseOffset, numFields, ordinal);
      int offset = (int) (offsetAndSize >> 32);
      int size = (int) offsetAndSize;
      return size;
    }
  }

  public static char[] getCharArray(byte[] baseObject,
                                    long baseOffset,
                                    int numFields,
                                    int ordinal) {
    assertIndexIsValid(ordinal, numFields);
    if (isNullAt(baseObject, baseOffset, numFields, ordinal)) {
      char[] ret = new char[] {'N', 'U', 'L', 'L'};
      return ret;
    }
    else {
      long offsetAndSize = getLong(baseObject, baseOffset, numFields, ordinal);
      int offset = (int) (offsetAndSize >> 32);
      int size = (int) offsetAndSize;
      char[] ret = new char[size];
      for (int i = 0; i < size; ++i) ret[i] = (char) baseObject[offset + i];
      return ret;
    }
  }

  public static byte[] getBinary(byte[] baseObject,
                                 long baseOffset,
                                 int numFields,
                                 int ordinal) {
    assertIndexIsValid(ordinal, numFields);
    if (isNullAt(baseObject, baseOffset, numFields, ordinal)) {
      byte[] ret = new byte[] {'N', 'U', 'L', 'L'};
      return ret;
    }
    else {
      long offsetAndSize = getLong(baseObject, baseOffset, numFields, ordinal);
      int offset = (int) (offsetAndSize >> 32);
      int size = (int) offsetAndSize;
      byte[] ret = new byte[size];
      for (int i = 0; i < size; ++i) ret[i] = baseObject[offset + i];
      return ret;
    }
  }
}
