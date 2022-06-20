import unittest
import endurox as e
import exutils as u
import gc
import pprint

class TestView(unittest.TestCase):

    # Normal view call
    def test_view_tpcall(self):
        w = u.NdrxStopwatch()
        while w.get_delta_sec() < u.test_duratation():
            tperrno, tpurcode, retbuf = e.tpcall("ECHO", { "buftype":"VIEW", "subtype":"UBTESTVIEW2", "data":{
                "tshort1": 100
                , "tlong1": 200000
                , "tchar1": "Y"
                , "tfloat1": 999.11
                , "tdouble1": 8888.99
                , "tstring1": "HELLO WORLD"
                , "tcarray1": [b'\x00\x03\x05\x07', b'\x00\x00\x05\x07'],
                }},);
            self.assertEqual(tperrno, 0)
            self.assertEqual(tpurcode, 0)
            self.assertEqual(retbuf["buftype"], "VIEW")
            self.assertEqual(retbuf["subtype"], "UBTESTVIEW2")
            self.assertEqual(retbuf["data"]["tstring1"][0], "HELLO WORLD")
            self.assertEqual(retbuf["data"]["tcarray1"][0], b'\x00\x03\x05\x07')
            self.assertEqual(retbuf["data"]["tcarray1"][1], b'\x00\x00\x05\x07')

if __name__ == '__main__':
    unittest.main()
