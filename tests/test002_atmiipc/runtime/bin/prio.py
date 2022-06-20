import unittest
import endurox as e

class TestPrio(unittest.TestCase):

    def test_prio(self):
        e.tpinit()
        e.tpsprio(40, e.TPABSOLUTE)
        e.tpcall("OKSVC", {})
        self.assertEqual(e.tpgprio(), 40)
        e.tpterm()

if __name__ == '__main__':
    unittest.main()

