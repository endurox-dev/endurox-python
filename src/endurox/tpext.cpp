/**
 * @brief ATMI Server extensions
 *
 * @file tpext.cpp
 */
/* -----------------------------------------------------------------------------
 * Python module for Enduro/X
 *
 * Copyright (C) 2021 - 2022, Mavimax, Ltd. All Rights Reserved.
 * See LICENSE file for full text.
 * -----------------------------------------------------------------------------
 * AGPL license:
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Affero General Public License, version 3 as published
 * by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
 * PARTICULAR PURPOSE. See the GNU Affero General Public License, version 3
 * for more details.
 *
 * You should have received a copy of the GNU Affero General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 * -----------------------------------------------------------------------------
 * A commercial use license is available from Mavimax, Ltd
 * contact@mavimax.com
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
#include <mutex>

#ifdef EX_OS_AIX
#undef __MULTILOCALE_API
#endif
#include <sys_unix.h>

/*---------------------------Externs------------------------------------*/
/*---------------------------Macros-------------------------------------*/
/*---------------------------Enums--------------------------------------*/
/*---------------------------Typedefs-----------------------------------*/
/*---------------------------Globals------------------------------------*/
/*---------------------------Statics------------------------------------*/

/** current handle for b4poll callback */
static ndrxpy_object_t * M_b4pollcb_handler = nullptr;

/** periodic server callback handler */
static ndrxpy_object_t * M_addperiodcb_handler = nullptr;

/** filedescriptor map to py callbacks */
static std::map<int, ndrxpy_object_t*> M_fdmap;

/*---------------------------Prototypes---------------------------------*/

namespace py = pybind11;

/**
 * @brief Avoid C++ destructors for Python objects.
 *  It is user programs responsiliby to clean up all poller fds or leave
 *  them for OS to collect at program exit.
 */
expublic void ndrxpy_fdmap_clear(void)
{
    if (!M_fdmap.empty())
    {
        for (auto const & map : M_fdmap)
        {
            //Cannot delete functions?
            //map.second->obj = py::none();
            map.second->obj2 = py::none();
            //Avoid leak?
	    delete map.second;
        }
    }
    M_fdmap.clear();
}

/**
 * @brief Dispatch b4 poll callback
 */
exprivate int ndrxpy_b4pollcb_callback(void)
{
    //Get the gil...
    int cret;
    try
    {
        py::gil_scoped_acquire acquire;

        py::object ret = M_b4pollcb_handler->obj();
        cret=ret.cast<int>();
    }
    catch (const std::exception &e)
    {
        NDRX_LOG(log_error, "Got exception b4pollcb: %s", e.what());
        userlog(const_cast<char *>("%s"), e.what());
        cret=EXFAIL;
    }

    return cret;
}

/**
 * @brief register b4 poll callback handler.
 * 
 * @param func callback function
 */
exprivate void ndrxpy_tpext_addb4pollcb (const py::object &func)
{
    //Allocate the object
    if (nullptr!=M_b4pollcb_handler)
    {
        delete M_b4pollcb_handler;
        M_b4pollcb_handler = nullptr;
    }

    M_b4pollcb_handler = new ndrxpy_object_t();
    M_b4pollcb_handler->obj = func;

    if (EXSUCCEED!=tpext_addb4pollcb(ndrxpy_b4pollcb_callback))
    {
        throw atmi_exception(tperrno);
    }
}


/**
 * @brief Periodic callback dispatch
 */
exprivate int ndrxpy_addperiodcb_callback(void)
{
    //Get the gil...
    int cret;
    try
    {
        py::gil_scoped_acquire acquire;
        py::object ret = M_addperiodcb_handler->obj();
        cret=ret.cast<int>();
    }
    catch (const std::exception &e)
    {
        NDRX_LOG(log_error, "Got exception periodcb: %s", e.what());
        userlog(const_cast<char *>("%s"), e.what());
        cret=EXFAIL;
    }

    return cret;
}

/**
 * @brief register periodic callback handler
 * @param secs number of seconds for callback interval
 * @param func callback function
 */
exprivate void ndrxpy_tpext_addperiodcb (int secs, const py::object &func)
{
    //Allocate the object
    if (nullptr!=M_addperiodcb_handler)
    {
        delete M_addperiodcb_handler;
        M_addperiodcb_handler = nullptr;
    }

    M_addperiodcb_handler = new ndrxpy_object_t();
    M_addperiodcb_handler->obj = func;

    if (EXSUCCEED!=tpext_addperiodcb(secs, ndrxpy_addperiodcb_callback))
    {
        throw atmi_exception(tperrno);
    }
}

/**
 * @brief pollevent callback
 * 
 * @param fd monitored file descriptor
 * @param events events monitored
 * @param ptr1 not used
 * @return -1 on failure, 0 ok 
 */
exprivate int ndrxpy_pollevent_cb(int fd, uint32_t events, void *ptr1)
{
    int cret;
    try
    {
        py::gil_scoped_acquire acquire;
        py::object ret=M_fdmap[fd]->obj(fd, events, M_fdmap[fd]->obj2);
        cret=ret.cast<int>();
    }
    catch (const std::exception &e)
    {
        NDRX_LOG(log_error, "Got exception at pollevent_cb: %s", e.what());
        userlog(const_cast<char *>("%s"), e.what());
        cret=EXFAIL;
    }


    return cret;
}

/**
 * @brief Register extensions callback
 * 
 * @param fd file descriptor
 * @param events poll events
 * @param ptr1 object to pass back
 * @param func callback func
 */
exprivate void ndrxpy_tpext_addpollerfd (int fd, uint32_t events, const py::object ptr1, const py::object &func)
{
    ndrxpy_object_t * obj = new ndrxpy_object_t();

    obj->obj = func;
    obj->obj2 = ptr1;

    if (EXSUCCEED!=tpext_addpollerfd(fd, (uint32_t)events, NULL, ndrxpy_pollevent_cb))
    {
        throw atmi_exception(tperrno);
    }

    M_fdmap[fd] = obj;
}

/**
 * @brief remove fd from polling
 * 
 * @param fd file descriptor
 */
exprivate void ndrxpy_tpext_delpollerfd(int fd)
{
    auto it = M_fdmap.find(fd);
    if (it != M_fdmap.end()) {
        delete M_fdmap[fd];
        M_fdmap.erase(it);
    }

    if (EXSUCCEED!=tpext_delpollerfd(fd))
    {
        throw atmi_exception(tperrno);
    }
}

/**
 * @brief Register ATMI server extensions
 * 
 * @param m Pybind11 module handle
 */
expublic void ndrxpy_register_tpext(py::module &m)
{
    m.def(
        "tpext_addb4pollcb", [](const py::object &func)
        { ndrxpy_tpext_addb4pollcb(func); },
        R"pbdoc(
        Register callback handler for server going. Work only from server thread. 
        API is not thread safe. For MT servers only use in tpsvrinit().

        This function applies to ATMI servers only.

        .. code-block:: python
            :caption: tpext_addb4pollcb example
            :name: tpext_addb4pollcb-example

            import sys
            import endurox as e

            def b4poll():
                e.tplog_info("Going to poll...");

            class Server:

                def tpsvrinit(self, args):
                    e.tpext_addb4pollcb(b4poll)
                    return 0
                    
            if __name__ == '__main__':
                e.tprun(Server(), sys.argv)

        For more details see **tpext_addb4pollcb(3)** C API call.

        :raise AtmiException: 
            | Following error codes may be present:
            | :data:`.TPEINVAL` - Invalid parameters.

        Parameters
        ----------
        func: object
            Callback function.

         )pbdoc", py::arg("func"));

    m.def(
        "tpext_delb4pollcb", [](void)
        {   
            if (EXSUCCEED!=tpext_delb4pollcb())
            {
                throw atmi_exception(tperrno);
            }

            //Reset handler...
            delete M_b4pollcb_handler;
            M_b4pollcb_handler = nullptr;
        },
        R"pbdoc(
        Remove current before server poll callback previously
        set by :func:`.tpext_addb4pollcb`.

        This function applies to ATMI servers only.

        For more details see **tpext_delb4pollcb(3)** C API call.

         )pbdoc");

     m.def(
        "tpext_addperiodcb", [](int secs, const py::object &func)
        { ndrxpy_tpext_addperiodcb(secs, func); },
        R"pbdoc(
        Register periodic callback handler. API is not thread safe. 
        For MT servers only use in tpsvrinit().

        This function applies to ATMI servers only.

        .. code-block:: python
            :caption: tpext_addperiodcb example
            :name: tpext_addperiodcb-example

            import sys
            import endurox as e

            def period():
                return 0

            class Server:

                def tpsvrinit(self, args):
                    e.userlog('Server startup')
                    e.tpext_addperiodcb(1, period)
                    return 0

            if __name__ == '__main__':
                e.tprun(Server(), sys.argv)


        For more details see **tpext_addperiodcb(3)** C API call.

        Parameters
        ----------
        secs: int
            Interval of seconds to call the callback while service
            is idling.
        func: object
            Callback function.
         )pbdoc",
        py::arg("secs"), py::arg("func"));

    m.def(
        "tpext_delperiodcb", [](void)
        {   
            if (EXSUCCEED!=tpext_delperiodcb())
            {
                throw atmi_exception(tperrno);
            }

            //Reset handler...
            delete M_addperiodcb_handler;
            M_addperiodcb_handler = nullptr;
        },
        R"pbdoc(
        Remove periodic XATMI server idle time callback handle previously set by :func:`.tpext_addperiodcb`. 
        API is not thread safe. For MT servers only use in tpsvrinit().

        This function applies to ATMI servers only.

        For more details see **tpext_delperiodcb(3)** C API call.

         )pbdoc"
        );

     m.def(
        "tpext_addpollerfd", [](int fd, uint32_t events, const py::object ptr1, const py::object &func)
        { ndrxpy_tpext_addpollerfd(fd, events, ptr1, func); },
        R"pbdoc(
        Monitor file descriptor in XATMI server main dispatcher.
        This allows the main thread of the server process to select either a service call
        or perform callback if file descriptor *fd* has events arrived. Events monitor
        are ones which are passed to poll() unix call, and shall be passed in *events* field,
        such as **select.POLLIN**.
        tpext_addpollerfd() can be only called when XATMI server has performed the init, i.e.
        outside of the tpsvrinit().

        Function is not thread safe. This function applies to ATMI servers only.

        .. code-block:: python
            :caption: tpext_addpollerfd example
            :name: tpext_addpollerfd-example

            import sys, os, select
            import endurox as e

            outx = None
            path = "/tmp/tmp_py"

            def cb(fd, events, ptr1):
                e.tplog_info("fd %d got event %d" % (fd, events));
                # process the events...
                return 0

            # use b4poll() callback to activate the fd callback 
            def b4poll():
                e.tpext_addpollerfd(outx, select.POLLIN, None, cb)
                e.tpext_delb4pollcb()
                return 0
                
            class Server:

                def tpsvrinit(self, args):
                    os.mkfifo( path, 0O644 )
                    outx = os.open(path, os.O_NONBLOCK | os.O_RDWR)
                    e.tpext_addb4pollcb(b4poll)
                    return 0

                def tpsvrdone(self):
                    global outx
                    global path
                    os.close(outx)
                    os.remove(path) if os.path.exists(path) else None
                    e.userlog('Server shutdown')

            if __name__ == '__main__':
                e.tprun(Server(), sys.argv)

        For more details see **tpext_addpollerfd(3)** C API call.

        :raise AtmiException: 
            | Following error codes may be present:
            | :data:`.TPEMATCH` - *fd* is already registered with callback.
            | :data:`.TPEPROTO` - Called from invalid place, e.g. **tpsvrinit()** func.
            | :data:`.TPESYSTEM` - System error occurred.
            | :data:`.TPEOS` - Operating system error occurred.

        Parameters
        ----------
        fd : int
            File descriptor to monitor.
        events : uint32
            Bitnmask of select events. In case if using kqueue, convert
            flags from signed value to unsigned, e.g. select.KQ_FILTER_READ + 2^32.
        ptr1 : object
            Custom object passed to callback.
        func : object
            Callback func used for notifying for events occurred on *fd*.
            Function signature must accept signature of "(fd, events, ptr1)".
            Where *fd* is file descriptor, *events* is poll() events occurred on
            *fd*, ptr1 is custom pointer passed when tpext_addpollerfd() was called.

         )pbdoc",
        py::arg("fd"), py::arg("events"), py::arg("ptr1"), py::arg("func"));

     m.def(
        "tpext_delpollerfd", [](int fd)
        { ndrxpy_tpext_delpollerfd(fd); },
        R"pbdoc(
        Delete file descriptor for ATMI server poller.

        Function is not thread safe. This function applies to ATMI servers only.

        For more details see **tpext_delpollerfd(3)** C API call.

        :raise AtmiException: 
            | Following error codes may be present:
            | :data:`.TPEINVAL` - Invalid file descriptor *fd* passed.
            | :data:`.TPEMATCH` - File descriptor was not registered.
            | :data:`.TPEPROTO` - Invalid call sequence. Called from **tpsvrinit()**.
            | :data:`.TPESYSTEM` - System error occurred.
            | :data:`.TPEOS` - Operating system error occurred.

        Parameters
        ----------
        fd : int
            File descriptor to remove.

         )pbdoc",
        py::arg("fd"));

    m.def(
        "ndrx_epoll_mode", [](void)
        {   
            return py::str(ndrx_epoll_mode());
        },
        R"pbdoc(
        Return poller code used by used Enduro/X Core build.

        Returns
        -------
        str
            - **epoll** - Linux build, using Posix queues and XATMI server waits
                for events with epoll_wait().
            - **kqueue** - BSD build, using Posix queues and XATMI server waits
                for events with kevent().
            - **SystemV** - Unix mode (except MacOS), two aux threads used by
                server process and one aux thread by client process.
                XATMI server waits for events with poll(). Using System V message
                queue.
            - **poll** - Posix queue mode for operating systems which does not
                support polling on queue. Additional thread is used by XATMI
                server for event delivery. XATMI server waits for events with poll().
            - **emq** - Emulated Posix queue mode used for MacOS.
                Additional thread is used by XATMI
                server for event delivery. XATMI server waits for events with poll().
            - **svapoll** - For IBM AIX, XATMI server is using poll() extension to monitor
                System V queues. 
         )pbdoc");
}

/* vim: set ts=4 sw=4 et smartindent: */
