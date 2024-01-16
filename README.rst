=======================
Enduro/X Python3 module
=======================

Python module for Enduro/X offers complete ATMI API access from the Python3 programming
language. The module includes such features as:

- A multi-threaded server
- Synchronous, asynchronous, conversational, event-based and notification IPC APIs
- Support for UBF (emulation of Tuxedo FML32, i.e. binary coded key/value buffer), NULL, STRING, CARRAY, VIEW (C structures), and JSON buffers
- Support for nested UBF buffers with PTR (pointer to other XATMI buffers) and VIEW support
- Dictionary like interface for UBF, having zero marshaling when client/server sends/receives UBF buffers, meaning ultra-fast processing times, even for large data. When buffers are exchanged between processes, only a few memcpy() operations are performed between processes and the OS kernel, as for UBF there is no serialization and when using UbfDict() interface, only requested fields from memory are exchanged with the Python VM upon get/set access.
- Receive a response even when the service returns TPFAIL (instead of exception)
- Enduro/X ATMI server extensions, such as periodic callbacks and resource polling
- Logging API.
- Simple, but yet powerful Python to Executable linker tool (named `expyld`), which pre-compiles and embeds pure Python packages/modules into executable files. During the runtime these packages/modules are resolved directly from the process memory, i.e. no temp files are used.
- See `documentation <https://www.endurox.org/dokuwiki>`_ for full API description.

Python module for Enduro/X is written in C++11 and `pybind11 <https://github.com/pybind/pybind11>`_.



Supported platforms
-------------------

The module is supported by all platforms where Enduro/X runs, Python3 and C++11 are available.

Installation process
--------------------

The following packages must be installed before installing the Enduro/X Python module from sources:

- Enduro/X Core (binary downloads here https://www.mavimax.com) starting from version 8.0.6
- pkg-config
- CMake version 3.1 or later


To build and install `endurox`, clone or download this repository and then, from within the repository, run:

.. code:: bash

    $ python3 ./setup.py install --user

Or if `pip3` is available, install it in the following way:

.. code:: bash

    $ pip3 install .  --user

The unit tests can be started with:

.. code:: bash

    $ python3 ./setup.py test

To build API documentation:

.. code:: bash

    $ sudo pip3 intall sphinx
    $ cd doc
    $ make html
    $ cd _build/html
    -- launch the browser at index (to view the docs):
    $ firefox index.html

If files binary release is required, prepare by:

.. code:: bash
    
    $ sudo pip3 install wheel
    $ python3 ./setup.py bdist_wheel

To prepare source distribution:

.. code:: bash

    $ python3 ./setup.py sdist

The distribution files are unloaded to ./dist folder.

General
-------

``endurox`` module supports all Enduro/X ATMI buffer types: 

- ``STRING``
- ``CARRAY``
- ``UBF``
- ``VIEW``
- ``JSON``
- ``NULL``

All buffers are encapsulated in Python dictionary. For example ``UBF`` (equivalent to Tuxedo FML32) buffer is encoded as:

.. code:: python

    {
        'buftype': 'UBF'
        , 'data':
        {
            'T_SHORT_FLD': [3200]
            , 'T_LONG_FLD': [99999111]
            , 'T_CHAR_FLD': ['X', 'Y', b'\x00']
            , 'T_FLOAT_FLD': [1000.989990234375]
            , 'T_DOUBLE_FLD': [1000111.99]
            , 'T_STRING_FLD': ['HELLO INPUT']
            , 'T_PTR_FLD': [{'buftype': 'STRING', 'data': 'HELLO WORLD'}]
            , 'T_UBF_FLD': [{'T_SHORT_FLD': [99], 'T_UBF_FLD': [{'T_LONG_2_FLD': [1000091]}]}]
            , 'T_VIEW_FLD': [{}, {'vname': 'UBTESTVIEW2', 'data': {
                'tshort1': [5]
                , 'tlong1': [100000]
                , 'tchar1': ['J']
                , 'tfloat1': [9999.900390625]
                , 'tdouble1': [11119999.9]
                , 'tstring1': ['HELLO VIEW']
                , 'tcarray1': [b'\x00\x00', b'\x01\x01']
            }}]
        }
    }

Or with UBF Dictionary interface class:

.. code:: python

    {
        'buftype': 'UBF'
        , 'data': e.UbfDict(
        {
            'T_SHORT_FLD': [3200]
            , 'T_LONG_FLD': [99999111]
            , 'T_CHAR_FLD': ['X', 'Y', b'\x00']
            , 'T_FLOAT_FLD': [1000.989990234375]
            , 'T_DOUBLE_FLD': [1000111.99]
            , 'T_STRING_FLD': ['HELLO INPUT']
            , 'T_PTR_FLD': [{'buftype': 'STRING', 'data': 'HELLO WORLD'}]
            , 'T_UBF_FLD': [{'T_SHORT_FLD': [99], 'T_UBF_FLD': [{'T_LONG_2_FLD': [1000091]}]}]
            , 'T_VIEW_FLD': [{}, {'vname': 'UBTESTVIEW2', 'data': {
                'tshort1': [5]
                , 'tlong1': [100000]
                , 'tchar1': ['J']
                , 'tfloat1': [9999.900390625]
                , 'tdouble1': [11119999.9]
                , 'tstring1': ['HELLO VIEW']
                , 'tcarray1': [b'\x00\x00', b'\x01\x01']
            }}]
        })
    }

``buftype`` is optional for ``CARRAY``, ``STRING``, ``UBF`` and ``NULL`` buffers. It is mandatory for ``JSON`` 
and ``VIEW`` buffers. For ``VIEW`` buffers ``subtype`` specifies view name. 
Buffer data is present in ``data`` root dictionary key.

``CARRAY`` is mapped to/from Python ``bytes`` type.

``STRING`` is mapped to/from Python ``str`` type.

``UBF`` (a ``FML32`` Tuxedo emulation) is mapped to/from Python ``dict`` type with field names 
(``str``) as keys and lists (``list``) of different types (``int``, ``str``, ``float`` or ``dict``
(for embedded ``BFLD_UBF``, ``BLFD_PTR`` or ``BFLD_VIEW``) as values. This is default type for the
``dict`` buffer if for root dictionary ``buftype`` key is not specified. ``dict`` to ``UBF``
conversion also treats types ``int``, ``str``, ``float`` or ``dict`` as lists with a
single element (the same rule applies to ``VIEW`` buffer keys):

.. code:: python

  {'data':{'T_STRING_FLD': 'Single value'}}

converted to ``UBF`` and then back to ``dict`` becomes

.. code:: python

  {'data':{'T_STRING_FLD': ['Single value']}}


All ATMI functions that take buffer and length arguments in C take only buffer argument in Python.

Calling a service
-----------------

``endurox.tpcall()`` and ``endurox.tpgetrply()`` functions return a tuple with 3
elements or throw an exception when no data is received. In case if service returned 
``TPFAIL`` status, the error is not thrown, but instead error code 
``endurox.TPESVCFAIL`` is returned in first return value. 
For all other errors, ``AtmiException`` is thrown.

``endurox.tpcall()`` and ``endurox.tpgetrply()`` returns following values:

- 0 or ``TPESVCFAIL``
- ``tpurcode`` (the second argument to ``tpreturn``)
- data buffer

.. code:: python

    import endurox

    tperrno, tpurcode, data = endurox.tpcall('TESTSV', {'data':{'T_STRING_FLD': 'HELLO', 'T_STRING_4_FLD': 'WORLD'}})
    if tperrno == 0:
        # Service returned TPSUCCESS
    else:
        # tperrno == endurox.TPESVCFAIL
        # Service returned TPFAIL 

Writing servers
---------------

Enduro/X servers are written as Python classes. ``tpsvrinit`` method of object will be
called when Enduro/X calls ``tpsvrinit()`` function and it must return 0 on success
or -1 on error. A common task for ``tpsvrinit`` is to advertise services the server
provides by calling ``endurox.tpadvertise()`` with a service name. Function accepts
service name (string), service function name (string) and callback to service function.
``tpsvrdone``, ``tpsvrthrinit`` and ``tpsvrthrdone`` will be called when Enduro/X calls 
corresponding functions. All of these 4 methods are optional.

Each service method receives a single argument with incoming buffer and service must end 
with either call to ``endurox.tpreturn()`` or ``endurox.tpforward()``, however 
some non ATMI code may be executed after these function calls. Service function 
return may be written in following ways:

.. code:: python

      def ECHO(self, args):
          return t.tpreturn(t.TPSUCCESS, 0, args.data)

.. code:: python

      def ECHO(self, args):
          t.tpreturn(t.TPSUCCESS, 0, args.data)

To start Enduro/X ATMI server process ``endurox.run()`` must be called with an instance of the class and command-line arguments.

.. code:: python

    #!/usr/bin/env python3
    import sys
    import endurox as e

    class Server:
        def tpsvrinit(self, args):
            e.tpadvertise('TESTSV', 'TESTSV', self.TESTSV)
            return 0

        def tpsvrthrinit(self, args):
            return 0

        def tpsvrthrdone(self):
            pass

        def tpsvrdone(self):
            pass

        def TESTSV(self, args):
            e.tplogprintubf(e.log_info, 'Incoming request', args.data)
            args.data['data']['T_STRING_2_FLD']='Hello World from XATMI server'
            return e.tpreturn(e.TPSUCCESS, 0, args.data)

    if __name__ == '__main__':
        e.tprun(Server(), sys.argv)

NDRXCONFIG.XML
--------------

To use Python code as Enduro/X server the file itself must be executable (``chmod +x *.py``)
and it must contain shebang line with Python:

.. code:: python

  #!/usr/bin/env python3

After that you can use the ``*.py`` file as server executable in ``UBBCONFIG``:

.. code:: xml

    <server name="testsv.py">
            <min>1</min>
            <max>1</max>
            <srvid>200</srvid>
            <sysopt>-e ${NDRX_ULOG}/testsv.log -r</sysopt>
    </server>


Writing clients
---------------

Nothing special is needed to implement Enduro/X clients, just import the module and 
start calling XATMI functions.

.. code:: python

    #!/usr/bin/env python3
    import endurox as e

    tperrno, tpurcode, data = e.tpcall('TESTSV', {'data':{'T_STRING_FLD': 'HELLO', 'T_STRING_4_FLD': 'WORLD'}})

    e.tplog_info("tperrno=%d tpurcode=%d" % (tperrno, tpurcode))
    e.tplogprintubf(e.log_info, 'Got response', data)

    # would print to log file:
    # t:USER:4:c9e5ad48:413519:7f35b9ad7740:001:20220619:233508180644:tplog       :/tplog.c:0582:tperrno=0 tpurcode=0
    # t:USER:4:c9e5ad48:413519:7f35b9ad7740:001:20220619:233518812671:plogprintubf:bf/ubf.c:1790:Got response
    # T_STRING_FLD	HELLO
    # T_STRING_2_FLD	Hello World from XATMI server
    # T_STRING_4_FLD	WORLD

Using Oracle Database
---------------------

You can access Oracle database with ``cx_Oracle`` library and local transactions by just 
following the documentation of ``cx_Oracle``.

If client or server needs to be written in Python to participate in the global transaction,
standard Enduro/X Oracle XA driver configuration is be applied, i.e. libndrxxaoras (for static registration)
or libndrxxaorad (for dynamic XA registration) configured.

Global transactions
===================

Transactions can be started and committed or aborted by using ``endurox.tpbegin()``, ``endurox.tpcommit()``, ``endurox.tpabort()``.

Client process example:

.. code:: python

    #!/usr/bin/env python3
    import cx_Oracle
    import endurox as e

    e.tpopen()
    db = cx_Oracle.connect(handle=e.xaoSvcCtx())

    e.tpbegin(60)

    with db.cursor() as cursor:
        cursor.execute("delete from pyaccounts")

    # Call any service in global transaction

    e.tpcommit()
    e.tpclose()
    e.tpterm()

When running the Enduro/X client which must participate in global transaction, CC tag shall be set
in the environment before running the client script:

.. code:: bash

    $ NDRX_CCTAG="ORA1" ./dbclient.py

When running ATMI server in the global transaction, the <cctag> XML tag can be used to assign the DB
configuration to it:

.. code:: xml

    <server name="dbserver.py">
            <min>1</min>
            <max>1</max>
            <srvid>200</srvid>
            <sysopt>-e ${NDRX_ULOG}/dbserver.log -r</sysopt>
            <cctag>ORA1</cctag>
    </server>


For a multi-threaded server, new connections for each thread must be created in 
``tpsvrthrinit()`` (instead of ``tpsvrinit()``) and stored in thread-local storage of ``threading.local()``.

**app.ini** settings for the Oracle DB:

.. code::

    [@global/ORA1]
    NDRX_XA_RES_ID=1
    NDRX_XA_OPEN_STR=ORACLE_XA+SqlNet=SID1+ACC=P/user1/pass1+SesTM=180+LogDir=/tmp+nolocal=f+Threads=true
    NDRX_XA_CLOSE_STR=${NDRX_XA_OPEN_STR}
    NDRX_XA_DRIVERLIB=libndrxxaoras.so
    NDRX_XA_RMLIB=libclntsh.so
    NDRX_XA_LAZY_INIT=1

Additionally, Enduro/X transaction manager must be configured to run global transactions, e.g.:

.. code::

    <server name="tmsrv">
        <min>1</min>
        <max>1</max>
        <srvid>40</srvid>
        <cctag>ORA1</cctag>
        <sysopt>-e ${NDRX_ULOG}/tmsrv-rm1.log -r -- -t1 -l${NDRX_APPHOME}/tmlogs/rm1</sysopt>
    </server>

UBF identifiers
-----------------

``Bname`` and ``Bfldid`` are available to find map from field identifier to name or the other way.

Functions to determine field number and type from identifier:

.. code:: python

    import endurox as e

    assert e.Bfldtype(e.Bmkfldid(e.BFLD_STRING, 10)) == e.BFLD_STRING
    assert e.Bfldno(e.Bmkfldid(e.BFLD_STRING, 10)) == 10

Exceptions
----------

On errors, either ``AtmiException`` or ``UbfException`` are raised by the module. Exceptions contain
additional attribute ``code`` that contains the Enduro/X error code and it can be
compared it with defined errors like ``TPENOENT`` or ``TPESYSTEM``.

.. code:: python

    import endurox as e
    try:
        e.tpcall("whatever", {})
    except e.AtmiException as ee:
        if ee.code == e.TPENOENT:
            print("Service does not exist")

Logging
-------

The Enduro/X Python module contains all logging features provided by Enduro/X Core.
functions such as ``tplog()`` (including syntactic sugars for log levels), ``tplogdump()``
for dumping bytes to hex dumps in logs, request/session log file contexting and
APIs manipulation with logfile file descriptors.

.. code:: python

    import endurox as e
    
    e.tplog_debug("This is debug message")

Unit testing
------------

``tests/`` contains test for all Enduro/X ATMIs provided by the module.
These test cases can be studied to get familiar with module APIs.


Licensing
---------

The module is licensed under GNU Affero General Public License Version 3,
which allows the use of the product for open-source software.
Mavimax SIA also provides a commercial license EULA which allows to use Enduro/X
Python module in closed source projects.

Conclusions
-----------

This document gave a short overview of the Enduro/X Python module.
For a full API overview please see API descriptions at https://www.endurox.org/dokuwiki

As all API descriptions are embedded as PyDoc, Python shell can be utilized to
get help with the overview, functions, constants, etc.

As the root package "endurox" actually embeds C biding code in another sub-module "endurox",
then full API doc can be viewed by:

.. code:: python

    import endurox as e

    help (e.endurox)

The individual identifiers can be looked by directly by:

.. code:: python

    import endurox as e

    help (e.tpcall)

Releases
--------

- Version 8.0.2 released on 18/08/2022 - First stable release
- Version 8.0.4 marked on 25/09/2022 - Feature #790
