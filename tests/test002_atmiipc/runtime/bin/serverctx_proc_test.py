import unittest
import endurox as e
import exutils as u

class TestServerCtxTransfer2proc(unittest.TestCase):

    # Validate tperrno, tpurcode, server UBF processing
    def test_tpcall_ok(self):
        w = u.NdrxStopwatch()
        while w.get_delta_sec() < u.test_duratation():
            tperrno, tpurcode, retbuf = e.tpcall("SRVCTXP", { "data":{"T_STRING_FLD":"Hi Jim"}})
            self.assertEqual(tperrno, 0)
            self.assertEqual(tpurcode, 0)
            self.assertEqual(retbuf["data"]["T_STRING_FLD"][0], "Hi Jim")

if __name__ == '__main__':
    unittest.main()

