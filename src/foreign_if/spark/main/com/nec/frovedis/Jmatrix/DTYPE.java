package com.nec.frovedis.Jmatrix;

// must comply with constants defined in com.nec.frovedis.matrix.DTYPE
public class DTYPE implements java.io.Serializable {
  public static final short NONE   = 0;
  public static final short INT    = 1;
  public static final short LONG   = 2;
  public static final short FLOAT  = 3;
  public static final short DOUBLE = 4;
  public static final short STRING = 5;
  public static final short BOOL   = 6;
  public static final short ULONG  = 7;
  public static final short WORDS  = 8;
  public static final short BYTE   = 9;

  public static long sizeof(short dtype) {
    long ret = 0;
    switch(dtype) {
      case BYTE:   ret = 1L; break; // for char etc...
      case INT:    ret = 4L; break;
      case BOOL:   ret = 4L; break;
      case LONG:   ret = 8L; break;
      case FLOAT:  ret = 4L; break;
      case DOUBLE: ret = 8L; break;
      default: throw new IllegalArgumentException("sizeof: Unknown primitive type!");
    }
    return ret;
  }
}
