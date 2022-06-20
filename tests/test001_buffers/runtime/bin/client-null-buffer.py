import unittest
import endurox as e
import exutils as u
import gc

class TestNull(unittest.TestCase):

    #
    # Null buffer tests
    #
    def test_null_tpcall(self):
        w = u.NdrxStopwatch()
        while w.get_delta_sec() < u.test_duratation():
            tperrno, tpurcode, retbuf = e.tpcall("ECHO", {});
            self.assertEqual(tperrno, 0)
            self.assertEqual(tpurcode, 0)
            self.assertEqual(retbuf["buftype"], "NULL")
    
if __name__ == '__main__':
    unittest.main()
