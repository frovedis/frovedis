package com.nec.frovedis.Jsql;

import java.lang.ref.ReferenceQueue;
import java.lang.ref.PhantomReference;
import java.util.ArrayList;
import com.nec.frovedis.sql.FrovedisGroupedDF;
import com.nec.frovedis.Jexrpc.JNISupport;
import com.nec.frovedis.Jexrpc.FrovedisServer;
import java.rmi.ServerException;

public class FrovedisGroupedDFFinalizer extends PhantomReference<FrovedisGroupedDF> {

  static AutoCloseThread thread = new AutoCloseThread();
  static { thread.start(); }

  private long proxy;

  private FrovedisGroupedDFFinalizer(FrovedisGroupedDF gdf, 
                                     ReferenceQueue<FrovedisGroupedDF> queue) { 
    super(gdf, queue);
    this.proxy = gdf.get();
  }

  public void release() throws ServerException {
    JNISupport.lockParallel();
    if (FrovedisServer.isUP() && proxy != -1) {
      FrovedisServer fs = FrovedisServer.getServerInstance();
      JNISupport.releaseFrovedisGroupedDF(fs.master_node, proxy);
      String info = JNISupport.checkServerException();
      if (!info.isEmpty()) throw new ServerException(info);
      //System.out.println("released grouped_dftable: " + proxy);
      proxy = -1;
      thread.removeObject(this);
    }
    //System.out.println("reference pool size: " + thread.list.size());
    JNISupport.unlockParallel();
  }

  public static FrovedisGroupedDFFinalizer 
  addObject(FrovedisGroupedDF gdf) {
    return thread.addObject(gdf);
  }
    
  public static class AutoCloseThread extends Thread {
    private ReferenceQueue<FrovedisGroupedDF> queue = new ReferenceQueue<>();
    private ArrayList<FrovedisGroupedDFFinalizer> list = new ArrayList<FrovedisGroupedDFFinalizer>();

    private AutoCloseThread() {
      setDaemon(true);
      setName("FrovedisGroupedDFFinalizer");
    }

    private FrovedisGroupedDFFinalizer 
    addObject(FrovedisGroupedDF gdf) {
      final FrovedisGroupedDFFinalizer ref = new FrovedisGroupedDFFinalizer(gdf, queue);
      list.add(ref);
      return ref;
    }

    private void 
    removeObject(FrovedisGroupedDFFinalizer ref) {
       list.remove(ref);
    }

    @Override
    public void run() {
      try {
        while (true) {
          // queue.remove(): blocking if queue is empty
          FrovedisGroupedDFFinalizer ref = (FrovedisGroupedDFFinalizer) queue.remove();
          //System.out.println("grouped_dftable: " + ref.proxy + " is not properly released, doing it now...");
          ref.release();
        }
      } catch (Exception e) {
         System.out.println(this.getName() + ": thread interrupted => " + e.getMessage());
      }
    }
  }
}
