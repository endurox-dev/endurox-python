import unittest
import endurox as e
import exutils as u

# UBF Dicitionary tests
class TestUbfDict(unittest.TestCase):

    # Check appender
    def test_ubf_append(self):
        w = u.NdrxStopwatch()
        while w.get_delta_sec() < u.test_duratation():
            # Check Ubf adding
            b1 = e.UbfDict()
            b2 = e.UbfDict({"T_STRING_FLD":"HELLO WORLD"})
            b1["T_UBF_FLD"].append(b2)
            # have ptr to buffer...
            b1["T_PTR_FLD"].append(b2)

    def test_ubf_in(self):
        w = u.NdrxStopwatch()
        while w.get_delta_sec() < u.test_duratation():
            # Check Ubf adding
            b2 = e.UbfDict({"T_STRING_FLD":"HELLO WORLD"})
            self.assertEqual("T_STRING_2_FLD" in b2, False)
            self.assertEqual("T_STRING_FLD" in b2, True)

    def test_ubf_get(self):
        w = u.NdrxStopwatch()
        while w.get_delta_sec() < u.test_duratation():
            # Check Ubf adding
            b2 = e.UbfDict({"T_STRING_FLD":"HELLO WORLD"})
            self.assertEqual(b2["T_STRING_FLD"][0], "HELLO WORLD")
            # check exception, field not pres
            #self.assertRaises(IndexError, b2["T_STRING_FLD"][1])

            with self.assertRaises(IndexError):
                b2["T_STRING_FLD"][1]

            # Emtpy UbfDictFld are returned with ref buf & contains fldid
            #self.assertRaises(KeyError, b2["T_STRING_2_FLD"])
            with self.assertRaises(KeyError):
                b2["T_STRING_2_FLDXX"]
    
if __name__ == '__main__':
    unittest.main()
