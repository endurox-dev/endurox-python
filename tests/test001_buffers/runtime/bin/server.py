#!/usr/bin/env python3

import sys
import endurox as e

class Server:
     
    def tpsvrinit(self, args):
        e.userlog('Server startup')
        # cannot pass self.ECHO, as bound function objects are short lived.
        #e.tpadvertise('ECHO', 'ECHO', Server.ECHO)
        e.tpadvertise('ECHO')
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
    e.tprun(Server(), sys.argv)
