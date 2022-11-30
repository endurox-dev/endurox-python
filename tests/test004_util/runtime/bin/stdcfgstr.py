import unittest
import endurox as e
import os
import time

class TestStdcfgstr(unittest.TestCase):

    def test_stdcfgstr(self):
        ret = e.ndrx_stdcfgstr_parse("a,b c='OK',X=112")

        self.assertEqual(ret[0][0], "a")
        self.assertEqual(len(ret[0]), 1)

        self.assertEqual(ret[1][0], "b")
        self.assertEqual(len(ret[1]), 1)
        
        self.assertEqual(ret[2][0], "c")
        self.assertEqual(len(ret[2]), 2)
        self.assertEqual(ret[2][1], "OK")

        self.assertEqual(ret[3][0], "X")
        self.assertEqual(len(ret[3]), 2)
        self.assertEqual(ret[3][1], "112")


if __name__ == '__main__':
    unittest.main()

