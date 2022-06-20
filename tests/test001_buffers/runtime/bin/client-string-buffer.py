import unittest
import endurox as e
import exutils as u
import gc

class TestNull(unittest.TestCase):

    #
    # String buffer tests
    #
    def test_string_tpcall(self):
        w = u.NdrxStopwatch()
        while w.get_delta_sec() < u.test_duratation():
            tperrno, tpurcode, retbuf = e.tpcall("ECHO", {"data":"THIS IS STRING BUFFERT TEST"});
            self.assertEqual(tperrno, 0)
            self.assertEqual(tpurcode, 0)
            self.assertEqual(retbuf["buftype"], "STRING")
            self.assertEqual(retbuf["data"], "THIS IS STRING BUFFERT TEST")
    
if __name__ == '__main__':
    unittest.main()
