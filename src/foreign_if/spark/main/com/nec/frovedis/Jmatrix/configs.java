package com.nec.frovedis.Jmatrix;

public class configs implements java.io.Serializable {
  public static boolean rawsend_enabled = true;    // TODO: make private; provide setter
  public static boolean transfer_by_batch = false; // TODO: make private; provide setter
  private static int buf_size = 20 * 1024 * 1024;
  private static int str_size = 250; // assumed max size of a string (like address, comments etc.)

  public static int get_default_buffer_size () { return buf_size; }
  public static void set_default_buffer_size (int size_in_mb) { // minimum 1 MB
    assert(size_in_mb > 0);
    buf_size = size_in_mb * 1024 * 1204;
  }

  public static int get_default_string_size () { return str_size; }
  public static void set_default_string_size (int size) {
    assert(size > 0);
    str_size = size;
  }
}
