/**
 * @brief Enduro/X utility function exports
 *
 * @file endurox_util.cpp
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
#include <nstdutil.h>
#undef _

#include "exceptions.h"
#include "ndrx_pymod.h"

#include <pybind11/functional.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <functional>

namespace py = pybind11;

/*---------------------------Externs------------------------------------*/
/*---------------------------Macros-------------------------------------*/
/*---------------------------Enums--------------------------------------*/
/*---------------------------Typedefs-----------------------------------*/
/*---------------------------Globals------------------------------------*/
/*---------------------------Statics------------------------------------*/
/*---------------------------Prototypes---------------------------------*/

/**
 * @brief Register Enduro/X utility functions
 * 
 * @param m Pybind11 module handle
 */
expublic void ndrxpy_register_util(py::module &m)
{
    m.def(
        "ndrx_stdcfgstr_parse", [](const char *cfgstr)
        { 

            std::vector<std::vector<std::string>> ret;
            ndrx_stdcfgstr_t* parsed = NULL, *el;

            if (EXSUCCEED!=ndrx_stdcfgstr_parse(const_cast<char *>(cfgstr), &parsed))
            {
                throw std::runtime_error("ndrx_stdcfgstr_parse failed, see logs");
            }

            try
            {
                //Load the results
                for(el=parsed;el;el=el->next)
                {
                    ret.emplace_back();
                    ret.back().push_back(el->key);

                    if (NULL!=el->value)
                    {
                        ret.back().push_back(el->value);
                    }
                }
            }
            catch(const std::exception& e)
            {
                if (NULL!=parsed)
                {
                    ndrx_stdcfgstr_free(parsed);
                }
                throw e;
            }
            
            if (NULL!=parsed)
            {
                ndrx_stdcfgstr_free(parsed);
            }

            return ret;
        
        },
        R"pbdoc(
        Enduro/X standard configuration string parser.

        :raise runtime_error: 
            | Runtime error occurred (malloc failed).

        Parameters
        ----------
        cfgstr : str
            String containing the standard configuration starting such as: 
            setting1, setting2 setting3\tsetting=4\t,,,setting="HELLO", settingA="=HELLO",settingB"=CCC"

        Returns
        -------
        ret : list
            list of lists, in second list there is always element 0 present, the element 1 may not
            be present, as setting>=value< is optional.

            )pbdoc"
        , py::arg("cfgstr"));
}

/* vim: set ts=4 sw=4 et smartindent: */
