/**
 * @brief Logging API
 *
 * @file tplog.cpp
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
#include <dlfcn.h>

#include <atmi.h>
#include <tpadm.h>
#include <nerror.h>
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

namespace py = pybind11;

/**
 * @brief Register ATMI logging api
 * 
 * @param m Pybind11 module handle
 */
expublic void ndrxpy_register_tplog(py::module &m)
{

    //Debug handle
    py::class_<pyndrxdebugptr>(m, "NdrxDebugHandle")
        //this is buffer for pointer...
        .def_readonly("ptr", &pyndrxdebugptr::ptr);

    //Logging functions:
    m.def(
        "tplog_debug",
        [](const char *message)
        {
            py::gil_scoped_release release;
            tplog(log_debug, const_cast<char *>(message));
        },
        R"pbdoc(
        Print debug message to log file. Debug is logged as level **5**.

        For more details see **tplog(3)** C API call

        Parameters
        ----------
        message : str
            Debug message to print.
        )pbdoc"
        , py::arg("message"));

    m.def(
        "tplog_info",
        [](const char *message)
        {
            py::gil_scoped_release release;
            tplog(log_info, const_cast<char *>(message));
        },
        R"pbdoc(
        Print info message to log file. Info is logged as level **4**.

        For more details see **tplog(3)** C API call

        Parameters
        ----------
        message : str
            Info message to print.
        )pbdoc"
        , py::arg("message"));

    m.def(
        "tplog_warn",
        [](const char *message)
        {
            py::gil_scoped_release release;
            tplog(log_error, const_cast<char *>(message));
        },
        R"pbdoc(
        Print warning message to log file. Warning is logged as level **3**.

        For more details see **tplog(3)** C API call

        Parameters
        ----------
        message : str
            Warning message to print.
        )pbdoc", py::arg("message"));

    m.def(
        "tplog_error",
        [](const char *message)
        {
            py::gil_scoped_release release;
            tplog(log_error, const_cast<char *>(message));
        },
        R"pbdoc(
        Print error message to log file. Error is logged as level **2**.

        For more details see **tplog(3)** C API call

        Parameters
        ----------
        message : str
            Error message to print.
        )pbdoc", py::arg("message"));

    m.def(
        "tplog_always",
        [](const char *message)
        {
            py::gil_scoped_release release;
            tplog(log_error, const_cast<char *>(message));
        },
        R"pbdoc(
        Print fatal message to log file. Fatal/always is logged as level **1**.

        For more details see **tplog(3)** C API call

        Parameters
        ----------
        message : str
            Fatal message to print.
        )pbdoc", py::arg("message"));

    m.def(
        "tplog",
        [](int lev, const char *message)
        {
            py::gil_scoped_release release;
            tplog(lev, const_cast<char *>(message));
        },
        R"pbdoc(
        Print logfile message with specified level.

        For more details see **tplog(3)** C API call

        Parameters
        ----------
        lev : int
            Log level with consts: :data:`.log_dump`, :data:`.log_debug`,
            :data:`.log_info`, :data:`.log_warn`, :data:`.log_error`, :data:`.log_always`
            or specify the number (1..6).
        message : str
            Message to log.
        )pbdoc", py::arg("lev"), py::arg("message"));

    m.def(
        "tplogconfig",
        [](int logger, int lev, const char *debug_string, const char *module, const char *new_file)
        {
            py::gil_scoped_release release;
            if (EXSUCCEED!=tplogconfig(logger, lev, const_cast<char *>(debug_string), 
                const_cast<char *>(module), const_cast<char *>(new_file)))
            {
                throw nstd_exception(Nerror);
            }
        },
        R"pbdoc(
        Configure Enduro/X logger.

        .. code-block:: python
            :caption: tplogconfig example
            :name: tplogconfig-example

                import endurox as e
                e.tplogconfig(e.LOG_FACILITY_TP, -1, "tp=4", "", "/dev/stdout")
                e.tplog_info("Test")
                e.tplog_debug("Test Debug")
                # would print to stdout:
                # t:USER:4:d190fd96:29754:7f35f54f2740:000:20220601:160215386056:tplog       :/tplog.c:0582:Test
                
        For more details see **tplogconfig(3)** C API call.

        :raise NstdException: 
            | Following error codes may be present:
            | :data:`.NEFORMAT` - Debug string format error.
            | :data:`.NESYSTEM` - System error.

        Parameters
        ----------
        logger : int
            Bitwise flags for logger/topic identification:
            :data:`.LOG_FACILITY_NDRX`, :data:`.LOG_FACILITY_UBF`, :data:`.LOG_FACILITY_TP`
            :data:`.LOG_FACILITY_TP_THREAD`, :data:`.LOG_FACILITY_TP_REQUEST`, :data:`.LOG_FACILITY_NDRX_THREAD`
            :data:`.LOG_FACILITY_UBF_THREAD`, :data:`.LOG_FACILITY_NDRX_REQUEST`, :data:`.LOG_FACILITY_UBF_REQUEST`
        lev: int
            Level to set. Use **-1** to ignore this setting.
        debug_string : str
            Debug string according to **ndrxdebug.conf(5)**.
        module : str
            Module name. Use empty string if not changing.
        new_file : str
            New logfile name. Use empty string if not changing.

         )pbdoc", py::arg("logger"), py::arg("lev"), 
        py::arg("debug_string"), py::arg("module"), py::arg("new_file"));

    m.def(
        "tplogqinfo",
        [](int lev, long flags)
        {
            py::gil_scoped_release release;
            long ret=tplogqinfo(lev,flags);
            if (EXFAIL==ret)
            {
                throw nstd_exception(Nerror); 
            }

            return ret;
        },
        R"pbdoc(
        Query logger information.

        .. code-block:: python
            :caption: tplogqinfo example
            :name: tplogqinfo-example

                import endurox as e
                e.tplogconfig(e.LOG_FACILITY_TP, -1, "tp=4", "", "/dev/stdout")
                info = e.tplogqinfo(4, e.TPLOGQI_GET_TP)

                if info & 0x0000ffff == e.LOG_FACILITY_TP:
                    e.tplog_info("LOG_FACILITY_TP Matched")
                
                if info >> 24 == 4:
                    e.tplog_info("Log level 4 matched")

                info = e.tplogqinfo(5, e.TPLOGQI_GET_TP)

                if info == 0:
                    e.tplog_info("Not logging level 5")

                # Above would print:
                # t:USER:4:c9e5ad48:23764:7fc434fdd740:000:20220603:232144217247:tplog       :/tplog.c:0582:LOG_FACILITY_TP Matched
                # t:USER:4:c9e5ad48:23764:7fc434fdd740:000:20220603:232144217275:tplog       :/tplog.c:0582:Log level 4 matched
                # t:USER:4:c9e5ad48:23764:7fc434fdd740:000:20220603:232144217288:tplog       :/tplog.c:0582:Not logging level 5
                
        For more details see **tplogqinfo(3)** C API call.

        :raise NstdException: 
            | Following error codes may be present:
            | :data:`.NEINVAL` - Invalid logger flag.

        Parameters
        ----------
        lev: int
            Check request log level. If given level is higher than configured, value **0** is returned
            by the function. Unless :data:`.TPLOGQI_EVAL_RETURN` flag is passed, in such case lev does not
            affect the result of the function.
        flags : int
            One of the following flag (exclusive) :data:`.TPLOGQI_GET_NDRX`, :data:`TPLOGQI_GET_UBF` or 
            :data:`.TPLOGQI_GET_TP`. The previous flag may be bitwise or'd with :data:`.TPLOGQI_EVAL_RETURN`
            and :data:`.TPLOGQI_EVAL_DETAILED`

        Returns
        -------
        ret : int
            Bit flags of currently active logger (one of) according to input *flags*:
            :data:`.LOG_FACILITY_NDRX`, :data:`.LOG_FACILITY_UBF`, :data:`.LOG_FACILITY_TP`, 
            :data:`.LOG_FACILITY_TP_THREAD`, :data:`.LOG_FACILITY_TP_REQUEST`, :data:`.LOG_FACILITY_NDRX_THREAD`,
            :data:`.LOG_FACILITY_UBF_THREAD`, :data:`.LOG_FACILITY_NDRX_REQUEST`, :data:`.LOG_FACILITY_UBF_REQUEST`.
            Additionally flag :data:`.TPLOGQI_RET_HAVDETAILED` may be present.
            Byte 4 in return value (according to mask **0xff000000**) contains currently active log level.
            The return value *ret* may be set to **0** in case if *lev* argument indicated higher log level
            than currently used and *flags* did not contain flag :data:`.TPLOGQI_EVAL_RETURN`.  

         )pbdoc", py::arg("lev"), py::arg("flags"));

    m.def(
        "tplogsetreqfile",
        [](py::object data, const char * filename, const char * filesvc)
        {

            atmibuf in;

            if (!py::isinstance<py::none>(data))
            {
                in = ndrx_from_py(data);
            }
            
            {
                char type[8]={EXEOS};
                char subtype[16]={EXEOS};
                py::gil_scoped_release release;

                //Check is it UBF or not?

                if (tptypes(*in.pp, type, subtype) == EXFAIL)
                {
                    NDRX_LOG(log_error, "Invalid buffer type");
                    throw std::invalid_argument("Invalid buffer type");
                }

                if (EXFAIL==tplogsetreqfile( (0==strcmp(type, "UBF")?in.pp:NULL), const_cast<char *>(filename), 
                    const_cast<char *>(filesvc)))
                {
                    // In case if buffer changed..
                    in.p=*in.pp;
                    throw atmi_exception(tperrno);   
                }
                // In case if buffer changed..
                in.p=*in.pp;
            }

            //Return python object... (in case if one was passed in...)
            return ndrx_to_py(in);
        },
        R"pbdoc(
        Redirect logger to request file extracted from buffer, filename or file name service.
        Note that in case if :data:`.TPESVCFAIL` error is received, exception is thrown
        and if *data* buffer was modified by the service, modification is lost.
        The new log file name must be present in one of the arguments. For full details
        see the C API call.

        .. code-block:: python
            :caption: tplogsetreqfile example
            :name: tplogsetreqfile-example

                import endurox as e
                e.tplogsetreqfile(None, "/tmp/req_1111", None)
                # would print to "/tmp/req_1111"
                e.tplog_info("OK")
                e.tplogclosereqfile()
                
        For more details see **tplogsetreqfile(3)** C API call.

        :raise AtmiException: 
            | Following error codes may be present:
            | :data:`.TPEINVAL` - Invalid parameters.
            | :data:`.TPENOENT` - *filesvc* is not available.
            | :data:`.TPETIME` - *filesvc* timed out.
            | :data:`.TPESVCFAIL` - *filesvc* failed.
            | :data:`.TPESVCERR`- *filesvc* crashed.
            | :data:`.TPESYSTEM` - System error.
            | :data:`.TPEOS` - Operating system error.

        Parameters
        ----------
        data: dict
            UBF buffer, where to search for **EX_NREQLOGFILE** field. Or if field is not found
            this buffer is used to call *filesvc*. Parameter is conditional.May use :data:`None` or 
            empty string if not present.
        filename : str
            New request file name. Parameter is conditional. May use :data:`None` or 
            empty string if not present.
        filesvc : str
            Service name to request. Parameter is conditional. And can use :data:`None`
            or empty string if not present.

        Returns
        -------
        ret : dict
            XATMI buffer passed in as *data* argument and/or used for service call.
            If not buffer is used, **NULL** buffer is returned.

         )pbdoc",
            py::arg("data"), py::arg("filename")="", py::arg("filesvc")="");

    m.def(
        "tplogsetreqfile_direct",
        [](std::string filename)
        {
            py::gil_scoped_release release;
            tplogsetreqfile_direct(const_cast<char *>(filename.c_str()));
        },
        R"pbdoc(
        Set logfile from given filename.

        For more details see **tplogsetreqfile_direct(3)** C API call.

        Parameters
        ----------
        filename: str
            Log file name. It is recommended that file name is different than
            log file name used for the process level logging.

         )pbdoc",
            py::arg("filename")="");

    m.def(
        "tploggetbufreqfile",
        [](py::object data)
        {
            char filename[PATH_MAX+1];
            auto in = ndrx_from_py(data);
            {
                py::gil_scoped_release release;
                if (EXSUCCEED!=tploggetbufreqfile(*in.pp, filename, sizeof(filename)))
                {
                    throw atmi_exception(tperrno); 
                }
            }
            return py::str(filename);
        },
        R"pbdoc(
        Extract request file from the UBF buffer. At current Enduro/X version
        this just reads **EX_NREQLOGFILE** field from the buffer and returns value.
                
        For more details see **tploggetbufreqfile(3)** C API call.

        :raise AtmiException: 
            | Following error codes may be present:
            | :data:`.TPENOENT` - Request file name not present or UBF error.
            | :data:`.TPEINVAL` - Invalid UBF buffer.

        Parameters
        ----------
        data: dict
            UBF buffer, where to search for **EX_NREQLOGFILE** field.

        Returns
        -------
        ret : str
            Request file name.
         )pbdoc",
        py::arg("data"));

     m.def(
        "tploggetreqfile",
        [](void)
        {
            char filename[PATH_MAX+1]="";
            {
                py::gil_scoped_release release;
                tploggetreqfile(filename, sizeof(filename));
            }
            return py::str(filename);
        },
        R"pbdoc(
        Get current request file name for **tp** topic. In case if request file
        is not used, empty string is returned.
                
        For more details see **tploggetreqfile(3)** C API call.

        Returns
        -------
        ret : str
            Request file name or empty string.
         )pbdoc");

     m.def(
        "tplogdelbufreqfile",
        [](py::object data)
        {
            auto in = ndrx_from_py(data);
            {
                py::gil_scoped_release release;
                if (EXSUCCEED!=tplogdelbufreqfile(*in.pp))
                {
                        throw atmi_exception(tperrno);
                }
            }

            return ndrx_to_py(in);
        },
        R"pbdoc(
        Delete request file name from the given UBF buffer. Altered buffer
        is returned.
                
        For more details see **tplogdelbufreqfile(3)** C API call.

        :raise AtmiException: 
            | Following error codes may be present:
            | :data:`.TPENOENT` - Request file name not present or UBF error.
            | :data:`.TPEINVAL` - Invalid UBF buffer.

        Parameters
        ----------
        data: dict
            UBF buffer, where to delete **EX_NREQLOGFILE** field.

        Returns
        -------
        ret : dict
            Altered UBF buffer.
         )pbdoc",
        py::arg("data"));

     m.def(
        "tplogclosereqfile",
        [](void)
        {
            py::gil_scoped_release release;
            tplogclosereqfile();
        },
        R"pbdoc(
        Close request logging file.
                
        For more details see **tplogclosereqfile(3)** C API call.

        )pbdoc");

     m.def(
        "tplogclosethread",
        [](void)
        {
            py::gil_scoped_release release;
            tplogclosethread();
        },
        R"pbdoc(
        Close thread logging file.

        .. code-block:: python
            :caption: tplogclosethread example
            :name: tplogclosethread-example

                import endurox as e
                
                e.tplogconfig(e.LOG_FACILITY_TP_THREAD, -1, "tp=4", "", "/tmp/thread1")
                e.tplog_info("OK")
                e.tplogclosethread()

        For more details see **tplogclosethread(3)** C API call.

        )pbdoc");

     m.def(
        "tplogdump",
        [](int lev, const char * comment, py::bytes data)
        {
            std::string val(PyBytes_AsString(data.ptr()), PyBytes_Size(data.ptr()));

            py::gil_scoped_release release;
            tplogdump(lev, const_cast<char *>(comment), 
                const_cast<char *>(val.data()), val.size());
        },
        R"pbdoc(
        Dump byte array to log file.

        .. code-block:: python
            :caption: tplogdump example
            :name: tplogdump-example

                import endurox as e
                
                e.tplogdump(e.log_info, "TEST TITLE", b'HELLO WORLD FROM ENDUROX')
                # current log file will contain:
                # t:USER:4:d190fd96:04714:7f15d6d9e740:000:20220607:170621814408:tplogdump   :/tplog.c:0614:TEST TITLE (nr bytes: 24)
                #   0000  48 45 4c 4c 4f 20 57 4f 52 4c 44 20 46 52 4f 4d  HELLO WORLD FROM
                #   0010  20 45 4e 44 55 52 4f 58                           ENDUROX

        For more details see **tplogdump(3)** C API call.

        Parameters
        ----------
        lev: int
            Log level.
        comment: str
            Log title.
        data: bytes
            Bytes to dump to log.
        )pbdoc",
        py::arg("lev"), py::arg("comment"), py::arg("data"));

     m.def(
        "tplogdumpdiff",
        [](int lev, const char * comment, py::bytes data1, py::bytes data2)
        {
            std::string val1(PyBytes_AsString(data1.ptr()), PyBytes_Size(data1.ptr()));
            std::string val2(PyBytes_AsString(data2.ptr()), PyBytes_Size(data2.ptr()));

            long len = std::min(val1.size(), val2.size());

            py::gil_scoped_release release;
            tplogdumpdiff(lev, const_cast<char *>(comment), 
                const_cast<char *>(val1.data()), const_cast<char *>(val2.data()), 
                len);
        },
        R"pbdoc(
        Compare two byte arrays and print differences for the common length.

        .. code-block:: python
            :caption: tplogdumpdiff example
            :name: tplogdumpdiff-example

                import endurox as e
                
                e.tplogdumpdiff(e.log_info, "TEST TITLE", b'HELLO WORLD FROM ENDUROX', 
                    b'HELLO FROM OTHER PLACE OR SYSTEM')
                # t:USER:4:d190fd96:25552:7f5ef2bb3740:000:20220607:185313821586:plogdumpdiff:/tplog.c:0627:TEST TITLE
                # <  0000  48 45 4c 4c 4f 20 57 4f 52 4c 44 20 46 52 4f 4d  HELLO WORLD FROM
                # >  0000  48 45 4c 4c 4f 20 46 52 4f 4d 20 4f 54 48 45 52  HELLO FROM OTHER
                # <  0010  20 45 4e 44 55 52 4f 58                           ENDUROX
                # >  0010  20 50 4c 41 43 45 20 4f                           PLACE O

        For more details see **tplogdumpdiff(3)** C API call.

        Parameters
        ----------
        lev: int
            Log level.
        comment: str
            Log title.
        data1: bytes
            Bytes to compare.
        data2: bytes
            Bytes to compare.
        )pbdoc",
        py::arg("lev"), py::arg("comment"), py::arg("data1"), py::arg("data2"));

     m.def(
        "tplogfplock",
        [](int lev, long flags)
        {
            ndrx_debug_t *ptr = tplogfplock(lev, flags);

            //Allow to return None
            if (nullptr!=ptr)
            {
                return std::unique_ptr<pyndrxdebugptr>(new pyndrxdebugptr(ptr)); 
            }
            else
            {
                return std::unique_ptr<pyndrxdebugptr>{};
            }
        },
        R"pbdoc(
        Locks and returns logging handle. During the time while handle is locked 
        system functions such as log-rotate would not work for given process.
        In case if logging is not required i.e. 'lev' is greater than currently configured 
        logging level, :data:`None` is returned.
                
        For more details see **tplogfplock(3)** C API call.

        Parameters
        ----------
        lev: int
            Request log level. If handle is needed by not checking the log level,
            use value *-1*.
        flags : int
            RFU.

        Returns
        -------
        ret : .NdrxDebugHandle
            Enduro/X logger handle or :data:`None` if logging is not required.

         )pbdoc",
        py::arg("lev")=-1, py::arg("flags")=0);

     m.def(
        "tplogfpget",
        [](pyndrxdebugptr dbg, long flags)
        {
            ndrx_debug_t *ptr = reinterpret_cast<ndrx_debug_t *>(dbg.ptr);
            return fileno(tplogfpget(ptr, flags));
        },
        R"pbdoc(
        Get fileno for currently locked logger (i.e. value returned from :func:`.tplogfplock`).

        .. code-block:: python
            :caption: tplogfpget example
            :name: tplogfpget-example

                import os
                import endurox as e
                h = e.tplogfplock(e.log_debug, 0)
                no = e.tplogfpget(h, 0)
                os.write(fd, b'HELLO WORLD')
                os.fsync(fd)
                e.tplogfpunlock(h)

        For more details see **tplogfpget(3)** C API call.

        Parameters
        ----------
        dbg: .NdrxDebugHandle
            Debug handle previously locked by :func:`.tplogfplock`
        flags : int
            RFU

        Returns
        -------
        ret : int
            File descriptor.

         )pbdoc",
        py::arg("dbg"), py::arg("flags")=0);

     m.def(
        "tplogfpunlock",
        [](pyndrxdebugptr dbg)
        {
            ndrx_debug_t *ptr = reinterpret_cast<ndrx_debug_t *>(dbg.ptr);
            tplogfpunlock(ptr);
        },
        R"pbdoc(
        Unlock logger handle for given process previously locked by 
        :func:`.tplogfplock`.

        For more details see **tplogfpunlock(3)** C API call.

        Parameters
        ----------
        dbg: .NdrxDebugHandle
            Debug handle previously locked by :func:`.tplogfplock`
        flags : int
            RFU

         )pbdoc",
        py::arg("dbg"));

     m.def(
        "tplogprintubf",
        [](int lev, const char *title, py::object data)
        {
            auto in = ndrx_from_py(data);
            py::gil_scoped_release release;
            tplogprintubf(lev, const_cast<char *>(title), reinterpret_cast<UBFH *>(*in.pp));
        },
        R"pbdoc(
        Print UBF buffer to log file.

        .. code-block:: python
            :caption: tplogprintubf example
            :name: tplogprintubf-example

                import endurox as e
                e.tplogprintubf(e.log_info, "TEST", {"data":{"T_STRING_FLD":["HELLO", "WORLD"], "T_LONG_FLD":9991}})

                # would print to log file:
                # t:USER:4:c9e5ad48:10162:7f623e424740:000:20220607:090939401481:plogprintubf:bf/ubf.c:1790:TEST
                # T_LONG_FLD	9991
                # T_STRING_FLD	HELLO
                # T_STRING_FLD	WORLD

        For more details see **tplogprintubf(3)** C API call.

        Parameters
        ----------
        lev: int
            Log level.
        title: str
            Dump title.
        data: dict
            UBF buffer to print.

         )pbdoc",
        py::arg("lev"), py::arg("title"), py::arg("data"));

}

/* vim: set ts=4 sw=4 et smartindent: */

