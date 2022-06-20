#!/usr/bin/env python3

import sys
import endurox as e
import time
import cx_Oracle
import threading

class Server:

    def tpsvrinit(self, args):
        e.userlog('Server startup')
        self.local = threading.local()
#        e.tpopen()
#        self.db = cx_Oracle.connect( handle=e.xaoSvcCtx())
        e.tpadvertise('DOINSERT', 'DOINSERT', self.DOINSERT)
        return 0

    # MT server
    def tpsvrthrinit(self, argv):
        e.userlog('Thr init...')
        e.tpopen()
        self.local.db = cx_Oracle.connect(
            handle=e.xaoSvcCtx(), threaded=True
        )
        return 0

    def tpsvrthrdone(self):
       e.userlog('Thr done...')
       e.tpclose()

    def tpsvrdone(self):
        e.userlog('Server shutdown')
        e.tpclose()

    # inserts some stuff to DB
    def DOINSERT(self, args):
        with self.local.db.cursor() as dbc:
            dbc.execute(
                "INSERT INTO pyaccounts VALUES (:1, :2)",
                [args.data["data"]["T_STRING_FLD"][0], args.data["data"]["T_LONG_FLD"][0]]
            )
        return e.tpreturn(e.TPSUCCESS, 0, args.data)

if __name__ == '__main__':
    e.run(Server(), sys.argv)

