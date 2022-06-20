/**
 * @brief Typed buffer to Python conversion, view type
 *
 * @file bufconv_view.cpp
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

/*---------------------------Externs------------------------------------*/
/*---------------------------Macros-------------------------------------*/
/*---------------------------Enums--------------------------------------*/
/*---------------------------Typedefs-----------------------------------*/
/*---------------------------Globals------------------------------------*/
/*---------------------------Statics------------------------------------*/
/*---------------------------Prototypes---------------------------------*/
namespace py = pybind11;

/**
 * @brief Covert VIEW buffer to python object
 * The output format is similar to UBF encoded in Python dictionary.
 * 
 * @param csturct C structure of the view
 * @param view view name
 * @param size size of the view buffer (used for temp storage)
 * @return py::object python dictionary.
 */
expublic py::object ndrxpy_to_py_view(char *cstruct, char *view, long size)
{
    /*throw std::invalid_argument("Not implemented");*/
    py::dict result;
    py::list val;
    int ret;
    Bvnext_state_t state;
    char cname[NDRX_VIEW_CNAME_LEN+1];
    int fldtype;
    BFLDOCC maxocc, fulloccs, occ;
    long dim_size;
    BFLDLEN len;
    int realoccs;
    bool first = true;
    /* allocate temporary buffer */
    tempbuf tmp(size);

    NDRX_LOG(log_debug, "To python view = [%s] size = [%ld]", view, size);

    while (1)
    {
        if (EXFAIL==(ret=Bvnext(&state, first?view:NULL, cname, 
                &fldtype, &maxocc, &dim_size)))
        {
            NDRX_LOG(log_error, "Failed to iterate VIEW [%s]: %s", view, Bstrerror(Berror));
            throw ubf_exception(Berror);

        }

        first = false;
        UBF_LOG(log_debug, "Converting view=[%s] cname=[%s]", view, cname);

        if (0==ret)
        {
            /* we are done */
            break;
        }

        /* Get real occurrences */
        if (EXFAIL==(fulloccs=Bvoccur(cstruct, view, cname, NULL, &realoccs, NULL, NULL)))
        {
            NDRX_LOG(log_error, "Failed to get view field %s.%s infos: %s", 
                    view, cname, Bstrerror(Berror));
            throw ubf_exception(Berror);
        }

        /* convert only initialized fields */
        for (occ=0; occ<realoccs; occ++)
        {
            if (occ == 0)
            {
                val = py::list();
                result[cname] = val;
            }
            
            /* read data according to the type... 
             * give it full buffer size
             */
            len = size;
            if (EXFAIL==CBvget(cstruct, view, cname, occ, tmp.buf, &len, fldtype, 0))
            {
                NDRX_LOG(log_error, "Failed to get view field %s.%s occ %d infos: %s", 
                        view, cname, occ, Bstrerror(Berror));
                throw ubf_exception(Berror);
            }

            switch (fldtype)
            {
            case BFLD_CHAR:
                /* if EOS char is used, convert to byte array.
                 * as it is possible to get this value from C
                 */
                if  (EXEOS==tmp.buf[0])
                {
                    val.append(py::bytes(tmp.buf, len));
                }
                else
                {
                    val.append(py::cast(tmp.buf[0]));
                }
                break;
            case BFLD_SHORT:
                val.append(py::cast(*reinterpret_cast<short *>(tmp.buf)));
                break;
            case BFLD_LONG:
                val.append(py::cast(*reinterpret_cast<long *>(tmp.buf)));
                break;
            case BFLD_FLOAT:
                val.append(py::cast(*reinterpret_cast<float *>(tmp.buf)));
                break;
            case BFLD_DOUBLE:
                val.append(py::cast(*reinterpret_cast<double *>(tmp.buf)));
                break;
            case BFLD_STRING:

                NDRX_LOG(log_dump, "Processing FLD_STRING...");
                val.append(
    #if PY_MAJOR_VERSION >= 3
                    py::str(tmp.buf)
                    //Seems like this one causes memory leak:
                    //Thus assume t
                    //py::str(PyUnicode_DecodeLocale(value.get(), "surrogateescape"))
    #else
                    py::bytes(tmp.buf, len - 1)
    #endif
                );
                break;
            case BFLD_CARRAY:
                val.append(py::bytes(tmp.buf, len));
                break;
            default:
                throw std::invalid_argument("Unsupported field type: " +
                                            std::to_string(fldtype));
            }
        }
    }
out:
    return result;
}

/**
 * @brief Process single view field
 * 
 * @param buf ATMI buffer where to unload the stuff
 * @param view view name
 * @param cname view field name
 * @param oc  occurrence to set
 * @param obj puthon dict key entry
 */
static void from_py1_view(atmibuf &buf, const char *view, const char *cname, BFLDOCC oc,
                     py::handle obj)
{

    NDRX_LOG(log_dump, "Processing %s.%s[%d]", view, cname, oc);
    if (obj.is_none())
    {

#if PY_MAJOR_VERSION >= 3
    }
    else if (py::isinstance<py::bytes>(obj))
    {
        std::string val(PyBytes_AsString(obj.ptr()), PyBytes_Size(obj.ptr()));

        //Set view field finally
        if (EXSUCCEED!=CBvchg(*buf.pp, const_cast<char *>(view), 
                const_cast<char *>(cname), oc, 
                const_cast<char *>(val.data()), val.size(), BFLD_CARRAY))
        {
            throw ubf_exception(Berror);    
        }

#endif
    }
    else if (py::isinstance<py::str>(obj))
    {
        char *ptr_val =NULL;
        BFLDLEN len;

#if PY_MAJOR_VERSION >= 3
       std::string val = "";
        py::bytes b = py::reinterpret_steal<py::bytes>(
            PyUnicode_EncodeLocale(obj.ptr(), "surrogateescape"));

        //If we get NULL ptr, then string contains null characters, and that is not supported
        //In case of string we get EOS
        //In case of single char field, will get NULL field.
        if (nullptr!=b.ptr())
        {
            val.assign(PyBytes_AsString(b.ptr()), PyBytes_Size(b.ptr()));
            ptr_val = const_cast<char *>(val.data());
            len = val.size();
        }
        else
        {
            PyErr_Print();
            char tmp[64];
            snprintf(tmp, sizeof(tmp), "Invalid string value probably contains 0x00 (len=%ld)",
                PyBytes_Size(obj.ptr()));
            throw std::invalid_argument(tmp);
        }

#else
        if (PyUnicode_Check(obj.ptr()))
        { 
            obj = PyUnicode_AsEncodedString(obj.ptr(), "utf-8", "surrogateescape");
        }
        std::string val(PyString_AsString(obj.ptr()), PyString_Size(obj.ptr()));

        ptr_val = const_cast<char *>(val.data());
        len = val.size();

#endif
        if (EXSUCCEED!=CBvchg(*buf.pp, const_cast<char *>(view), 
                    const_cast<char *>(cname), oc, ptr_val,
                    len, BFLD_CARRAY))
        {
            NDRX_LOG(log_error, "Failed to set view=[%s] cname=[%s] occ=%d: %s",
                view, cname, oc, Bstrerror(Berror));
            throw ubf_exception(Berror); 
        }
    }
    else if (py::isinstance<py::int_>(obj))
    {
        long val = obj.cast<py::int_>();

        if (EXSUCCEED!=CBvchg(*buf.pp, const_cast<char *>(view), 
                    const_cast<char *>(cname), oc, reinterpret_cast<char *>(&val), 0,
                                  BFLD_LONG))
        {
            NDRX_LOG(log_error, "Failed to set view=[%s] cname=[%s] occ=%d: %s",
                view, cname, oc, Bstrerror(Berror));
            throw ubf_exception(Berror); 
        }
    }
    else if (py::isinstance<py::float_>(obj))
    {
        double val = obj.cast<py::float_>();
         if (EXSUCCEED!=CBvchg(*buf.pp, const_cast<char *>(view), 
                    const_cast<char *>(cname), oc, reinterpret_cast<char *>(&val), 0,
                                  BFLD_DOUBLE))
        {
            NDRX_LOG(log_error, "Failed to set view=[%s] cname=[%s] occ=%d: %s",
                view, cname, oc, Bstrerror(Berror));
            throw ubf_exception(Berror); 
        }
    }
    else
    {
        NDRX_LOG(log_error, "Unsuported field type for view");
        throw std::invalid_argument("Unsupported type");
    }
}

/**
 * @brief Convert PY to VIEW
 * TODO: in case if operating view VIEW ocurrences in UBF, 
 * we might get NULL views. While these are valid from UBF perspective
 * for stand-alone perspective they are not valid.
 * 
 * @param obj 
 * @param b 
 * @param view VIEW name to process
 */
expublic void ndrxpy_from_py_view(py::dict obj, atmibuf &b, const char *view)
{

    NDRX_LOG(log_debug, "into ndrxpy_from_py_view() %p", b.pp);
    for (auto it : obj)
    {
        auto cname = std::string(py::str(it.first));
        py::handle o = it.second;
        if (py::isinstance<py::list>(o))
        {
            BFLDOCC oc = 0;
            for (auto e : o.cast<py::list>())
            {
                from_py1_view(b, view, cname.c_str(), oc++, e);
            }
        }
        else
        {
            // Handle single elements instead of lists for convenience
            from_py1_view(b, view, cname.c_str(), 0, o);
        }
    }

    NDRX_LOG(log_debug, "into ndrxpy_from_py_view() %p -> done", b.pp);
}

/* vim: set ts=4 sw=4 et smartindent: */
