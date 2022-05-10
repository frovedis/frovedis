"""
server.py
"""

import warnings
from . import node, rpclib

class ServerID(object):
    """A python container for generating IDs for frovedis server"""
    __initial = 0
    __id = __initial
    # A threshold value, assuming it is safe to re-iterate
    # server ID after reaching this value
    __max_id = (1 << 31)

    @staticmethod
    def get():
        """
        NAME: get
        """
        ServerID.__id = (ServerID.__id + 1) % ServerID.__max_id
        if ServerID.__id == 0:
            ServerID.__id = ServerID.__initial + 1
        return ServerID.__id

def explain(server_inst):
    """ to_string() for server instance """
    if server_inst is not None:
        withmsg = "with " + str(server_inst.wsize) + " MPI process"
        if server_inst.wsize > 1:
            withmsg += "es."
        else:
            withmsg += "."
        return "[ID: " + str(server_inst.sid) + "] FrovedisServer (" + \
               str(server_inst.mnode) + ") has been initialized " + withmsg
    else: 
        return "No active server is found!!";

class FrovedisServer(object):
    """A singleton implementation to store Frovedis server information"""

    #default command
    __cmd = "/opt/nec/ve/bin/mpirun -np 8 " + \
            "/opt/nec/frovedis/ve/bin/frovedis_server"
    __instance = None

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
            FrovedisServer.__instance.sid = ServerID.get()
            excpt = rpclib.check_server_exception()
            if excpt["status"]:
                raise RuntimeError(excpt["info"])
        return FrovedisServer.__instance

    @classmethod
    def setCommand(cls, command):
        """
        sets the default command for server initialization
        """
        if not isinstance(command, str):
            raise ValueError(\
            "expected a string as for server initialization command!")
        FrovedisServer.__cmd = command

    @classmethod
    def getCommand(cls):
        """
        returns the default command in case the server is not initialized;
        otherwise returns the command used for server initialization
        """
        return FrovedisServer.__cmd

    @classmethod
    def getServerInstance(cls):
        """
        getServerInstance
        """
        inst = cls()
        return (inst.mnode.get_host(), inst.mnode.get_port())

    @classmethod
    def getSize(cls):
        """
        no. of mpi process at server side
        """
        return cls().wsize

    @classmethod
    def initialize(cls, command):
        """ 
        to initialize a new server (if no server is running) 
        with specified command
        """
        if FrovedisServer.__instance is None:
            cls.setCommand(command)
            cls.getServerInstance()
        else:
            print("FrovedisServer is already initialized!!")
        return explain(FrovedisServer.__instance)

    @classmethod
    def shut_down(cls):
        """ to shut_down the current server """
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

    @classmethod
    def display(cls):
        """ to display server information"""
        print(explain(FrovedisServer.__instance))

    @classmethod
    def getID(cls):
        """ to get id of the current server """
        if FrovedisServer.__instance is None:
            warnings.warn("FrovedisServer is not initialized, hence "
                          "initializing the server with default command!",
                          category=UserWarning)
            FrovedisServer.getServerInstance()
        return FrovedisServer.__instance.sid

    @classmethod
    def isUP(cls, server_id=None):
        """ 
        to confirm if the current server or 
        the specified server with given id is UP 
        """
        if FrovedisServer.__instance is None:
            return False
        else: 
            if server_id is None: # query made for existing server
                return True
            else:  # query made for some specific server
                return server_id == FrovedisServer.getID()

    @classmethod
    def reset(cls):
        """ 
        resets server instance -> should be called only when it is 
        guaranteed that the server is already terminated.
        """
        FrovedisServer.__instance = None

def check_server_state(server_id, inst_class_name):
    if not FrovedisServer.isUP(server_id):
        raise RuntimeError("FrovedisServer (ID: %d) associated with target "\
                           "'%s' object could not be reached!\n"\
                           "In case it has already been shut-down, "\
                           "you would need to re-fit the object.\n" \
                           % (server_id, inst_class_name))
    return True

# decorator functions used for setting/checking server association
def set_association(func):
    def set_assoc_wrapper(*args, **kwargs):
        obj = args[0] # args[0] of func() must be self
        obj.__sid = FrovedisServer.getID()
        return func(*args, **kwargs)
    return set_assoc_wrapper

def check_association(func):
    def check_assoc_wrapper(*args, **kwargs):
        obj = args[0] # args[0] of func() must be self
        if not obj.is_fitted():
            raise AttributeError(func.__name__ + ": is called before the "\
            "object is actually constructed (fitted) or the object might " \
            "have already been released!")
        check_server_state(obj.__sid, obj.__class__.__name__)
        return func(*args, **kwargs)
    return check_assoc_wrapper

def do_if_active_association(func):
    def do_if_active_assoc_wrapper(*args, **kwargs):
        obj = args[0] # args[0] of func() must be self
        if obj.is_fitted():
            if FrovedisServer.isUP(obj.__sid):
                return func(*args, **kwargs)
            #else:
            #    print("no active server found associated with caller object!")
    return do_if_active_assoc_wrapper

