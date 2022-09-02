/**
 * @brief Typed buffer to Python conversion, UBF type
 *
 * @file bufconv_ubf.cpp
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
 * @param loc last position for fast add operation
 */
static void from_py1_ubf(atmibuf &buf, BFLDID fieldid, BFLDOCC oc,
                     py::handle obj, atmibuf &b, Bfld_loc_info_t *loc)
{
    if (obj.is_none())
    {

#if PY_MAJOR_VERSION >= 3
    }
    else if (py::isinstance<py::bytes>(obj))
    {
        std::string val(PyBytes_AsString(obj.ptr()), PyBytes_Size(obj.ptr()));

        buf.mutate([&](UBFH *fbfr)
                   { return CBaddfast(fbfr, fieldid, const_cast<char *>(val.data()),
                                  val.size(), BFLD_CARRAY, loc); }, loc);
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
                   { return CBaddfast(fbfr, fieldid, ptr_val, len, BFLD_CARRAY, loc); }, loc);
    }
    else if (py::isinstance<py::int_>(obj))
    {
        long val = obj.cast<py::int_>();
        buf.mutate([&](UBFH *fbfr)
                   { return CBaddfast(fbfr, fieldid, reinterpret_cast<char *>(&val), 0,
                                  BFLD_LONG, loc); }, loc);
    }
    else if (py::isinstance<py::float_>(obj))
    {
        double val = obj.cast<py::float_>();
        buf.mutate([&](UBFH *fbfr)
                   { return CBaddfast(fbfr, fieldid, reinterpret_cast<char *>(&val), 0,
                                  BFLD_DOUBLE, loc); }, loc);
    }
    else if (py::isinstance<py::dict>(obj))
    {
        if (BFLD_UBF==Bfldtype(fieldid))
        {
            ndrxpy_from_py_ubf(obj.cast<py::dict>(), b);
            buf.mutate([&](UBFH *fbfr)
                    { return Baddfast(fbfr, fieldid, reinterpret_cast<char *>(*b.fbfr()), 0, loc); }, loc);
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
                memset(&vf, 0, sizeof(vf));
                buf.mutate([&](UBFH *fbfr)
                    { return Baddfast(fbfr, fieldid, reinterpret_cast<char *>(&vf), 0, loc); }, loc);
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
                    { return Baddfast(fbfr, fieldid, reinterpret_cast<char *>(&vf), 0, loc); }, loc);
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
                    { return Baddfast(fbfr, fieldid, reinterpret_cast<char *>(tmp.pp), 0, loc); }, loc);

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
 * Resolve field id.
 */
exprivate BFLDID ndrxpy_fldid_resolve(py::object fld)
{
	BFLDID fieldid;
	if (py::isinstance<py::int_>(fld))
	{
		fieldid = fld.cast<py::int_>();

		if (fieldid<=BBADFLDID)
		{
			NDRX_LOG(log_error, "Invalid field id %d", fieldid);
			throw ubf_exception(BBADFLD);
		}
	}
	else
	{
		std::string s(py::str(fld));
		char *fldstr = const_cast<char *>(s.c_str());
		fieldid = Bfldid(fldstr);

		if (BBADFLDID==fieldid)
		{
			NDRX_LOG(log_error, "Failed to resolve field [%s]: %s", fldstr, Bstrerror(Berror));
			throw ubf_exception(Berror);
		}
	}
	
	return fieldid;
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
    BFLDID fieldid_prev = EXFAIL;
    Bfld_loc_info_t loc;
    memset(&loc, 0, sizeof(loc));
    BFLDID max_seen = EXFAIL;

    for (auto it : obj)
    {
        BFLDID fieldid = ndrxpy_fldid_resolve(it.first);

        /* check the location optimizations.. 
         * or if there was re-alloc
         * => reset
         * We could keep re-using the loc if next field is higher
         * than max ever seen, that would mean that we are still at the
         * end of the buffer. As Enduro/X print the buffers in such order
         * the un-modified buffers could get benefit from this.
         */
        if (fieldid!=loc.last_Baddfast)
        {
            if (max_seen==loc.last_Baddfast && fieldid > max_seen)
            {
                /* keep the ptr valid, as next id field follows */
            }
            else
            {
                /* ptr not valid anymore... */
                memset(&loc, 0, sizeof(loc));
            }
        }

        if (fieldid>max_seen)
        {
            max_seen = fieldid;
        }

        py::handle o = it.second;
        if (py::isinstance<py::list>(o))
        {
            BFLDOCC oc = 0;
            
            for (auto e : o.cast<py::list>())
            {
                from_py1_ubf(b, fieldid, oc++, e, f, &loc);
            }
        }
        else
        {
            // Handle single elements instead of lists for convenience
            from_py1_ubf(b, fieldid, 0, o, f, &loc);
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
                        { return Bextread(fbfr, fiop.get());}, NULL);
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

    m.def(
        "tpalloc",
        [](const char *buf_type, const char *sub_type, long size)
        {
            auto buf = new atmibuf(buf_type, size);
            ndrx_longptr_t ptr = reinterpret_cast<ndrx_longptr_t>(buf);
            return ptr;
        },
        R"pbdoc(
        Allocate XATMI buffer.
            
        For more details see **tpalloc(3)** C API call.

        :raise AtmiException: 
            | Following error codes may be present:
            | :data:`.TPEINVAL` - Invalid arguments passed to function.
            | :data:`.TPEOTYPE` - Invalid type specified.
            | :data:`.TPESYSTEM` - System error occurred.
            | :data:`.TPEOS` - Operating system error occurred, e.g. out of memory.

        Parameters
        ----------
        buf_type : str
            XATMI buffer type.
        sub_type : str
            XATMI buffer sub type.
        size : int
            Number of bytes for buffer to allocate.

        Returns
        -------
        ret : ptr
            C pointer to XATMI buffer object.
            )pbdoc", py::arg("buf_type"), py::arg("sub_type"), py::arg("size"));
    
    m.def(
        "tpfree",
        [](ndrx_longptr_t ptr)
        {
	        atmibuf *buf = reinterpret_cast<atmibuf *>(ptr);
            delete buf;
        },
        R"pbdoc(
        Free XATMI buffer.

        Parameters
        ----------
        ptr: int
            C pointer to XATMI buffer object

        )pbdoc", py::arg("ptr"));


    /* dict = UbfDict({"SOME_FIELD":999, "SOME_OTHER_FIELD":{"A_FLD":["A", "B", "C"]}})*/
    m.def(
        "Bload",
        [](ndrx_longptr_t ptr, py::object data)
        {
            atmibuf *buf = reinterpret_cast<atmibuf *>(ptr);
            ndrxpy_from_py_ubf(data, *buf);
        },
        R"pbdoc(
        Load UBF buffer from dictionary into ATMI buffer

        Parameters
        ----------
        ptr: int
            C pointer to buffer
        data: dict
            Initialization source (dictionary)

        )pbdoc", py::arg("ptr"), py::arg("data"));
    
    m.def(
        "Bchg",
        [](ndrx_longptr_t ptr, py::object fldid, py::object data)
        {
		
		    atmibuf *buf = reinterpret_cast<atmibuf *>(ptr);
		    BFLDID fieldid = ndrxpy_fldid_resolve(it.first);
		
		    if (py::isinstance<py::list>(data))
		    {
			    BFLDOCC oc = 0;
			
			    for (auto e : data.cast<py::list>())
			    {
				    from_py1_ubf(*buf, fieldid, oc++, e, f, &loc);
			    }
		    }
		    else
		    {
			    // Handle single elements instead of lists for convenience
			    from_py1_ubf(*buf, fieldid, 0, o, f, &loc);
		    }
        },
        R"pbdoc(
        Load value into UBF buffer field

        Parameters
        ----------
        ptr: int
            C pointer to buffer
        fldid: object
            String or integer field id.
	data: object
            Data to load (list or single value)
        )pbdoc", py::arg("ptr"), py::arg("data"));

/*_Bchg - change whole filed or single occ, may take list, _Bget() - return list, _Bdel fullkey or list entry, _Bnext (transfer to py), _Bloadkw() */

}

/* vim: set ts=4 sw=4 et smartindent: */
