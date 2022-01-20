package com.nec.frovedis.Jsql;

import java.lang.ref.ReferenceQueue;
import java.lang.ref.PhantomReference;
import java.util.ArrayList;
import com.nec.frovedis.sql.FrovedisDataFrame;
import com.nec.frovedis.Jexrpc.JNISupport;
import com.nec.frovedis.Jexrpc.FrovedisServer;
import java.rmi.ServerException;

public class FrovedisDataFrameFinalizer extends PhantomReference<FrovedisDataFrame> {

  static AutoCloseThread thread = new AutoCloseThread();
  static { thread.start(); }

  private long proxy;

  private FrovedisDataFrameFinalizer(FrovedisDataFrame df, 
                                    ReferenceQueue<FrovedisDataFrame> queue) { 
    super(df, queue);
    this.proxy = df.get();
  }

  public void release() throws ServerException {
    JNISupport.lockParallel();
    if (FrovedisServer.isUP() && proxy != -1) {
      FrovedisServer fs = FrovedisServer.getServerInstance();
      JNISupport.releaseFrovedisDataframe(fs.master_node, proxy);
      String info = JNISupport.checkServerException();
      if (!info.isEmpty()) throw new ServerException(info);
      //System.out.println("released " + proxy);
      proxy = -1;
      thread.removeObject(this);
    }
    //System.out.println("reference pool size: " + thread.list.size());
    JNISupport.unlockParallel();
  }

  public static FrovedisDataFrameFinalizer 
  addObject(FrovedisDataFrame df) {
    return thread.addObject(df);
  }
    
  public static class AutoCloseThread extends Thread {
    private ReferenceQueue<FrovedisDataFrame> queue = new ReferenceQueue<>();
    private ArrayList<FrovedisDataFrameFinalizer> list = new ArrayList<FrovedisDataFrameFinalizer>();

    private AutoCloseThread() {
      setDaemon(true);
      setName("FrovedisDataFrameFinalizer");
    }

    private FrovedisDataFrameFinalizer 
    addObject(FrovedisDataFrame df) {
      final FrovedisDataFrameFinalizer ref = new FrovedisDataFrameFinalizer(df, queue);
      list.add(ref);
      return ref;
    }

    private void 
    removeObject(FrovedisDataFrameFinalizer ref) {
       list.remove(ref);
    }

    @Override
    public void run() {
      try {
        while (true) {
          // queue.remove(): blocking if queue is empty
          FrovedisDataFrameFinalizer ref = (FrovedisDataFrameFinalizer) queue.remove();
          //System.out.println(ref.proxy + " is not properly released, doing it now...");
          ref.release();
        }
      } catch (Exception e) {
         System.out.println(this.getName() + ": thread interrupted => " + e.getMessage());
      }
    }
  }
}
