import unittest
import endurox as e
import exutils as u

class TestTpgetctxt(unittest.TestCase):

    # Test contexting...
    def test_tpgetctxt_cd(self):

        ret, _ = e.tpgetctxt()

        self.assertEqual(ret, e.TPNULLCONTEXT)

        w = u.NdrxStopwatch()
        while w.get_delta_sec() < u.test_duratation():


            t1 = e.tpnewctxt(False, False)
            e.tpsetctxt(t1)
            cd1 = e.tpacall("OKSVC", { "data":{"T_STRING_FLD":"Hi Jim1"}})
            ret, t1 = e.tpgetctxt()
            self.assertEqual(ret, e.TPMULTICONTEXTS)

            t2 = e.tpnewctxt(False, False)
            e.tpsetctxt(t2)
            cd2 = e.tpacall("OKSVC", { "data":{"T_STRING_FLD":"Hi Jim2"}})
            ret, t2 = e.tpgetctxt()

            t3 = e.tpnewctxt(False, False)
            e.tpsetctxt(t3)
            cd3 = e.tpacall("OKSVC", { "data":{"T_STRING_FLD":"Hi Jim3"}})
            ret, t3 = e.tpgetctxt()

            e.tpsetctxt(t2)
            tperrno, tpurcode, retbuf, cd = e.tpgetrply(cd2)
            self.assertEqual(cd, cd)
            self.assertEqual(tperrno, 0)
            self.assertEqual(tpurcode, 5)
            self.assertEqual(retbuf["data"]["T_STRING_2_FLD"][0], "Hi Jim2")
            e.tpterm();
            ret, t22 = e.tpgetctxt()

            e.tpsetctxt(t3)
            tperrno, tpurcode, retbuf, cd = e.tpgetrply(cd3)
            self.assertEqual(cd, cd)
            self.assertEqual(tperrno, 0)
            self.assertEqual(tpurcode, 5)
            self.assertEqual(retbuf["data"]["T_STRING_2_FLD"][0], "Hi Jim3")
            e.tpterm();
            ret, t33 = e.tpgetctxt()

            e.tpsetctxt(t1)
            tperrno, tpurcode, retbuf, cd = e.tpgetrply(cd1)
            self.assertEqual(cd, cd)
            self.assertEqual(tperrno, 0)
            self.assertEqual(tpurcode, 5)
            self.assertEqual(retbuf["data"]["T_STRING_2_FLD"][0], "Hi Jim1")
            e.tpterm();
            ret, t11 = e.tpgetctxt()

            e.tpfreectxt(t11)
            e.tpfreectxt(t22)
            e.tpfreectxt(t33)


if __name__ == '__main__':
    unittest.main()

