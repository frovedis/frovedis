package com.nec.frovedis.Jsql;

import java.util.Arrays;
import java.util.ArrayList;
import java.util.Iterator;
import org.apache.spark.api.java.JavaRDD;
import org.apache.spark.sql.catalyst.expressions.UnsafeRow;
import com.nec.frovedis.Jexrpc.Node;

public class jDFTransfer implements java.io.Serializable {
/*
  public static long execute(JavaRDD<UnsafeRow> rddData,
                             Node[] fw_nodes,
                             long[] vptrs,
                             short[] types,
                             long[] block_sizes,
                             int ncol, int nproc) {
*/
  public static long execute(JavaRDD<UnsafeRow> rddData,
                             String[] types, int ncol) { // for prototyping
    long ret = rddData.mapPartitionsWithIndex((index, x) -> {
      int k = 0;
      ArrayList<Object> b_obj = new ArrayList<Object>();
      ArrayList<Long> b_off = new ArrayList<Long>();
      while(x.hasNext()) {
        UnsafeRow row = x.next();
        b_obj.add(row.getBytes());
        b_off.add(row.getBaseOffset());
        k++;
      }
      for(int i = 0; i < ncol; ++i) {
        String tt = types[i];
        if (tt.equals("IntegerType")) {
          int[] iArr = new int[k];
          for(int j = 0; j < k; ++j) iArr[j] = jPlatform.getInt(b_obj.get(j), b_off.get(j), ncol, i);
        }
        else if (tt.equals("LongType")) {
          long[] lArr = new long[k];
          for(int j = 0; j < k; ++j) lArr[j] = jPlatform.getLong(b_obj.get(j), b_off.get(j), ncol, i);
        }
        else if (tt.equals("FloatType")) {
          float[] fArr = new float[k];
          for(int j = 0; j < k; ++j) fArr[j] = jPlatform.getFloat(b_obj.get(j), b_off.get(j), ncol, i);
        }
        else if (tt.equals("DoubleType")) {
          double[] dArr = new double[k];
          for(int j = 0; j < k; ++j) dArr[j] = jPlatform.getDouble(b_obj.get(j), b_off.get(j), ncol, i);
        }
        else throw new IllegalArgumentException("Unsupported type is encountered!\n");
      }
      int[] stat = {1};
      return Arrays.stream(stat).iterator();
    }, true).count(); // for action
    return ret;
  }
}

