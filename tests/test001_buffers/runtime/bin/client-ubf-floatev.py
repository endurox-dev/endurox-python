import unittest
import endurox as e
import exutils as u
import gc

class TestUbf(unittest.TestCase):

    #
    # Test float ev
    #
    def test_ubf_Bfloatev(self):
        w = u.NdrxStopwatch()
        while w.get_delta_sec() < u.test_duratation():
            self.assertAlmostEqual(e.Bfloatev({"data":{ "T_LONG_FLD":["10", "7"]}}, "T_LONG_FLD[0]-T_LONG_FLD[1]"), 3, places=3, msg=None, delta=None)

if __name__ == '__main__':
    unittest.main()
