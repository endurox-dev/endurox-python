#!/usr/bin/env python3

import sys, os, select
import endurox as e

outx = None
path = "/tmp/tmp_py"

class SomeClass:
    field=199

obj = SomeClass()

#
# se
#
def cb(fd, events, ptr1):
    global outx
    global path
    global obj
#    e.tplog_error("got ev %d" % events)
    e.tpext_delpollerfd(outx)

    # shall get valid object back...
    assert ptr1.field == 199
    assert(ptr1==obj)

    if events & select.POLLIN:
        data = os.read(outx, 1) 
        e.tpbroadcast("", "", "python", {"data":{"T_LONG_FLD":data[0]}}, e.TPREGEXMATCH|e.TPNOBLOCK)

    if events & select.POLLHUP:
        os.close(outx)
        outx = os.open(path, os.O_NONBLOCK | os.O_RDWR)

    # add back...
    e.tpext_addpollerfd(outx, select.POLLIN, obj, cb)
    return 0

#
# first time poller init...
#
def b4poll():
    # add poller....
    global obj
    e.tpext_addpollerfd(outx, select.POLLIN, obj, cb)
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
        e.tpadvertise('POLLERSYNC', 'POLLERSYNC', self.POLLERSYNC)
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
    e.run(Server(), sys.argv)
