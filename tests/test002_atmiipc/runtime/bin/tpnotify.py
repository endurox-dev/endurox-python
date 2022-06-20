import unittest
import endurox as e
import exutils as u

class TestTpnotify(unittest.TestCase):

    cnt = 0

    def unsol_handler(self, data):
        if data["data"] == "HELLO WORLD":
            TestTpnotify.cnt=TestTpnotify.cnt+1

    # Validate tperrno, tpurcode, server UBF processing
    def test_tpnotify(self):
        w = u.NdrxStopwatch()
        while w.get_delta_sec() < u.test_duratation():
            e.tpsetunsol(self.unsol_handler)
            cnt_prev = TestTpnotify.cnt
            tperrno, tpurcode, retbuf = e.tpcall("NOTIFSV", { "data":{"T_STRING_FLD":"Hi Jim"}})
            self.assertEqual(tperrno, 0)
            self.assertEqual(cnt_prev+1, TestTpnotify.cnt)
        # remove queues...
        e.tpterm()

if __name__ == '__main__':
    unittest.main()

