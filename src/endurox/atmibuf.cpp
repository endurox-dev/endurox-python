/**
 * @brief Enduro/X Python module
 *
 * @file atmibuf.cpp
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

/*---------------------------Externs------------------------------------*/
/*---------------------------Macros-------------------------------------*/
/*---------------------------Enums--------------------------------------*/
/*---------------------------Typedefs-----------------------------------*/
/*---------------------------Globals------------------------------------*/
/*---------------------------Statics------------------------------------*/
/*---------------------------Prototypes---------------------------------*/

namespace py = pybind11;


atmibuf::atmibuf() : pp(&p), len(0), p(nullptr) {}

atmibuf::atmibuf(TPSVCINFO *svcinfo)
    : pp(&p), len(svcinfo->len), p(svcinfo->data) {}

/**
 * @brief Sub-type based allocation
 * 
 * @param type 
 * @param subtype 
 */
atmibuf::atmibuf(const char *type, const char *subtype) : pp(&p), p(nullptr)
{
    reinit(type, subtype, 1024);
}

atmibuf::atmibuf(const char *type, long len) : pp(&p), len(len), p(nullptr)
{
    reinit(type, nullptr, len);
}


/**
 * @brief Allocate / reallocate
 * 
 * @param type ATMI type
 * @param subtype ATMI sub-type
 * @param len_ len (where required)
 */
void atmibuf::reinit(const char *type, const char *subtype, long len_)
{    
    //Free up ptr if have any
    if (nullptr==*pp)
    {
        len = len_;
        *pp = tpalloc(const_cast<char *>(type), const_cast<char *>(subtype), len);
        //For null buffers we can accept NULL return
        if (*pp == nullptr && 0!=strcmp(type, "NULL"))
        {
            NDRX_LOG(log_error, "Failed to realloc: %s", tpstrerror(tperrno));
            throw atmi_exception(tperrno);
        }
    }
    else
    {
        /* always UBF? 
         * used for recursive buffer processing
         */
        UBFH *fbfr = reinterpret_cast<UBFH *>(*pp);
        Binit(fbfr, Bsizeof(fbfr));
    }
}

/* atmibuf::atmibuf(atmibuf &&other) : atmibuf() ? */
atmibuf::atmibuf(atmibuf &&other) : atmibuf()
{
    swap(other);
}

atmibuf &atmibuf::operator=(atmibuf &&other)
{
    swap(other);
    return *this;
}

extern "C" {
    void ndrx_mbuf_Bnext_ptr_first(UBFH *p_ub, Bnext_state_t *state);
}

/**
 * @brief Standard destructor
 * 
 */
atmibuf::~atmibuf()
{
    if (p != nullptr)
    {
        tpfree(p);
    }
}

/**
 * @release the buffer (do not destruct, as ATMI already freed)
 * 
 * @return char* 
 */
char *atmibuf::release()
{
    char *ret = p;

    //Disable destructor
    p = nullptr;
    return ret;
}

UBFH **atmibuf::fbfr() { return reinterpret_cast<UBFH **>(pp); }

/**
 * @brief resize the buffer if space is missing
 * 
 * @param f UBF buffer
 * @param loc location to reset in case if ptr changed
 */
void atmibuf::mutate(std::function<int(UBFH *)> f, Bfld_loc_info_t *loc)
{
    while (true)
    {
        int rc = f(*fbfr());
        if (rc == -1)
        {
            if (Berror == BNOSPACE)
            {
                len *= 2;
                char *prev_ptr =*pp; 
                *pp = tprealloc(*pp, len);

                //reset the loc as not valid anymore.
                if (NULL!=loc && prev_ptr!=*pp)
                {
                    memset(loc, 0, sizeof(*loc));
                }
            }
            else
            {
                throw ubf_exception(Berror);
            }
        }
        else
        {
            break;
        }
    }
}

void atmibuf::swap(atmibuf &other) noexcept
{
    std::swap(p, other.p);
    std::swap(len, other.len);

    //In case if using different pp
    if (&other.p!=other.pp)
    {
        std::swap(pp, other.pp);
    }
}

/* vim: set ts=4 sw=4 et smartindent: */

