#!/usr/bin/env python3

import sys
import endurox as e

import random
def randbytes(n):
    for _ in range(n):
        yield random.getrandbits(8)

class Server:
     
    def tpsvrinit(self, args):
        e.userlog('Server startup')
        # cannot pass self.ECHO, as bound function objects are short lived.
        #e.tpadvertise('ECHO', 'ECHO', Server.ECHO)
        e.tpadvertise('ECHO')
        e.tpadvertise('REALLOC')
        return 0

    def tpsvrdone(self):
        e.userlog('Server shutdown')

    # 
    # return the same request data back
    #
    def ECHO(self, args):
        #print(args.data)
        return e.tpreturn(e.TPSUCCESS, 0, args.data)

    #
    # Add some big data to UBF buffer
    #
    def REALLOC(self, args):
        args.data["data"].T_CARRAY_FLD = bytearray(randbytes(56000))
        return e.tpreturn(e.TPSUCCESS, 0, args.data)

if __name__ == '__main__':
    e.tprun(Server(), sys.argv)
