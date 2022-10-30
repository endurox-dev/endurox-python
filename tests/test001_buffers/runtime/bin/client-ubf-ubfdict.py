import unittest
import endurox as e
from endurox.ubfdict import UbfDict
import exutils as u
from copy import deepcopy

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
            # now b1 becomes master of the buffer.
            b1["T_PTR_FLD"].append({"data":b2})
            # XATMI object ptr is reset.
            self.assertEqual(b2._is_sub_buffer, 2)

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
            self.assertEqual(str1, "{'T_SHORT_FLD': [100, 99], 'T_STRING_FLD': ['HELLO', 'WORLD']}")

    # Test UbfDict Disable
    def test_ubf_ubfdict_disable(self):
        w = u.NdrxStopwatch()

        while w.get_delta_sec() < u.test_duratation():
            
            # default is false
            self.assertEqual(e.ndrxpy_ubfdict_enable(True), True)

            # Change to dict() mode.
            self.assertEqual(e.ndrxpy_ubfdict_enable(False), True)
            tperrno, tpurcode, retbuf = e.tpcall("ECHO", { "data":{"T_STRING_FLD":"HELLO", "T_UBF_FLD":{}}})
            self.assertEqual(type(retbuf["data"]), type({}))
            self.assertEqual(type(retbuf["data"]["T_UBF_FLD"][0]), type({}))

            # reset back to UbfDict()
            self.assertEqual(e.ndrxpy_ubfdict_enable(True), False)
            tperrno, tpurcode, retbuf = e.tpcall("ECHO", { "data":{"T_STRING_FLD":"HELLO", "T_UBF_FLD":{}}})
            self.assertEqual(type(retbuf["data"]), type(UbfDict()))
            self.assertEqual(type(retbuf["data"]["T_UBF_FLD"][0]), type(UbfDict()))


    # Field sequence.
    def test_ubf_ubfdictfld_seq(self):
        w = u.NdrxStopwatch()

        while w.get_delta_sec() < u.test_duratation():
            
             b1 = e.UbfDict({"T_SHORT_FLD":[100,99,7,4]})
             seq = [100,99,7,4]
             i=0
             for v in b1["T_SHORT_FLD"]:
                self.assertEqual(v, seq[i])
                i+=1

    # Test buffer representation
    def test_ubfdictfld_repr(self):
        w = u.NdrxStopwatch()

        while w.get_delta_sec() < u.test_duratation():
            b1 = e.UbfDict({"T_SHORT_FLD":[100,99], "T_STRING_FLD":["HELLO", "WORLD"]})
            str1 = str(b1["T_SHORT_FLD"])
            self.assertEqual(str1, "[100, 99]")

    def test_ubfdictfld_len(self):
        w = u.NdrxStopwatch()
        while w.get_delta_sec() < u.test_duratation():
            b1 = e.UbfDict({"T_SHORT_FLD":[100,99], "T_STRING_FLD":["HELLO", "WORLD"]})
            self.assertEqual(len(b1["T_SHORT_FLD"]), 2)
            self.assertEqual(b1["T_SHORT_FLD"].pop(), 99)
            self.assertEqual(len(b1["T_SHORT_FLD"]), 1)

    def test_ubfdict_attribs(self):
        w = u.NdrxStopwatch()
        while w.get_delta_sec() < u.test_duratation():
            b1 = e.UbfDict({"T_SHORT_FLD":[100,99], "T_STRING_FLD":["HELLO", "WORLD"]})
            b1.T_STRING_2_FLD="OK1"
            b1.T_STRING_2_FLD="OK2"
            b1.T_STRING_2_FLD="OK3"

            self.assertEqual(len(b1.T_STRING_2_FLD), 1)
            self.assertEqual(b1.T_STRING_2_FLD[0], "OK3")

    # check negative index access to fields.
    def test_ubfdictfld_neg_index(self):
        w = u.NdrxStopwatch()
        while w.get_delta_sec() < u.test_duratation():
            b1 = e.UbfDict({"T_SHORT_FLD":[100,99,11,33,8]})
            b1.T_SHORT_FLD[-2]=5

            self.assertEqual(b1.T_SHORT_FLD[3], 5)
            self.assertEqual(b1.T_SHORT_FLD[-2], 5)
            del (b1.T_SHORT_FLD[-2])
            self.assertEqual(b1.T_SHORT_FLD[-2], 11)

    # server reallocates and returns 56k of data
    def test_ubfdict_svrealloc(self):
        w = u.NdrxStopwatch()
        while w.get_delta_sec() < u.test_duratation():
            errno, tpurcode, buf = e.tpcall("REALLOC", {"data":{}})
            self.assertEqual(errno, 0)
            self.assertEqual(tpurcode, 0)
            self.assertEqual(len(buf["data"].T_CARRAY_FLD[0]), 56000)

    # Ready only checks..
    def test_ubfdict_ro(self):
        w = u.NdrxStopwatch()
        while w.get_delta_sec() < u.test_duratation():
            
            b1 = UbfDict({"T_UBF_FLD":{"T_STRING_FLD":"OK"}
                , "T_UBF_2_FLD":{"T_STRING_FLD":"OK2"}})
            b1.T_UBF_FLD[2]={"T_STRING_3_FLD":"HELLO"}

            # cannot modify sub-buffers
            with self.assertRaises(AttributeError):
                b1.T_UBF_FLD[2].T_STRING_3_FLD[4]="WORLD"

            with self.assertRaises(AttributeError):
                del b1.T_UBF_FLD[2].T_STRING_3_FLD[4]

            with self.assertRaises(AttributeError):
                del b1.T_UBF_FLD[2].T_STRING_3_FLD
            
    # PTR Checks.
    def test_ubfdict_ptr(self):
        w = u.NdrxStopwatch()
        while w.get_delta_sec() < u.test_duratation():

            tmp_b = e.UbfDict({"T_STRING_FLD":"PTR"})
            
            b2 = e.UbfDict({"T_PTR_FLD":{"data":tmp_b}})
            self.assertEqual(b2.T_PTR_FLD[0]["data"].T_STRING_FLD[0], "PTR")
            self.assertEqual(tmp_b._is_sub_buffer, 2)

            # check that buffer keeps RO when extracted
            tmp_b2 = b2.T_PTR_FLD[0]["data"]
            self.assertEqual(tmp_b2._is_sub_buffer, 2)

            self.assertEqual(tmp_b.T_STRING_FLD[0], "PTR")
            self.assertEqual(b2.T_PTR_FLD[0]["data"].T_STRING_FLD[0], "PTR")
            self.assertEqual(tmp_b2.T_STRING_FLD[0], "PTR")

            # check transfer between Ubfs

            tmp_b = e.UbfDict({"T_STRING_FLD":"PTR"})
            self.assertEqual(tmp_b._is_sub_buffer, 0)
            b2 = e.UbfDict({"T_PTR_FLD":{"data":tmp_b}})
            self.assertEqual(tmp_b._is_sub_buffer, 2)
            # we are sub-buffers also as free of the UBF will kill the sub-buffers...
            self.assertEqual(b2.T_PTR_FLD[0]["data"]._is_sub_buffer, 2)

            # check the transfer to other ubf...
            b3 = e.UbfDict()

            b3.T_PTR_FLD = b2.T_PTR_FLD
            self.assertEqual(b3.T_PTR_FLD[0]["data"]._is_sub_buffer, 2)

            # remove b3 field, as on GC, both b2 & b3 then would attempt to delete XAMTI buffer
            # causing segmentation fault.
            del b3.T_PTR_FLD

    # Copy the buffer
    def test_ubfdict_copy(self):
        w = u.NdrxStopwatch()
        while w.get_delta_sec() < u.test_duratation():
            b1 = {"data":e.UbfDict({"T_STRING_FLD":["HELLO", "WORLD"], "T_PTR_FLD":{"data":{"T_STRING_2_FLD":"EHLO"}}})}
            b2 = deepcopy(b1)

            self.assertEqual(b1["data"].T_STRING_FLD, b2["data"].T_STRING_FLD)
            self.assertEqual(b1["data"].T_PTR_FLD, b2["data"].T_PTR_FLD)

            b1["data"].T_STRING_FLD.append("TEST1")
            b1["data"].T_PTR_FLD[0]["data"].T_STRING_2_FLD.append("TEST2")
            self.assertNotEqual(b1["data"].T_STRING_FLD, b2["data"].T_STRING_FLD)
            self.assertEqual(b1["data"].T_PTR_FLD, b2["data"].T_PTR_FLD)

            b3 = {"data":e.UbfDict(b1["data"])}
            self.assertEqual(b1, b3)
            # delete ptr field from b3, as GC will remove the same PTR from b1 & b3 causing invalid buffers
            del b3["data"].T_PTR_FLD

    # check dict field assign
    def test_ubfdict_fldassign(self):
        w = u.NdrxStopwatch()
        while w.get_delta_sec() < u.test_duratation():
            b1 = e.UbfDict({"T_STRING_FLD":["HELLO", "WORLD"]})
            b2 = e.UbfDict({"T_STRING_FLD":b1.T_STRING_FLD})
            self.assertEqual(b2.T_STRING_FLD[0], "HELLO")
            self.assertEqual(b2.T_STRING_FLD[1], "WORLD")


if __name__ == '__main__':
    unittest.main()
