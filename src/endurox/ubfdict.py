from collections.abc import MutableMapping
from collections.abc import MutableSequence
from .endurox import *

# Constants used in module
class UbfDictConst:

    # Normal XATMI buffer
    NDRXPY_SUBBUF_NORM  = 0           

    # Embedded UBF
    NDRXPY_SUBBUF_UBF   = 1

    # This is PTR buffer
    NDRXPY_SUBBUF_PTR   = 2

# UBF Dictionary field, kind of array
class UbfDictFld(MutableSequence):
    """Access to UBF field dictionary. Provides
    list-like interface for UBF buffer field occurrences
    """

    # parent buffer to access to
    _ubf_dict = None

    # Resolved field id we want to access
    fldid = 0

    # get the item
    def __getitem__(self, i):
        """
        Get UBF dictionary field object

        Parameters
        ----------
        i: int
            Index/occurrence to get

        Returns
        -------
        ret : :class:`.UbfDictFld`
            Value from UBF buffer field.

        Raises
        ------
        IndexError
            Invalid index specified
        UbfException
            Following error codes may be present:
            :data:`.BALIGNERR` - Corrupted UBF buffer.
            :data:`.BNOTFLD` - Buffer not UBF.
        """

        return UbfDictFld_get(self, i)

    # delete the item
    def __delitem__(self, i):
        """Delete UBF field occurrence

        Parameters
        ----------
        i: int
            Index/occurrence to delete

        Raises
        ------
        IndexError
            Invalid index specified (occurrence not present)
        AttributeError
            Read only buffer (sub-UBF)
        UbfException
            Following error codes may be present:
            :data:`.BALIGNERR` - Corrupted UBF buffer.
            :data:`.BNOTFLD` - Buffer not UBF.
            :data:`.BBADFLD` - Invalid field ID given (normally would not be thrown).
        """

        # Validate the parent buffer
        if self._ubf_dict._is_sub_buffer==UbfDictConst.NDRXPY_SUBBUF_UBF:
            raise AttributeError('Cannot change sub-buffer')

        return UbfDictFld_del(self, i)
    
    # get field length / occurrences
    def __len__(self):
        """Return number field occurrences

        Raises
        ------
        UbfException
            Following error codes may be present:
            :data:`.BALIGNERR` - Corrupted UBF buffer.
            :data:`.BNOTFLD` - Buffer not UBF.
            :data:`.BBADFLD` - Invalid field ID given (normally would not be thrown).
        """

        return UbfDictFld_len(self)
    
    # set item
    def __setitem__(self, i, value):
        """Set UBF buffer field at given index

        Parameters
        ----------
        i: int
            Index/occurrence to set
        value: object
            Value to set

        Raises
        ------
        UbfException
            Following error codes may be present:
            :data:`.BALIGNERR` - Corrupted UBF buffer.
            :data:`.BNOTFLD` - Buffer not UBF.
            :data:`.BBADFLD` - Invalid field ID given (normally would not be thrown).
        """

        # Validate the parent buffer
        if self._ubf_dict._is_sub_buffer==UbfDictConst.NDRXPY_SUBBUF_UBF:
            raise AttributeError('Cannot change sub-buffer')

        return UbfDictFld_set(self, i, value)

    # insert item
    def insert(self, i, value):
        """Set UBF buffer field at given index

        Parameters
        ----------
        i: int
            Index/occurrence to set
        value: object
            Value to set

        Raises
        ------
        UbfException
            Following error codes may be present:
            :data:`.BALIGNERR` - Corrupted UBF buffer.
            :data:`.BNOTFLD` - Buffer not UBF.
            :data:`.BBADFLD` - Invalid field ID given (normally would not be thrown).
        """

        # Validate the parent buffer
        if self._ubf_dict._is_sub_buffer==UbfDictConst.NDRXPY_SUBBUF_UBF:
            raise AttributeError('Cannot change sub-buffer')

        return UbfDictFld_set(self, i, value)

    # Compare two lists...
    def __eq__(self, other):
        """Compare this field value with other field.
        The other field shall be standard list or UbfDictFld.

        Parameters
        ----------
        other: list or UbfDictFld
            Field list to check with this one
        value: object
            Value to set

        Returns
        -------
        ret : bool
            True if matched, False if not.

        Raises
        ------
        UbfException
            Following error codes may be present:
            :data:`.BALIGNERR` - Corrupted UBF buffer.
            :data:`.BNOTFLD` - Buffer not UBF.
            :data:`.BBADFLD` - Invalid field ID given (normally would not be thrown).
        """

        if (len(self)!=len(other)):
            return False

        for a, b in zip(self, other):
            if a != b:
                return False
        return True

    # print the field occs... in standard list format
    def __repr__(self) -> str:
        """Return the field/occurrence representation in standard list format.
        This transfers all UBF field occurrences to the standard Python list
        and standard list generates the representation.

        Returns
        -------
        ret : str
            UBF field representation in standard list format.

        Raises
        ------
        UbfException
            Following error codes may be present:
            :data:`.BALIGNERR` - Corrupted UBF buffer.
            :data:`.BNOTFLD` - Buffer not UBF.
            :data:`.BBADFLD` - Invalid field ID given (normally would not be thrown).
        """

        ret = []
        for v in self:
            ret.append(v)
        return ret.__repr__()

# items iteration class    
class UbfDictItems:
    """Class provides UbfDict key/value iteration interface
    Object is created by :class:`.UbfDict.items()` method call.
    """

    def __init__(self, ubf_dict):
        """Internal initialised

        Parameters
        ----------
        ubf_dict: UbfDict
            UBF Dictionary object to iterate over
        """
        self.ubf_dict = ubf_dict
        self._buf = ubf_dict._buf

    # Start iteration
    def __iter__(self):
        """Start iteration over the dictionary.
        """
        return UbfDict_iter(self, self._buf)
    
    # next field
    def __next__(self): 
        """Return next field from UBF buffer

        Returns
        -------
        fname : str or int
            Field name, if not resolved int typed field id returned.

        dictfld : UbfDictFld
            UbfDictFld allocated object.

        UbfException
            Following error codes may be present:
            :data:`.BALIGNERR` - Corrupted UBF buffer.
            :data:`.BNOTFLD` Buffer not fielded, not correctly allocated or corrupted.
        """
        return UbfDict_next(self, self._buf)

# Items expanded to occurrences
# instead of the lists
class UbfDictItemsOcc:
    """Class provides UbfDict key/value iteration interface
    Object is created by :class:`.UbfDict.itemsocc()` method call.
    This iterator returns each field occurrence value instead of the list of values
    as with :class:`.UbfDictItems`.
    """

    def __init__(self, ubf_dict):
        """Internal initialised

        Parameters
        ----------
        ubf_dict: UbfDict
            UBF Dictionary object to iterate over
        """
        self.ubf_dict = ubf_dict
        self._buf = ubf_dict._buf

    # Start iteration
    def __iter__(self):
        """Start iteration over the dictionary.
        """
        return UbfDict_iter(self, self._buf)
    
    # next field
    def __next__(self): 
        """Return next field from UBF buffer

        Returns
        -------
        fname : str or int
            Field name, if not resolved int typed field id returned.

        dictfld : object
            Actual field value.

        UbfException
            Following error codes may be present:
            :data:`.BALIGNERR` - Corrupted UBF buffer.
            :data:`.BNOTFLD` Buffer not fielded, not correctly allocated or corrupted.
        """
        return UbfDict_next_occ(self, self._buf)

    # return len by counting occurrences, instead of the keys
    def __len__(self):
        """UBF Buffer length in number of fields in the buffer
        thus counts every occurrence o

        Returns
        -------
        cnt : int
            Total number of fields present in UBF buffer.

        UbfException
            Following error codes may be present:
            :data:`.BALIGNERR` - Corrupted UBF buffer.
            :data:`.BNOTFLD` Buffer not fielded, not correctly allocated or corrupted.
        """
        return UbfDict_len_occ(self._buf)

# UBF <-> Dictionary mapping
class UbfDict(MutableMapping):
    """UBF Based dictionary, direct access to fields
       without full transformation. When using UbfDict there are some considerations to take:

       1) In case if using BFLD_PTR only one UBF buffer may hold the reference when
            the buffer are garbage collected. As when XATMI buffer is freed it removes
            any ptrs inside the buffer. When assinging free standing UbfDict object to
            the BFLD_PTR field of some UBF buffer, the free standing buffer is marked
	    as ptr kind sub-buffer for which GC is disabled.

       2) If different buffers reference the he same buffer via BFLD_PTR, then programmer
            is responsible for having the buffer alive (not freed).

       3) If using BFLD_UBF sub-buffers and having the UbfDict reference to it,
            programmer is responsible for having the main buffer (for which given field is
            subfield) alive and not changed, as when accessing to the BFLD_UBF subfield
            the UbfDict is allocated with C pointer to the data offset in the main bufer.

       """

    # are we operating from sub-buffer?
    _is_sub_buffer = 0
    
    # XATMI buffer ptr
    _buf = 0

    # Create new buffer from dictionary
    # or if UbfDict passed, then do direct copy
    def __init__(self, *args, **kwargs):
        """Initialize UbfDict object. This normally allocates XATMI buffer
        linked to given object and buffer being initialized by passed in value.

        Parameters
        ----------
        args: object
            If only one argument is passed and it is UbfDict typed, then value
            then buffer is copied from UbfDict param. If value type is bool with value
            False, no buffer is allocated. If param is dictionary, then buffer is initialised from
            dictionary.
            If value
        kwargs: object
            Used for building dictionary argument for buffer initialization.

        Returns
        -------
        ret : UbfDict
            Python UbfDict object.

        Raises
        ------
        UbfException
            Following error codes may be present:
            :data:`.BFTOPEN` - Failed to open field definition files.
            :data:`.BBADNAME` - Field not found.
            :data:`.BALIGNERR` - Corrupted buffer or pointing to not aligned memory area.
            :data:`.BNOTFLD` -  Buffer not fielded, not correctly allocated or corrupted. p_ub is NUL
            :data:`.BALIGNERR` - Corrupted buffer or pointing to not aligned memory area.
            :data:`.BNOSPACE` - No space in buffer for string data (not likely to be throw).
        AtmiException
            Following error codes may be present:
            :data:`.TPEINVAL` - Enduro/X is not configured or buffer pointer is NULL
                or not allocated by tpalloc(), invalid environment.
            :data:`.TPENOENT` - Invalid type specified to function. (not likely to be throw)
            :data:`.TPESYSTEM` - System failure occurred during serving.
                See logs i.e. user log, or debugs for more info.
            :data:`.TPEOS` - System failure occurred during serving.
                See logs i.e. user log, or debugs for more info.
        """

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
        """Return initialized UbfDictFld object which allows to access
        to field occurrences. This method by itself does not validate
        that field is present in UBF buffer, instead it resolves the field
        identifier and returns the UbfDictFld with the field id and reference
        to given UbfDict object.

        Parameters
        ----------
        key: object
            Field name (str) or field id (int)

        Returns
        -------
        ret : UbfDictFld
            initialized UBF Dictionary Field.

        Raises
        ------
        UbfException
            Following error codes may be present:
            :data:`.BFTOPEN` - Failed to open field definition files.
            :data:`.BBADNAME` - Field not found.
        """

        return UbfDict_get(self, key)

    #
    # If we are sub-suffer, throw exception of read only access.
    # cope with scenarios:
    # This can be: list, dict, UbfDictFLd. Depending on the field type
    # we shall take corresponding action? from list load all fields, with type cast?
    # with dict only for FLD_PTR or FLD_UFB. And UbfDictFld the same as LFD_UBF
    # 
    def __setitem__(self, key, value):
        """Set field value. Either single occurrence value or list of values.
        For performance reasons, note that this does not delete existing fields,
        the existing matching occurrences are replaced. If buffer exists
        more occurrences than setting,
        those will not be changed, nor deleted. If full replacement of the field
        is required, firstly delete the key.

        Parameters
        ----------
        key: str or int
            Key to set
        value: object
            Single value or list of values. Depending on the field type, the value
            can be str, int, double, byte array, or dictionary (in case of :data:`.BFLD_UBF`,
            :data:`.BFLD_PTR` or :data:`.BFLD_VIEW`) and UbfDict in case of :data:`.BFLD_UBF`.

        Raises
        ------
        AttributeError
            If given buffer is sub-buffer (sub-UBF), values of the buffer is read
            only.
        ValueError
            Given value cannot be converted to UBF format.

        UbfException
            Following error codes may be present:
            :data:`.BALIGNERR Corrupted buffer or pointing to not aligned memory area.
            :data:`.BNOTFLD Buffer not fielded, not correctly allocated or corrupted.
            :data:`.BFTOPEN` - Failed to open field definition files.
            :data:`.BBADNAME` - Field not found.

        AtmiException
            :data:`.BBADNAME` - Field not found.
            :data:`.TPEINVAL` Invalid ptr pointer passed in.
                Either buffer not allocated by tpalloc() or ptr is NULL.
            :data:`.TPEOS` System failure occurred during serving.
                See logs i.e. user log, or debugs for more info.
        """
        # validate the parent buffer
        if self._is_sub_buffer==UbfDictConst.NDRXPY_SUBBUF_UBF:
            raise AttributeError('Cannot change sub-buffer')

        UbfDict_set(self._buf, key, value)

    #
    # Delete full key (all occs)
    #
    def __delitem__(self, key):
        """Delete key from UbfDict. This removes all occurrences of the
        key from the UBF buffer.

        Parameters
        ----------
        key: str or int
            Field name (str) or field id (int)

        Raises
        ------
        UbfException
            Following error codes may be present:
            :data:`.BALIGNERR` Corrupted buffer or pointing to not aligned memory area.
            :data:`.BNOTFLD` Buffer not fielded, not correctly allocated or corrupted.
            :data:`.BBADFLD` Invalid field id passed.
            :data:`.BNOTPRES` Field not present thus not deleted.
        """

        # validate the parent buffer
        if self._is_sub_buffer==UbfDictConst.NDRXPY_SUBBUF_UBF:
            raise AttributeError('Cannot change sub-buffer')

        return UbfDict_del(self._buf, key)

    #
    # Have some object loop over
    #
    def items(self):
        """Returns object for iterating over the buffer in form of key/value.

        Returns
        -------
        ret : UbfDictItems
            Iterator for key/value loop over the buffer. Value returned
            contains the list of field occurrences.
        """

        iter = UbfDictItems(self)
        return  iter

    #
    # Expand occurrences
    #
    def itemsocc(self):
        """Returns object for iterating over the buffer in form of key/value.
        The values returns here each field occurrences. During the iteration
        if field have several occurrences, then the same key is returned several
        times, for each of the occurrence of the given key value.

        Returns
        -------
        ret : UbfDictItemsOcc
            Iterator for key/value loop over the buffer. Value returned
            contains the end value present in every field occurrences.
        """

        iter = UbfDictItemsOcc(self)
        return  iter

    # Compare two lists...
    def __eq__(self, other):
        """Compare two UbfDict object. The comparator also accepts the
        Python standard dict object.

        Parameters
        ----------
        other: UbfDict or dict.
            Reference to UbfDict or dict object to compare.

        Returns
        -------
        ret : bool
            True if buffers matches, False if not.
        """

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
        """Return number if keys in UbfDict object.

        Returns
        -------
        ret : int
            Number of unique keys / fieldids in the buffer.

        Raises
        ------
        UbfException
            Following error codes may be present:
            ????

        """
        return UbfDict_len(self._buf)

    # Deleting (Calling destructor)
    def __del__(self):
        """Free linked XATMI buffer.
        """
        if self._buf!=0:
            tpfree(self._buf, self._is_sub_buffer);
            self._buf=0

    # Override copy interface, without this _buf is
    # copied over resulting in two objects pointing
    # to the one Ubf...
    def __copy__(self):
        """Make deep copy of the UBF buffer.

        Returns
        -------
        ret : UbfDict
            Newly allocated buffer with data from the original buffer.
        """

        inst = UbfDict()
        inst._buf = UbfDict_copy(self._buf)
        return inst

    #
    # Copy in the same way...
    #
    def __deepcopy__(self, memodict={}):
        """Make deep copy of the UBF buffer.

        Returns
        -------
        ret : UbfDict
            Newly allocated buffer with data from the original buffer.
        """

        inst = UbfDict()
        inst._buf = UbfDict_copy(self._buf)
        return inst

    # manual free up of the buffer
    def free(self):
        """Free linked XATMI buffer. Does nothing for sub-ubf buffers.
        """
        __del__(self)

    # Start iteration
    def __iter__(self):
        """Start iteration over the dictionary.
        """

        return UbfDict_iter(self, self._buf)
    
    # next field, keys only
    def __next__(self):
        """Return next key in the UBF buffer. This lists
        only unique keys.
        """
        return UbfDict_next_keys(self, self._buf)

    # return the UBF buffer..
    def __repr__(self):
        """Returns UBF buffer representation in
        dictionary initialization format.

        Returns
        -------
        ret : str
            Returns buffer in dictionary string form

        Raises
        ------
        UbfException
            Following error codes may be present:
            :data:`.BALIGNERR` - Corrupted UBF buffer.
            :data:`.BNOTFLD` - Buffer not UBF.

        """
        return self.to_dict().__repr__()

    def __contains__(self, key):
        """Check the UBF field is present in UBF buffer
        
        Returns
        -------
        result : bool
            **True** if present, **False** if not

        """

        return UbfDict_contains(self._buf, key)

    # Convert given UbfDict() to generic Python dictionary
    def to_dict(self):
        """Convert given UbfDict object to Python standard dictionary.

        Returns
        -------
        ret : dict
            Python dictionary.

        Raises
        ------
        UbfException
            Following error codes may be present:
            :data:`.BALIGNERR` - Corrupted UBF buffer.
            :data:`.BNOTFLD` - Buffer not UBF.

        """
        return UbfDict_to_dict(self, self._buf)

    # access as attribs
    def __getattr__(self, attr):
        """Access to UBF field values as of class attributes.
        Attributed names **__is_sub_buffer** and **_buf** are
        reserved for internal purpose only.
        If such name shall be read from UBF buffer, access them by
        :func:`.UbfDict.__getitem__` (i.e. index access by the key).


        Returns
        -------
        ret : :class:`.UbfDictFld`
            Initialized dictionary field is returned (ready for
            occurrence access).

        Raises
        ------
        UbfException
            Following error codes may be present:
            :data:`.BALIGNERR` - Corrupted UBF buffer.
            :data:`.BNOTFLD` - Buffer not UBF.

        """
        if attr=="_is_sub_buffer":
            super(UbfDict, self).__getattr__(attr)
        elif attr=="_buf":
            super(UbfDict, self).__getattr__(attr)
        else:
            return self[attr]

    # access as attribs
    def __setattr__(self, attr, value):
        """Set UBF field value as an attribute.
        Note that two names are reserved for this purpose:
        **__is_sub_buffer** and **_buf**, which are internal
        use members and for these values must not be changed.
        If such name shall be stored in UBF buffer, access them by
        :func:`.UbfDict.__setitem__` (i.e. index access by the key).

        Parameters
        ----------
        attr: str
            Field name
        value: object
            Single value or list of values. Depending on the field type, the value
            can be str, int, double, byte array, or dictionary (in case of :data:`.BFLD_UBF`,
            :data:`.BFLD_PTR` or :data:`.BFLD_VIEW`) and UbfDict in case of :data:`.BFLD_UBF`.

        Returns
        -------
        ret : dict
            Python dictionary.

        Raises
        ------
        AttributeError
            If given buffer is sub-buffer (sub-UBF), values of the buffer is read
            only.
        ValueError
            Given value cannot be converted to UBF format.

        UbfException
            Following error codes may be present:
            :data:`.BALIGNERR Corrupted buffer or pointing to not aligned memory area.
            :data:`.BNOTFLD Buffer not fielded, not correctly allocated or corrupted.
            :data:`.BFTOPEN` - Failed to open field definition files.
            :data:`.BBADNAME` - Field not found.

        AtmiException
            :data:`.BBADNAME` - Field not found.
            :data:`.TPEINVAL` Invalid ptr pointer passed in.
                Either buffer not allocated by tpalloc() or ptr is NULL.
            :data:`.TPEOS` System failure occurred during serving.
                See logs i.e. user log, or debugs for more info.

        """
        if attr=="_is_sub_buffer":
            super(UbfDict, self).__setattr__(attr, value)
        elif attr=="_buf":
            super(UbfDict, self).__setattr__(attr, value)
        else:
            self[attr] = value

    # delete attribute
    def __delattr__(self, name):
        """Delete UBF field occurrence

        Parameters
        ----------
        name: str
            field name to remove from buffer (delete all occurrences).

        Raises
        ------
        IndexError
            Invalid index specified (occurrence not present)
        AttributeError
            Read only buffer (sub-UBF)
        UbfException
            Following error codes may be present:
            :data:`.BALIGNERR` - Corrupted UBF buffer.
            :data:`.BNOTFLD` - Buffer not UBF.
            :data:`.BBADFLD` - Invalid field ID given (normally would not be thrown).
        """
        self.__delitem__(name)

# vim: set ts=4 sw=4 et smartindent:
