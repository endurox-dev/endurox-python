/**
 * @brief Enduro/X Python module
 *
 * @file ndrx_pymod.h
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

#ifndef NDRX_PYMOD_H
#define NDRX_PYMOD_H

/*---------------------------Includes-----------------------------------*/
#include <atmi.h>
#include <tpadm.h>
#include <userlog.h>
#include <xa.h>
#include <ubf.h>
#include <ndebug.h>
#undef _

/*---------------------------Externs------------------------------------*/

extern __thread bool ndrxpy_G_ubfdict_enable; /**< Use UbfDict() by default */
extern __thread bool ndrxpy_G_ubfdict_delonset; /**< Use UbfDict() by default */

/*---------------------------Macros-------------------------------------*/
#define NDRXPY_DATA_DATA        "data"      /**< Actual data field          */
#define NDRXPY_DATA_BUFTYPE     "buftype"   /**< optional buffer type field */
#define NDRXPY_DATA_SUBTYPE     "subtype"   /**< subtype buffer             */
#define NDRXPY_DATA_CALLINFO    "callinfo"  /**< callinfo block             */


#define NDRXPY_DO_DFLT          0           /**< No init                    */
#define NDRXPY_DO_FREE          1           /**< free up buffer recursive   */
#define NDRXPY_DO_NEVERFREE     2           /**< never free up buffer , recu*/

#define NDRXPY_SUBBUF_NORM      0           /**< Normal XATMI buffer        */
#define NDRXPY_SUBBUF_UBF       1           /**< Embedded UBF               */
#define NDRXPY_SUBBUF_PTR       2           /**< This is PTR buffer         */

/*---------------------------Enums--------------------------------------*/
/*---------------------------Typedefs-----------------------------------*/

/**
 * @brief CLIENTID mirror in py
 */
struct pyclientid
{
    py::bytes pycltid;

    pyclientid(){}
    
    pyclientid(char *buf, long len)
    {
        pycltid = py::bytes(buf, len);
    }
};

/**
 * @brief TPTRANID mirror in py
 */
struct pytptranid
{
    py::bytes tranid;

    pytptranid(){}

    pytptranid(char *buf, long len)
    {
        tranid = py::bytes(buf, len);
    }
};


/**
 * @brief Extend the ATMI C struct with python specific fields
 */
struct pytpsvcinfo: TPSVCINFO
{
    //Override clientid as pyclientid
    pyclientid cltid;

    pytpsvcinfo(TPSVCINFO *inf)
    {
        NDRX_STRCPY_SAFE(name, inf->name);
        NDRX_STRCPY_SAFE(fname, inf->fname);
        len = inf->len;
        flags = inf->flags;
        cd = inf->cd;
        appkey = inf->appkey;
        cltid.pycltid = py::bytes(reinterpret_cast<char *>(&inf->cltid), sizeof(inf->cltid));
    }
    py::object data;
};

/**
 * ATMI buffer handling routines
 */
class atmibuf
{
public:
    atmibuf();
    atmibuf(TPSVCINFO *svcinfo);
    atmibuf(const char *type, long len);
    atmibuf(const char *type, const char *subtype);
    void reinit(const char *type, const char *subtype, long len_);

    atmibuf(const atmibuf &) = delete;
    atmibuf &operator=(const atmibuf &) = delete;
    
    atmibuf(atmibuf &&other);
    atmibuf &operator=(atmibuf &&other);
    ~atmibuf();

    char *release();
    UBFH **fbfr();
    char **pp;

    /**
     * @brief This is used to release the buffer
     *  in case if processing embedded views, this is set nullptr,
     */
    char *p;
    long len;

    /** used for UbfDict iteration */
    Bnext_state_t iter_state;
    /** field id used by iteration */
    BFLDID iter_fldid;
    
    void mutate(std::function<int(UBFH *)> f, Bfld_loc_info_t *loc);

private:
    void swap(atmibuf &other) noexcept;
};

/**
 * Temporary buffer allocator
 */
class tempbuf
{

public:

    char *buf;
    long size;
    tempbuf(long size)
    {
        buf = reinterpret_cast<char *>(NDRX_FPMALLOC(size, 0));
        this->size = size;

        if (NULL==buf)
        {
            throw std::bad_alloc();
        }

    }
    ~tempbuf()
    {
        NDRX_FPFREE(buf);
    }
};


typedef void *(xao_svc_ctx)(void *);

/**
 * Reply structore 
 */
struct pytpreply
{
    int pytperrno;
    long pytpurcode;
    py::object data;
    int cd;

    pytpreply(int pytperrno, long pytpurcode, py::object data, int cd = -1)
        : pytperrno(pytperrno), pytpurcode(pytpurcode), data(data), cd(cd) {}
};

/**
 * Reply structure with call descriptor
 */
struct pytpreplycd:pytpreply
{
    using pytpreply::pytpreply;
};

/**
 * @brief tpsend() return value
 */
struct pytpsendret
{
    int pytperrno;   /**< tperrno current (if not thrown) */
    long pytpurcode; /**< tpurcode value                  */
    long revent;    /**< any event published         */

    pytpsendret(int pytperrno, long pytpurcode, long revent)
        : pytperrno(pytperrno), pytpurcode(pytpurcode), revent(revent) {}

};

/**
 * @brief tprecv() return value
 */
struct pytprecvret:pytpsendret
{
    py::object data; /**< receive data                  */

    pytprecvret(int pytperrno, long pytpurcode, long revent, py::object data)
        : pytpsendret(pytperrno, pytpurcode, revent), data(data) {}
};

/**
 * @brief Atmi server context data type
 */
struct pytpsrvctxdata
{
    py::bytes pyctxt;

    pytpsrvctxdata(py::bytes &pyctxt): pyctxt(pyctxt) {}

    pytpsrvctxdata(char *buf, long len)
    {
        pyctxt = py::bytes(buf, len);
    }
};

/**
 * @brief extended struct to expose msgid and corrid 
 *  to Pybind11 as byte arrays (instead of strings).
 *  and provide interface for converting to the base
 *  struct.
 */
struct ndrxpy_tpqctl_t:tpqctl_t
{
    std::string replyqueue; /**< to have setter */
    std::string failurequeue;   /**< to have setter */
    py::bytes msgid;    /**< have a byte array for python mapping */
    py::bytes corrid;   /**< have a byte arrray for python mapping */

    /**
     * @brief Reset all to zero..
     */
    ndrxpy_tpqctl_t(void)
    {
        //Reset  base struct
        tpqctl_t* base = dynamic_cast<tpqctl_t*>(this);
        memset(base, 0, sizeof(tpqctl_t));
    }

    /**
     * @brief Load stuff to base class
     * 
     */
    void convert_to_base(void)
    {
        std::string msgid_val(PyBytes_AsString(msgid.ptr()), PyBytes_Size(msgid.ptr()));
        std::string corrid_val(PyBytes_AsString(corrid.ptr()), PyBytes_Size(corrid.ptr()));

        if (msgid_val.size() >= sizeof(tpqctl_t::msgid))
        {
            memcpy(tpqctl_t::msgid, msgid_val.data(), sizeof(tpqctl_t::msgid));
        }
        else
        {
            memset(tpqctl_t::msgid, 0, sizeof(tpqctl_t::msgid));
            memcpy(tpqctl_t::msgid, msgid_val.data(), msgid_val.size());
        }

        if (corrid_val.size() >= sizeof(tpqctl_t::corrid))
        {
            memcpy(tpqctl_t::corrid, corrid_val.data(), sizeof(tpqctl_t::corrid));
        }
        else
        {
            memset(tpqctl_t::corrid, 0, sizeof(tpqctl_t::corrid));
            memcpy(tpqctl_t::corrid, corrid_val.data(), corrid_val.size());
        }

        NDRX_STRCPY_SAFE(tpqctl_t::replyqueue, replyqueue.c_str());
        NDRX_STRCPY_SAFE(tpqctl_t::failurequeue, failurequeue.c_str());
    }
    
    /**
     * @brief Load stuff from base class to python bytes
     * 
     */
    void convert_from_base(void)
    {
        msgid = py::bytes(tpqctl_t::msgid, sizeof(tpqctl_t::msgid));
        corrid = py::bytes(tpqctl_t::corrid, sizeof(tpqctl_t::corrid));
        replyqueue = std::string(tpqctl_t::replyqueue);
        failurequeue = std::string(tpqctl_t::failurequeue);
    }
};

typedef struct ndrxpy_tpqctl_t NDRXPY_TPQCTL;


/**
 * @brief to Have a ptr to object
 *
 */
typedef struct
{
    py::object obj;
    
    /* secondary object */
    py::object obj2;

} ndrxpy_object_t;


/**
 * @brief Enduro/X debug handle
 */
struct pyndrxdebugptr
{
    pyndrxdebugptr(ndrx_debug_t * dbg)
    {
        ptr = reinterpret_cast<ndrx_longptr_t>(dbg);
    }
    ndrx_longptr_t ptr;
};

/**
 * @brief Tp context holder
 */
struct pytpcontext
{
    pytpcontext(TPCONTEXT_T *ctxt)
    {
        ctx_bytes = py::bytes(reinterpret_cast<char *>(ctxt), sizeof(TPCONTEXT_T));
    }

    /**
     * @brief Get current context in C format
     * 
     * @param ctxt C context
     */
    void getCtxt(TPCONTEXT_T *ctxt)
    {
        std::string val(PyBytes_AsString(ctx_bytes.ptr()), PyBytes_Size(ctx_bytes.ptr()));
        memcpy(reinterpret_cast<char *>(ctxt), 
            const_cast<char *>(val.data()), val.size());
    }
    py::bytes ctx_bytes;
};

/**
 * @brief tpgetctxt() return value
 */
struct pytpgetctxtret
{
    int pyret;   /**< Return value */
    pytpcontext pyctxt; /**< context value*/

    pytpgetctxtret(int pyret, pytpcontext pyctxt)
        : pyret(pyret), pyctxt(pyctxt) {}
};

/*---------------------------Globals------------------------------------*/
/*---------------------------Statics------------------------------------*/
/*---------------------------Prototypes---------------------------------*/

extern xao_svc_ctx *xao_svc_ctx_ptr;

extern atmibuf ndrx_from_py(py::object obj, bool reset_ptr);
extern py::object ndrx_to_py(atmibuf &buf, int is_sub_buffer);

//Buffer conversion support:
extern void ndrxpy_from_py_view(py::dict obj, atmibuf &b, const char *view);
extern py::object ndrxpy_to_py_view(char *cstruct, char *vname, long size);

extern bool ndrxpy_is_UbfDict(py::handle data);
extern bool ndrxpy_is_UbfDictFld(py::handle data);
extern bool ndrxpy_is_atmibuf_UbfDict(py::handle data);
extern void ndrxpy_reset_ptr_UbfDict(py::object data);
extern py::object ndrxpy_alloc_UbfDict(char *data, int is_sub_buffer, BFLDLEN buflen);
extern py::object ndrxpy_to_py_ubf(UBFH *fbfr, BFLDLEN buflen);
extern void ndrxpy_from_py_ubf(py::dict obj, atmibuf &b);

extern void pytpadvertise(std::string svcname, std::string funcname, const py::function &func);
extern void ndrxpy_pyrun(py::object svr, std::vector<std::string> args);

extern void ndrxpy_pytpreturn(int rval, long rcode, py::object data, long flags);
extern void ndrxpy_pytpforward(const std::string &svc, py::object data, long flags);

extern void ndrxpy_pytpunadvertise(const char * svcname);
extern pytpreply ndrxpy_pytpadmcall(py::object idata, long flags);
extern NDRXPY_TPQCTL ndrxpy_pytpenqueue(const char *qspace, const char *qname, NDRXPY_TPQCTL *ctl,
                          py::object data, long flags);
extern std::pair<NDRXPY_TPQCTL, py::object> ndrx_pytpdequeue(const char *qspace,
                                                 const char *qname, NDRXPY_TPQCTL *ctl,
                                                 long flags);
extern pytpreply ndrxpy_pytpcall(const char *svc, py::object idata, long flags);
extern int ndrxpy_pytpacall(const char *svc, py::object idata, long flags);

extern py::object ndrxpy_pytpexport(py::object idata, long flags);
extern py::object ndrxpy_pytpimport(const std::string istr, long flags);

extern pytpreplycd ndrxpy_pytpgetrply(int cd, long flags);
extern int ndrxpy_pytppost(const std::string eventname, py::object data, long flags);
extern long ndrxpy_pytpsubscribe(char *eventexpr, char *filter, TPEVCTL *ctl, long flags);

extern void ndrxpy_fdmap_clear(void);

extern void ndrxpy_register_atmi(py::module &m);
extern void ndrxpy_register_ubf(py::module &m);
extern void ndrxpy_register_srv(py::module &m);
extern void ndrxpy_register_util(py::module &m);
extern void ndrxpy_register_tpext(py::module &m);
extern void ndrxpy_register_tplog(py::module &m);
#endif /* NDRX_PYMOD.H */

/* vim: set ts=4 sw=4 et smartindent: */
