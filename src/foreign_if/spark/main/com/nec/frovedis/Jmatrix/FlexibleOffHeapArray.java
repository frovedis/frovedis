package com.nec.frovedis.Jmatrix;

import java.util.ArrayList;

public class FlexibleOffHeapArray {
    private ArrayList<OffHeapArray> bufs;
    private OffHeapArray active_buf;
    private int size;
    private int curpos;
    private short dtype;
    private OffHeapArray flatten_array;
    private boolean flatten_movable;

    public FlexibleOffHeapArray(int size, short dtype) {
      this.size = size;
      this.dtype = dtype;
      active_buf = new OffHeapArray(size, dtype);
      bufs = new ArrayList<OffHeapArray>();
      bufs.add(active_buf);
      curpos = 0; 
      flatten_array = null;
      flatten_movable = false;
    }

    public int get_active_length ()  { 
      return curpos + size * (bufs.size() - 1); 
    }

    public int size ()  { 
      return size * bufs.size(); 
    }

    public OffHeapArray getFlattenMemory () {
      if (flatten_movable) flatten_array.freeMemory(); // to free previously flattened memory, if any

      int tot = bufs.size(); 
      if (tot == 1) {
        flatten_array = active_buf;
        flatten_array.set_active_length(curpos);
        flatten_movable = false;
      }
      else {
        int alen = get_active_length();
        flatten_array = new OffHeapArray(alen, dtype);
        flatten_movable = true;
        for(int i = 0; i < tot; ++i) {
          int cur = i * size;
          int tsz = (alen >= size) ? size : alen;
          OffHeapArray b = bufs.get(i);
          switch(dtype) {
            case DTYPE.BOOL: 
            case DTYPE.INT: {
              flatten_array.putInts(cur, tsz, b.get(), 0);     break;
            }
            case DTYPE.LONG: {
              flatten_array.putLongs(cur, tsz, b.get(), 0);    break;
            }
            case DTYPE.FLOAT: {
              flatten_array.putFloats(cur, tsz, b.get(), 0);   break;
            }
            case DTYPE.DOUBLE: {
              flatten_array.putDoubles(cur, tsz, b.get(), 0);  break;
            }
            case DTYPE.BYTE: {
              flatten_array.putBytes(cur, tsz, b.get(), 0);    break;
            }
            default: throw new 
              IllegalArgumentException("Unsupported type is encountered!\n");
          }
          alen -= size;
        }
      }
      return flatten_array;
    }

    public void show () {
      System.out.println("size: " + size());
      System.out.println("active length: " + get_active_length());
      //for(int i = 0; i < bufs.size(); ++i) bufs.get(i).show();
      OffHeapArray tmp = getFlattenMemory(); tmp.show();
      if (flatten_movable) flatten_array.freeMemory(); // to free in case of actual flattening
    }

    public void freeMemory() { 
      for(int i = 0; i < bufs.size(); ++i) bufs.get(i).freeMemory();
      bufs = null;
      active_buf = null;
      size = 0;
      curpos = 0;
      dtype = DTYPE.NONE;
      if (flatten_movable) {
        flatten_array.freeMemory();
        flatten_array = null;
        flatten_movable = false;
      }
    }

    private void allocateNext() {
      active_buf = new OffHeapArray(size, dtype);
      bufs.add(active_buf);
      curpos = 0;
    }

    // -------- for double data --------
    public double getDouble(int idx) {
      int rid = idx / size; 
      assert(rid < bufs.size());
      int cid = idx % size; 
      return bufs.get(rid).getDouble(cid); 
    }

    public double[] getDoubles(int idx, int count) {
      int rid = idx / size; 
      assert(rid < bufs.size());
      int cid = idx % size;
      int rem = size - cid - 1; 
      if (rem < count) throw new 
        UnsupportedOperationException("getDoubles: count is too big!");
      return bufs.get(rid).getDoubles(cid, count); 
    }

    public void putDouble(double value) {
      if (curpos >= size) allocateNext();
      active_buf.putDouble(curpos, value);
      ++curpos;
    }

    public void putDoubles(double[] src) {
      putDoubles(src.length, src, 0);
    }

    public void putDoubles(int count, double[] src, int srcIndex) {
      int rem = size - curpos; 
      if (rem >= count) {
        active_buf.putDoubles(curpos, count, src, srcIndex);
        curpos += count;
      } else {
        active_buf.putDoubles(curpos, rem, src, srcIndex);
        allocateNext();
        putDoubles(count - rem, src, srcIndex + rem);
      }
    }
    
    public void putDoubles(int count, long srcAddress, int srcIndex) { 
      int rem = size - curpos; 
      if (rem >= count) {
        active_buf.putDoubles(curpos, count, srcAddress, srcIndex);
        curpos += count;
      } else {
        active_buf.putDoubles(curpos, rem, srcAddress, srcIndex);
        allocateNext();
        putDoubles(count - rem, srcAddress, srcIndex + rem);
      }
    }

    // -------- for float data --------
    public float getFloat(int idx) {
      int rid = idx / size;
      assert(rid < bufs.size());
      int cid = idx % size;
      return bufs.get(rid).getFloat(cid);
    }

    public float[] getFloats(int idx, int count) {
      int rid = idx / size;
      assert(rid < bufs.size());
      int cid = idx % size;
      int rem = size - cid - 1;
      if (rem < count) throw new
        UnsupportedOperationException("getFloats: count is too big!");
      return bufs.get(rid).getFloats(cid, count);
    }

    public void putFloat(float value) {
      if (curpos >= size) allocateNext();
      active_buf.putFloat(curpos, value);
      ++curpos;
    }

    public void putFloats(float[] src) {
      putFloats(src.length, src, 0);
    }

    public void putFloats(int count, float[] src, int srcIndex) {
      int rem = size - curpos;
      if (rem >= count) {
        active_buf.putFloats(curpos, count, src, srcIndex);
        curpos += count;
      } else {
        active_buf.putFloats(curpos, rem, src, srcIndex);
        allocateNext();
        putFloats(count - rem, src, srcIndex + rem);
      }
    }

    public void putFloats(int count, long srcAddress, int srcIndex) {
      int rem = size - curpos;
      if (rem >= count) {
        active_buf.putFloats(curpos, count, srcAddress, srcIndex);
        curpos += count;
      } else {
        active_buf.putFloats(curpos, rem, srcAddress, srcIndex);
        allocateNext();
        putFloats(count - rem, srcAddress, srcIndex + rem);
      }
    }

    // -------- for long data --------
    public long getLong(int idx) {
      int rid = idx / size;
      assert(rid < bufs.size());
      int cid = idx % size;
      return bufs.get(rid).getLong(cid);
    }

    public long[] getLongs(int idx, int count) {
      int rid = idx / size;
      assert(rid < bufs.size());
      int cid = idx % size;
      int rem = size - cid - 1;
      if (rem < count) throw new
        UnsupportedOperationException("getLongs: count is too big!");
      return bufs.get(rid).getLongs(cid, count);
    }

    public void putLong(long value) {
      if (curpos >= size) allocateNext();
      active_buf.putLong(curpos, value);
      ++curpos;
    }

    public void putLongs(long[] src) {
      putLongs(src.length, src, 0);
    }

    public void putLongs(int count, long[] src, int srcIndex) {
      int rem = size - curpos;
      if (rem >= count) {
        active_buf.putLongs(curpos, count, src, srcIndex);
        curpos += count;
      } else {
        active_buf.putLongs(curpos, rem, src, srcIndex);
        allocateNext();
        putLongs(count - rem, src, srcIndex + rem);
      }
    }

    public void putLongs(int count, long srcAddress, int srcIndex) {
      int rem = size - curpos;
      if (rem >= count) {
        active_buf.putLongs(curpos, count, srcAddress, srcIndex);
        curpos += count;
      } else {
        active_buf.putLongs(curpos, rem, srcAddress, srcIndex);
        allocateNext();
        putLongs(count - rem, srcAddress, srcIndex + rem);
      }
    }

    // -------- for int data --------
    public int getInt(int idx) {
      int rid = idx / size;
      assert(rid < bufs.size());
      int cid = idx % size;
      return bufs.get(rid).getInt(cid);
    }

    public int[] getInts(int idx, int count) {
      int rid = idx / size;
      assert(rid < bufs.size());
      int cid = idx % size;
      int rem = size - cid - 1;
      if (rem < count) throw new
        UnsupportedOperationException("getInts: count is too big!");
      return bufs.get(rid).getInts(cid, count);
    }

    public void putInt(int value) {
      if (curpos >= size) allocateNext();
      active_buf.putInt(curpos, value);
      ++curpos;
    }

    public void putInts(int[] src) {
      putInts(src.length, src, 0);
    }

    public void putInts(int count, int[] src, int srcIndex) {
      int rem = size - curpos;
      if (rem >= count) {
        active_buf.putInts(curpos, count, src, srcIndex);
        curpos += count;
      } else {
        active_buf.putInts(curpos, rem, src, srcIndex);
        allocateNext();
        putInts(count - rem, src, srcIndex + rem);
      }
    }

    public void putInts(int count, long srcAddress, int srcIndex) {
      int rem = size - curpos;
      if (rem >= count) {
        active_buf.putInts(curpos, count, srcAddress, srcIndex);
        curpos += count;
      } else {
        active_buf.putInts(curpos, rem, srcAddress, srcIndex);
        allocateNext();
        putInts(count - rem, srcAddress, srcIndex + rem);
      }
    }

    // -------- for boolean data --------
    public boolean getBoolean(int idx) {
      int rid = idx / size;
      assert(rid < bufs.size());
      int cid = idx % size;
      return bufs.get(rid).getBoolean(cid);
    }

    public boolean[] getBooleans(int idx, int count) {
      int rid = idx / size;
      assert(rid < bufs.size());
      int cid = idx % size;
      int rem = size - cid - 1;
      if (rem < count) throw new
        UnsupportedOperationException("getBooleans: count is too big!");
      return bufs.get(rid).getBooleans(cid, count);
    }

    public void putBoolean(boolean value) {
      if (curpos >= size) allocateNext();
      active_buf.putBoolean(curpos, value);
      ++curpos;
    }

    public void putBooleans(boolean[] src) {
      putBooleans(src.length, src, 0);
    }

    public void putBooleans(int count, boolean[] src, int srcIndex) {
      int rem = size - curpos;
      if (rem >= count) {
        active_buf.putBooleans(curpos, count, src, srcIndex);
        curpos += count;
      } else {
        active_buf.putBooleans(curpos, rem, src, srcIndex);
        allocateNext();
        putBooleans(count - rem, src, srcIndex + rem);
      }
    }

    public void putBooleans(int count, long srcAddress, int srcIndex) {
      int rem = size - curpos;
      if (rem >= count) {
        active_buf.putBooleans(curpos, count, srcAddress, srcIndex);
        curpos += count;
      } else {
        active_buf.putBooleans(curpos, rem, srcAddress, srcIndex);
        allocateNext();
        putBooleans(count - rem, srcAddress, srcIndex + rem);
      }
    }

    // -------- for byte data --------
    public byte getByte(int idx) {
      int rid = idx / size;
      assert(rid < bufs.size());
      int cid = idx % size;
      return bufs.get(rid).getByte(cid);
    }

    public byte[] getBytes(int idx, int count) {
      int rid = idx / size;
      assert(rid < bufs.size());
      int cid = idx % size;
      int rem = size - cid - 1;
      if (rem < count) throw new
        UnsupportedOperationException("getBytes: count is too big!");
      return bufs.get(rid).getBytes(cid, count);
    }

    public void putByte(byte value) {
      if (curpos >= size) allocateNext();
      active_buf.putByte(curpos, value);
      ++curpos;
    }

    public void putBytes(byte[] src) {
      putBytes(src.length, src, 0);
    }

    public void putBytes(int count, byte[] src, int srcIndex) {
      int rem = size - curpos;
      if (rem >= count) {
        active_buf.putBytes(curpos, count, src, srcIndex);
        curpos += count;
      } else {
        active_buf.putBytes(curpos, rem, src, srcIndex);
        allocateNext();
        putBytes(count - rem, src, srcIndex + rem);
      }
    }

    public void putBytes(int count, long srcAddress, int srcIndex) {
      int rem = size - curpos;
      if (rem >= count) {
        active_buf.putBytes(curpos, count, srcAddress, srcIndex);
        curpos += count;
      } else {
        active_buf.putBytes(curpos, rem, srcAddress, srcIndex);
        allocateNext();
        putBytes(count - rem, srcAddress, srcIndex + rem);
      }
    }
}
