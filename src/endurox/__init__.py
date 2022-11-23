"""Python3 bindings for writing Enduro/X clients and servers"""

import sys
import ctypes
import io
import traceback

flags = sys.getdlopenflags()

# Need as Enduro/X XA drivers are dynamically loaded, and
# they need to see Enduro/X runtime.
sys.setdlopenflags(flags | ctypes.RTLD_GLOBAL)
from .endurox import *
# change module name... for importeds symbols
sys.setdlopenflags(flags)

from .ubfdict import UbfDict
from .ubfdict import UbfDictFld
from .ubfdict import UbfDictItems
from .ubfdict import UbfDictItemsOcc

__all__ = ['endurox']

################################################################################
# python based functions for the module
################################################################################

def tplog_exception(msg):
    '''Print exception backtrace at error level. Backtraces currently activate
    exception.

    Paramters
    ---------
    msg: str
        Log message

    '''
    sio = io.StringIO()

    t, v, tb = sys.exc_info()
    traceback.print_exception(t, v, tb, None, sio)
    s = sio.getvalue()
    sio.close()
    if s[-1:] == "\n":
        s = s[:-1]
    tplog_error(msg + ":\n" + s)

# vim: set ts=4 sw=4 et smartindent:
