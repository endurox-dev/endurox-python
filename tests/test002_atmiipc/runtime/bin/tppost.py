import unittest
import endurox as e
import exutils as u

class TestTppost(unittest.TestCase):

    # Validate tperrno, tpurcode, server UBF processing
    def test_tppost_ok(self):
        w = u.NdrxStopwatch()
        while w.get_delta_sec() < u.test_duratation():
            self.assertEqual(e.tppost("TESTEV", {"data":{"T_STRING_FLD":"HELLO EVENT"}}, 0), 1)

if __name__ == '__main__':
    unittest.main()

