#!/usr/bin/env python3

#
# Server contexting, between different processes, read the state and reply
#

import sys
import endurox as e
from pathlib import Path


class Server:

    def tpsvrinit(self, args):
        e.userlog('Server startup')
        e.tpadvertise('SRVCTXP2', 'SRVCTXP2', self.SRVCTXP2)
        return 0

    def tpsvrdone(self):
        e.userlog('Server shutdown')

    # 
    # Generate response from the state file
    #
    def SRVCTXP2(self, args):
        
        # OK let the thread to finish the request.
        ctxt = e.tpsrvgetctxdata()

        data = Path('/tmp/binary.state').read_bytes()

        # restore the state
        state = e.PyTpSrvCtxtData(data)
        e.tpsrvsetctxdata(state)

        return e.tpreturn(e.TPSUCCESS, 0, args.data)
        

if __name__ == '__main__':
    e.run(Server(), sys.argv)

