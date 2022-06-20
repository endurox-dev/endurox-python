import unittest
import endurox as e
import exutils as u
import gc

class TestJson(unittest.TestCase):

    #
    # Json buffer tests
    #
    def test_json_tpcall(self):
        w = u.NdrxStopwatch()
        while w.get_delta_sec() < u.test_duratation():
            tperrno, tpurcode, retbuf = e.tpcall("ECHO", {"buftype":"JSON", "data":"{}"});
            self.assertEqual(tperrno, 0)
            self.assertEqual(tpurcode, 0)
            self.assertEqual(retbuf["buftype"], "JSON")
            self.assertEqual(retbuf["data"], "{}")
    
if __name__ == '__main__':
    unittest.main()
