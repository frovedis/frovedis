package com.nec.frovedis.Jsql;

import org.apache.spark.unsafe.Platform;
import org.apache.spark.unsafe.bitset.BitSetMethods;

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
}
