from collections.abc import MutableMapping
from collections.abc import MutableSequence
from .endurox import *

# UBF Dictionary field, kind of array
class UbfDictFld(MutableSequence):
    """Access to UBF field dictionary"""
    # parent buffer to access to
    ubf_dict = ""
    # Resolved field id we want to access
    fldid = 0

    # get the item
    def __getitem__(self, i):
        return UbfDictFld_get(self, i)

    # delete the item
    def __delitem__(self, i):
        return UbfDictFld_del(self, i)
    
    # get field length / occurrences
    def __len__(self):
        return UbfDictFld_len(self)
    
    # set item
    def __setitem__(self, i, value):
        return UbfDictFld_set(self, i, value)

    # insert item
    def insert(self, i, value):
        return UbfDictFld_set(self, i, value)
        
# UBF <-> Dictionary mapping
class UbfDict(MutableMapping):
    """UBF Based dictionary, direct access to fields
       without full transformation"""

    # are we operating from sub-buffer?
    is_sub_buffer = False
    
    # XATMI buffer ptr
    buf = 0

    #  TODO: 
    is_rw_ptr = True

    # Create new buffer from dictionary
    # or if UbfDict passed, then do direct copy
    def __init__(self, *args, **kwargs):

        if len(args) == 1 and isinstance(args[0], UbfDict):
            # Copy buffer
            self.buf = UbfDict_copy(args[0].buf)
        else:
            # allocate UBF
            #self.store = dict()
            self.buf = tpalloc("UBF", "", 1024)
            # Load the dictionary with fields
            UbfDict_load(self.buf, dict(*args, **kwargs))

    # In case if having ptr, to UBF -> return new buffer
    # In case if having VIEW -> convert to dict()
    # In case if having UBF, return sub-buffer / read only.
    def __getitem__(self, key):
        return UbfDict_get(self, key)

    #
    # If we are sub-subffer, throw exception of read only access.
    # cope with scenarious:
    # This can be: list, dict, UbfDictFLd. Depending on the field type
    # we shall take correspoding action? from list load all fields, with type cast?
    # with dict only for FLD_PTR or FLD_UFB. And UbfDictFld the same as LFD_UBF
    # 
    def __setitem__(self, key, value):
        UbfDict_set(self.buf, key, value)
        #self.store[self._keytransform(key)] = value

    #
    # Delete full key (all occs)
    #
    def __delitem__(self, key):
        return UbfDict_del(self.buf, key)

    # Start iteration
    def __iter__(self):
        return UbfDict_iter(self.buf)
    
    # next field
    def __next__(next):
        return UbfDict_next(self.buf)

    # number of fields?
    def __len__(self):
        return UbfDict_len(self.buf)

    # Deleting (Calling destructor)
    def __del__(self):
        #print('Destructor called, Employee deleted.')
        if self.buf!=0:
            tpfree(self.buf);
            self.buf=0

    # manual free up of the buffer
    def free(self):
        __del__(self)

# vim: set ts=4 sw=4 et smartindent:
