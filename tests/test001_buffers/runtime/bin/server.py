#!/usr/bin/env python3

import sys
import endurox as e

class Server:

    def tpsvrinit(self, args):
        e.userlog('Server startup')
        e.tpadvertise('ECHO', 'ECHO', self.ECHO)
        return 0

    def tpsvrdone(self):
        e.userlog('Server shutdown')

    # 
    # return the same request data back
    #
    def ECHO(self, args):
        #print(args.data)
        return e.tpreturn(e.TPSUCCESS, 0, args.data)

if __name__ == '__main__':
    e.run(Server(), sys.argv)
