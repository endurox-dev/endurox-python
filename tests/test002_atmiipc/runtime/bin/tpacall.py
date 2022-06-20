import unittest
import endurox as e
import exutils as u
import gc

class TestTpacall(unittest.TestCase):

    # Validate async call by cd
    def test_tapcall_cd(self):
        w = u.NdrxStopwatch()
        while w.get_delta_sec() < u.test_duratation():
            cd = e.tpacall("OKSVC", { "data":{"T_STRING_FLD":"Hi Jim"}})
            tperrno, tpurcode, retbuf, cd = e.tpgetrply(cd)
            self.assertEqual(cd, cd)
            self.assertEqual(tperrno, 0)
            self.assertEqual(tpurcode, 5)
            self.assertEqual(retbuf["data"]["T_STRING_2_FLD"][0], "Hi Jim")

    def test_tapcall_any(self):
        w = u.NdrxStopwatch()
        while w.get_delta_sec() < u.test_duratation():

            cds = list()

            for i in range(0, 10):
                cd = e.tpacall("OKSVC", { "data":{"T_STRING_FLD":"Hi Jim"}})
                cds.append(cd)

            for i in range(0, 10):
                tperrno, tpurcode, retbuf, cd = e.tpgetrply(0, e.TPGETANY)
                self.assertEqual(tperrno, 0)
                self.assertEqual(tpurcode, 5)
                self.assertEqual(retbuf["data"]["T_STRING_2_FLD"][0], "Hi Jim")
                if not cd in cds:
                    assertRaises(RuntimeError, msg="cd %d not in the list" % cd)

if __name__ == '__main__':
    unittest.main()

