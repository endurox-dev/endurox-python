import unittest
import endurox as e
from endurox.ubfdict import UbfDict
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
    
    def test_ubf_del(self):
        w = u.NdrxStopwatch()
        while w.get_delta_sec() < u.test_duratation():
            # Check Ubf adding
            b2 = e.UbfDict({"T_STRING_FLD":["HELLO WORLD", "2"]})
            self.assertEqual("T_STRING_FLD" in b2, True)
            del b2["T_STRING_FLD"]
            self.assertEqual("T_STRING_FLD" in b2, False)

    def test_ubf_len(self):
        w = u.NdrxStopwatch()
        while w.get_delta_sec() < u.test_duratation():
            # Check Ubf adding
            b2 = e.UbfDict({"T_SHORT_FLD":55, "T_STRING_FLD":["HELLO WORLD", "2"]})
            self.assertEqual(len(b2), 2)
            self.assertEqual(len(b2.itemsocc()), 3)

    # test modes of UbfDict constructors
    def test_ubf_ctor(self):

        w = u.NdrxStopwatch()
        while w.get_delta_sec() < u.test_duratation():
            # Check Ubf adding
            b2 = e.UbfDict({"T_SHORT_FLD":55, "T_STRING_FLD":["HELLO WORLD", "2"]})
            b3 = e.UbfDict(b2)
            b4 = e.UbfDict()

            self.assertTrue("T_SHORT_FLD" in b2)
            self.assertTrue("T_SHORT_FLD" in b3)
            self.assertFalse("T_SHORT_FLD" in b4)

            self.assertNotEqual(b2._buf, b3._buf)
            self.assertNotEqual(b3._buf, b4._buf)
            self.assertNotEqual(b4._buf, 0)

    # Iterate UBF dict..
    def test_ubf_iter(self):
        w = u.NdrxStopwatch()
        val = [ {"key":"T_SHORT_FLD", "vals":[100,99]}, {"key":"T_STRING_FLD", "vals":["HELLO", "WORLD"]} ]
        valocc = [ {"key":"T_SHORT_FLD", "vals":100}, {"key":"T_SHORT_FLD", "vals":99},
            {"key":"T_STRING_FLD", "vals":"HELLO"}, {"key":"T_STRING_FLD", "vals":"WORLD"}]

        b1 = e.UbfDict({"T_SHORT_FLD":[100,99], "T_STRING_FLD":["HELLO", "WORLD"]})
        while w.get_delta_sec() < u.test_duratation():
            i=0
            for k in b1:
                self.assertEqual(k, val[i]["key"])
                # check list compare?
                #self.assertEqual(v, val[i]["vals"])
                i+=1
            i=0
            for k,v in b1.items():
                self.assertEqual(k, val[i]["key"])
                # check list compare?
                self.assertEqual(v, val[i]["vals"])
                i+=1

            i=0
            for k,v in b1.itemsocc():
                self.assertEqual(k, valocc[i]["key"])
                # check list compare?
                self.assertEqual(v, valocc[i]["vals"])
                i+=1

    # Test dictionary converter
    def test_ubf_to_dict(self):
        w = u.NdrxStopwatch()

        while w.get_delta_sec() < u.test_duratation():
            b1 = e.UbfDict({"T_SHORT_FLD":[100,99], "T_STRING_FLD":["HELLO", "WORLD"]})
            b2 = b1.to_dict()
            self.assertEqual(type(b2), type({}))

    # Test buffer representation
    def test_ubf_repr(self):
        w = u.NdrxStopwatch()

        while w.get_delta_sec() < u.test_duratation():
            b1 = e.UbfDict({"T_SHORT_FLD":[100,99], "T_STRING_FLD":["HELLO", "WORLD"]})
            str1 = str(b1)
            comp='''\
T_SHORT_FLD\N{TAB}100
T_SHORT_FLD\N{TAB}99
T_STRING_FLD\N{TAB}HELLO
T_STRING_FLD\N{TAB}WORLD
'''
            self.assertEqual(str1, comp)

    # Test UbfDict Disable
    def test_ubf_ubfdict_disable(self):
        w = u.NdrxStopwatch()

        while w.get_delta_sec() < u.test_duratation():
            
            # default is false
            self.assertEqual(e.ndrxpy_use_ubfdict(True), True)

            # Change to dict() mode.
            self.assertEqual(e.ndrxpy_use_ubfdict(False), True)
            tperrno, tpurcode, retbuf = e.tpcall("ECHO", { "data":{"T_STRING_FLD":"HELLO", "T_UBF_FLD":{}}})
            self.assertEqual(type(retbuf["data"]), type({}))
            self.assertEqual(type(retbuf["data"]["T_UBF_FLD"][0]), type({}))

            # reset back to UbfDict()
            self.assertEqual(e.ndrxpy_use_ubfdict(True), False)
            tperrno, tpurcode, retbuf = e.tpcall("ECHO", { "data":{"T_STRING_FLD":"HELLO", "T_UBF_FLD":{}}})
            self.assertEqual(type(retbuf["data"]), type(UbfDict()))
            self.assertEqual(type(retbuf["data"]["T_UBF_FLD"][0]), type(UbfDict()))


if __name__ == '__main__':
    unittest.main()