import threading
import time
import unittest
import endurox as e
import exutils as u

class TestTpchkunsol(unittest.TestCase):

    cnt1 = 0
    cnt2 = 0
    shutdown = 0
    ############################################################################
    # thread functions
    ############################################################################
    def unsol_handler2(self, data):
        if data["data"] == "HELLO WORLD BCAST":
            self.cnt2=self.cnt2+1

    # thread functions
    def thread_function(self, name):
        e.tpinit()
        e.tpsetunsol(self.unsol_handler2)
        while 0==self.shutdown:
            while e.tpchkunsol() >0:
                True
            time.sleep(1)
        # consume any messages left over...
        while e.tpchkunsol() >0:
            True

    def unsol_handler1(self, data):
        if data["data"] == "HELLO WORLD BCAST":
            self.cnt1=self.cnt1+1

    ############################################################################
    # main test
    ############################################################################
    def test_tpnotify(self):
        w = u.NdrxStopwatch()

        x = threading.Thread(target=self.thread_function, args=(1,))
        x.start()
        # let other thread to init...
        #time.sleep(5)

        while w.get_delta_sec() < u.test_duratation():
            e.tpsetunsol(self.unsol_handler1)
            cnt_prev = self.cnt1
            tperrno, tpurcode, retbuf = e.tpcall("BCASTSV", { "data":{"T_STRING_FLD":"Hi Jim"}})
            self.assertEqual(tperrno, 0)
            self.assertEqual(cnt_prev+1, self.cnt1)
        self.shutdown=1
        x.join()

        self.assertEqual(self.cnt1, self.cnt2)

if __name__ == '__main__':
    unittest.main()

