#!/usr/bin/env python3

import sys
import endurox as e

# counter of b4polls
M_cnt = 0

def period():
    global M_cnt
    M_cnt+=1
    return 0

def period2():
    global M_cnt
    M_cnt+=1
    return -1

#
# Test periodic server callback func.
#
class Server:

    def tpsvrinit(self, args):
        e.userlog('Server startup')
        e.tpext_addperiodcb(1, period)
        e.tpadvertise('PERIOD_OFF', 'PERIOD_OFF', self.PERIOD_OFF)
        e.tpadvertise('PERIOD_ON2', 'PERIOD_ON2', self.PERIOD_ON2)
        e.tpadvertise('PERIODSTATS', 'PERIODSTATS', self.PERIODSTATS)
        return 0

    def tpsvrdone(self):
        e.userlog('Server shutdown')

    def PERIOD_OFF(self, args):
        e.tpext_delperiodcb()
        return e.tpreturn(e.TPSUCCESS, 0, {})

    # this makes server to reboot.
    def PERIOD_ON2(self, args):
        e.tpext_addperiodcb(1, period2)
        return e.tpreturn(e.TPSUCCESS, 0, {})

    # get current statistics...
    def PERIODSTATS(self, args):
        global M_cnt
        retbuf = {"data":{"T_SHORT_FLD":M_cnt}}
        return e.tpreturn(e.TPSUCCESS, 0, retbuf)

if __name__ == '__main__':
    e.run(Server(), sys.argv)
