#!/usr/bin/env python3

import sys
import endurox as e
import time

class Server:

    # TODO - check current server object?

    def tpsvrinit(self, args):
        e.userlog('Server startup')
        e.tpadvertise('FAILSVC', 'FAILSVC', Server.FAILSVC)
        e.tpadvertise('OKSVC', 'OKSVC', Server.OKSVC)
        e.tpadvertise('FWDSVC', 'FWDSVC', Server.FWDSVC)
        e.tpadvertise('EVSVC', 'EVSVC', Server.EVSVC)
        e.tpadvertise('EVSVC2', 'EVSVC2', Server.EVSVC)
        e.tpadvertise('CONVSVC', 'CONVSVC', Server.CONVSVC)
        e.tpadvertise('NOTIFSV', 'NOTIFSV', Server.NOTIFSV)
        e.tpadvertise('BCASTSV', 'BCASTSV', Server.BCASTSV)
        e.tpadvertise('TOUT', 'TOUT', Server.TOUT)

        # subscribe to TESTEV event.
        e.tplog_info("ev subs %d" % e.tpsubscribe('TESTEV', None, e.TPEVCTL(name1="EVSVC", flags=e.TPEVSERVICE)))

        for i in range(1, 10000):
            sb = e.tpsubscribe('TEST..', None, e.TPEVCTL(name1="EVSVC2", flags=(e.TPEVSERVICE)))
            e.tpunsubscribe(sb, 0)

        return 0

    def tpsvrdone(self):
        e.userlog('Server shutdown')

    # 
    # return failure + tpurcode
    #
    def FAILSVC(self, args):
        if args.data["buftype"]=="UBF":
            args.data["data"]["T_STRING_2_FLD"]=args.data["data"]["T_STRING_FLD"][0]
        return e.tpreturn(e.TPFAIL, 5, args.data)

    #
    # return some ok
    #
    def OKSVC(self, args):
        # alter some data, in case if buffer type if UBF
        if args.data["buftype"]=="UBF":
            args.data["data"]["T_STRING_2_FLD"]=args.data["data"]["T_STRING_FLD"][0]
        return e.tpreturn(e.TPSUCCESS, 5, args.data)

    #
    # Forwarding service
    #
    def FWDSVC(self, args):
        # alter some data, in case if buffer type if UBF
        if args.data["buftype"]=="UBF":
            args.data["data"]["T_STRING_3_FLD"]=args.data["data"]["T_STRING_FLD"][0]
        return e.tpforward("OKSVC", args.data, 0)

    #
    # Just consume event, return NULL buffer.
    #
    def EVSVC(self, args):
        return e.tpreturn(e.TPSUCCESS, 0, {})

    #
    # event service 2
    # 
    def EVSVC2(self, args):
        return e.tpreturn(e.TPSUCCESS, 0, {})

    #
    # Run conversational data
    #
    def CONVSVC(self, args):

        # receive something
        rc, urcode, ev, data = e.tprecv(args.cd)

        assert (rc == e.TPEEVENT)
        #assert (urcode == 0)
        assert (ev == e.TPEV_SENDONLY)
        assert (data["data"]["T_STRING_FLD"][0] == "From client")

        # send something
        rc, ur, ev = e.tpsend(args.cd, {"data":{"T_STRING_FLD":"From server"}})
        assert (rc == 0)
        #assert (ur == 0)
        assert (ev == 0)

        return e.tpreturn(e.TPSUCCESS, 6, {"data":{"T_STRING_FLD":"From server 2"}})

    # send notification to the client
    def NOTIFSV(self, args):
        e.tpnotify(args.cltid, {"data":"HELLO WORLD"}, 0)
        return e.tpreturn(e.TPSUCCESS, 0, {})

    def BCASTSV(self, args):
        # may fill all python qs...
        e.tpbroadcast("", "", "python|Python", {"data":"HELLO WORLD BCAST"}, e.TPREGEXMATCH)
        return e.tpreturn(e.TPSUCCESS, 0, {})

    # generate timeout ...
    def TOUT(self, args):
        time.sleep(args.data["data"]["T_SHORT_FLD"][0])
        return e.tpreturn(e.TPSUCCESS, 0, {})


if __name__ == '__main__':
    e.tprun(Server(), sys.argv)

