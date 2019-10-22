"""
node.py
"""
#!/usr/bin/env python

class exrpc_node:
    """ Python side node structure to contain Frovedis side node information """

    def __init__(self, hostname, rpcport):
        """
        init
        """
        self.__hostname = hostname
        self.__rpcport = rpcport

    def display(self):
        """
        display
        """
        print("Hostname: ",self.__hostname, ", Port: ",str(self.__rpcport))

    def get_host(self):
        """
        get_host
        """
        return self.__hostname

    def get_port(self):
        """
        get_port
        """
        return self.__rpcport
