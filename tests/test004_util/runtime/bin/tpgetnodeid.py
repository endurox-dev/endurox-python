import unittest
import endurox as e

class TestTpgetnodeid(unittest.TestCase):


    def test_tpgetnodeid_ok(self):
        self.assertEqual(e.tpgetnodeid(), 1)

if __name__ == '__main__':
    unittest.main()

