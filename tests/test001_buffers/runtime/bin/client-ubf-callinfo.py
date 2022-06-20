import unittest
import endurox as e
import exutils as u
import gc

class TestUbf(unittest.TestCase):

    #
    # Test call info processing
    # 
    def test_ubf_callinfo(self):
        w = u.NdrxStopwatch()
        while w.get_delta_sec() < u.test_duratation():
            tperrno, tpurcode, retbuf = e.tpcall("ECHO", { "data":{}, "callinfo":{
                "T_CHAR_FLD": ["X", "Y"]
                }})
            self.assertEqual(tperrno, 0)
            self.assertEqual(tpurcode, 0)
            self.assertEqual(retbuf["callinfo"]["T_CHAR_FLD"][0], "X")
            self.assertEqual(retbuf["callinfo"]["T_CHAR_FLD"][1], "Y")

if __name__ == '__main__':
    unittest.main()
