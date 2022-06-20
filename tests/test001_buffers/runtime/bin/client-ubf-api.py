import unittest
import endurox as e
import exutils as u

class TestUbf(unittest.TestCase):

    #
    # Test boolean expersions
    #
    def test_ubf_util(self):
        e.Bprint({"data":{"T_STRING_FLD":"HELLO SINGLE TEST"}})
        w = u.NdrxStopwatch()
        while w.get_delta_sec() < u.test_duratation():
            self.assertEqual(e.Bfldtype(e.Bfldid("T_STRING_FLD")), e.BFLD_STRING)
            self.assertNotEqual(e.Bmkfldid(e.BFLD_STRING, 99), 99)
            self.assertEqual(e.Bfldno(e.Bmkfldid(e.BFLD_STRING, 99)), 99)
            self.assertEqual(e.Bfname(e.Bfldid("T_STRING_FLD")), "T_STRING_FLD")
            self.assertEqual(e.Bboolev({"data":{"T_STRING_FLD":"HELLO"}}, "T_STRING_FLD=='HELLO'"), True)
            self.assertEqual(e.Bfloatev({"data":{"T_STRING_FLD":"127"}}, "T_STRING_FLD"), 127.0)
            # write expr to file
            f = open("%s/tmp/pr_tmp" % e.tuxgetenv("NDRX_APPHOME"), "w")
            e.Bboolpr("1==1 && 2==2", f)
            f.close()
            f = open("%s/tmp/pr_tmp" % e.tuxgetenv("NDRX_APPHOME"), "r")
            exp = f.read()
            self.assertEqual(exp, "((1 == 1) && (2 == 2))\n")
            f.close()
            # read buffers...
            f = open("%s/tmp/buf_tmp" % e.tuxgetenv("NDRX_APPHOME"), "w")
            e.Bfprint({"data":{"T_STRING_FLD":"HELLO_WORLD", "T_LONG_FLD":777}}, f)
            f.close()

            f = open("%s/tmp/buf_tmp" % e.tuxgetenv("NDRX_APPHOME"), "r")
            buf = e.Bextread(f)
            f.close()
            self.assertEqual(buf, {'buftype': 'UBF', "data":{"T_STRING_FLD":["HELLO_WORLD"], "T_LONG_FLD":[777]}})
            

if __name__ == '__main__':
    unittest.main()
