import unittest
import endurox as e
import exutils as u
import subprocess
import time

class TestTpexit(unittest.TestCase):


    # test pid restarted
    def test_tpexit(self):
        p1 = subprocess.run("(xadmin ppm | grep server | awk '{print $3}')2>/dev/null", shell=True)
        e.tpcall("RESTART", {})
        time.sleep( 15 )
        p2 = subprocess.run("(xadmin ppm | grep server | awk '{print $3}')2>/dev/null", shell=True)
        self.assertNotEqual(p1, p2)

if __name__ == '__main__':
    unittest.main()

