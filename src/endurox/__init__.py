"""Python3 bindings for writing Enduro/X clients and servers"""

import sys
import ctypes
from collections.abc import MutableMapping

flags = sys.getdlopenflags()

# Need as Enduro/X XA drivers are dynamically loaded, and
# they need to see Enduro/X runtime.
sys.setdlopenflags(flags | ctypes.RTLD_GLOBAL)

from .endurox import *

# change module name... for importeds symbols
sys.setdlopenflags(flags)

__all__ = ['endurox']

#
# High performance UBF dictionary access without any serialization
# required.
#
#
# Access by index, 
class UbfDictFLd(MutableMapping):
    """Access to UBF field dictionary"""
    # parent buffer to access to
    parent_ubf = ""
    # Resolved field id we want to access
    fld_id = 0
    # also have delete?
#
class UbfDict(MutableMapping):
    """UBF Based dictionary, direct access to fields
       without full transformation"""

    # are we operating from sub-buffer?
    is_sub_buffer = False
    
    # XATMI buffer ptr
    buf = 0

    def __init__(self, *args, **kwargs):
        # allocate UBF
        #self.store = dict()
        self.buf = tpalloc("UBF", "", 1024)
        # Load the dictionary with fields
        Bload(self.buf, dict(*args, **kwargs))

    # In case if having ptr, to UBF -> return new buffer
    # In case if having VIEW -> convert to dict()
    # In case if having UBF, return sub-buffer / read only.
    def __getitem__(self, key):
        return self.store[self._keytransform(key)]

    #
    # If we are sub-subffer, throw exception of read only access.
    # cope with scenarious:
    # This can be: list, dict, UbfDictFLd. Depending on the field type
    # we shall take correspoding action? from list load all fields, with type cast?
    # with dict only for FLD_PTR or FLD_UFB. And UbfDictFld the same as LFD_UBF
    # 
    def __setitem__(self, key, value):
        Bchg(self.buf, key, value)
        #self.store[self._keytransform(key)] = value

    #
    # Delete full key (all occs)
    #
    def __delitem__(self, key):
        del self.store[self._keytransform(key)]

    def __iter__(self):
        return iter(self.store)
    
    # number of fields?
    def __len__(self):
        return len(self.store)

    def _keytransform(self, key):
        return key

    # Deleting (Calling destructor)
    def __del__(self):
        #print('Destructor called, Employee deleted.')
        if self.buf!=0:
            tpfree(self.buf);
            self.buf=0

    # manual free up of the buffer
    def free(self):
        __del__(self)
        

