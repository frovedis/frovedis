package com.nec.frovedis.Jmatrix;

import sun.misc.Unsafe;
import java.lang.reflect.Field;
import com.nec.frovedis.Jexrpc.Node;
import com.nec.frovedis.Jexrpc.JNISupport;
import com.nec.frovedis.Jexrpc.FrovedisServer;

public class OffHeapArray {
    private long size, address;
    private Unsafe _Unsafe;
    private short dtype;

    public OffHeapArray(long size, short dtype) { 
      // TODO: make _Unsafe as static final
      try {
        Field f = Unsafe.class.getDeclaredField("theUnsafe");
        f.setAccessible(true);
        _Unsafe = (Unsafe) f.get(null);
      } catch (Throwable t) {
        System.err.println("exception caught: " + t.getMessage());
        System.exit(-1);
      }
      this.dtype = dtype;
      this.size = size;
      long size_in_bytes = size * DTYPE.sizeof(dtype);
      this.address = _Unsafe.allocateMemory(size_in_bytes);
    }

    public long size () { return size; }
    public long get ()  { return address; }
    private long sizeof (short dtype) { return DTYPE.sizeof(dtype); }

    public void freeMemory() { 
      size = address = 0;
      dtype = DTYPE.NONE;
      _Unsafe.freeMemory(address);
    }

    // for debugging
    public void sendData() 
    throws java.rmi.ServerException {
      FrovedisServer fs = FrovedisServer.getServerInstance();
      JNISupport.loadVectorData(fs.master_node, address, size, dtype);
      String err = JNISupport.checkServerException();
      if (!err.isEmpty()) throw new java.rmi.ServerException(err);
    }

    // -------- for double data --------
    public double getDouble(int idx) {
      assert(this.dtype == DTYPE.DOUBLE); 
      return _Unsafe.getDouble(address + idx * sizeof(DTYPE.DOUBLE));
    }

    public double[] getDoubles(int idx, int count) { 
      assert(this.dtype == DTYPE.DOUBLE); 
      double[] ret = new double[count];
      _Unsafe.copyMemory(null, address + idx * sizeof(DTYPE.DOUBLE),
                         ret, Unsafe.ARRAY_DOUBLE_BASE_OFFSET, 
                         count * sizeof(DTYPE.DOUBLE));
      return ret;
    }

    public void putDouble(int idx, double value) {
      assert(this.dtype == DTYPE.DOUBLE); 
      _Unsafe.putDouble(null, address + idx * sizeof(DTYPE.DOUBLE), value);
    }

    public void putDoubles(int idx, int count, double[] src, int srcIndex) { 
      assert(this.dtype == DTYPE.DOUBLE); 
      _Unsafe.copyMemory(src, Unsafe.ARRAY_DOUBLE_BASE_OFFSET + srcIndex * sizeof(DTYPE.DOUBLE),
                         null, address + idx * sizeof(DTYPE.DOUBLE), 
                         count * sizeof(DTYPE.DOUBLE));
    }
    
    public void putDoubles(int idx, int count, long srcAddress, int srcIndex) { 
      assert(this.dtype == DTYPE.DOUBLE); 
      _Unsafe.copyMemory(null, srcAddress + srcIndex * sizeof(DTYPE.DOUBLE),
                         null, address + idx * sizeof(DTYPE.DOUBLE), 
                         count * sizeof(DTYPE.DOUBLE));
    }

    // -------- for float data --------
    public float getFloat(int idx) {
      assert(this.dtype == DTYPE.FLOAT); 
      return _Unsafe.getFloat(address + idx * sizeof(DTYPE.FLOAT));
    }

    public float[] getFloats(int idx, int count) {
      assert(this.dtype == DTYPE.FLOAT); 
      float[] ret = new float[count];
      _Unsafe.copyMemory(null, address + idx * sizeof(DTYPE.FLOAT),
                        ret, Unsafe.ARRAY_FLOAT_BASE_OFFSET, 
                        count * sizeof(DTYPE.FLOAT));
      return ret;
    }

    public void putFloat(int idx, float value) {
      assert(this.dtype == DTYPE.FLOAT); 
      _Unsafe.putFloat(null, address + idx * sizeof(DTYPE.FLOAT), value);
    }

    public void putFloats(int idx, int count, float[] src, int srcIndex) {
      assert(this.dtype == DTYPE.FLOAT); 
      _Unsafe.copyMemory(src, Unsafe.ARRAY_FLOAT_BASE_OFFSET + srcIndex * sizeof(DTYPE.FLOAT),
                         null, address + idx * sizeof(DTYPE.FLOAT), 
                         count * sizeof(DTYPE.FLOAT));
    }

    public void putFloats(int idx, int count, long srcAddress, int srcIndex) {
      assert(this.dtype == DTYPE.FLOAT); 
      _Unsafe.copyMemory(null, srcAddress + srcIndex * sizeof(DTYPE.FLOAT),
                         null, address + idx * sizeof(DTYPE.FLOAT), 
                         count * sizeof(DTYPE.FLOAT));
    }

    // -------- for long data --------
    public long getLong(int idx) {
      assert(this.dtype == DTYPE.LONG); 
      return _Unsafe.getLong(address + idx * sizeof(DTYPE.LONG));
    }

    public long[] getLongs(int idx, int count) {
      assert(this.dtype == DTYPE.LONG); 
      long[] ret = new long[count];
      _Unsafe.copyMemory(null, address + idx * sizeof(DTYPE.LONG),
                         ret, Unsafe.ARRAY_LONG_BASE_OFFSET, 
                         count * sizeof(DTYPE.LONG));
      return ret;
    }

    public void putLong(int idx, long value) {
      assert(this.dtype == DTYPE.LONG); 
      _Unsafe.putLong(null, address + idx * sizeof(DTYPE.LONG), value);
    }

    public void putLongs(int idx, int count, long[] src, int srcIndex) {
      assert(this.dtype == DTYPE.LONG); 
      _Unsafe.copyMemory(src, Unsafe.ARRAY_LONG_BASE_OFFSET + srcIndex * sizeof(DTYPE.LONG),
                         null, address + idx * sizeof(DTYPE.LONG), 
                         count * sizeof(DTYPE.LONG));
    }

    public void putLongs(int idx, int count, long srcAddress, int srcIndex) {
      assert(this.dtype == DTYPE.LONG); 
      _Unsafe.copyMemory(null, srcAddress + srcIndex * sizeof(DTYPE.LONG),
                         null, address + idx * sizeof(DTYPE.LONG), 
                         count * sizeof(DTYPE.LONG));
    }

    // -------- for int data --------
    public int getInt(int idx) {
      assert(this.dtype == DTYPE.INT); 
      return _Unsafe.getInt(address + idx * sizeof(DTYPE.INT));
    }

    public int[] getInts(int idx, int count) {
      assert(this.dtype == DTYPE.INT); 
      int[] ret = new int[count];
      _Unsafe.copyMemory(null, address + idx * sizeof(DTYPE.INT),
                         ret, Unsafe.ARRAY_INT_BASE_OFFSET, 
                         count * sizeof(DTYPE.INT));
      return ret;
    }

    public void putInt(int idx, int value) {
      assert(this.dtype == DTYPE.INT); 
      _Unsafe.putInt(null, address + idx * sizeof(DTYPE.INT), value);
    }

    public void putInts(int idx, int count, int[] src, int srcIndex) {
      assert(this.dtype == DTYPE.INT); 
      _Unsafe.copyMemory(src, Unsafe.ARRAY_INT_BASE_OFFSET + srcIndex * sizeof(DTYPE.INT),
                         null, address + idx * sizeof(DTYPE.INT), 
                         count * sizeof(DTYPE.INT));
    }

    public void putInts(int idx, int count, long srcAddress, int srcIndex) {
      assert(this.dtype == DTYPE.INT); 
      _Unsafe.copyMemory(null, srcAddress + srcIndex * sizeof(DTYPE.INT),
                         null, address + idx * sizeof(DTYPE.INT), 
                         count * sizeof(DTYPE.INT));
    }

    // -------- for boolean data --------
    public boolean getBoolean(int idx) {
      assert(this.dtype == DTYPE.BOOL); 
      return _Unsafe.getInt(address + idx * sizeof(DTYPE.INT)) == 1;
    }

    public boolean[] getBooleans(int idx, int count) {
      assert(this.dtype == DTYPE.BOOL); 
      int[] iret = new int[count];
      _Unsafe.copyMemory(null, address + idx * sizeof(DTYPE.INT),
                         iret, Unsafe.ARRAY_INT_BASE_OFFSET,
                         count * sizeof(DTYPE.INT));
      boolean[] ret = new boolean[count];
      for(int i = 0; i < count; ++i) ret[i] = (iret[i] == 1);
      return ret;
    }

    public void putBoolean(int idx, boolean value) {
      assert(this.dtype == DTYPE.BOOL); 
      _Unsafe.putInt(null, address + idx * sizeof(DTYPE.INT), value ? 1 : 0);
    }

    public void putBoolean(int idx, int count, boolean[] src, int srcIndex) {
      assert(this.dtype == DTYPE.BOOL); 
      int[] tmp = new int[count];
      for(int i = 0; i < count; ++i) tmp[i] = src[srcIndex + i] ? 1 : 0;
      _Unsafe.copyMemory(tmp, Unsafe.ARRAY_INT_BASE_OFFSET,
                         null, address + idx * sizeof(DTYPE.INT),
                         count * sizeof(DTYPE.INT));
    }

    public void putBooleans(int idx, int count, long srcAddress, int srcIndex) {
      assert(this.dtype == DTYPE.BOOL); 
      _Unsafe.copyMemory(null, srcAddress + srcIndex * sizeof(DTYPE.INT),
                         null, address + idx * sizeof(DTYPE.INT),
                         count * sizeof(DTYPE.INT));
    }

    // -------- for byte data --------
    public byte getByte(int idx) {
      assert(this.dtype == DTYPE.BYTE); 
      return _Unsafe.getByte(address + idx * sizeof(DTYPE.BYTE));
    }

    public byte[] getBytes(int idx, int count) {
      assert(this.dtype == DTYPE.BYTE); 
      byte[] ret = new byte[count];
      _Unsafe.copyMemory(null, address + idx * sizeof(DTYPE.BYTE),
                         ret, Unsafe.ARRAY_BYTE_BASE_OFFSET,
                         count * sizeof(DTYPE.BYTE));
      return ret;
    }

    public void putByte(int idx, byte value) {
      assert(this.dtype == DTYPE.BYTE); 
      _Unsafe.putByte(null, address + idx * sizeof(DTYPE.BYTE), value);
    }

    public void putBytes(int idx, int count, byte[] src, int srcIndex) {
      assert(this.dtype == DTYPE.BYTE); 
      _Unsafe.copyMemory(src, Unsafe.ARRAY_BYTE_BASE_OFFSET + srcIndex * sizeof(DTYPE.BYTE),
                         null, address + idx * sizeof(DTYPE.BYTE),
                         count * sizeof(DTYPE.BYTE));
    }

    public void putBytes(int idx, int count, long srcAddress, int srcIndex) {
      assert(this.dtype == DTYPE.BYTE); 
      _Unsafe.copyMemory(null, srcAddress + srcIndex * sizeof(DTYPE.BYTE),
                         null, address + idx * sizeof(DTYPE.BYTE),
                         count * sizeof(DTYPE.BYTE));
    }
}
