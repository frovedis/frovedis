"""
server.py
"""

#!/usr/bin/env python

from . import node, rpclib
class FrovedisServer(object):
    """A singleton implementation to store Frovedis server information"""

    __instance = None
    __cmd = "mpirun -np 1 /opt/nec/nosupport/frovedis/ve/bin/frovedis_server"
    #default command

    def __new__(cls):
        if FrovedisServer.__instance is None:
            n_init_server = rpclib.initialize_server(\
                            FrovedisServer.__cmd.encode('ascii'))
            excpt = rpclib.check_server_exception()
            if excpt["status"]:
                raise RuntimeError(excpt["info"])
            #encoding hostname string to ascii,  since it is the key-parameter to
            #all rpc call
            host = (n_init_server['hostname']).encode('ascii')
            port = n_init_server['rpcport']
            FrovedisServer.__instance = object.__new__(cls)
            FrovedisServer.__instance.mnode = node.exrpc_node(host, port)
            FrovedisServer.__instance.wsize = rpclib.get_worker_size(host, port)
            excpt = rpclib.check_server_exception()
            if excpt["status"]:
                raise RuntimeError(excpt["info"])
        return FrovedisServer.__instance

    @classmethod
    def getServerInstance(cls):
        """
        getServerInstance
        """
        inst = cls()
        return (inst.mnode.get_host(), inst.mnode.get_port())

    @classmethod
    def initialize(cls, command):
        """
        initialized
        """
        if FrovedisServer.__instance is None:
            FrovedisServer.__cmd = command
        #else:
        #   print("Frovedis server is already initialized!!")
        cls.getServerInstance()

    @classmethod
    def shut_down(cls):
        """
        shut_down
        """
        if FrovedisServer.__instance is not None:
            (host, port) = cls.getServerInstance()
            rpclib.clean_server(host, port)
            excpt = rpclib.check_server_exception()
            if excpt["status"]:
                raise RuntimeError(excpt["info"])
            rpclib.finalize_server(host, port)
            excpt = rpclib.check_server_exception()
            if excpt["status"]:
                raise RuntimeError(excpt["info"])
            FrovedisServer.__instance = None
        #else:
        #    print("No server to finalize!")

    def display(self):
        """
        display
        """
        self.mnode.display()
        print("Frovedis master has ",str(self.wsize)," workers.")

    @classmethod
    def isUP(cls):
        """
        isUP
        """
        if FrovedisServer.__instance is None:
            return False
        else: 
            return True

# ensuring Frovedis Server will  definitely shut-down on termination of
# a python program which will import this module.
import atexit
atexit.register(FrovedisServer.shut_down)
