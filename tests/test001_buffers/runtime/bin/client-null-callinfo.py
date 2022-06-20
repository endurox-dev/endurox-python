import unittest
import endurox as e
import exutils as u
import gc

class TestNull(unittest.TestCase):

    #
    # Null buffer tests, callinfo not supported
    #
    def test_null_callinfo(self):
        w = u.NdrxStopwatch()
        while w.get_delta_sec() < u.test_duratation():
            try:
                tperrno, tpurcode, retbuf = e.tpcall("ECHO", {"callinfo":{"T_STRING_FLD":"HELLO CALL INFO"}});
            except ValueError as ex:
                self.assertEqual(True,True)
            else:
                self.assertEqual(True,False)
    
if __name__ == '__main__':
    unittest.main()
