/**
 * @brief Typed buffer to Python conversion, UBF type
 *
 * @file bufconv_ubf.cpp
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
/*---------------------------Prototypes---------------------------------*/
namespace py = pybind11;

/**
 * @brief Convert UBF buffer to python object
 * 
 * @param fbfr UBF buffer handler
 * @param buflen buffer len (opt)
 * @return py::object converted object
 */
expublic py::object ndrxpy_to_py_ubf(UBFH *fbfr, BFLDLEN buflen = 0)
{
    BFLDID fieldid = BFIRSTFLDID;
    Bnext_state_t state;
    BFLDOCC oc = 0;
    char *d_ptr;
    BVIEWFLD *p_vf;

    py::dict result;
    py::list val;

    NDRX_LOG(log_debug, "Into ndrxpy_to_py_ubf()");

    if (buflen == 0)
    {
        buflen = Bsizeof(fbfr);
    }
    /*std::unique_ptr<char[]> value(new char[buflen]); */

    //Bprint(fbfr);

    for (;;)
    {
        BFLDLEN len = buflen;
        //Seems in Enduro/X state is not associate with particular buffer
        //Looks like Tuxedo stores iteration state within buffer it self.
        int r = Bnext2(&state, fbfr, &fieldid, &oc, NULL, &len, &d_ptr);
        if (r == -1)
        {
            throw ubf_exception(Berror);
        }
        else if (r == 0)
        {
            break;
        }

        if (oc == 0)
        {
            val = py::list();

            char *name = Bfname(fieldid);
            if (name != nullptr)
            {
                result[name] = val;
            }
            else
            {
                result[py::int_(fieldid)] = val;
            }
        }

        switch (Bfldtype(fieldid))
        {
        case BFLD_CHAR:
            /* if EOS char is used, convert to byte array.
             * as it is possible to get this value from C
             */
            if  (EXEOS==d_ptr[0])
            {
                val.append(py::bytes(d_ptr, 1));
            }
            else
            {
                val.append(py::cast(d_ptr[0]));
            }            
            break;
        case BFLD_SHORT:
            val.append(py::cast(*reinterpret_cast<short *>(d_ptr)));
            break;
        case BFLD_LONG:
            val.append(py::cast(*reinterpret_cast<long *>(d_ptr)));
            break;
        case BFLD_FLOAT:
            val.append(py::cast(*reinterpret_cast<float *>(d_ptr)));
            break;
        case BFLD_DOUBLE:
            val.append(py::cast(*reinterpret_cast<double *>(d_ptr)));
            break;
        case BFLD_STRING:

            NDRX_LOG(log_dump, "Processing FLD_STRING... [%s]", d_ptr);
            val.append(
#if PY_MAJOR_VERSION >= 3
                py::str(d_ptr)
                //Seems like this one causes memory leak:
                //Thus assume t
                //py::str(PyUnicode_DecodeLocale(value.get(), "surrogateescape"))
#else
                py::bytes(d_ptr, len - 1)
#endif
            );
            break;
        case BFLD_CARRAY:
            val.append(py::bytes(d_ptr, len));
            break;
        case BFLD_UBF:
            val.append(ndrxpy_to_py_ubf(reinterpret_cast<UBFH *>(d_ptr), buflen));
            break;
        case BFLD_VIEW:
        {
            py::dict vdict;

            /* d_ptr points to BVIEWFIELD */
            p_vf = reinterpret_cast<BVIEWFLD *>(d_ptr);

            if (EXEOS!=p_vf->vname[0])
            {
                /* not empty occ */
                vdict["vname"] = p_vf->vname;
                vdict["data"]= ndrxpy_to_py_view(p_vf->data, p_vf->vname, len);
            }

            val.append(vdict);
            break;
        }
        case BFLD_PTR:
        {
            atmibuf ptrbuf;
            ptrbuf.p = nullptr;
            ptrbuf.pp = reinterpret_cast<char **>(d_ptr);

            /* process stuff recursively + free up leave buffers,
             * as we are not using them any more
             */
            val.append(ndrx_to_py(ptrbuf));
        }
        break;
        default:
            throw std::invalid_argument("Unsupported field " +
                                        std::to_string(fieldid));
        }
    }
    return result;
}

/**
 * @brief Build UBF buffer from PY dict
 * 
 * @param buf 
 * @param fieldid 
 * @param oc 
 * @param obj 
 * @param b temporary buffer
 */
static void from_py1_ubf(atmibuf &buf, BFLDID fieldid, BFLDOCC oc,
                     py::handle obj, atmibuf &b)
{
    if (obj.is_none())
    {

#if PY_MAJOR_VERSION >= 3
    }
    else if (py::isinstance<py::bytes>(obj))
    {
        std::string val(PyBytes_AsString(obj.ptr()), PyBytes_Size(obj.ptr()));

        buf.mutate([&](UBFH *fbfr)
                   { return CBchg(fbfr, fieldid, oc, const_cast<char *>(val.data()),
                                  val.size(), BFLD_CARRAY); });
#endif
    }
    else if (py::isinstance<py::str>(obj))
    {
        char *ptr_val =NULL;
        BFLDLEN len;

#if PY_MAJOR_VERSION >= 3
        py::bytes b = py::reinterpret_steal<py::bytes>(
            PyUnicode_EncodeLocale(obj.ptr(), "surrogateescape"));

        //If we get NULL ptr, then string contains null characters, and that is not supported
        //In case of string we get EOS
        //In case of single char field, will get NULL field.
        std::string val = "";

        if (nullptr!=b.ptr())
        {
            //TODO: well this goes out of the scope?
            val.assign(PyBytes_AsString(b.ptr()), PyBytes_Size(b.ptr()));
            ptr_val = const_cast<char *>(val.data());
            len = val.size();
        }
        else
        {
            PyErr_Print();
            char tmp[128];
            snprintf(tmp, sizeof(tmp), "Invalid string value probably contains 0x00 (len=%ld), field=%d",
                PyBytes_Size(obj.ptr()), fieldid);
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
        buf.mutate([&](UBFH *fbfr)
                   { return CBchg(fbfr, fieldid, oc, ptr_val, len, BFLD_CARRAY); });
    }
    else if (py::isinstance<py::int_>(obj))
    {
        long val = obj.cast<py::int_>();
        buf.mutate([&](UBFH *fbfr)
                   { return CBchg(fbfr, fieldid, oc, reinterpret_cast<char *>(&val), 0,
                                  BFLD_LONG); });
    }
    else if (py::isinstance<py::float_>(obj))
    {
        double val = obj.cast<py::float_>();
        buf.mutate([&](UBFH *fbfr)
                   { return CBchg(fbfr, fieldid, oc, reinterpret_cast<char *>(&val), 0,
                                  BFLD_DOUBLE); });
    }
    else if (py::isinstance<py::dict>(obj))
    {
        if (BFLD_UBF==Bfldtype(fieldid))
        {
            ndrxpy_from_py_ubf(obj.cast<py::dict>(), b);
            buf.mutate([&](UBFH *fbfr)
                    { return Bchg(fbfr, fieldid, oc, reinterpret_cast<char *>(*b.fbfr()), 0); });
        }
        else if (BFLD_VIEW==Bfldtype(fieldid))
        {
            /*
             * Syntax: data: { "VIEW_FIELD":{"vname":"VIEW_NAME", "data":{}} } 
             */
            auto view_d = obj.cast<py::dict>();

            auto have_vnamed = view_d.contains("vname");
            auto have_data = view_d.contains("data");
            BVIEWFLD vf;
            memset(&vf, 0, sizeof(vf));

            if (have_vnamed && !have_data)
            {
                //Invalid condition, but must be present
                UBF_LOG(log_debug, "Failed to convert view field %d: vname present but no data", fieldid);
                throw std::invalid_argument("vname present but no data");
            }
            else if (!have_vnamed && have_data)
            {
                //Invalid condition
               UBF_LOG(log_debug, "Failed to convert view field %d: data present but no vname", fieldid);
                throw std::invalid_argument("data present but no vname");
            }
            else if (!have_vnamed && !have_data)
            {
                //put empty..
            }
            else
            {
                auto vnamed = view_d["vname"];
                auto vdata = view_d["data"];
                std::string vname = py::str(vnamed);

                NDRX_STRCPY_SAFE(vf.vname, vname.c_str());
                atmibuf vbuf("VIEW", vf.vname);

                vf.data = *vbuf.pp;
                vf.vflags=0;

                ndrxpy_from_py_view(vdata.cast<py::dict>(), vbuf, vf.vname);

                buf.mutate([&](UBFH *fbfr)
                    { return Bchg(fbfr, fieldid, oc, reinterpret_cast<char *>(&vf), 0); });
            }
        }
        else if (BFLD_PTR==Bfldtype(fieldid))
        {
            if (!py::isinstance<py::dict>(obj))
            {
                char tmp[128];
                snprintf(tmp, sizeof(tmp), "Field id=%d is PTR, expected dictionary, but is not", 
                        fieldid);
                NDRX_LOG(log_error, "%s", tmp);
                throw std::invalid_argument(tmp);
            }
            atmibuf tmp = ndrx_from_py(obj.cast<py::object>());
            
            buf.mutate([&](UBFH *fbfr)
                    { return Bchg(fbfr, fieldid, oc, reinterpret_cast<char *>(tmp.pp), 0); });

            //Do not remove this buffer... as needed by mbuf
            tmp.p = nullptr;

        }
    }
    else
    {
        throw std::invalid_argument("Unsupported type");
    }
}

/**
 * @brief Convert PY to UBF
 * 
 * @param obj 
 * @param b 
 */
expublic void ndrxpy_from_py_ubf(py::dict obj, atmibuf &b)
{
    b.reinit("UBF", nullptr, 1024);
    atmibuf f;

    for (auto it : obj)
    {
        BFLDID fieldid;
        if (py::isinstance<py::int_>(it.first))
        {
            fieldid = it.first.cast<py::int_>();
        }
        else
        {
            fieldid =
                Bfldid(const_cast<char *>(std::string(py::str(it.first)).c_str()));
        }

        py::handle o = it.second;
        if (py::isinstance<py::list>(o))
        {
            BFLDOCC oc = 0;
            for (auto e : o.cast<py::list>())
            {
                from_py1_ubf(b, fieldid, oc++, e, f);
            }
        }
        else
        {
            // Handle single elements instead of lists for convenience
            from_py1_ubf(b, fieldid, 0, o, f);
        }
    }

    //Bprint(*b.fbfr());
}

/**
 * @brief Register UBF specific functions
 * 
 * @param m Pybind11 module handle
 */
expublic void ndrxpy_register_ubf(py::module &m)
{
    m.def(
        "Bfldtype", [](BFLDID fieldid)
        { return Bfldtype(fieldid); },
        R"pbdoc(
        Return UBF field type by given field id.
            
        For more details see **Bfldtype(3)** C API call.

        Parameters
        ----------
        fieldid : int
            Compiled UBF field id.

        Returns
        -------
        type : int
            One of :data:`.BFLD_SHORT`, :data:`.BFLD_LONG`, :data:`.BFLD_FLOAT`
            :data:`.BFLD_DOUBLE`, :data:`.BFLD_STRING`, :data:`.BFLD_CARRAY`,
            :data:`.BFLD_PTR`, :data:`.BFLD_UBF`, :data:`.BFLD_VIEW`

            )pbdoc"
        , py::arg("fieldid"));
    m.def(
        "Bfldno", [](BFLDID fieldid)
        { return Bfldno(fieldid); },
        R"pbdoc(
        Return field number from compiled field id.
            
        For more details see **Bfldno(3)** C API call.

        Parameters
        ----------
        fieldid : int
            Compiled field id.

        Returns
        -------
        type : int
            Field number as defined in FD file.

            )pbdoc", py::arg("fieldid"));
    m.def(
        "Bmkfldid", [](int type, BFLDID num)
        { return Bmkfldid(type, num); },
        R"pbdoc(
        Create field identifier (compiled id) from field type and
        field number.
            
        For more details see **Bmkfldid(3)** C API call.

        Parameters
        ----------
        type : type
            | Field type, one of: :data:`.BFLD_CHAR`,
            | :data:`.BFLD_SHORT`, :data:`.BFLD_LONG`, :data:`.BFLD_FLOAT`
            | :data:`.BFLD_DOUBLE`, :data:`.BFLD_STRING`, :data:`.BFLD_CARRAY`,
            | :data:`.BFLD_PTR`, :data:`.BFLD_UBF`, :data:`.BFLD_VIEW`
        num : int
            Field number.

        Returns
        -------
        fldid : int
            Compiled field id.

            )pbdoc"
        , py::arg("type"), py::arg("num"));

    m.def(
        "Bfname",
        [](BFLDID fieldid)
        {
            auto *name = Bfname(fieldid);
            if (name == nullptr)
            {
                throw ubf_exception(Berror);
            }
            return name;
        },
        R"pbdoc(
        Get field name from compiled field id.
            
        For more details see **Bfname(3)** C API call.

        :raise UbfException: 
            | Following error codes may be present:
            | :data:`.BBADFLD` - Field not found in FD files or UBFDB.
            | :data:`.BFTOPEN` - Unable to open field tables.

        Parameters
        ----------
        fieldid : int
            Field number.

        Returns
        -------
        fldnm : str
            Field name.

            )pbdoc"
        , py::arg("fieldid"));
    m.def(
        "Bfldid",
        [](const char *name)
        {
            auto id = Bfldid(const_cast<char *>(name));
            if (id == BBADFLDID)
            {
                throw ubf_exception(Berror);
            }
            return id;
        },
        R"pbdoc(
        Get field name from compiled field id.
            
        For more details see **Bfldid(3)** C API call.

        :raise UbfException: 
            | Following error codes may be present:
            | :data:`.BBADNAME` - Field not found in FD files or UBFDB.
            | :data:`.BFTOPEN` - Unable to open field tables.

        Parameters
        ----------
        fieldid : int
            | Field number.

        Returns
        -------
        fldnm : str
            Field name.

            )pbdoc"
        , py::arg("name"));

    m.def(
        "Bboolpr",
        [](const char *expression, py::object iop)
        {
            std::unique_ptr<char, decltype(&Btreefree)> guard(
                Bboolco(const_cast<char *>(expression)), &Btreefree);
            if (guard.get() == nullptr)
            {
                throw ubf_exception(Berror);
            }

            int fd = iop.attr("fileno")().cast<py::int_>();
            std::unique_ptr<FILE, decltype(&fclose)> fiop(fdopen(dup(fd), "w"),
                                                          &fclose);
            Bboolpr(guard.get(), fiop.get());
        },
        R"pbdoc(
        Compile and print boolean expression to file descriptor.

        .. code-block:: python
            :caption: Bboolpr example
            :name: Bboolpr-example

                import endurox
                
                f = open("output.txt", "w")
                endurox.Bboolpr("(1==1) && T_FLD_STRING=='ABC'", f)
                f.close()
            
        For more details see **Bboolpr(3)** C API call.

        :raise UbfException: 
            | Following error codes may be present:
            | :data:`.BBADNAME` - Field not found in FD files or UBFDB.
            | :data:`.BSYNTAX` - Bad boolean expression syntax
            | :data:`.BFTOPEN` - Unable to open field tables.

        Parameters
        ----------
        expression : str
            Enduro/X UBF boolean expression (full text)
        iop : file
            Output file (shall be in write mode)

            )pbdoc",
            py::arg("expression"), py::arg("iop"));

    m.def(
        "Bboolev",
        [](py::object fbfr, const char *expression)
        {
            std::unique_ptr<char, decltype(&Btreefree)> guard(
                Bboolco(const_cast<char *>(expression)), &Btreefree);
            if (guard.get() == nullptr)
            {
                throw ubf_exception(Berror);
            }
            auto buf = ndrx_from_py(fbfr);
            auto rc = Bboolev(*buf.fbfr(), guard.get());
            if (rc == -1)
            {
                throw ubf_exception(Berror);
            }
            return rc == 1;
        },
        R"pbdoc(
        Evaluate Boolean expression on given UBF buffer.

        .. code-block:: python
            :caption: Bboolev example
            :name: Bboolev-example

                import endurox
                
                print(endurox.Bboolev({"data":{"T_STRING_FLD":"ABC"}}, "T_STRING_FLD=='ABC'"))
                # will print True
            
        For more details see **Bboolev(3)** C API call.

        :raise UbfException: 
            | Following error codes may be present:
            | :data:`.BALIGNERR` - Corrupted UBF buffer.
            | :data:`.BNOTFLD` - Invalid ATMI buffer format, not UBF.
            | :data:`.BEINVAL` - Invalid arguments passed.
            | :data:`.BBADNAME` - Field not found in FD files or UBFDB.
            | :data:`.BSYNTAX` - Bad boolean expression syntax
            | :data:`.BFTOPEN` - Unable to open field tables.
            | :data:`.BEBADOP` - Operation not supported on given field types.

        Parameters
        ----------
        fbfr : dict
            ATMI buffer on which to test the expression
        expression : str
            Boolean expression

        Returns
        -------
        ret : bool
            Result true (matches) or false (buffer not matches expression).

            )pbdoc", py::arg("fbfr"),
        py::arg("expression"));

    m.def(
        "Bfloatev",
        [](py::object fbfr, const char *expression)
        {
            std::unique_ptr<char, decltype(&Btreefree)> guard(
                Bboolco(const_cast<char *>(expression)), &Btreefree);
            if (guard.get() == nullptr)
            {
                throw ubf_exception(Berror);
            }
            auto buf = ndrx_from_py(fbfr);
            auto rc = Bfloatev(*buf.fbfr(), guard.get());
            if (rc == -1)
            {
                throw ubf_exception(Berror);
            }
            return rc;
        },
        R"pbdoc(
        Evaluate Boolean expression as float number on given UBF buffer.

        .. code-block:: python
            :caption: Bfloatev example
            :name: Bfloatev-example

                import endurox
                
                print(endurox.Bfloatev({"data":{"T_STRING_FLD":"99", "T_SHORT_FLD":2}}, "T_STRING_FLD-T_SHORT_FLD"))
                # will print 97.0
            
        For more details see **Bfloatev(3)** C API call.

        :raise UbfException: 
            | Following error codes may be present:
            | :data:`.BALIGNERR` - Corrupted UBF buffer.
            | :data:`.BNOTFLD` - Invalid ATMI buffer format, not UBF.
            | :data:`.BEINVAL` - Invalid arguments passed.
            | :data:`.BBADNAME` - Field not found in FD files or UBFDB.
            | :data:`.BSYNTAX` - Bad boolean expression syntax
            | :data:`.BFTOPEN` - Unable to open field tables.
            | :data:`.BEBADOP` - Operation not supported on given field types.

        Parameters
        ----------
        fbfr : dict
            ATMI buffer on which to test the expression
        expression : str
            Boolean expression

        Returns
        -------
        ret : float
            Returns result as float.

            )pbdoc", py::arg("fbfr"),
        py::arg("expression"));

    m.def(
        "Bfprint",
        [](py::object fbfr, py::object iop)
        {
            auto buf = ndrx_from_py(fbfr);
            int fd = iop.attr("fileno")().cast<py::int_>();
            std::unique_ptr<FILE, decltype(&fclose)> fiop(fdopen(dup(fd), "w"),
                                                          &fclose);
            auto rc = Bfprint(*buf.fbfr(), fiop.get());
            if (rc == -1)
            {
                throw ubf_exception(Berror);
            }
        },
        R"pbdoc(
        Print UBF buffer to specified stream.
            
        For more details see **Bfprint(3)** C API call.

        :raise UbfException: 
            | Following error codes may be present:
            | :data:`.BALIGNERR` - Invalid ATMI buffer format, not UBF.
            | :data:`.BNOTFLD` - Not UBF buffer.

        Parameters
        ----------
        fbfr : dict
            ATMI buffer on which to test the expression.
        iop : file
            Output stream in write mode.
            )pbdoc", py::arg("fbfr"),
        py::arg("iop"));

    m.def(
        "Bprint",
        [](py::object fbfr)
        {
            auto buf = ndrx_from_py(fbfr);
            auto rc = Bprint(*buf.fbfr());
            if (rc == -1)
            {
                throw ubf_exception(Berror);
            }
        },
        R"pbdoc(
        Print UBF buffer to stdout.
            
        For more details see **Bprint(3)** C API call.

        :raise UbfException: 
            | Following error codes may be present:
            | :data:`.BALIGNERR` - Invalid ATMI buffer format, not UBF.
            | :data:`.BNOTFLD` - Not UBF buffer.

        Parameters
        ----------
        fbfr : dict
            ATMI buffer on which to test the expression

            )pbdoc", py::arg("fbfr"));
    m.def(
        "Bextread",
        [](py::object iop)
        {
            atmibuf obuf("UBF", 1024);
            int fd = iop.attr("fileno")().cast<py::int_>();
            std::unique_ptr<FILE, decltype(&fclose)> fiop(fdopen(dup(fd), "r"),
                                                          &fclose);

            obuf.mutate([&](UBFH *fbfr)
                        { return Bextread(fbfr, fiop.get()); });
            return ndrx_to_py(obuf);
        },
        R"pbdoc(
        Restore ATMI UBF buffer from :func:`.Bfprint` output.
            
        For more details see **Bextread(3)** C API call.

        :raise UbfException: 
            | Following error codes may be present:
            | :data:`.BALIGNERR` - Corrupted output buffer (internal error).
            | :data:`.BNOTFLD` - Corrupted output buffer (internal error).
            | :data:`.BEINVAL` - Invalid input file.
            | :data:`.BSYNTAX` - Input file syntax error.
            | :data:`.BBADNAME` - Field not found in UBF field tables.
            | :data:`.BBADVIEW` - View not found.

        Parameters
        ----------
        iop : file
            Input stream in read mode.

        Returns
        -------
        ret : dict
            Restored UBF buffer.
            )pbdoc", py::arg("iop"));
}

/* vim: set ts=4 sw=4 et smartindent: */
