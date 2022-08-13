import unittest
import endurox as e
import exutils as u
import gc
import pprint

class TestView(unittest.TestCase):

    # Test view name not found
    def test_view_missing(self):
        try:
           tperrno, tpurcode, retbuf = e.tpcall("ECHO", { "buftype":"VIEW", "subtype":"NO_SUCH_VIEW", "data":{
               "tstring1": "HELLO WORLD ANOTHER WORLD..............."
               }},);
        except e.AtmiException as ex:
           self.assertEqual(ex.code,e.TPENOENT)
        else:
            self.assertEqual(True,False)
if __name__ == '__main__':
    unittest.main()
