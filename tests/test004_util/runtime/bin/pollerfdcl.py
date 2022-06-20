import unittest
import endurox as e
import exutils as u
import os
import time


sent=0
handled=0

class TestFdpoll(unittest.TestCase):

    #
    # Get the broadcast feedback
    # 
    def unsol_handler1(self, data):
        global handled
        self.assertEqual(data["data"]["T_LONG_FLD"][0], 101)
        handled+=1
    
    def test_fdpoll_ok(self):
        global sent
        global handled
        path = "/tmp/tmp_py"
        e.tpsetunsol(self.unsol_handler1)
        w = u.NdrxStopwatch()
        while w.get_delta_sec() < u.test_duratation():
            # open the pipe and post 101 there
            outx = os.open(path, os.O_WRONLY)
            os.write(outx, b'\x65')
            os.close(outx)
            e.tpcall("POLLERSYNC", {})
            # well ... some feedback is needed
            # otherwise we will deadlock... here...
            e.tpchkunsol()
            sent+=1
            
        while e.tpchkunsol() > 0:
            # consume all msgs...
            None

        self.assertEqual(sent, handled)

if __name__ == '__main__':
    unittest.main()

