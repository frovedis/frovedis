__version__ = "1.1.0"

import signal
native_handler = { 
  signal.SIGINT: signal.getsignal(signal.SIGINT),
  signal.SIGTERM: signal.getsignal(signal.SIGTERM)
}

from .exrpc.server import FrovedisServer

def clean_dump(sig, frame):
    import os
    import sys
    import glob
    import shutil
    #print("caught signal %d" % sig)
    if "FROVEDIS_TMPDIR" in os.environ:
        tmpdir = os.environ["FROVEDIS_TMPDIR"] + "/frovedis_w2v_dump_*"
    else:
        tmpdir = "/var/tmp/frovedis_w2v_dump_*"
    w2v_dump = glob.glob(tmpdir)
    for each in w2v_dump:
        #print("removing dump: " + each)
        shutil.rmtree(each)
    FrovedisServer.reset() # safe, since signal handler has alreday terminated the server process
    native_handler[sig]()

# ensuring Frovedis Server will definitely be shut-down on termination of
# a python program which will import this module.
import atexit
atexit.register(FrovedisServer.shut_down)

signal.signal(signal.SIGINT, clean_dump)
signal.signal(signal.SIGTERM, clean_dump)
