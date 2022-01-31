package com.nec.frovedis.Jsql;

import java.util.Arrays;
import java.util.ArrayList;
import java.util.Iterator;
import org.apache.spark.api.java.JavaRDD;
import org.apache.spark.sql.catalyst.expressions.UnsafeRow;
import org.apache.spark.sql.vectorized.ColumnarBatch;
import org.apache.spark.sql.vectorized.ColumnVector;
import java.util.Iterator;
import com.nec.frovedis.Jexrpc.Node;
import com.nec.frovedis.Jexrpc.JNISupport;
import com.nec.frovedis.Jmatrix.DTYPE;
import com.nec.frovedis.Jmatrix.OffHeapArray;
import com.nec.frovedis.Jmatrix.configs;
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
                                         int[] colIds,
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
      int cid = colIds[i];
      ColumnVector tcol = batch.column(cid);
      boolean check_null = tcol.hasNull();
      int row_offset = offset[i];
      long vptr = vptrs[row_offset * nproc + destId];
      switch(types[i]) {
        case DTYPE.INT: { 
          OffHeapArray buf = new OffHeapArray(k, DTYPE.INT);
          if (check_null) {
            for(int j = 0; j < k; ++j) buf.putInt(j, tcol.isNullAt(j) ? Integer.MAX_VALUE : tcol.getInt(j));
          } else {
            for(int j = 0; j < k; ++j) buf.putInt(j, tcol.getInt(j));
          }
          t0.show("ColumnVector -> IntArray: ");
          JNISupport.loadFrovedisWorkerTypedVector(w_node, vptr, localId, 
                                                   buf.get(), k, DTYPE.INT, 
                                                   configs.rawsend_enabled);
          buf.freeMemory();
          break;
        }
        case DTYPE.BOOL: { 
          OffHeapArray buf = new OffHeapArray(k, DTYPE.INT);
          if (check_null) {
            for(int j = 0; j < k; ++j) {
              if (tcol.isNullAt(j)) buf.putInt(j, Integer.MAX_VALUE);
              else                  buf.putInt(j, tcol.getBoolean(j) ? 1 : 0);
            }
          } else {
            for(int j = 0; j < k; ++j) buf.putInt(j, tcol.getBoolean(j) ? 1 : 0);
          }
          t0.show("ColumnVector -> (Boolean) IntArray: ");
          JNISupport.loadFrovedisWorkerTypedVector(w_node, vptr, localId, 
                                                   buf.get(), k, DTYPE.INT, 
                                                   configs.rawsend_enabled);
          break;
        }
        case DTYPE.LONG: { 
          OffHeapArray buf = new OffHeapArray(k, DTYPE.LONG);
          if (check_null) {
            for(int j = 0; j < k; ++j) buf.putLong(j, tcol.isNullAt(j) ? Long.MAX_VALUE : tcol.getLong(j));
          } else {
            for(int j = 0; j < k; ++j) buf.putLong(j, tcol.getLong(j));
          }
          t0.show("ColumnVector -> LongArray: ");
          JNISupport.loadFrovedisWorkerTypedVector(w_node, vptr, localId, 
                                                   buf.get(), k, DTYPE.LONG,
                                                   configs.rawsend_enabled);
          buf.freeMemory();
          break;
        }
        case DTYPE.FLOAT: { 
          OffHeapArray buf = new OffHeapArray(k, DTYPE.FLOAT);
          if (check_null) {
            for(int j = 0; j < k; ++j) buf.putFloat(j, tcol.isNullAt(j) ? Float.MAX_VALUE : tcol.getFloat(j));
          } else {
            for(int j = 0; j < k; ++j) buf.putFloat(j, tcol.getFloat(j));
          }
          t0.show("ColumnVector -> FloatArray: ");
          JNISupport.loadFrovedisWorkerTypedVector(w_node, vptr, localId, 
                                                   buf.get(), k, DTYPE.FLOAT,
                                                   configs.rawsend_enabled);
          buf.freeMemory();
          break;
        }
        case DTYPE.DOUBLE: { 
          OffHeapArray buf = new OffHeapArray(k, DTYPE.DOUBLE);
          if (check_null) {
            for(int j = 0; j < k; ++j) buf.putDouble(j, tcol.isNullAt(j) ? Double.MAX_VALUE : tcol.getDouble(j));
          } else {
            for(int j = 0; j < k; ++j) buf.putDouble(j, tcol.getDouble(j));
          }
          t0.show("ColumnVector -> DoubleArray: ");
          JNISupport.loadFrovedisWorkerTypedVector(w_node, vptr, localId, 
                                                   buf.get(), k, DTYPE.DOUBLE,
                                                   configs.rawsend_enabled);
          buf.freeMemory();
          break;
        }
        case DTYPE.STRING: { // mostly used for debugging 
          String[] sArr = new String[k];
          if (check_null) {
            for(int j = 0; j < k; ++j) sArr[j] = tcol.isNullAt(j) ? "NULL" : tcol.getUTF8String(j).toString();
          } else {
            for(int j = 0; j < k; ++j) sArr[j] = tcol.getUTF8String(j).toString();
          }
          t0.show("ColumnVector -> StringArray: ");
          JNISupport.loadFrovedisWorkerStringVector(w_node, vptr, localId, sArr, k);
          break;
        }
        case DTYPE.WORDS: {
          byte[][] buffer = new byte[k][]; 
          int flat_size = 0;
          if (check_null) {
            byte[] nulls = new byte[4];
            nulls[0] = 'N'; nulls[1] = 'U'; nulls[2] = 'L'; nulls[3] = 'L';
            for(int j = 0; j < k; ++j) {
              buffer[j] = tcol.isNullAt(j) ? nulls : tcol.getBinary(j);
              flat_size += buffer[j].length;
            }
          } else {
            for(int j = 0; j < k; ++j) {
              buffer[j] = tcol.getBinary(j);
              flat_size += buffer[j].length;
            }
          }
          int cur = 0;
          OffHeapArray szbuf = new OffHeapArray(k, DTYPE.INT);
          OffHeapArray charbuf = new OffHeapArray(flat_size, DTYPE.BYTE);
          for(int j = 0; j < k; ++j) {
            int size = buffer[j].length;
            for(int c = 0; c < size; ++c) charbuf.putByte(cur + c, buffer[j][c]);
            szbuf.putInt(j, size);
            cur += size;
          }
          t0.show("ColumnVector -> flatten-byteArray: ");
          int next_row_offset = row_offset + 1;
          long sptr = vptrs[next_row_offset * nproc + destId];
          JNISupport.loadFrovedisWorkerByteSizePair2(w_node, vptr, sptr, localId, 
                                                     charbuf.get(), szbuf.get(), 
                                                     flat_size, k,
                                                     configs.rawsend_enabled);
          szbuf.freeMemory();
          charbuf.freeMemory();
          break;
        }
        default: throw new IllegalArgumentException("Unsupported type is encountered!\n");
      } // end of switch
      String err = JNISupport.checkServerException();
      if (!err.isEmpty()) throw new java.rmi.ServerException(err);
      t0.show("spark-worker to frovedis-rank local data copy: ");
    } // end of for-loop (iterating columns)
  } 

  public static OffHeapArray[]
  copy_batch_data(ColumnarBatch batch,
                  int[] colIds,
                  int[] offset,
                  short[] types,
                  int ncol, int word_count,
                  TimeSpent t0)
    throws java.rmi.ServerException, IllegalArgumentException {

    int ntargets = ncol + word_count;
    OffHeapArray[] out = new OffHeapArray[ntargets];
    int k = batch.numRows();
    for (int i = 0; i < ncol; ++i) {
      int cid = colIds[i];
      ColumnVector tcol = batch.column(cid);
      boolean check_null = tcol.hasNull();
      int row_offset = offset[i];
      switch(types[i]) {
        case DTYPE.INT: {
          OffHeapArray buf = new OffHeapArray(k, DTYPE.INT);
          if (check_null) {
            for(int j = 0; j < k; ++j) buf.putInt(j, tcol.isNullAt(j) ? Integer.MAX_VALUE : tcol.getInt(j));
          } else {
            for(int j = 0; j < k; ++j) buf.putInt(j, tcol.getInt(j));
          }
          t0.show("ColumnVector -> IntArray: ");
          out[row_offset] = buf;
          break;
        }
        case DTYPE.BOOL: {
          OffHeapArray buf = new OffHeapArray(k, DTYPE.INT);
          if (check_null) {
            for(int j = 0; j < k; ++j) {
              if (tcol.isNullAt(j)) buf.putInt(j, Integer.MAX_VALUE);
              else                  buf.putInt(j, tcol.getBoolean(j) ? 1 : 0);
            }
          } else {
            for(int j = 0; j < k; ++j) buf.putInt(j, tcol.getBoolean(j) ? 1 : 0);
          }
          t0.show("ColumnVector -> (Boolean) IntArray: ");
          out[row_offset] = buf;
          break;
        }
        case DTYPE.LONG: {
          OffHeapArray buf = new OffHeapArray(k, DTYPE.LONG);
          if (check_null) {
            for(int j = 0; j < k; ++j) buf.putLong(j, tcol.isNullAt(j) ? Long.MAX_VALUE : tcol.getLong(j));
          } else {
            for(int j = 0; j < k; ++j) buf.putLong(j, tcol.getLong(j));
          }
          t0.show("ColumnVector -> LongArray: ");
          out[row_offset] = buf;
          break;
        }
        case DTYPE.FLOAT: {
          OffHeapArray buf = new OffHeapArray(k, DTYPE.FLOAT);
          if (check_null) {
            for(int j = 0; j < k; ++j) buf.putFloat(j, tcol.isNullAt(j) ? Float.MAX_VALUE : tcol.getFloat(j));
          } else {
            for(int j = 0; j < k; ++j) buf.putFloat(j, tcol.getFloat(j));
          }
          t0.show("ColumnVector -> FloatArray: ");
          out[row_offset] = buf;
          break;
        }
        case DTYPE.DOUBLE: {
          OffHeapArray buf = new OffHeapArray(k, DTYPE.DOUBLE);
          if (check_null) {
            for(int j = 0; j < k; ++j) buf.putDouble(j, tcol.isNullAt(j) ? Double.MAX_VALUE : tcol.getDouble(j));
          } else {
            for(int j = 0; j < k; ++j) buf.putDouble(j, tcol.getDouble(j));
          }
          t0.show("ColumnVector -> DoubleArray: ");
          out[row_offset] = buf;
          break;
        }
        case DTYPE.STRING: { 
          throw new IllegalArgumentException("copy_batch_data: 'String' type is not supported!");
        } 
        case DTYPE.WORDS: {
          byte[][] buffer = new byte[k][];
          int flat_size = 0;
          if (check_null) {
            byte[] nulls = new byte[4];
            nulls[0] = 'N'; nulls[1] = 'U'; nulls[2] = 'L'; nulls[3] = 'L';
            for(int j = 0; j < k; ++j) {
              buffer[j] = tcol.isNullAt(j) ? nulls : tcol.getBinary(j);
              flat_size += buffer[j].length;
            }
          } else {
            for(int j = 0; j < k; ++j) {
              buffer[j] = tcol.getBinary(j);
              flat_size += buffer[j].length;
            }
          }
          int cur = 0;
          OffHeapArray szbuf = new OffHeapArray(k, DTYPE.INT);
          OffHeapArray charbuf = new OffHeapArray(flat_size, DTYPE.BYTE);
          for(int j = 0; j < k; ++j) {
            int size = buffer[j].length;
            for(int c = 0; c < size; ++c) charbuf.putByte(cur + c, buffer[j][c]);
            szbuf.putInt(j, size);
            cur += size;
          }
          t0.show("ColumnVector -> flatten-byteArray: ");
          int next_row_offset = row_offset + 1;
          out[row_offset] = charbuf;
          out[next_row_offset] = szbuf;
          break;
        }
        default: throw new IllegalArgumentException("Unsupported type is encountered!\n");
      } // end of switch
    } // end of for-loop (iterating columns)
    return out;
  }
}
