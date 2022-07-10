#!/usr/bin/env python3

#
# Server contexting, between different processes
#

import sys
import endurox as e

class Server:

    def tpsvrinit(self, args):
        e.userlog('Server startup')
        e.tpadvertise('SRVCTXP', 'SRVCTXP', self.SRVCTXP)
        return 0

    def tpsvrdone(self):
        e.userlog('Server shutdown')

    # 
    # capture the context, save to file, activate server2
    #
    def SRVCTXP(self, args):
        
        # OK let the thread to finish the request.
        ctxt = e.tpsrvgetctxdata()

        # Transfer the state to File
        f = open("/tmp/binary.state", "wb")
        f.write(ctxt.pyctxt)
        f.close()
        # activate the responder, it will read the state & restore
        e.tpacall("SRVCTXP2", args.data, e.TPNOREPLY)
        e.tpcontinue()

if __name__ == '__main__':
    e.tprun(Server(), sys.argv)

