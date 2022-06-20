import unittest
import endurox as e
import exutils as u

class TestTpencrypt(unittest.TestCase):


    # Test data encryption
    def test_tpencrypt_ok(self):
        w = u.NdrxStopwatch()
        while w.get_delta_sec() < u.test_duratation():
            
            # binary data:
            buf=e.tpencrypt(b'\x00\x01\xff')
            self.assertNotEqual(buf, b'\x00\x01\xff')
            buf_org=e.tpdecrypt(buf)
            self.assertEqual(buf_org, b'\x00\x01\xff')

            # string based:
            buf=e.tpencrypt("HELLO WORLD")
            self.assertNotEqual(buf, "HELLO WORLD")
            buf_org=e.tpdecrypt(buf)
            self.assertEqual(buf_org, "HELLO WORLD")
            

if __name__ == '__main__':
    unittest.main()

