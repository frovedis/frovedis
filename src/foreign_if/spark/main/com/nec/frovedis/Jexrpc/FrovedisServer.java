package com.nec.frovedis.Jexrpc;

import java.rmi.ServerException;

//--- singleton class ---
public class FrovedisServer implements java.io.Serializable {
  public Node master_node;
  public int worker_size;
  private static boolean instantiated = false;
  // default command
  private static String command = "mpirun -np 1 /opt/nec/nosupport/frovedis/ve/bin/frovedis_server"; 
  private static FrovedisServer server;

  private FrovedisServer() throws ServerException {
    master_node = JNISupport.getMasterInfo(command); // native call
    String info = JNISupport.checkServerException();
    if (!info.isEmpty()) throw new ServerException(info);
    worker_size = JNISupport.getWorkerSize(master_node); // native call
    info = JNISupport.checkServerException();
    if (!info.isEmpty()) throw new ServerException(info);
    instantiated = true;
    //System.out.println("\nServer instance created with command: " + command);
    //System.out.println(toString());
  }
  private static FrovedisServer createOrGetServer() throws ServerException {
    if (!instantiated) server = new FrovedisServer();
    return server;
  }
  // if called, before calling initialize,
  // it will try to create the server instance with default command and return
  // if server is already initialized, it simply returns the same server
  public static FrovedisServer getServerInstance() throws ServerException {
      return createOrGetServer();
  }
  // Spark programmar can call this method in the very begining,
  // if they want to initialize the Frovedis server with
  // different command other than the default one.
  public static FrovedisServer initialize(String cmd) throws ServerException {
      if(!instantiated) command = cmd;
      else System.out.println("Frovedis server is already initialized!!");
      return createOrGetServer();
  }
  public static void shut_down() throws ServerException{
    if(instantiated) {
      Node fm_node = getServerInstance().master_node; 
      JNISupport.cleanUPFrovedisServer(fm_node); // native call
      String info = JNISupport.checkServerException();
      if (!info.isEmpty()) throw new ServerException(info);
      JNISupport.finalizeFrovedisServer(fm_node); // native call
      info = JNISupport.checkServerException();
      if (!info.isEmpty()) throw new ServerException(info);
      instantiated = false;
    }
  }
  public static void display() throws ServerException {
    if(instantiated) System.out.println(getServerInstance().toString());
    else System.out.println("No server to display!\n");
  }
  @Override
  public String toString() { 
    if(instantiated) 
      return "Master Node (" + master_node.get_host() + "," + 
              master_node.get_port() + ") has " + worker_size + " workers.\n";
    else return "Server not found";
  }
	/*
  @Override
  protected void finalize() {
    shut_down();
  }
	*/
}
