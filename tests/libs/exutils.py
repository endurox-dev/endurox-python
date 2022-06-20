#
# Test utils
#

import time
import os
import endurox as e

#
# Stopwatch used by tests
#
class NdrxStopwatch:

    w = 0
    
    #
    # get time spent
    # 
    def get_delta_sec(self):
        return time.time() - self.w

    #
    # reset time
    #
    def reset(self):
        self.w = time.time()

    #
    # Constructor
    #
    def __init__(self):
        self.w = time.time()

#
# Return current test settings
# 
def test_duratation():
    duratation = os.getenv('NDRXPY_TEST_DURATATION') or '30'
    return int(duratation)

#
# Configure logger with new settings
# and give option to restore old settings.
#
class NdrxLogConfig:

    prev_lev = 0

    # Set new level save, prev
    def set_lev(self, lev):
        self.prev_lev = (e.tplogqinfo(1, e.TPLOGQI_GET_NDRX | e.TPLOGQI_EVAL_RETURN) >> 24)
        e.tplogconfig(e.LOG_FACILITY_NDRX, lev, "", "", "")

    # restore original level
    def restore(self):
        e.tplogconfig(e.LOG_FACILITY_NDRX, self.prev_lev, "", "", "")


