import unittest
import endurox as e
import exutils as u
import gc

class TestUbf(unittest.TestCase):

    #
    # Test boolean expersions
    #
    def test_ubf_Bboolev(self):
        w = u.NdrxStopwatch()
        while w.get_delta_sec() < u.test_duratation():
            self.assertEqual(e.Bboolev({"data":{ "T_STRING_FLD":["ABC", "CCC"]}}, "T_STRING_FLD[0]=='ABC' && T_STRING_FLD[1]=='CCC'"), True)
            self.assertEqual(e.Bboolev({"data":{ "T_STRING_FLD":["ABC", "CCC"]}}, "!T_STRING_FLD"), False)

if __name__ == '__main__':
    unittest.main()
