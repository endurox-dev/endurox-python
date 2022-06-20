import unittest
import endurox as e
import subprocess
import time

class TestTpperiodcb(unittest.TestCase):

    # periodic callback
    def test_tpperiodcb(self):

        time.sleep( 10 )

        # get first
        tperrno, _, retbuf = e.tpcall("PERIODSTATS", {})
        self.assertEqual(tperrno, 0)
        # it shall count to 4...
        self.assertGreater(retbuf["data"]["T_SHORT_FLD"][0], 4)

        tperrno, _, retbuf = e.tpcall("PERIOD_OFF", {})
        self.assertEqual(tperrno, 0)

        # If off... does not change the stats...
        tperrno, _, retbuf1 = e.tpcall("PERIODSTATS", {})
        self.assertEqual(tperrno, 0)
        time.sleep( 10 )
        tperrno, _, retbuf2 = e.tpcall("PERIODSTATS", {})
        self.assertEqual(retbuf1, retbuf2)

        # restart happens, as callback returns failure
        p1 = subprocess.run("(xadmin ppm | grep b4poll | awk '{print $3}')2>/dev/null", shell=True)
        tperrno, _, retbuf1 = e.tpcall("PERIOD_ON2", {})
        self.assertEqual(tperrno, 0)
        # wait for respawn...
        time.sleep( 15 )
        p2 = subprocess.run("(xadmin ppm | grep b4poll | awk '{print $3}')2>/dev/null", shell=True)
        # pid have changed
        self.assertNotEqual(p1, p2)

if __name__ == '__main__':
    unittest.main()

