import unittest
import endurox as e
import exutils as u

class TestTpcancel(unittest.TestCase):

    # Validate async call by cd
    def test_tapcall_cd(self):
        w = u.NdrxStopwatch()
        while w.get_delta_sec() < u.test_duratation():
            cd = e.tpacall("OKSVC", { "data":{"T_STRING_FLD":"Hi Jim"}})
            e.tpcancel(cd)

if __name__ == '__main__':
    unittest.main()

