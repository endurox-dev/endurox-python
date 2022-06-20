#!/usr/bin/env python3

import sys
import endurox as e
import time
import cx_Oracle
import unittest
import exutils as u

class TestOracleXA(unittest.TestCase):

    def test_xa(self):

        e.tpinit()
        e.tpopen()
        e.tplogconfig(e.LOG_FACILITY_NDRX, -1, "", "TEST", "%s/client.py" % e.tuxgetenv("NDRX_ULOG"))
        w = u.NdrxStopwatch()
        while w.get_delta_sec() < u.test_duratation():

            e.tplog_info("clean up the DB...")
            db = cx_Oracle.connect(handle=e.xaoSvcCtx())
            e.tpbegin(99, 0)
            with db.cursor() as cursor:
                 cursor.execute("delete from pyaccounts")
            e.tpcommit(0)
           
            # check 0...
            e.tpbegin(99, 0)
            with db.cursor() as cursor:
                cursor.execute("SELECT count(*) FROM pyaccounts")
                self.assertEqual(cursor.fetchone()[0], 0)
            e.tpcommit(0)
      

            e.tpbegin(99, 0)
            # do some insert
            e.tpcall("DOINSERT", {"data":{"T_STRING_FLD":"HELLO1", "T_LONG_FLD":100}})
            e.tpcall("DOINSERT", {"data":{"T_STRING_FLD":"HELLO2", "T_LONG_FLD":102}})
            e.tpcall("DOINSERT", {"data":{"T_STRING_FLD":"HELLO3", "T_LONG_FLD":103}})
            e.tpcall("DOINSERT", {"data":{"T_STRING_FLD":"HELLO4", "T_LONG_FLD":104}})

            # check account, shall be 4...
            with db.cursor() as cursor:
                cursor.execute("SELECT count(*) FROM pyaccounts")
                self.assertEqual(cursor.fetchone()[0], 4)

            # if we abort.., shall be 0 again
            e.tpabort(0)

            with db.cursor() as cursor:
                cursor.execute("SELECT count(*) FROM pyaccounts")
                self.assertEqual(cursor.fetchone()[0], 0)

            e.tpbegin(99, 0)
            # do some insert
            e.tpcall("DOINSERT", {"data":{"T_STRING_FLD":"HELLO1", "T_LONG_FLD":100}})
            e.tpcall("DOINSERT", {"data":{"T_STRING_FLD":"HELLO2", "T_LONG_FLD":102}})
            e.tpcall("DOINSERT", {"data":{"T_STRING_FLD":"HELLO3", "T_LONG_FLD":103}})
            e.tpcall("DOINSERT", {"data":{"T_STRING_FLD":"HELLO4", "T_LONG_FLD":104}})

            e.tpcommit(0)

            # commit was ok, have the records:
            with db.cursor() as cursor:
                cursor.execute("SELECT count(*) FROM pyaccounts")
                self.assertEqual(cursor.fetchone()[0], 4)

            e.tpbegin(99, 0)

            # do some insert, fails with TPESVCERR as exception is not catched
            try:
                e.tpcall("DOINSERT", {"data":{"T_STRING_FLD":"HELLO1", "T_LONG_FLD":100}})
            except e.AtmiException as ex:
                self.assertEqual(ex.code,e.TPESVCERR)
            else:
                self.assertEqual(True,False)
            
            # abort-only:
            try:
                e.tpcommit(0)
            except e.AtmiException as ex:
                self.assertEqual(ex.code,e.TPEABORT)
            else:
                self.assertEqual(True,False)

            # change the commit control:
            e.tpscmt(e.TP_CMT_LOGGED)
            e.tpbegin(99, 0)
            e.tpcall("DOINSERT", {"data":{"T_STRING_FLD":"HELLO5", "T_LONG_FLD":105}})
            e.tpcommit(0)

        e.tpterm()

if __name__ == '__main__':
    unittest.main()

