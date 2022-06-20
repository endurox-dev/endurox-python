import unittest
import endurox as e
import exutils as u

class TestTpforward(unittest.TestCase):

    # Validate tperrno, tpurcode, server UBF processing
    def test_tpforward_ok(self):
        w = u.NdrxStopwatch()
        while w.get_delta_sec() < u.test_duratation():
            tperrno, tpurcode, retbuf = e.tpcall("FWDSVC", { "data":{"T_STRING_FLD":"Hi Jim"}})
            self.assertEqual(tperrno, 0)
            self.assertEqual(tpurcode, 5)
            self.assertEqual(retbuf["data"]["T_STRING_2_FLD"][0], "Hi Jim")
            self.assertEqual(retbuf["data"]["T_STRING_3_FLD"][0], "Hi Jim")

if __name__ == '__main__':
    unittest.main()

