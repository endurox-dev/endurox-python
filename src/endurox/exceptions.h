/**
 * @brief Register exceptions
 *
 * @file exceptions.h
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
#include <pybind11/functional.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <nerror.h>

/*---------------------------Externs------------------------------------*/
/*---------------------------Macros-------------------------------------*/
/*---------------------------Enums--------------------------------------*/
/*---------------------------Typedefs-----------------------------------*/

namespace py = pybind11;

/**
 * ATMI Sub-systme exception
 */
struct atmi_exception : public std::exception
{
private:
    int code_;
    std::string message_;

protected:
    atmi_exception(int code, const std::string &message)
        : code_(code), message_(message) {}

public:
    explicit atmi_exception(int code)
        : code_(code), message_(tpstrerror(code)) {}

    const char *what() const noexcept override { return message_.c_str(); }
    int code() const noexcept { return code_; }
};

/**
 * TMQ persisten queue related exceptions.
 * returned from tpenqueue(), tpdequeue().
 */
struct qm_exception : public atmi_exception
{
public:
    explicit qm_exception(int code) : atmi_exception(code, qmstrerror(code)) {}
    explicit qm_exception(int code, char *msg) : atmi_exception(code, msg) {}

    static const char *qmstrerror(int code)
    {
        switch (code)
        {
        case QMEINVAL:
            return "An invalid flag value was specified.";
        case QMEBADRMID:
            return "An invalid resource manager identifier was specified.";
        case QMENOTOPEN:
            return "The resource manager is not currently open.";
        case QMETRAN:
            return "Transaction error.";
        case QMEBADMSGID:
            return "An invalid message identifier was specified.";
        case QMESYSTEM:
            return "A system error occurred. The exact nature of the error is "
                   "written to a log file.";
        case QMEOS:
            return "An operating system error occurred.";
        case QMEABORTED:
            return "The operation was aborted.";
        case QMEPROTO:
            return "An enqueue was done when the transaction state was not active.";
        case QMEBADQUEUE:
            return "An invalid or deleted queue name was specified.";
        case QMENOSPACE:
            return "Insufficient resources.";
        case QMERELEASE:
            return "Unsupported feature.";
        case QMESHARE:
            return "Queue is opened exclusively by another application.";
        case QMENOMSG:
            return "No message was available for dequeuing.";
        case QMEINUSE:
            return "Message is in use by another transaction.";
        default:
            return "?";
        }
    }
};

/**
 * UBF buffer related exception handling
 */
struct ubf_exception : public std::exception
{
private:
    int code_;
    std::string message_;

public:
    explicit ubf_exception(int code)
        : code_(code), message_(Bstrerror(code)) {}

    const char *what() const noexcept override { return message_.c_str(); }
    int code() const noexcept { return code_; }
};

struct nstd_exception : public std::exception
{
private:
    int code_;
    std::string message_;

public:
    explicit nstd_exception(int code)
        : code_(code), message_(Nstrerror(code)) {}

    const char *what() const noexcept override { return message_.c_str(); }
    int code() const noexcept { return code_; }
};


/*---------------------------Globals------------------------------------*/
/*---------------------------Statics------------------------------------*/
/*---------------------------Prototypes---------------------------------*/


/* vim: set ts=4 sw=4 et smartindent: */