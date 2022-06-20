import unittest
import endurox as e
import exutils as u
import gc
import pprint

class TestView(unittest.TestCase):

    # Test no space.
    def test_view_nospace(self):
        w = u.NdrxStopwatch()
        while w.get_delta_sec() < u.test_duratation():
            try:
                tperrno, tpurcode, retbuf = e.tpcall("ECHO", { "buftype":"VIEW", "subtype":"UBTESTVIEW2", "data":{
                    "tstring1": ["EHLO", "HELLO WORLD ANOTHER WORLD..............."]
                    }},);
            except e.UbfException as ex:
                self.assertEqual(ex.code,14)
                self.assertTrue("Invalid occurrence requested" in ex.args[0]) 
            else:
                self.assertEqual(True,False)
if __name__ == '__main__':
    unittest.main()
