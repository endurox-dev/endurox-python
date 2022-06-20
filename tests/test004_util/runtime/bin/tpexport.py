import unittest
import endurox as e
import exutils as u

class TestTpexport(unittest.TestCase):


    def test_tpexport_ok(self):
        w = u.NdrxStopwatch()
        while w.get_delta_sec() < u.test_duratation():
            
            # binary data:
            buf=e.tpexport({"buftype":"UBF", "data":{"T_STRING_FLD":["HELLO TEST", "HELLO 2"]}})
            buf2=e.tpimport(buf)
            self.assertEqual(buf2, {"buftype":"UBF", "data":{"T_STRING_FLD":["HELLO TEST", "HELLO 2"]}})

            # string fmt
            buf=e.tpexport({"buftype":"UBF", "data":{"T_STRING_FLD":["HELLO TEST", "HELLO 2"]}}, e.TPEX_STRING)
            buf2=e.tpimport(buf, e.TPEX_STRING)
            self.assertEqual(buf2, {"buftype":"UBF", "data":{"T_STRING_FLD":["HELLO TEST", "HELLO 2"]}})
            

if __name__ == '__main__':
    unittest.main()

