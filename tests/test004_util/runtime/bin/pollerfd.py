#!/usr/bin/env python3

import sys, os, select, io
import endurox as e

outx = None
path = "/tmp/tmp_py"

class SomeClass:
    field=199

obj = SomeClass()

POLLIN = select.POLLIN

#
# se
#
def cb(fd, events, ptr1):
    global outx
    global path
    global obj
    global POLLIN
    #e.tplog_error("!!!got ev %d %d %d" % (events, fd, POLLIN))
    e.tpext_delpollerfd(outx)

    # shall get valid object back...
    assert ptr1.field == 199
    assert(ptr1==obj)

    # process all events of queue
    if (events & POLLIN) or ('kqueue' == e.ndrx_epoll_mode()):
        #
        # WARNING:
        # In SystemV mode there is no guarantee that pipe would be called
        # first. It might be POLLERSYNC service and only then pipe.
        # As at moment pipe might be already woken up (but message is not
        # yet read, but events to main thread have emitted),
        # the return from POLLERSYNC would send to event thread
        # syncfd=true, which would trigger again events on POLLIN of the pipe
        # in such race condition it is possible to receive twice the polled event
        # notification. The first would be read, the second would be EAGAIN or EWOULDBLOCK
        # if there is no message in pipe.
        #
        try:
            data = os.read(outx, 1) 
            e.tpbroadcast("", "", "python|Python", {"data":{"T_LONG_FLD":data[0]}}, e.TPREGEXMATCH|e.TPNOBLOCK)
        except io.BlockingIOError:
            e.tpext_addpollerfd(outx, POLLIN, obj, cb)
            return 0

    # not used.
    #if events & select.POLLHUP:
    #    os.close(outx)
    #    outx = os.open(path, os.O_NONBLOCK | os.O_RDWR)

    # add back...
    e.tpext_addpollerfd(outx, POLLIN, obj, cb)
    return 0

#
# first time poller init...
#
def b4poll():
    # add poller....
    global obj
    global outx
    global POLLIN
    e.tpext_addpollerfd(outx, POLLIN, obj, cb)
    e.tpext_delb4pollcb()
    return 0
#
# Test before poll callback
#
class Server:

    def tpsvrinit(self, args):
        global outx
        global path
        e.userlog('Server startup')
        path = "/tmp/tmp_py"
        os.remove(path) if os.path.exists(path) else None
        os.mkfifo( path, 0O644 )
        outx = os.open(path, os.O_NONBLOCK | os.O_RDWR)
        e.tpext_addb4pollcb(b4poll)
        e.tpadvertise('POLLERSYNC', 'POLLERSYNC', Server.POLLERSYNC)

        # configure OS specifics
        if 'kqueue' == e.ndrx_epoll_mode():
            global POLLIN
            # Enduro/X uses uint32 type for poller flags.
            POLLIN = select.KQ_FILTER_READ + 2**32
        return 0

    def tpsvrdone(self):
        global outx
        global path
        os.close(outx)
        os.remove(path) if os.path.exists(path) else None
        e.userlog('Server shutdown')

    def POLLERSYNC(self, args):
        return e.tpreturn(e.TPSUCCESS, 0, {})


if __name__ == '__main__':
    e.tprun(Server(), sys.argv)
