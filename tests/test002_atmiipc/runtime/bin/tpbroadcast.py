import unittest
import endurox as e
import exutils as u

class TestTpbroadcast(unittest.TestCase):

    cnt = 0

    def unsol_handler(self, data):
        if data["data"] == "HELLO WORLD BCAST":
            TestTpbroadcast.cnt=TestTpbroadcast.cnt+1

    # Validate tperrno, tpurcode, server UBF processing
    def test_tpbroadcast(self):
        w = u.NdrxStopwatch()
        while w.get_delta_sec() < u.test_duratation():
            e.tpsetunsol(self.unsol_handler)
            cnt_prev = TestTpbroadcast.cnt
            tperrno, tpurcode, retbuf = e.tpcall("BCASTSV", { "data":{"T_STRING_FLD":"Hi Jim"}})
            self.assertEqual(tperrno, 0)
            self.assertEqual(cnt_prev+1, TestTpbroadcast.cnt)

if __name__ == '__main__':
    unittest.main()

