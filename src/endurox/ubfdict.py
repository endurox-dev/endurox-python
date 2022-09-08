from collections.abc import MutableMapping
from collections.abc import MutableSequence
from .endurox import *

# UBF Dictionary field, kind of array
class UbfDictFld(MutableSequence):
    """Access to UBF field dictionary"""
    # parent buffer to access to
    _ubf_dict = ""
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
    _buf = 0

    #  TODO: 
    is_rw_ptr = True

    # Create new buffer from dictionary
    # or if UbfDict passed, then do direct copy
    def __init__(self, *args, **kwargs):

        if len(args) == 1 and isinstance(args[0], UbfDict):
            # Copy buffer
            self._buf = UbfDict_copy(args[0]._buf)
        else:
            # allocate UBF
            #self.store = dict()
            self._buf = tpalloc("UBF", "", 1024)
            # Load the dictionary with fields
            UbfDict_load(self._buf, dict(*args, **kwargs))

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
        UbfDict_set(self._buf, key, value)

    #
    # Delete full key (all occs)
    #
    def __delitem__(self, key):
        return UbfDict_del(self._buf, key)

    # Start iteration
    def __iter__(self):
        return UbfDict_iter(self, self._buf)
    
    # next field
    def __next__(self):
        return UbfDict_next(self, self._buf)

    # compare this Ubf with Other...
    def __eq__(self, other):
        return UbfDict_cmp(self._buf, other._buf)

    # number of fields?
    def __len__(self):
        return UbfDict_len(self._buf)

    # Deleting (Calling destructor)
    def __del__(self):
        if self._buf!=0 and not self.is_sub_buffer:
            tpfree(self._buf);
            self._buf=0

    # Override copy interface, without this _buf is
    # copied over resulting in two objects pointing
    # to the one Ubf...
    def __copy__(self):
        inst = UbfDict()
        inst._buf = UbfDict_copy(self._buf)
        return inst

    #
    # Copy in the same way...
    #
    def __deepcopy__(self, memodict={}):
        inst = UbfDict()
        inst._buf = UbfDict_copy(self._buf)
        return inst

    # manual free up of the buffer
    def free(self):
        __del__(self)

# vim: set ts=4 sw=4 et smartindent:
