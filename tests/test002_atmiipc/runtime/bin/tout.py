import unittest
import endurox as e
import time

class TestTout(unittest.TestCase):

    # Validate async call by cd
    def test_tout(self):
        e.tpinit()
        e.tptoutset(5)
        self.assertEqual(e.tptoutget(), 5)
        try:
            tperrno, tpurcode, retbuf = e.tpcall("TOUT", { "data":{"T_SHORT_FLD":"10"}})
        except e.AtmiException as ex:
            self.assertEqual(ex.code,e.TPETIME)
        else:
            self.assertEqual(True,False)

        # complete previous call
        time.sleep(10)

        e.tptoutset(20)
        # system wide
        # seems like we have a bug (and fixed in E/X core, as after the tout call
        # this will return 0, instead of 20 of system tout...
        self.assertEqual(e.tpgblktime(0), 20)
        # next call...
        e.tpsblktime(5,e.TPBLK_NEXT)
        self.assertEqual(e.tpgblktime(e.TPBLK_NEXT), 5)

        try:
            tperrno, tpurcode, retbuf = e.tpcall("TOUT", { "data":{"T_SHORT_FLD":"10"}})
        except e.AtmiException as ex:
            self.assertEqual(ex.code,e.TPETIME)
        else:
            self.assertEqual(True,False)

        time.sleep(10)
        self.assertEqual(e.tpgblktime(e.TPBLK_NEXT), 0)
        self.assertEqual(e.tpgblktime(e.TPBLK_ALL), 0)

        e.tpsblktime(5, e.TPBLK_ALL)
        self.assertEqual(e.tpgblktime(e.TPBLK_ALL), 5)

        try:
            tperrno, tpurcode, retbuf = e.tpcall("TOUT", { "data":{"T_SHORT_FLD":"10"}})
        except e.AtmiException as ex:
            self.assertEqual(ex.code,e.TPETIME)
        else:
            self.assertEqual(True,False)

        time.sleep(10)
        # still relevant...
        self.assertEqual(e.tpgblktime(e.TPBLK_ALL), 5)

if __name__ == '__main__':
    unittest.main()

