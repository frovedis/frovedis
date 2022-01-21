package com.nec.frovedis.Jsql;

import java.util.Arrays;
import java.util.ArrayList;
import java.util.Iterator;
import org.apache.spark.api.java.JavaRDD;
import org.apache.spark.sql.catalyst.expressions.UnsafeRow;
import org.apache.spark.sql.vectorized.ColumnarBatch;
import org.apache.spark.sql.vectorized.ColumnVector;
import com.nec.frovedis.Jexrpc.Node;
import com.nec.frovedis.Jexrpc.JNISupport;
import com.nec.frovedis.Jmatrix.DTYPE;
import com.nec.frovedis.matrix.TimeSpent;

public class jDFTransfer implements java.io.Serializable {
/*
  public static long execute(JavaRDD<UnsafeRow> rddData,
                             Node[] fw_nodes,
                             long[] vptrs,
                             short[] types,
                             long[] block_sizes,
                             int ncol, int nproc) {
*/
  // this is just a temporary prototype; cosplete the implementation before using it
  public static long execute(JavaRDD<UnsafeRow> rddData,
                             String[] types, int ncol) 
    throws IllegalArgumentException {

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

  public static void transfer_batch_data(ColumnarBatch batch,
                                         Node w_node,
                                         long[] vptrs,
                                         int[] offset,
                                         short[] types,
                                         int ncol, int nproc,
                                         int destId, long localId,
                                         TimeSpent t0) 
    throws java.rmi.ServerException, IllegalArgumentException {

    int k = batch.numRows();
    for (int i = 0; i < ncol; ++i) {
      ColumnVector tcol = batch.column(i);
      int row_offset = offset[i];
      long vptr = vptrs[row_offset * nproc + destId];
      switch(types[i]) {
        case DTYPE.INT: { 
          //int[] iArr = tcol.getInts(0, k);
          int[] iArr = new int[k];
          for(int j = 0; j < k; ++j) iArr[j] = tcol.isNullAt(j) ? Integer.MAX_VALUE : tcol.getInt(j);
          t0.show("ColumnVector -> IntArray: ");
          JNISupport.loadFrovedisWorkerIntVector(w_node, vptr, localId, iArr, k);
          break;
        }
        case DTYPE.BOOL: { 
          int[] iArr = new int[k];
          for(int j = 0; j < k; ++j) {
            if (tcol.isNullAt(j)) iArr[j] = Integer.MAX_VALUE;
            else                  iArr[j] = tcol.getBoolean(j) ? 1 : 0;
          }
          t0.show("ColumnVector -> (Boolean) IntArray: ");
          JNISupport.loadFrovedisWorkerIntVector(w_node, vptr, localId, iArr, k);
          break;
        }
        case DTYPE.LONG: { 
          //long[] lArr = tcol.getLongs(0, k);
          long[] lArr = new long[k];
          for(int j = 0; j < k; ++j) lArr[j] = tcol.isNullAt(j) ? Long.MAX_VALUE : tcol.getLong(j);
          t0.show("ColumnVector -> LongArray: ");
          JNISupport.loadFrovedisWorkerLongVector(w_node, vptr, localId, lArr, k);
          break;
        }
        case DTYPE.FLOAT: { 
          //float[] fArr = tcol.getFloats(0, k);
          float[] fArr = new float[k];
          for(int j = 0; j < k; ++j) fArr[j] = tcol.isNullAt(j) ? Float.MAX_VALUE : tcol.getFloat(j);
          t0.show("ColumnVector -> FloatArray: ");
          JNISupport.loadFrovedisWorkerFloatVector(w_node, vptr, localId, fArr, k);
          break;
        }
        case DTYPE.DOUBLE: { 
          //double[] dArr = tcol.getDoubles(0, k);
          double[] dArr = new double[k];
          for(int j = 0; j < k; ++j) dArr[j] = tcol.isNullAt(j) ? Double.MAX_VALUE : tcol.getDouble(j);
          t0.show("ColumnVector -> DoubleArray: ");
          JNISupport.loadFrovedisWorkerDoubleVector(w_node, vptr, localId, dArr, k);
          break;
        }
        case DTYPE.STRING: { 
          String[] sArr = new String[k];
          for(int j = 0; j < k; ++j) sArr[j] = tcol.isNullAt(j) ? "NULL" : tcol.getUTF8String(j).toString();
          t0.show("ColumnVector -> StringArray: ");
          JNISupport.loadFrovedisWorkerStringVector(w_node, vptr, localId, sArr, k);
          break;
        }
        case DTYPE.WORDS: { 
          int flat_size = 0;
          int[] szArr = new int[k];
          String[] sArr = new String[k];
          for(int j = 0; j < k; ++j) {
            String tmp = tcol.isNullAt(j) ? "NULL" : tcol.getUTF8String(j).toString();
            sArr[j] = tmp;
            szArr[j] = tmp.length();
            flat_size += tmp.length();
          }
          int cur = 0;
          char[] cArr = new char[flat_size];
          for(int j = 0; j < k; ++j) {
            char[] tmp = sArr[j].toCharArray();
            System.arraycopy(tmp, 0, cArr, cur, tmp.length);
            cur += tmp.length;
          }
          t0.show("ColumnVector -> flatten-charArray: ");
          int next_row_offset = row_offset + 1;
          long sptr = vptrs[next_row_offset * nproc + destId];
          JNISupport.loadFrovedisWorkerCharSizePair(w_node, vptr, sptr, localId, 
                                                    cArr, szArr, flat_size, k);
          break;
        }
        default: throw new IllegalArgumentException("Unsupported type is encountered!\n");
      }
      String err = JNISupport.checkServerException();
      if (!err.isEmpty()) throw new java.rmi.ServerException(err);
      t0.show("spark-worker to frovedis-rank local data copy: ");
    }
  }
}
