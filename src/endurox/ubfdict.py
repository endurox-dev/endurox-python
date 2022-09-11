from collections.abc import MutableMapping
from collections.abc import MutableSequence
from .endurox import *

# UBF Dictionary field, kind of array
class UbfDictFld(MutableSequence):
    """Access to UBF field dictionary"""
    # parent buffer to access to
    # TODO: Validate that we have a buffer set..
    _ubf_dict = None
    # Resolved field id we want to access
    fldid = 0

    # get the item
    def __getitem__(self, i):
        return UbfDictFld_get(self, i)

    # delete the item
    def __delitem__(self, i):

        # Valdiat ethe parent buffer
        if _ubf_dict.is_sub_buffer:
            raise AttributeError('Cannot change sub-buffer')

        return UbfDictFld_del(self, i)
    
    # get field length / occurrences
    def __len__(self):
        return UbfDictFld_len(self)
    
    # set item
    def __setitem__(self, i, value):

        # Valdiat ethe parent buffer
        if _ubf_dict.is_sub_buffer:
            raise AttributeError('Cannot change sub-buffer')

        return UbfDictFld_set(self, i, value)

    # insert item
    def insert(self, i, value):

        # Valdiat ethe parent buffer
        if _ubf_dict.is_sub_buffer:
            raise AttributeError('Cannot change sub-buffer')

        return UbfDictFld_set(self, i, value)

    # Compare two lists...
    def __eq__(self, other):

        if (len(self)!=len(other)):
            return False

        for a, b in zip(self, other):
            if a != b:
                return False
        return True

    # print the field occs... in UD format
    def __repr__(self) -> str:
        return UbfDictFld_repr(self);

# items iteration class    
class UbfDictItems:

    def __init__(self, ubf_dict):
        self.ubf_dict = ubf_dict
        self._buf = ubf_dict._buf

    # Start iteration
    def __iter__(self):
        return UbfDict_iter(self, self._buf)
    
    # next field
    def __next__(self): 
        return UbfDict_next(self, self._buf)

# Items expanded to occurrences
# instead of the lists
class UbfDictItemsOcc:

    def __init__(self, ubf_dict):
        self.ubf_dict = ubf_dict
        self._buf = ubf_dict._buf

    # Start iteration
    def __iter__(self):
        return UbfDict_iter(self, self._buf)
    
    # next field
    def __next__(self): 
        return UbfDict_next_occ(self, self._buf)

    # return len by counting occurrences, instead of the keys
    def __len__(self): 
        return UbfDict_len_occ(self, self._buf)

# UBF <-> Dictionary mapping
class UbfDict(MutableMapping):
    """UBF Based dictionary, direct access to fields
       without full transformation"""

    # are we operating from sub-buffer?
    is_sub_buffer = False
    
    # XATMI buffer ptr
    _buf = 0

    # Create new buffer from dictionary
    # or if UbfDict passed, then do direct copy
    def __init__(self, *args, **kwargs):

        if len(args) == 1 and isinstance(args[0], UbfDict):
            # Copy buffer
            self._buf = UbfDict_copy(args[0]._buf)
        elif len(args) == 1 and isinstance(args[0], bool):

            # Just allocate empty buffer on True
            # For False, keep null, used for constructing
            # the object.
            if True==args[0]:
                self._buf = tpalloc("UBF", "", 1024)
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

        # validate the parent buffer
        if self.is_sub_buffer:
            raise AttributeError('Cannot change sub-buffer')

        UbfDict_set(self._buf, key, value)

    #
    # Delete full key (all occs)
    #
    def __delitem__(self, key):

        # validate the parent buffer
        if self.is_sub_buffer:
            raise AttributeError('Cannot change sub-buffer')

        return UbfDict_del(self._buf, key)

    #
    # Have some object loop over
    #
    def items(self):
        iter = UbfDictItems(self)
        return  iter

    #
    # Expand occurrances
    #
    def itemsocc(self):
        iter = UbfDictItemsOcc(self)
        return  iter

    def items(self):
        iter = UbfDictItems(self)
        return  iter

    # Compare two lists...
    def __eq__(self, other):

        # in case if both are UbfDict()
        # to UBF level compare, it is faster of course
        if isinstance( other, UbfDict):
            return UbfDict_cmp(self._buf, other._buf)

        if len(self)!=len(other):
            return False

        # iterate our keys...
        # check in theirs...
        # check length...
        for k,v in self.items():
            vv = other[k]
            if v!=vv:
                return False

        return True

    # number of fields?
    def __len__(self):
        return UbfDict_len(self._buf)

    # Deleting (Calling destructor)
    def __del__(self):
        if self._buf!=0:
            tpfree(self._buf, self.is_sub_buffer);
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
        # validate the parent buffer
        if self.is_sub_buffer:
            raise AttributeError('Cannot change sub-buffer')
        __del__(self)

    # Start iteration
    def __iter__(self):
        return UbfDict_iter(self, self._buf)
    
    # next field, keys only
    def __next__(self):
        return UbfDict_next_keys(self, self._buf)

    # return the UBF buffer..
    def __repr__(self):
        return UbfDict_repr(self, self._buf)

    # Convert given UbfDict() to generic Python dictionary
    def to_dict(self):
        return UbfDict_to_dict(self, self._buf)


# vim: set ts=4 sw=4 et smartindent:
