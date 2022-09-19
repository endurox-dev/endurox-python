/**
 * @brief Typed buffer to Python conversion
 *
 * @file bufconv.cpp
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
#include <ndebug.h>
#undef _

#include "exceptions.h"
#include "ndrx_pymod.h"

#include <pybind11/functional.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <functional>

/*---------------------------Externs------------------------------------*/
/*---------------------------Macros-------------------------------------*/
/*---------------------------Enums--------------------------------------*/
/*---------------------------Typedefs-----------------------------------*/
/*---------------------------Globals------------------------------------*/
/*---------------------------Statics------------------------------------*/
expublic thread_local bool ndrxpy_G_use_ubfdict = true; /**< Use UbfDict() by default */
/*---------------------------Prototypes---------------------------------*/
namespace py = pybind11;

/**
 * @brief This will add all ATMI related stuff under the {"data":<ATMI data...>}
 *  TODO: Free incoming UBF buffer (somehere marking shall be put)
 * @param buf ATMI buffer to conver to Python
 * @param is_sub_buffer is master buffer, ubf or ptr?
 * @return python object (dict)
 */
expublic py::object ndrx_to_py(atmibuf &buf, int is_sub_buffer)
{
    char type[8]={EXEOS};
    char subtype[16]={EXEOS};
    long size;
    py::dict result;
    int ret;
    char *tmp_ptr;

    if ((size=tptypes(*buf.pp, type, subtype)) == EXFAIL)
    {
        NDRX_LOG(log_error, "Invalid buffer type (ptr=%p): %s", 
            *buf.pp, tpstrerror(tperrno));
        throw std::invalid_argument("Invalid buffer type");
    }

    NDRX_LOG(log_debug, "Into ndrx_to_py() type=[%s] subtype=[%s] size=%ld pp=%p is_sub_buffer=%d", 
        type, subtype, size, *buf.pp, is_sub_buffer);

    //Return buffer sub-type
    result["buftype"] = type;

    if (EXEOS!=subtype[0])
    {
        result["subtype"]=subtype;
    }

    NDRX_LOG(log_debug, "Converting buffer type [%s]", type);

    if (strcmp(type, "STRING") == 0 || strcmp(type, "JSON") == 0)
    {
        result["data"]=py::cast(*buf.pp);
    }
    else if (strcmp(type, "CARRAY") == 0 || strcmp(type, "X_OCTET") == 0)
    {
        result["data"]=py::bytes(*buf.pp, buf.len);
    }
    else if (strcmp(type, "UBF") == 0)
    {
        if (ndrxpy_G_use_ubfdict)
        {
            NDRX_LOG(log_debug, "Using UbfDict() len =%ld", size);

            result["data"]=ndrxpy_alloc_UbfDict(*buf.pp, is_sub_buffer, size);

            //Parent may free up master buffers...
            if (NDRXPY_SUBBUF_NORM==is_sub_buffer)
            {
                tmp_ptr = buf.p;
                buf.pp = &tmp_ptr;
                //release buffer ptr, as now handled by data
                buf.p=nullptr;
            }
        }
        else
        {
            NDRX_LOG(log_debug, "Using dict()");
            result["data"]=ndrxpy_to_py_ubf(*buf.fbfr(), 0);
        }
    }
    else if (strcmp(type, "VIEW") == 0)
    {
        result["data"] = ndrxpy_to_py_view(*buf.pp, subtype, size);
    }
    else if (strcmp(type, "NULL") == 0)
    {
        /* data field not present -> NULL */
    } 
    else
    {
        throw std::invalid_argument("Unsupported buffer type");
    }

    // attach call info, if have any.
    if (strcmp(type, "NULL") != 0)
    {
        char *p_buf = nullptr;
        ret = tpgetcallinfo(*buf.pp, reinterpret_cast<UBFH **>(&p_buf), TPCI_NOEOFERR);
        
        if (EXTRUE==ret)
        {
            //Get buffer size, as later used by auto-realloc
            if ((size=tptypes(p_buf, type, subtype)) == EXFAIL)
            {
                throw std::invalid_argument("Failed to get callinfo buffer size");
            }

            // setup callinfo block
            result[NDRXPY_DATA_CALLINFO]=ndrxpy_alloc_UbfDict(p_buf, NDRXPY_SUBBUF_NORM, size);
        }
        else if (EXFAIL==ret)
        {
            NDRX_LOG(log_debug, "Error checking tpgetcallinfo()");
            throw atmi_exception(tperrno);
        }
    }

    return result;
}

/**
 * @brief Process call info from main call dict
 * 
 * @param dict dictionary used for call
 * @param buf prepared ATMI buffer
 */
exprivate void set_callinfo(py::dict & dict, atmibuf &buf)
{
    if (dict.contains(NDRXPY_DATA_CALLINFO))
    {
        atmibuf cibuf;
        char *ci_ptr = nullptr;
        auto cibufdata = dict[NDRXPY_DATA_CALLINFO];

        NDRX_LOG(log_debug, "Setting call info");

        if (NULL==*buf.pp)
        {
            NDRX_LOG(log_error, "callinfo cannot be set for NULL buffers!");
            throw std::invalid_argument("callinfo cannot be set for NULL buffers");
        }

        //OK we have support of UbfDict Too
        if (py::isinstance<py::dict>(cibufdata))
        {
            ndrxpy_from_py_ubf(static_cast<py::dict>(cibufdata), cibuf);   

            ci_ptr = *cibuf.pp;
        }
        else if (ndrxpy_is_UbfDict(cibufdata))
        {
            ndrx_longptr_t ptr = cibufdata.attr("_buf").cast<py::int_>();
            atmibuf *data_buf = reinterpret_cast<atmibuf *>(ptr);
            ci_ptr = data_buf->p;
        }
        else
        {
            NDRX_LOG(log_error, "callinfo must be dict or UbfDict but is not!");
            throw std::invalid_argument("callinfo must be dict or UbfDict but is not!");
        }

        if (EXSUCCEED!=tpsetcallinfo(*buf.pp, reinterpret_cast<UBFH *>(ci_ptr), 0))
        {
            throw atmi_exception(tperrno);
        }
    }
}

/**
 * @brief Must be dict with "data" key. So valid buffer is:
 * 
 * {"data":<ATMI_BUFFER>, "buftype":"UBF|VIEW|STRING|JSON|CARRAY|NULL", "subtype":"<VIEW_TYPE>", ["callinfo":{<UBF_DATA>}]}
 * 
 * For NULL buffers, data field is not present.
 * 
 * @param obj Pyton object
 * @param reset_ptr reset Python buffer ptr (as no longer valid). 
 *  This is in case of UbfDict
 * @return converted ATMI buffer
 */
expublic atmibuf ndrx_from_py(py::object obj, bool reset_ptr)
{
    std::string buftype = "";
    std::string subtype = "";
    atmibuf buf;

    NDRX_LOG(log_debug, "Into ndrx_from_py()");

    if (!py::isinstance<py::dict>(obj))
    {
        throw std::invalid_argument("Unsupported buffer type");
    }
    //Get the data field

    auto dict = static_cast<py::dict>(obj);

    //for NULL buffers, we do not contain this...
    py::object data;
    
    if (dict.contains(NDRXPY_DATA_DATA))
    {
        data = dict[NDRXPY_DATA_DATA];
    }

    if (dict.contains(NDRXPY_DATA_BUFTYPE))
    {
        buftype = py::str(dict[NDRXPY_DATA_BUFTYPE]);
    }
    
    if (dict.contains(NDRXPY_DATA_SUBTYPE))
    {
        subtype = py::str(dict[NDRXPY_DATA_SUBTYPE]);
    }

    NDRX_LOG(log_debug, "Converting out: [%s] / [%s]", buftype.c_str(), subtype.c_str());

    /* process JSON data... as string */
    if (buftype=="JSON")
    {
        if (!py::isinstance<py::str>(data))
        {
            throw std::invalid_argument("String expected for JSON buftype, got: "+buftype);
        } 

        std::string s = py::str(data);

        buf = atmibuf("JSON", s.size() + 1);
        strcpy(*buf.pp, s.c_str());
    }
    else if (buftype=="VIEW")
    {
        if (subtype=="")
        {
            throw std::invalid_argument("subtype expected for VIEW buffer");
        }

        buf = atmibuf("VIEW", subtype.c_str());

        ndrxpy_from_py_view(static_cast<py::dict>(data), buf, subtype.c_str());
    }
    else if (py::isinstance<py::bytes>(data))
    {
        if (buftype!="" && buftype!="CARRAY")
        {
            throw std::invalid_argument("For byte array data "
                "expected CARRAY buftype, got: "+buftype);
        }
        
        buf = atmibuf("CARRAY", PyBytes_Size(data.ptr()));
        memcpy(*buf.pp, PyBytes_AsString(data.ptr()), PyBytes_Size(data.ptr()));
    }
    else if (py::isinstance<py::str>(data))
    {
        if (buftype!="" && buftype!="STRING")
        {
            throw std::invalid_argument("For string data "
                "expected STRING buftype, got: "+buftype);
        }

        std::string s = py::str(data);
        buf = atmibuf("STRING", s.size() + 1);
        strcpy(*buf.pp, s.c_str());
    }
    else if (!dict.contains(NDRXPY_DATA_DATA))
    {
        NDRX_LOG(log_debug, "Converting out NULL buffer");
        buf = atmibuf("NULL", 1024);
    }
    else if (ndrxpy_is_UbfDict(data))
    {
        NDRX_LOG(log_debug, "Converting out UBF/UbfDict...");

        ndrx_longptr_t ptr = data.attr("_buf").cast<py::int_>();
        atmibuf *data_buf = reinterpret_cast<atmibuf *>(ptr);

        if (reset_ptr)
        {
            buf.p = data_buf->p;
            ndrxpy_reset_ptr_UbfDict(data);
        }
        else
        {
            //If no reset, then let return buffer
            //not to free up, just use reference to UbfDict()
            buf.pp=data_buf->pp;
            buf.p = nullptr;
        }
    }
    else if (py::isinstance<py::dict>(data))
    {
        NDRX_LOG(log_debug, "Converting out UBF/pydict...");

        if (buftype!="" && buftype!="UBF")
        {
            NDRX_LOG(log_error, "For dict data "
                "expected UBF buftype, got [%s]", buftype.c_str());

            throw std::invalid_argument("For dict data "
                "expected UBF buftype, got: "+buftype);
        }
        buf = atmibuf("UBF", 1024);
        ndrxpy_from_py_ubf(static_cast<py::dict>(data), buf);
    }
    else
    {
        throw std::invalid_argument("Unsupported buffer type");
    }

    set_callinfo(dict, buf);

    return buf;
}


/* vim: set ts=4 sw=4 et smartindent: */
