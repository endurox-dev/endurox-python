import unittest
import endurox as e
from endurox.ubfdict import UbfDict
import exutils as u
from copy import deepcopy

# UBF Dicitionary tests, slices
class TestUbfDictSlice(unittest.TestCase):

    def test_ubf_slices(self):
        w = u.NdrxStopwatch()
        while w.get_delta_sec() < u.test_duratation():
            b2 = e.UbfDict({"T_STRING_FLD":["HELLO WORLD", "ONE", "TWO", "THREE", "4"]})

            rev_sl = b2["T_STRING_FLD"][::-1]

            self.assertEqual(rev_sl[0], "4")
            self.assertEqual(rev_sl[1], "THREE")
            self.assertEqual(rev_sl[2], "TWO")
            self.assertEqual(rev_sl[3], "ONE")
            self.assertEqual(rev_sl[4], "HELLO WORLD")
            self.assertEqual(len(rev_sl), 5)

            rev_sl = b2["T_STRING_FLD"][::2]
            self.assertEqual(rev_sl[0], "HELLO WORLD")
            self.assertEqual(rev_sl[1], "TWO")
            self.assertEqual(rev_sl[2], "4")
            self.assertEqual(len(rev_sl), 3)

            rev_sl = b2["T_STRING_FLD"][1::2]
            self.assertEqual(rev_sl[0], "ONE")
            self.assertEqual(rev_sl[1], "THREE")
            self.assertEqual(len(rev_sl), 2)

            rev_sl = b2["T_STRING_FLD"][:3:2]
            self.assertEqual(rev_sl[0], "HELLO WORLD")
            self.assertEqual(rev_sl[1], "TWO")
            self.assertEqual(len(rev_sl), 2)

            rev_sl = b2["T_STRING_FLD"][1:-2:]
            self.assertEqual(rev_sl[0], "ONE")
            self.assertEqual(rev_sl[1], "TWO")
            self.assertEqual(len(rev_sl), 2)

            rev_sl = b2["T_STRING_FLD"][-1]
            self.assertEqual(rev_sl[0], "4")
            self.assertEqual(len(rev_sl), 1)

    #
    # test field access modes...
    #
    def test_ubf_fldaccess(self):
        w = u.NdrxStopwatch()
        while w.get_delta_sec() < u.test_duratation():
            b2 = e.UbfDict({"T_STRING_FLD":["HELLO WORLD", "ONE", "TWO"]})

            # default mode is not to delete other fields.
            b2.T_STRING_FLD = "XYZ"
            self.assertEqual(len(b2.T_STRING_FLD), 3)
            self.assertEqual(b2.T_STRING_FLD[0], "XYZ")
            self.assertEqual(b2.T_STRING_FLD[1], "ONE")
            self.assertEqual(b2.T_STRING_FLD[2], "TWO")

            # check prev value
            self.assertEqual(e.ndrxpy_ubfdict_delonset(True), False)
            b2.T_STRING_FLD = "CCC"

            self.assertEqual(len(b2.T_STRING_FLD), 1)
            self.assertEqual(b2.T_STRING_FLD[0], "CCC")
            self.assertEqual(e.ndrxpy_ubfdict_delonset(False), True)

if __name__ == '__main__':
    unittest.main()
