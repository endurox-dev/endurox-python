/**
 * @brief Enduro/X Python module
 *
 * @file endurox.cpp
 */
/* -----------------------------------------------------------------------------
 * Python module for Enduro/X
 * This software is released under MIT license.
 * 
 * -----------------------------------------------------------------------------
 * MIT License
 * Copyright (C) 2019 Aivars Kalvans <aivars.kalvans@gmail.com> 
 * Copyright (C) 2022 Mavimax SIA
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 * -----------------------------------------------------------------------------
 */

/*---------------------------Includes-----------------------------------*/

#include <dlfcn.h>

#include <atmi.h>
#include <tpadm.h>
#include <userlog.h>
#include <xa.h>
#include <ubf.h>
#undef _

#include "exceptions.h"
#include "ndrx_pymod.h"

#include <pybind11/functional.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <functional>
#include <map>

/*---------------------------Externs------------------------------------*/
/*---------------------------Macros-------------------------------------*/

#define MODULE "endurox"

/*---------------------------Enums--------------------------------------*/
/*---------------------------Typedefs-----------------------------------*/
/*---------------------------Globals------------------------------------*/
/*---------------------------Statics------------------------------------*/

namespace py = pybind11;

static PyObject *EnduroxException_code(PyObject *selfPtr, void *closure)
{
    try
    {
        py::handle self(selfPtr);
        py::tuple args = self.attr("args");
        py::object code = args[1];
        code.inc_ref();
        return code.ptr();
    }
    catch (py::error_already_set &e)
    {
        py::none ret;
        ret.inc_ref();
        return ret.ptr();
    }
}

static PyGetSetDef EnduroxException_getsetters[] = {
    {const_cast<char *>("code"), EnduroxException_code, nullptr, nullptr,
     nullptr},
    {nullptr}};

static PyObject *EnduroxException_tp_str(PyObject *selfPtr)
{
    py::str ret;
    try
    {
        py::handle self(selfPtr);
        py::tuple args = self.attr("args");
        ret = py::str(args[0]);
    }
    catch (py::error_already_set &e)
    {
        ret = "";
    }

    ret.inc_ref();
    return ret.ptr();
}

static void register_exceptions(py::module &m)
{
    static PyObject *AtmiException =
        PyErr_NewException(MODULE ".AtmiException", nullptr, nullptr);
    if (AtmiException)
    {
        PyTypeObject *as_type = reinterpret_cast<PyTypeObject *>(AtmiException);
        as_type->tp_str = EnduroxException_tp_str;
        PyObject *descr = PyDescr_NewGetSet(as_type, EnduroxException_getsetters);
        auto dict = py::reinterpret_borrow<py::dict>(as_type->tp_dict);
        dict[py::handle(((PyDescrObject *)(descr))->d_name)] = py::handle(descr);

        Py_XINCREF(AtmiException);
        m.add_object("AtmiException", py::handle(AtmiException));
    }

    static PyObject *QmException =
        PyErr_NewException(MODULE ".QmException", nullptr, nullptr);
    if (QmException)
    {
        PyTypeObject *as_type = reinterpret_cast<PyTypeObject *>(QmException);
        as_type->tp_str = EnduroxException_tp_str;
        PyObject *descr = PyDescr_NewGetSet(as_type, EnduroxException_getsetters);
        auto dict = py::reinterpret_borrow<py::dict>(as_type->tp_dict);
        dict[py::handle(((PyDescrObject *)(descr))->d_name)] = py::handle(descr);

        Py_XINCREF(QmException);
        m.add_object("QmException", py::handle(QmException));
    }

    static PyObject *UbfException =
        PyErr_NewException(MODULE ".UbfException", nullptr, nullptr);
    if (UbfException)
    {
        PyTypeObject *as_type = reinterpret_cast<PyTypeObject *>(UbfException);
        as_type->tp_str = EnduroxException_tp_str;
        PyObject *descr = PyDescr_NewGetSet(as_type, EnduroxException_getsetters);
        auto dict = py::reinterpret_borrow<py::dict>(as_type->tp_dict);
        dict[py::handle(((PyDescrObject *)(descr))->d_name)] = py::handle(descr);

        Py_XINCREF(UbfException);
        m.add_object("UbfException", py::handle(UbfException));
    }

    static PyObject *NstdException =
        PyErr_NewException(MODULE ".NstdException", nullptr, nullptr);
    if (NstdException)
    {
        PyTypeObject *as_type = reinterpret_cast<PyTypeObject *>(NstdException);
        as_type->tp_str = EnduroxException_tp_str;
        PyObject *descr = PyDescr_NewGetSet(as_type, EnduroxException_getsetters);
        auto dict = py::reinterpret_borrow<py::dict>(as_type->tp_dict);
        dict[py::handle(((PyDescrObject *)(descr))->d_name)] = py::handle(descr);

        Py_XINCREF(NstdException);
        m.add_object("NstdException", py::handle(NstdException));
    }

    py::register_exception_translator([](std::exception_ptr p)
                                      {
    try {
      if (p) {
        std::rethrow_exception(p);
      }
    } catch (const qm_exception &e) {
      py::tuple args(2);
      args[0] = e.what();
      args[1] = e.code();
      PyErr_SetObject(QmException, args.ptr());
    } catch (const atmi_exception &e) {
      py::tuple args(2);
      args[0] = e.what();
      args[1] = e.code();
      PyErr_SetObject(AtmiException, args.ptr());
    } catch (const ubf_exception &e) {
      py::tuple args(2);
      args[0] = e.what();
      args[1] = e.code();
      PyErr_SetObject(UbfException, args.ptr());
    } catch (const nstd_exception &e) {
      py::tuple args(2);
      args[0] = e.what();
      args[1] = e.code();
      PyErr_SetObject(NstdException, args.ptr());
    } });
}

PYBIND11_MODULE(endurox, m)
{
    register_exceptions(m);

    ndrxpy_register_ubf(m);
    ndrxpy_register_atmi(m);
    ndrxpy_register_srv(m);
    ndrxpy_register_tpext(m);
    ndrxpy_register_tplog(m);

    m.attr("TPEV_DISCONIMM") = py::int_(TPEV_DISCONIMM);
    m.attr("TPEV_SVCERR") = py::int_(TPEV_SVCERR);
    m.attr("TPEV_SVCFAIL") = py::int_(TPEV_SVCFAIL);
    m.attr("TPEV_SVCSUCC") = py::int_(TPEV_SVCSUCC);
    m.attr("TPEV_SENDONLY") = py::int_(TPEV_SENDONLY);

    //Event subscribtions
    m.attr("TPEVSERVICE") = py::int_(TPEVSERVICE);
    m.attr("TPEVQUEUE") = py::int_(TPEVQUEUE);//RFU
    m.attr("TPEVTRAN") = py::int_(TPEVTRAN);//RFU
    m.attr("TPEVPERSIST") = py::int_(TPEVPERSIST);//RFU

    //tplogqinfo flags:
    m.attr("TPLOGQI_GET_NDRX") = py::int_(TPLOGQI_GET_NDRX);
    m.attr("TPLOGQI_GET_UBF") = py::int_(TPLOGQI_GET_UBF);
    m.attr("TPLOGQI_GET_TP") = py::int_(TPLOGQI_GET_TP);
    m.attr("TPLOGQI_EVAL_RETURN") = py::int_(TPLOGQI_EVAL_RETURN);
    m.attr("TPLOGQI_RET_HAVDETAILED") = py::int_(TPLOGQI_RET_HAVDETAILED);

    //ATMI IPC flags:
    m.attr("TPNOFLAGS") = py::int_(TPNOFLAGS);
    m.attr("TPNOBLOCK") = py::int_(TPNOBLOCK);
    m.attr("TPSIGRSTRT") = py::int_(TPSIGRSTRT);
    m.attr("TPNOREPLY") = py::int_(TPNOREPLY);
    m.attr("TPNOTRAN") = py::int_(TPNOTRAN);
    m.attr("TPTRAN") = py::int_(TPTRAN);
    m.attr("TPNOTIME") = py::int_(TPNOTIME);
    m.attr("TPABSOLUTE") = py::int_(TPABSOLUTE);
    m.attr("TPGETANY") = py::int_(TPGETANY);
    m.attr("TPNOCHANGE") = py::int_(TPNOCHANGE);
    m.attr("TPCONV") = py::int_(TPCONV);
    m.attr("TPSENDONLY") = py::int_(TPSENDONLY);
    m.attr("TPRECVONLY") = py::int_(TPRECVONLY);
    m.attr("TPREGEXMATCH") = py::int_(TPREGEXMATCH);

    m.attr("TPFAIL") = py::int_(TPFAIL);
    m.attr("TPSUCCESS") = py::int_(TPSUCCESS);
    m.attr("TPEXIT") = py::int_(TPEXIT);
    
    //ATMI errors:
    m.attr("TPMINVAL") = py::int_(TPMINVAL);
    m.attr("TPEABORT") = py::int_(TPEABORT);
    m.attr("TPEBADDESC") = py::int_(TPEBADDESC);
    m.attr("TPEBLOCK") = py::int_(TPEBLOCK);
    m.attr("TPEINVAL") = py::int_(TPEINVAL);
    m.attr("TPELIMIT") = py::int_(TPELIMIT);
    m.attr("TPENOENT") = py::int_(TPENOENT);
    m.attr("TPEOS") = py::int_(TPEOS);
    m.attr("TPEPERM") = py::int_(TPEPERM);
    m.attr("TPEPROTO") = py::int_(TPEPROTO);
    m.attr("TPESVCERR") = py::int_(TPESVCERR);
    m.attr("TPESVCFAIL") = py::int_(TPESVCFAIL);
    m.attr("TPESYSTEM") = py::int_(TPESYSTEM);
    m.attr("TPETIME") = py::int_(TPETIME);
    m.attr("TPETRAN") = py::int_(TPETRAN);
    m.attr("TPGOTSIG") = py::int_(TPGOTSIG);
    m.attr("TPERMERR") = py::int_(TPERMERR);
    m.attr("TPEITYPE") = py::int_(TPEITYPE);
    m.attr("TPEOTYPE") = py::int_(TPEOTYPE);
    m.attr("TPERELEASE") = py::int_(TPERELEASE);
    m.attr("TPEHAZARD") = py::int_(TPEHAZARD);
    m.attr("TPEHEURISTIC") = py::int_(TPEHEURISTIC);
    m.attr("TPEEVENT") = py::int_(TPEEVENT);
    m.attr("TPEMATCH") = py::int_(TPEMATCH);
    m.attr("TPEDIAGNOSTIC") = py::int_(TPEDIAGNOSTIC);
    m.attr("TPEMIB") = py::int_(TPEMIB);
    m.attr("TPMAXVAL") = py::int_(TPMAXVAL);
    
    //UBF errors:
    m.attr("BMINVAL") = py::int_(BMINVAL);
    m.attr("BERFU0") = py::int_(BERFU0);
    m.attr("BALIGNERR") = py::int_(BALIGNERR);
    m.attr("BNOTFLD") = py::int_(BNOTFLD);
    m.attr("BNOSPACE") = py::int_(BNOSPACE);
    m.attr("BNOTPRES") = py::int_(BNOTPRES);
    m.attr("BBADFLD") = py::int_(BBADFLD);
    m.attr("BTYPERR") = py::int_(BTYPERR);
    m.attr("BEUNIX") = py::int_(BEUNIX);
    m.attr("BBADNAME") = py::int_(BBADNAME);
    m.attr("BMALLOC") = py::int_(BMALLOC);
    m.attr("BSYNTAX") = py::int_(BSYNTAX);
    m.attr("BFTOPEN") = py::int_(BFTOPEN);
    m.attr("BFTSYNTAX") = py::int_(BFTSYNTAX);
    m.attr("BEINVAL") = py::int_(BEINVAL);
    m.attr("BERFU1") = py::int_(BERFU1);
    m.attr("BBADTBL") = py::int_(BBADTBL);
    m.attr("BBADVIEW") = py::int_(BBADVIEW);
    m.attr("BVFSYNTAX") = py::int_(BVFSYNTAX);
    m.attr("BVFOPEN") = py::int_(BVFOPEN);
    m.attr("BBADACM") = py::int_(BBADACM);
    m.attr("BNOCNAME") = py::int_(BNOCNAME);
    m.attr("BEBADOP") = py::int_(BEBADOP);
    m.attr("BMAXVAL") = py::int_(BMAXVAL);
    
    //Queue errors:
    m.attr("QMEINVAL") = py::int_(QMEINVAL);
    m.attr("QMEBADRMID") = py::int_(QMEBADRMID);
    m.attr("QMENOTOPEN") = py::int_(QMENOTOPEN);
    m.attr("QMETRAN") = py::int_(QMETRAN);
    m.attr("QMEBADMSGID") = py::int_(QMEBADMSGID);
    m.attr("QMESYSTEM") = py::int_(QMESYSTEM);
    m.attr("QMEOS") = py::int_(QMEOS);
    m.attr("QMEABORTED") = py::int_(QMEABORTED);
    m.attr("QMENOTA") = py::int_(QMENOTA);
    m.attr("QMEPROTO") = py::int_(QMEPROTO);
    m.attr("QMEBADQUEUE") = py::int_(QMEBADQUEUE);
    m.attr("QMENOMSG") = py::int_(QMENOMSG);
    m.attr("QMEINUSE") = py::int_(QMEINUSE);
    m.attr("QMENOSPACE") = py::int_(QMENOSPACE);
    m.attr("QMERELEASE") = py::int_(QMERELEASE);
    m.attr("QMEINVHANDLE") = py::int_(QMEINVHANDLE);
    m.attr("QMESHARE") = py::int_(QMESHARE);

    m.attr("BFLD_SHORT") = py::int_(BFLD_SHORT);
    m.attr("BFLD_LONG") = py::int_(BFLD_LONG);
    m.attr("BFLD_CHAR") = py::int_(BFLD_CHAR);
    m.attr("BFLD_FLOAT") = py::int_(BFLD_FLOAT);
    m.attr("BFLD_DOUBLE") = py::int_(BFLD_DOUBLE);
    m.attr("BFLD_STRING") = py::int_(BFLD_STRING);
    m.attr("BFLD_CARRAY") = py::int_(BFLD_CARRAY);
    m.attr("BFLD_UBF") = py::int_(BFLD_UBF);
    m.attr("BBADFLDID") = py::int_(BBADFLDID);

    //Enduro/X standard library errors:

    m.attr("NMINVAL") = py::int_(NMINVAL);
    m.attr("NEINVALINI") = py::int_(NEINVALINI);
    m.attr("NEMALLOC") = py::int_(NEMALLOC);
    m.attr("NEUNIX") = py::int_(NEUNIX);
    m.attr("NEINVAL") = py::int_(NEINVAL);
    m.attr("NESYSTEM") = py::int_(NESYSTEM);
    m.attr("NEMANDATORY") = py::int_(NEMANDATORY);
    m.attr("NEFORMAT") = py::int_(NEFORMAT);
    m.attr("NETOUT") = py::int_(NETOUT);
    m.attr("NENOCONN") = py::int_(NENOCONN);
    m.attr("NELIMIT") = py::int_(NELIMIT);
    m.attr("NEPLUGIN") = py::int_(NEPLUGIN);
    m.attr("NENOSPACE") = py::int_(NENOSPACE);
    m.attr("NEINVALKEY") = py::int_(NEINVALKEY);
    m.attr("NENOENT") = py::int_(NENOENT);
    m.attr("NEWRITE") = py::int_(NEWRITE);
    m.attr("NEEXEC") = py::int_(NEEXEC);
    m.attr("NESUPPORT") = py::int_(NESUPPORT);
    m.attr("NEEXISTS") = py::int_(NEEXISTS);
    m.attr("NEVERSION") = py::int_(NEVERSION);
    m.attr("NMAXVAL") = py::int_(NMAXVAL);

    m.attr("TPEX_STRING") = py::int_(TPEX_STRING);

    m.attr("TPMULTICONTEXTS") = py::int_(TPMULTICONTEXTS);
    m.attr("TPNULLCONTEXT") = py::int_(TPNULLCONTEXT);

    m.attr("MIB_LOCAL") = py::int_(MIB_LOCAL);

    m.attr("TAOK") = py::int_(TAOK);
    m.attr("TAUPDATED") = py::int_(TAUPDATED);
    m.attr("TAPARTIAL") = py::int_(TAPARTIAL);

    m.attr("TPBLK_NEXT") = py::int_(TPBLK_NEXT);
    m.attr("TPBLK_ALL") = py::int_(TPBLK_ALL);

    m.attr("TPQCORRID") = py::int_(TPQCORRID);
    m.attr("TPQFAILUREQ") = py::int_(TPQFAILUREQ);
    m.attr("TPQBEFOREMSGID") = py::int_(TPQBEFOREMSGID);
    m.attr("TPQGETBYMSGIDOLD") = py::int_(TPQGETBYMSGIDOLD);
    m.attr("TPQMSGID") = py::int_(TPQMSGID);
    m.attr("TPQPRIORITY") = py::int_(TPQPRIORITY);
    m.attr("TPQTOP") = py::int_(TPQTOP);
    m.attr("TPQWAIT") = py::int_(TPQWAIT);
    m.attr("TPQREPLYQ") = py::int_(TPQREPLYQ);
    m.attr("TPQTIME_ABS") = py::int_(TPQTIME_ABS);
    m.attr("TPQTIME_REL") = py::int_(TPQTIME_REL);
    m.attr("TPQGETBYCORRIDOLD") = py::int_(TPQGETBYCORRIDOLD);
    m.attr("TPQPEEK") = py::int_(TPQPEEK);
    m.attr("TPQDELIVERYQOS") = py::int_(TPQDELIVERYQOS);
    m.attr("TPQREPLYQOS  ") = py::int_(TPQREPLYQOS);
    m.attr("TPQEXPTIME_ABS") = py::int_(TPQEXPTIME_ABS);
    m.attr("TPQEXPTIME_REL") = py::int_(TPQEXPTIME_REL);
    m.attr("TPQEXPTIME_NONE ") = py::int_(TPQEXPTIME_NONE);
    m.attr("TPQGETBYMSGID") = py::int_(TPQGETBYMSGID);
    m.attr("TPQGETBYCORRID") = py::int_(TPQGETBYCORRID);
    m.attr("TPQQOSDEFAULTPERSIST") = py::int_(TPQQOSDEFAULTPERSIST);
    m.attr("TPQQOSPERSISTENT ") = py::int_(TPQQOSPERSISTENT);
    m.attr("TPQQOSNONPERSISTENT") = py::int_(TPQQOSNONPERSISTENT);

    //Logger topics:

    m.attr("LOG_FACILITY_NDRX") = py::int_(LOG_FACILITY_NDRX);
    m.attr("LOG_FACILITY_UBF") = py::int_(LOG_FACILITY_UBF);
    m.attr("LOG_FACILITY_TP") = py::int_(LOG_FACILITY_TP);
    m.attr("LOG_FACILITY_TP_THREAD") = py::int_(LOG_FACILITY_TP_THREAD);
    m.attr("LOG_FACILITY_TP_REQUEST") = py::int_(LOG_FACILITY_TP_REQUEST);
    m.attr("LOG_FACILITY_NDRX_THREAD") = py::int_(LOG_FACILITY_NDRX_THREAD);
    m.attr("LOG_FACILITY_UBF_THREAD") = py::int_(LOG_FACILITY_UBF_THREAD);
    m.attr("LOG_FACILITY_NDRX_REQUEST") = py::int_(LOG_FACILITY_NDRX_REQUEST);
    m.attr("LOG_FACILITY_UBF_REQUEST") = py::int_(LOG_FACILITY_UBF_REQUEST);

    //Log levels:
    m.attr("log_always") = py::int_(log_always);
    m.attr("log_error") = py::int_(log_error);
    m.attr("log_warn") = py::int_(log_warn);
    m.attr("log_info") = py::int_(log_info);
    m.attr("log_debug") = py::int_(log_debug);
    m.attr("log_dump") = py::int_(log_dump);

    m.attr("EXSUCCEED") = py::int_(EXSUCCEED);
    m.attr("EXFAIL") = py::int_(EXFAIL);

    m.attr("TP_CMT_LOGGED") = py::int_(TP_CMT_LOGGED);
    m.attr("TP_CMT_COMPLETE") = py::int_(TP_CMT_COMPLETE);

    //Doc syntax
    //https://www.sphinx-doc.org/en/master/usage/restructuredtext/domains.html#cross-referencing-python-objects
    m.doc() =
        R"pbdoc(
Python3 bindings for writing Enduro/X clients and servers
#########################################################

    .. module:: endurox
    .. currentmodule:: endurox

    .. autosummary::
        :toctree: _generate

        tplog
        tplog_debug
        tplog_info
        tplog_warn
        tplog_error
        tplog_always
        tplogconfig
        tplogqinfo
        tplogsetreqfile
        tplogsetreqfile_direct
        tploggetreqfile
        tploggetbufreqfile
        tplogdelbufreqfile
        tplogclosethread
        tplogdump
        tplogdumpdiff
        tplogfplock
        tplogfpget
        tplogfpunlock
        tplogprintubf
        Bfldtype
        Bfldno
        Bmkfldid
        Bfname
        Bfldid
        Bboolpr
        Bboolev
        Bfloatev
        Bfprint
        Bprint
        Bextread
        tpinit
        tptoutset
        tptoutget
        tpsprio
        tpgprio
        tpgetnodeid
        tpterm
        tpgetctxt
        tpnewctxt
        tpsetctxt
        tpfreectxt
        tuxgetenv
        tpsblktime
        tpgblktime
        tpcall
        tpacall
        tpgetrply
        tpcancel
        tpconnect
        tpsend
        tprecv
        tpdiscon
        tpnotify
        tpbroadcast
        tpsetunsol
        tpchkunsol
        tppost
        tpbegin
        tpsuspend
        tpresume
        tpcommit
        tpabort
        tpgetlev
        tpopen
        tpclose
        tpexport
        tpimport
        tpenqueue
        tpdequeue
        tpscmt
        tpencrypt
        tpdecrypt
        xaoSvcCtx
        run
        tpsubscribe
        tpunsubscribe
        tpreturn
        tpforward
        tpadvertise
        tpunadvertise
        tpsrvgetctxdata
        tpsrvsetctxdata
        tpcontinue
        tpexit
        tpext_addb4pollcb
        tpext_delb4pollcb
        tpext_delperiodcb
        tpext_addpollerfd
        tpext_delpollerfd

How to read this documentation
==============================

This documentation contains only short description of the API calls which mentions
core functionality provided by the API. Each API call contains reference to underlaying
C call which explains in deep details how exactly given function behaves.


Sample application
==================

This sections lists basic ATMI client and server examples using endurox-python module.


ATMI Server
-----------

.. code-block:: python
   :caption: ATMI Python server example
   :name: testsv.py
    #!/usr/bin/env python3

    import sys
    import endurox as e

    class Server:

        def tpsvrinit(self, args):
            e.tplog_info("Doing server init...");
            e.tpadvertise("TESTSV", "TESTSV", self.TESTSV)
            e.tpadvertise("ECHO", "ECHO", self.ECHO)
            return 0

        def tpsvrdone(self):
            e.log_info("Server shutdown")

        # This is advertised service
        def TESTSV(self, args):
            e.tplogprintubf(e.log_info, "Incoming request:", args.data)
            args.data["data"]["T_STRING_2_FLD"]="Hello World from XATMI server"
            return e.tpreturn(e.TPSUCCESS, 0, args.data)

        # another service, just echo request buffer
        def ECHO(self, args):
            return e.tpreturn(e.TPSUCCESS, 0, args.data)

    if __name__ == "__main__":
        e.run(Server(), sys.argv)

The testsv.py shall be present in system **PATH**, and script may be started when
it is registered in ndrxconfig.xml configuration file:

.. code-block:: xml
   :caption: ATMI server registration
   :name: ndrxconfig.xml

    <server name="testsv.py">
            <min>1</min>
            <max>1</max>
            <srvid>140</srvid>
            <sysopt>-e ${NDRX_ULOG}/testsv.log -r --</sysopt>
    </server>

For full instructions how to run server or client programs on Enduro/X platform
see **getting_started_tutorial(guides)(Getting Started Tutorial)** user guide from
Enduro/X Core package.

ATMI Client
-----------

.. code-block:: python
   :caption: ATMI Client example
   :name: testcl.py

    #!/usr/bin/env python3

    import sys
    import endurox as e

    def run():

        # Do some work here

        buf = dict()
        buf["data"] = dict()
        buf["data"]["T_STRING_FLD"] = "Hello world!"
        
        tperrno, tpurcode, buf = e.tpcall("TESTSV", buf)
        
        if 0!=tperrno: 
            e.tplog_error("Failed to get configuration: %d" % tperrno)
            raise AtmiExcept(e.TPESVCFAIL, "Failed to call TESTSV")

        e.tplogprintubf(e.log_info, "Got server reply", buf);

    def appinit():
        e.tplog_info("Doing client init...");
        e.tpinit()

    def unInit():
        e.tpterm()

    if __name__ == '__main__':
        try:
            appinit()
            run()
            unInit()
        except Exception as ee:
            e.tplog_error("Exception: %s occurred: %s" % (ee.__class__, str(ee)))

ATMI buffer formats
====================

Core of **ATMI** **IPC** consists of messages being sent between binaries. Message may
encode different type of data. Enduro/X supports following data buffer types:

- | **UBF** (Unified Buffer Format) which is similar to **JSON** or **YAML** buffer format, except
  | that it is typed and all fields must be defined in definition (fd) files. Basically
  | it is dictionary where every field may have several occurrences (i.e. kind of array).
  | Following field types are supported: *BFLD_CHAR* (C char type), *BFLD_SHORT* (C short type),
  | *BFLD_LONG* (C long type), *BFLD_FLOAT* (C float type), *BFLD_DOUBLE* (C double type),
  | *BFLD_STRING* (C zero terminated string type), *BFLD_CARRAY* (byte array), *BFLD_VIEW*
  | (C structure record), *BFLD_UBF* (recursive buffer) and *BFLD_PTR* (pointer to another
  | ATMI buffer).
- | **STRING** this is plain C string buffer. When using with Python, data is converted
  | from to/from *UTF-8* format.
- | **CARRAY** byte array buffer.
- | **NULL** this is empty buffer without any data and type. This buffer cannot be associated
  | with call-info buffer.
- | **JSON** this basically is C string buffer, but with indication that it contains JSON
  | formatted data. These buffer may be automatically converted to UBF and vice versa
  | for certain ATMI server configurations.
- | **VIEW** this buffer basically may hold a C structure record.

Following chapters lists ATMI data encoding principles.

UBF Data encoding
-----------------

When building ATMI buffer from Python dictionary, endurox-python library accepts
values to be present as list of values, in such case values are loaded into UBF occurrences
accordingly. value may be presented directly without the list, in such case the value
is loaded into UBF field occurrence **0**.

When ATMI UBF buffer dictionary is received from Enduro/X, all values are loaded into lists,
regardless of did field had several occurrences or just one.

UBF buffer type is selected by following rules:

- *data* key is dictionary and *buftype* key is not present.

- *data* key is dictionary and *buftype* key is set to **UBF**.

Example call to echo service:

.. code-block:: python
   :caption: UBF buffer encoding call
   :name: ubf-call

    import endurox as e

    tperrno, tpurcode, retbuf = e.tpcall("ECHO", { "data":{
        # 3x occs:
        "T_CHAR_FLD": ["X", "Y", 0]
        , "T_SHORT_FLD": 3200
        , "T_LONG_FLD": 99999111
        , "T_FLOAT_FLD": 1000.99
        , "T_DOUBLE_FLD": 1000111.99
        , "T_STRING_FLD": "HELLO INPUT"
        # contains sub-ubf buffer, which againt contains sub-buffer
        , "T_UBF_FLD": {"T_SHORT_FLD":99, "T_UBF_FLD":{"T_LONG_2_FLD":1000091}}
        # at occ 0 EMPTY view is used
        , "T_VIEW_FLD": [ {}, {"vname":"UBTESTVIEW2", "data":{
                        "tshort1":5
                        , "tlong1":100000
                        , "tchar1":"J"
                        , "tfloat1":9999.9
                        , "tdouble1":11119999.9
                        , "tstring1":"HELLO VIEW"
                        , "tcarray1":[b'\x00\x00', b'\x01\x01'] 
                        }}]
        # contains pointer to STRING buffer:
        , "T_PTR_FLD":{"data":"HELLO WORLD"}
    }})
    print(retbuf)


.. code-block:: python
   :caption: UBF buffer encoding output (line wrapped)
   :name: ubf-call-output
   
    {
        'buftype': 'UBF', 'data':
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

Following **exceptions** may be throw, when ATMI buffer is instantiated:

- | AtmiException with code: :data:`.TPENOENT` - view name in vname is not found. 
- | UbfException with code: :data:`.BEINVAL` - invalid view field occurrance.
  | :data:`.BNOSPACE` - no space in view field.

STRING Data encoding
--------------------

STRING data buffer may contain arbitrary UTF-8 string.
STRING buffer type is selected by following rules:

- *data* key value is string (does not contain 0x00 byte) and *buftype* key is not present.

- *buftype* key is set and contains **STRING** keyword.

.. code-block:: python
   :caption: STRING buffer encoding call
   :name: string-call
    import endurox as e

    tperrno, tpurcode, retbuf = e.tpcall("ECHO", { "data":"HELLO WORLD" })

    print(retbuf)

.. code-block:: python
   :caption: STRING buffer encoding output
   :name: sring-call-output

    {'buftype': 'STRING', 'data': 'HELLO WORLD'}


CARRAY Data encoding
--------------------

CARRAY buffer type may transport arbitrary byte array.
CARRAY buffer type is selected by following rules:

- *data* key value is byte array and *buftype* key is not present.
- *data* key value is byte array and *buftype* is set to *CARRAY*.

.. code-block:: python
   :caption: CARRAY buffer encoding call
   :name: carray-call

    import endurox as e

    tperrno, tpurcode, retbuf = e.tpcall("ECHO", { "data":b'\x00\x00\x01\x02\x04' })

    print(retbuf)

.. code-block:: python
   :caption: CARRAY buffer encoding output
   :name: carray-call-output

    {'buftype': 'CARRAY', 'data': b'\x00\x00\x01\x02\x04'}

NULL Data encoding
------------------

NULL buffers are empty dictionaries, selected by following rules:

- *data* key value is empty dictionary and *buftype* key is not present.
- *data* key value is empty dictionary and *buftype* is set to **NULL**.

.. code-block:: python
   :caption: NULL buffer encoding call
   :name: null-call

    import endurox as e

    tperrno, tpurcode, retbuf = e.tpcall("ECHO", {})

    print(retbuf)
    
.. code-block:: python
   :caption: NULL buffer encoding output
   :name: null-call-output

    {'buftype': 'NULL'}

JSON Data encoding
------------------

JSON buffer type basically is valid UTF-8 string, but with indication that
it contains json formatted data. JSON buffer is selected by following rules:

- *data* is string value and *buftype* is set to **JSON**.

.. code-block:: python
   :caption: JSON buffer encoding call
   :name: json-call

    import endurox as e

    tperrno, tpurcode, retbuf = e.tpcall("ECHO", { "buftype":"JSON", "data":'{"name":"Jim", "age":30, "car":null}'})

    print(retbuf)

.. code-block:: python
   :caption: JSON buffer encoding output
   :name: json-call-output

    {'buftype': 'JSON', 'data': '{"name":"Jim", "age":30, "car":null}'}

VIEW Data encoding
------------------

VIEW buffer encodes record/structure data. On the Python side data is encoded in dictionary,
and similarly as with UBF, values may be set as direct values for the dictionary keys
(and are loaded into occurrence 0 of the view field). Or lists may be used to encode
values, if the view field is array, in such case values are loaded in corresponding
occurrences.

When Python code receives VIEW buffer, any NULL fields (as set by **NULL_VAL** see **viewfile(5)**)
are not converted to Python dictionary values, except in case if NULLs proceed valid array values.

For received buffers all values are encapsulated in lists.

VIEW buffer type is selected by following rules:

- *buftype* is set to **VIEW**, *subtype* is set to valid view name and *data* is dictionary.

.. code-block:: python
   :caption: VIEW buffer encoding call
   :name: view-call

    import endurox as e

    tperrno, tpurcode, retbuf = e.tpcall("ECHO", { "buftype":"VIEW", "subtype":"UBTESTVIEW2", "data":{
        "tshort1":5
        , "tlong1":100000
        , "tchar1":"J"
        , "tfloat1":9999.9
        , "tdouble1":11119999.9
        , "tstring1":"HELLO VIEW"
        , "tcarray1":[b'\x00\x00', b'\x01\x01']
    }})

    print(retbuf)

.. code-block:: python
   :caption: VIEW buffer encoding output
   :name: view-call-output

    {
        'buftype': 'VIEW', 'subtype': 'UBTESTVIEW2', 'data': 
        {
            'tshort1': [5]
            , 'tlong1': [100000]
            , 'tchar1': ['J']
            , 'tfloat1': [9999.900390625]
            , 'tdouble1': [11119999.9]
            , 'tstring1': ['HELLO VIEW']
            , 'tcarray1': [b'\x00\x00', b'\x01\x01']
        }
    }

CALL-INFO ATMI buffer association
----------------------------------

Call-info block is additional UBF buffer that may be linked with Any ATMI buffer 
(except **NULL** buffer). The concept behind with call-info block is similar like
HTTP headers information, i.e. additional data linked to the message body.

.. code-block:: python
   :caption: Call info example
   :name: call-info

    import endurox as e

    tperrno, tpurcode, retbuf = e.tpcall("ECHO", { 
            "data":"HELLO STRING"
            , "callinfo":{"T_SHORT_FLD":55, "T_STRING_FLD":"HELLO"}
        })
    print(retbuf)

.. code-block:: python
   :caption: Call info example
   :name: call-info-output

    {'buftype': 'STRING', 'data': 'HELLO STRING'
        , 'callinfo': {'T_SHORT_FLD': [55], 'T_STRING_FLD': ['HELLO']}}

Key Classes
===========

This section describes key classes used by Enduro/X API.

TPQCTL
------

This class is used to pass/receive additional information to/from
tpenqueue() and tpdequeue() module function.

.. py:class:: TPQCTL()
   :module: endurox

   Persistent queue API control class

   .. attribute:: flags

      *int* -- See bellow flags

   .. attribute:: deq_time

      *int* -- RFU

   .. attribute:: msgid

      *bytes* -- is assigned by Enduro/X when message is enqueued. 
        Message id is 32 bytes long. When doing dequeue, may specify
        message id to read from Q.

   .. attribute:: diagnostic

      *int* -- See exception codes bellow.

   .. attribute:: diagmsg

      *str* -- Diagnostic messages. Used in **QmException**.

   .. attribute:: priority

      *int* -- RFU.

   .. attribute:: corrid

      *bytes* -- is correlator between messages. ID is 32 bytes long.

   .. attribute:: urcode

      *int* -- RFU.

   .. attribute:: cltid

      *CLIENTID* -- RFU.

   .. attribute:: replyqueue

      *str* -- is queue name where automatic queues may post the
        response provided by destination service.

   .. attribute:: failurequeue

      *str* -- is queue name where failed message 
        (destination automatic service failed all attempts) are enqueued.

   .. attribute:: delivery_qos

      *int* -- RFU.

   .. attribute:: reply_qos

      *int* -- RFU.

   .. attribute:: exp_time

      *int* -- RFU.

:attr:`TPQCTL.flags` may be set to following values:

.. data:: TPQCORRID
    
    Use :attr:`TPQCTL.corrid` identifier, set correlator id when performing
    enqueue.

.. data:: TPQGETBYCORRID

    Dequeue message by :attr:`TPQCTL.corrid`.

.. data:: TPQGETBYMSGID
    
    Dequeue message by :attr:`TPQCTL.msgid`.

.. data:: TPQREPLYQ
    
    Use :attr:`TPQCTL.replyqueue` set reply queue for automatic queues.

.. data:: TPQFAILUREQ
    
    Use :attr:`TPQCTL.failurequeue` use failure queue for failed 
    automatic queue messages.

TPEVCTL
-------

Class used to control event subscription for the ATMI servers.
Used by :func:`.tpsubscribe` and :func:`.tpunsubscribe`.

.. py:class:: TPQCTL()
   :module: endurox

   Event control class

   .. attribute:: flags

      *int* -- Bitwise flags of: :data:`.TPEVSERVICE` and :data:`.TPEVPERSIST`.

   .. attribute:: name1

      *str* -- Data field 1

   .. attribute:: name2

      *str* -- Data field 2

Flags
=====

Flags to service routines
-------------------------

.. data:: TPNOBLOCK
    
    Non-blocking send/rcv

.. data:: TPSIGRSTRT
    
    Restart rcv on interrupt

.. data:: TPNOREPLY
    
    No reply expected

.. data:: TPNOTRAN
    
    Not sent in transaction mode

.. data:: TPTRAN
    
    Sent in transaction mode

.. data:: TPNOTIME
    
    No timeout

.. data:: TPABSOLUTE
    
    Absolute value on tmsetprio

.. data:: TPGETANY
    
    Get any valid reply

.. data:: TPNOCHANGE
    
    Force incoming buffer to match

.. data:: RESERVED_BIT1
    
    Reserved for future use

.. data:: TPCONV
    
    Conversational service

.. data:: TPSENDONLY
    
    Send-only mode

.. data:: TPRECVONLY
    
    Recv-only mode

.. data:: TPREGEXMATCH

    Match by regular expression.

.. data:: TPTRANSUSPEND

    Suspend global transaction.

.. data:: TPNOABORT

    Do not abort global transaction in case of failure.


Flags to tpreturn
-----------------

.. data:: TPFAIL
    
    Service FAILURE for tpreturn

.. data:: TPEXIT
    
    Service FAILURE with server exit

.. data:: TPSUCCESS
    
    Service SUCCESS for tpreturn

Flags to tpsblktime/tpgblktime
------------------------------

.. data:: TPBLK_SECOND
    
    This flag sets the blocktime value, in seconds. 
    This is default behavior.

.. data:: TPBLK_NEXT
    
    This flag sets the blocktime value for the
    next potential blocking API.

.. data:: TPBLK_ALL
    
    This flag sets the blocktime value for 
    the all subsequent potential blocking APIs.

Flags to tpenqueue/tpdequeue
----------------------------

.. data:: TPQCORRID

    Set/get correlation id

.. data:: TPQFAILUREQ

    Set/get failure queue

.. data:: TPQBEFOREMSGID

    Enqueue before message id

.. data:: TPQGETBYMSGIDOLD

    Deprecated, RFU

.. data:: TPQMSGID
    
    Get msgid of enq/deq message

.. data:: TPQPRIORITY

    Set/get message priority, RFU

.. data:: TPQTOP 
    
    Enqueue at queue top, RFU

.. data:: TPQWAIT
    
    Wait for dequeuing, RFU

.. data:: TPQREPLYQ
    
    Set/get reply queue, RFU

.. data:: TPQTIME_ABS
    
    Set absolute time, RFU

.. data:: TPQTIME_REL
    
    Set absolute time, RFU

.. data:: TPQGETBYCORRIDOLD
    
    Deprecated, RFU

.. data:: TPQPEEK 
    
    Peek

.. data:: TPQDELIVERYQOS
    
    Delivery quality of service, RFU

.. data:: TPQREPLYQOS
    
    Reply message quality of service, RFU

.. data:: TPQEXPTIME_ABS

    Absolute expiration time, RFU

.. data:: TPQEXPTIME_REL

    Relative expiration time, RFU

.. data:: TPQEXPTIME_NONE

    Never expire, RFU

.. data:: TPQGETBYMSGID
    
    Dequeue by msgid

.. data:: TPQGETBYCORRID
    
    Dequeue by corrid

.. data:: TPQQOSDEFAULTPERSIST
    
    Queue's default persistence policy

.. data:: TPQQOSPERSISTENT
    
    Disk message, RFU

.. data:: TPQQOSNONPERSISTENT
    
    Memory message, RFU

Flags to tpsubscribe/tpunsubscribe (:attr:`TPEVCTL.flags`)
----------------------------------------------------------

.. data:: TPEVSERVICE

    Must be present when ATMI server subscribes to event.

.. data:: TPEVPERSIST

    Do not unsubscribe from event in case if service failed
    when event was delivered.

.. data:: TPEVQUEUE

    RFU.

.. data:: TPEVTRAN

    RFU.

Events returned by conversational API
-------------------------------------

.. data:: TPEV_DISCONIMM

    Immediate disconnect event.

.. data:: TPEV_SVCERR

    Server died or :func:`.tpreturn` failed.
    
.. data:: TPEV_SVCFAIL

    Server returned :data:`.TPFAIL` with :func:`.tpreturn`.
    
.. data:: TPEV_SVCSUCC

    Server returned with success.
    
.. data:: TPEV_SENDONLY

    Sender puts receiving party in sender mode.


Flags for :func:`.tplogqinfo` input and return
----------------------------------------------

.. data:: TPLOGQI_GET_NDRX

    Query logging information about **NDRX** topic.

.. data:: TPLOGQI_GET_UBF

    Query logging information about **UBF** topic.

.. data:: TPLOGQI_GET_TP

    Query logging information about **TP** topic.

.. data:: TPLOGQI_EVAL_RETURN

    Evaluate request regardless of log level.

.. data:: TPLOGQI_RET_HAVDETAILED

    Detailed flag have been set for logger.

Error Codes
===========

This section lists error codes used in API interface.

ATMI Errors
-----------

Atmi Errors are thrown in **AtmiException** object.
Following constants may be set in *code* field.

Error code values are given for reference reasons only. The codes may change
in future without a notice.

.. data:: TPMINVAL
    
    0 - Minimum error, no error.
        
.. data:: TPEABORT

    1 - Transaction aborted.

.. data:: TPEBADDESC

    2 - Bad descriptor.

.. data:: TPEBLOCK

    3 - Blocking condition found but resource is configured as
    non blocking (e.g. flag :data:`.TPNOBLOCK` was passed).

.. data:: TPEINVAL

    4 - Invalid arguments.

.. data:: TPELIMIT

    5 - Limit reached.

.. data:: TPENOENT

    6 - No entry.

.. data:: TPEOS

    7 - Operating system error.

.. data:: TPEPERM

    8 - Permissions error.

.. data:: TPEPROTO

    9 - Protocol error. Invalid call sequence.

.. data:: TPESVCERR

    10 - Service crashed.

.. data:: TPESVCFAIL

    11 - Service user code failed.

.. data:: TPESYSTEM

    12 - Enduro/X system error.

.. data:: TPETIME

    13 - Call timed out.

.. data:: TPETRAN

    14 - Transaction error.

.. data:: TPGOTSIG

    15 - Got signal.

.. data:: TPERMERR

    15 - Resource manager error.

.. data:: TPEITYPE

    17 - Input ATMI buffer error.

.. data:: TPEOTYPE

    18 - Output ATMI buffer error.

.. data:: TPERELEASE

    19 - Invalid release version.

.. data:: TPEHAZARD

    20 - Partial transaction commit/abort error.

.. data:: TPEHEURISTIC

    21 - Partial transaction commit/abort error.

.. data:: TPEEVENT

    22 - Event occurred.

.. data:: TPEMATCH

    23 - Matching identifier, duplicate.

.. data:: TPEDIAGNOSTIC

    24 - Diagnostic error returned in :attr:`TPQCTL.diagnostic`.

.. data:: TPEMIB

    25 - RFU.

UBF Errors
----------

Unified Buffer Format (UBF) errors are thrown in **UbfException** object.
Following constants may be set in *code* field.

Error code values are given for reference reasons only. The codes may change
in future without a notice.

.. data:: BMINVAL
    
        0 - Minimum error, no error.
        
.. data:: BERFU0
    
        1 - Reserved for future use.

.. data:: BALIGNERR           
    
        2 - Buffer not aligned to platform address or corrupted.

.. data:: BNOTFLD

        3 - Buffer not fielded / TLV formatted.

.. data:: BNOSPACE            
    
        4 - No space in buffer.

.. data:: BNOTPRES            
        
        5 - Field not present.

.. data:: BBADFLD
        
        6 - Bad field ID.

.. data:: BTYPERR
    
        7 - Bad field type.

.. data:: BEUNIX

        8 - System error occurred.

.. data:: BBADNAME
        
        9 - Bad field name.

.. data:: BMALLOC
        
        10 - Malloc failed, out of mem?

.. data:: BSYNTAX
    
        11 - UBF Boolean expression error or UBF bad text format.

.. data:: BFTOPEN
    
        12 - Failed to open field tables.

.. data:: BFTSYNTAX
    
        13 - Field table syntax error.

.. data:: BEINVAL
        
        14 - Invalid value.

.. data:: BERFU1
        
        15 - Reserved for future use.

.. data:: BBADTBL
    
        16 - Reserved for future use.

.. data:: BBADVIEW
    
        17 - Invalid compiled VIEW file.

.. data:: BVFSYNTAX
    
        18 - Source VIEW file syntax error.

.. data:: BVFOPEN
        
        19 - Failed to open VIEW file.

.. data:: BBADACM
        
        20 - Reserved for future use.

.. data:: BNOCNAME
    
        21 - Structure field not found for VIEW.

.. data:: BEBADOP
        
        22 - Buffer operation not supported (complex type).
        
.. data:: NMAXVAL
        
        22 - Maximum error code.

Enduro/X standard errors
------------------------

Enduro/X standard library errors are thrown in **NstdException** object.
Following constants may be set in *code* field.

Error code values are given for reference reasons only. The codes may change
in future without a notice.

.. data:: NMINVAL
        
    0 - Minimum error code.
        
.. data:: NEINVALINI
    
    1 - Invalid INI file.
        
.. data:: NEMALLOC
    
    2 - Malloc failed.
        
.. data:: NEUNIX
    
    3 - Unix error occurred.
        
.. data:: NEINVAL
    
    4 - Invalid value passed to function.
        
.. data:: NESYSTEM
    
    5 - System failure.
        
.. data:: NEMANDATORY

    6 - Mandatory field is missing.
        
.. data:: NEFORMAT
    
    7 - Format error.
        
.. data:: NETOUT

    8 - Time-out condition.
        
.. data:: NENOCONN
        
    9 - Connection not found.
        
.. data:: NELIMIT
        
    10 - Limit reached.
        
.. data:: NEPLUGIN
        
    11 - Plugin error.
        
.. data:: NENOSPACE
        
    12 - No space.
        
.. data:: NEINVALKEY
        
    13 - Invalid key (probably).
        
.. data:: NENOENT
        
    14 - No such file or directory.
        
.. data:: NEWRITE
        
    15 - Failed to open/write.
        
.. data:: NEEXEC
        
    16 - Failed to execute.
        
.. data:: NESUPPORT
        
    17 - Command not supported.
        
.. data:: NEEXISTS
        
    18 - Duplicate action.
        
.. data:: NEVERSION
        
    19 - API version conflict.

.. data:: NMAXVAL
        
    19 - Maximum error code.

Persistent queue errors
-----------------------

Following :attr:`TPQCTL.diagnostic` (*QmException.code*) codes may be returned.

Error code values are given for reference reasons only. The codes may change
in future without a notice.

.. data:: QMEINVAL
    
    -1 - Invalid data.

.. data:: QMEBADRMID
    
    -2 - RFU.

.. data:: QMENOTOPEN
    
    -3 - RFU.

.. data:: QMETRAN
    
    -4 - RFU.

.. data:: QMEBADMSGID
    
    -5 - RFU.

.. data:: QMESYSTEM
    
    -6 - System error.

.. data:: QMEOS
    
    -7 - OS error.

.. data:: QMEABORTED
    
    -8 - RFU.

.. data:: QMENOTA
    
    -8 - RFU.

.. data:: QMEPROTO
    
    -9 - RFU.

.. data:: QMEBADQUEUE
    
    -10 - Bad queue name.

.. data:: QMENOMSG
    
    -11 - No messages in queue.

.. data:: QMEINUSE
    
    -12 - RFU.

.. data:: QMENOSPACE
    
    -13 - RFU.

.. data:: QMERELEASE
    
    -14 - RFU.

.. data:: QMEINVHANDLE
    
    -15 - RFU.

.. data:: QMESHARE
    
    -16 - RFU.

Other constants
===============

This section lists other constants used in the Enduro/X Python
module.

Log levels
----------

.. data:: log_always
    
    2 - Fatal loging level.

.. data:: log_error
    
    2 - Error loging level.

.. data:: log_warn
    
    3 - Warning logging level.

.. data:: log_info
    
    4 - Info logging level.

.. data:: log_debug
    
    5 - Debug logging level.

.. data:: log_dump
    
    6 - Unofficial log level, dump.

Logging topics aka facilities
-----------------------------

.. data:: LOG_FACILITY_NDRX
    
    Process level, Enduro/X core logging topic.

.. data:: LOG_FACILITY_UBF
    
    Process level, Enduro/X UBF library logging topic.

.. data:: LOG_FACILITY_TP

    Process level, user logging topic.

.. data:: LOG_FACILITY_NDRX_THREAD
    
    Thread level, Enduro/X core logging topic.
        
.. data:: LOG_FACILITY_UBF_THREAD
    
    Thread level, Enduro/X UBF library logging topic.

.. data:: LOG_FACILITY_TP_THREAD
    
    Thread level, user logging topic.

.. data:: LOG_FACILITY_NDRX_REQUEST
    
    Request logging (per context), Enduro/X core logging topic.

.. data:: LOG_FACILITY_UBF_REQUEST
    
    Request logging (per context), Enduro/X UBF library logging topic.

.. data:: LOG_FACILITY_TP_REQUEST
    
    Request logging (per context), user logging topic.

Transaction completion
----------------------

.. data:: TP_CMT_LOGGED
    
    Return from commit when logged.

.. data:: TP_CMT_COMPLETE
    
    Return from commit when fully complete.

Transaction completion
----------------------

.. data:: TPMULTICONTEXTS
    
    Atmi context was intialized.

.. data:: TPNULLCONTEXT
    
    Atmi context as not initialized.

MIB interface
-------------

.. data:: TAOK
    
    Value for **TA_ERROR**, success.

.. data:: TAUPDATED
    
    Value for **TA_ERROR**, success, data updated.

.. data:: TAPARTIAL
    
    Value for **TA_ERROR**, Partial succeed, have updates.

Generic status codes
--------------------

.. data:: EXSUCCEED
    
    Success

.. data:: EXFAIL
    
    Failure

)pbdoc";
}

/* vim: set ts=4 sw=4 et smartindent: */

