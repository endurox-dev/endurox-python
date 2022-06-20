#!/usr/bin/env python3

import sys
import endurox as e

# counter of b4polls
M_cnt = 0

def b4poll():
    global M_cnt
    M_cnt+=1
    return 0

# this one fails...
# and make server to reboot
def b4poll2():
    global M_cnt
    M_cnt+=1
    return -1

#
# Test before poll callback
#
class Server:

    def tpsvrinit(self, args):
        e.userlog('Server startup')
        e.tpext_addb4pollcb(b4poll)
        e.tpadvertise('B4POLL', 'B4POLL', self.B4POLL)
        return 0

    def tpsvrdone(self):
        e.userlog('Server shutdown')

    # first call T_SHORT_FLD -> 1
    # 2nd call T_SHORT_FLD -> 2
    # 3nd call T_SHORT_FLD -> 3 && server restarted
    def B4POLL(self, args):
        # return current counter...
        global M_cnt
        retbuf = {"data":{"T_SHORT_FLD":M_cnt}}
        if M_cnt==1:
            e.tpext_delb4pollcb()
            M_cnt+=1
        elif  M_cnt==2:
            e.tpext_addb4pollcb(b4poll2)

        return e.tpreturn(e.TPSUCCESS, 0, retbuf)

if __name__ == '__main__':
    e.run(Server(), sys.argv)
