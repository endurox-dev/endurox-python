import unittest
import endurox as e
import subprocess
import time

class TestTpb4poll(unittest.TestCase):

    # test pid restarted
    def test_tpb4poll(self):

        # get first
        tperrno, _, retbuf = e.tpcall("B4POLL", {})
        self.assertEqual(tperrno, 0)
        self.assertEqual(retbuf["data"]["T_SHORT_FLD"][0], 1)

        # restart happens, as callback returns failure
        p1 = subprocess.run("(xadmin ppm | grep b4poll | awk '{print $3}')2>/dev/null", shell=True)

        tperrno, _, retbuf = e.tpcall("B4POLL", {})
        self.assertEqual(tperrno, 0)
        self.assertEqual(retbuf["data"]["T_SHORT_FLD"][0], 2)
        # wait for respawn...
        time.sleep( 15 )

        tperrno, _, retbuf = e.tpcall("B4POLL", {})
        self.assertEqual(tperrno, 0)
        self.assertEqual(retbuf["data"]["T_SHORT_FLD"][0], 1)
        p2 = subprocess.run("(xadmin ppm | grep b4poll | awk '{print $3}')2>/dev/null", shell=True)
        
        # pid have changed
        self.assertNotEqual(p1, p2)

if __name__ == '__main__':
    unittest.main()

