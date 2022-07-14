#!/usr/bin/env python3

import sys
import endurox as e

class Server:

    def tpsvrinit(self, args):
        e.userlog('Server startup')
        e.tpadvertise('RESTART', 'RESTART', Server.RESTART)
        return 0

    def tpsvrdone(self):
        e.userlog('Server shutdown')

    def RESTART(self, args):
        e.tpexit()
        return e.tpreturn(e.TPSUCCESS, 5, args.data)

if __name__ == '__main__':
    e.tprun(Server(), sys.argv)
