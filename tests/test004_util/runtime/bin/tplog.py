import unittest
import endurox as e
import os
import time


#
# Check text exists in file
#
def chk_file(fname, t):
    with open(fname) as f:
        if t in f.read():
            return 1
    return 0

class TestTplog(unittest.TestCase):

    def test_tplog_ok(self):
        # load the env...
        e.tpinit()

        filename = "%s/tplog_ok" % e.tuxgetenv('NDRX_ULOG')
        os.remove(filename) if os.path.exists(filename) else None
        # set logfile output
        e.tplogconfig(e.LOG_FACILITY_TP, e.log_info, None, "TEST", filename)

        e.tplog_error("HELLO ERROR")
        self.assertEqual(chk_file(filename, "HELLO ERROR"), 1)

        e.tplog_always("HELLO ALWAYS")
        self.assertEqual(chk_file(filename, "HELLO ALWAYS"), 1)

        e.tplog_warn("HELLO WARNING")
        self.assertEqual(chk_file(filename, "HELLO WARNING"), 1)

        e.tplog_info("HELLO INFO")
        self.assertEqual(chk_file(filename, "HELLO INFO"), 1)

        e.tplog_debug("HELLO DEBUG")
        self.assertEqual(chk_file(filename, "HELLO DEBUG"), 0)

        e.tplog(e.log_error, "HELLO ERR2")
        self.assertEqual(chk_file(filename, "HELLO ERR2"), 1)

        e.tplogconfig(e.LOG_FACILITY_TP, -1, "tp=5", None, None)
        e.tplog_debug("HELLO DEBUG")
        self.assertEqual(chk_file(filename, "HELLO DEBUG"), 1)

        e.tpterm()

    # request logging...
    def test_tplog_reqfile(self):
        e.tpinit()
        filename_def = "%s/tplog_req_def" % e.tuxgetenv('NDRX_ULOG')
        filename_th = "%s/tplog_req_th" % e.tuxgetenv('NDRX_ULOG')
        filename = "%s/tplog_req" % e.tuxgetenv('NDRX_ULOG')
        os.remove(filename) if os.path.exists(filename) else None
        os.remove(filename_def) if os.path.exists(filename_def) else None
        os.remove(filename_th) if os.path.exists(filename_th) else None

        e.tplogconfig(e.LOG_FACILITY_TP, e.log_info, "file=%s" % filename_def, "TEST", None)

        out = e.tplogsetreqfile({"data":{"EX_NREQLOGFILE":filename}}, None, None)
        self.assertEqual(e.tploggetreqfile(), filename)
        self.assertEqual(e.tploggetbufreqfile(out), filename)
        out = e.tplogdelbufreqfile(out)

        with self.assertRaises(e.AtmiException):
            e.tploggetbufreqfile(out)

        e.tplog_error("HELLO ERROR")
        self.assertEqual(chk_file(filename, "HELLO ERROR"), 1)

        e.tplogclosereqfile()
        self.assertEqual(e.tploggetreqfile(), "")
        
        e.tplogsetreqfile_direct(filename)
        e.tplog_error("HELLO ERROR2")
        self.assertEqual(chk_file(filename, "HELLO ERROR2"), 1)

        out = e.tplogsetreqfile(None, filename, None)
        self.assertEqual(out["buftype"], "NULL")
        e.tplogclosereqfile()


        # set thread logger
        e.tplogconfig(e.LOG_FACILITY_TP_THREAD, e.log_info, "file=%s" % filename_th, "TEST", None)
        e.tplog_error("HELLO TH")
        self.assertEqual(chk_file(filename_th, "HELLO TH"), 1)
        e.tplogclosethread()

        e.tplog_error("LOGGER BACK")
        self.assertEqual(chk_file(filename_def, "LOGGER BACK"), 1)

        # log some stuff...
        e.tpterm()
    
    # test dump commands...
    def test_tplog_dump(self):
        e.tpinit()
        filename = "%s/tplog_dump" % e.tuxgetenv('NDRX_ULOG')
        os.remove(filename) if os.path.exists(filename) else None
        e.tplogconfig(e.LOG_FACILITY_TP, e.log_info, "file=%s" % filename, "TEST", None)

        e.tplogdump(e.log_error, "HELLO DUMP", b'\x00\x01\x02')
        self.assertEqual(chk_file(filename, "HELLO DUMP"), 1)
        self.assertEqual(chk_file(filename, "00 01 02"), 1)

        e.tplogdumpdiff(e.log_error, "HELLO DUMP2", b'\xff\x01\x02', b'\xff\x02\04\05')
        self.assertEqual(chk_file(filename, "HELLO DUMP2"), 1)
        self.assertEqual(chk_file(filename, "ff 01 02"), 1)
        self.assertEqual(chk_file(filename, "ff 02 04"), 1)
        self.assertEqual(chk_file(filename, "ff 02 04 05"), 0)

        e.tpterm()

    # test fileno...
    def test_tplog_dump(self):
        e.tpinit()
        filename = "%s/tplog_fd" % e.tuxgetenv('NDRX_ULOG')
        os.remove(filename) if os.path.exists(filename) else None
        e.tplogconfig(e.LOG_FACILITY_TP, e.log_info, "file=%s" % filename, "TEST", None)
        self.assertEqual(e.tplogfplock(9, 0), None)
        handle = e.tplogfplock()
        fd = e.tplogfpget(handle)
        os.write(fd, b'\x41\x42\x43\x20\x48\x45\x4c\x4c\x4f')
        os.fsync(fd)
        self.assertEqual(chk_file(filename, "ABC HELLO"), 1)
        e.tplogfpunlock(handle)
        e.tpterm()


    def test_tplogprintubf(self):
        e.tpinit()
        filename = "%s/tplog_ubf" % e.tuxgetenv('NDRX_ULOG')
        os.remove(filename) if os.path.exists(filename) else None
        e.tplogconfig(e.LOG_FACILITY_TP, e.log_info, "file=%s" % filename, "TEST", None)

        e.tplogprintubf(e.log_info, "TEST_BUFFER", {"buftype":"UBF", "data":{"T_STRING_FLD":"STRING1", "T_STRING_10_FLD":"HELLO STRING"}})

        self.assertEqual(chk_file(filename, "TEST_BUFFER"), 1)
        self.assertEqual(chk_file(filename, "T_STRING_FLD\tSTRING1"), 1)
        self.assertEqual(chk_file(filename, "T_STRING_10_FLD\tHELLO STRING"), 1)

        e.tpterm()

    def test_tplogqinfo(self):

        # test invalid flags via Nstd error
        try:
            e.tplogqinfo(-1, 16)
        except e.NstdException as ex:
            self.assertEqual(ex.code,e.NEINVAL)
        else:
            self.assertEqual(True,False)
  
        e.tplogconfig(e.LOG_FACILITY_TP|e.TPLOGQI_GET_NDRX, e.log_error, "", None, None)
        info = (e.tplogqinfo(1, e.TPLOGQI_GET_TP) >> 24)
        self.assertEqual(info,e.log_error)

if __name__ == '__main__':
    unittest.main()

