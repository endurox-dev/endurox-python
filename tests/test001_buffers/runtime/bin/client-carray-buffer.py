import unittest
import endurox as e
import exutils as u
import gc

class TestNull(unittest.TestCase):

    #
    # Carray buffer tests
    #
    def test_carray_tpcall(self):
        w = u.NdrxStopwatch()
        while w.get_delta_sec() < u.test_duratation():
            tperrno, tpurcode, retbuf = e.tpcall("ECHO", {"data":b'\x00\x00\x05\x07\x00'});
            self.assertEqual(tperrno, 0)
            self.assertEqual(tpurcode, 0)
            self.assertEqual(retbuf["buftype"], "CARRAY")
            self.assertEqual(retbuf["data"], b'\x00\x00\x05\x07\x00')

    #
    # Carray call 2
    #
    def test_carray_tpcall2(self):
        w = u.NdrxStopwatch()
        while w.get_delta_sec() < u.test_duratation():
            tperrno, tpurcode, retbuf = e.tpcall("ECHO", {"buftype":"CARRAY", "data":"HELLO WORLD"});
            self.assertEqual(tperrno, 0)
            self.assertEqual(tpurcode, 0)
            self.assertEqual(retbuf["buftype"], "CARRAY")
            self.assertEqual(retbuf["data"], "HELLO WORLD")
    
if __name__ == '__main__':
    unittest.main()
