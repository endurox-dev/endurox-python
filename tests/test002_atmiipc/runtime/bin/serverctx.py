#!/usr/bin/env python3

#
# Server contexting
#

import sys
import endurox as e
from threading import Thread


# thread func
def run(ctxt, data):
    e.tpinit()
    e.tpsrvsetctxdata(ctxt)
    e.tpreturn(e.TPSUCCESS, 0, data)
    e.tpterm();

class Server:

    def tpsvrinit(self, args):
        e.userlog('Server startup')
        e.tpadvertise('SRVCTX', 'SRVCTX', self.SRVCTX)
        return 0

    def tpsvrdone(self):
        e.userlog('Server shutdown')

    # 
    # capture the context & pass it to new thread
    #
    def SRVCTX(self, args):
        
        # OK let the thread to finish the request.
        ctxt = e.tpsrvgetctxdata()
        T=Thread(target=run,args=(ctxt,args.data))
        T.start()
        T.join()
        e.tpcontinue()

if __name__ == '__main__':
    e.run(Server(), sys.argv)

