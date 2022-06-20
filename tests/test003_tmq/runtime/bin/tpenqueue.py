import unittest
import endurox as e
import exutils as u

class TestTpenqueue(unittest.TestCase):

    # enqueue something and dequeue
    def test_tpenqueue(self):
        e.tpopen()
        w = u.NdrxStopwatch()
        while w.get_delta_sec() < u.test_duratation():
            e.tpbegin(60, 0)
            qctl = e.tpenqueue("SAMPLESPACE", "TESTQ", e.TPQCTL(), {"data":"SOME DATA"})
            e.tpcommit(0)
            e.tpbegin(60, 0)
            qctl, retbuf = e.tpdequeue("SAMPLESPACE", "TESTQ", e.TPQCTL())
            e.tpcommit(0);
            self.assertEqual(retbuf["data"], "SOME DATA")

        e.tpclose()
    
    def test_tpenqueue_susp(self):
        e.tpopen()
        w = u.NdrxStopwatch()
        while w.get_delta_sec() < u.test_duratation():

            e.tpbegin(60, 0)
            qctl = e.tpenqueue("SAMPLESPACE", "TESTQ", e.TPQCTL(), {"data":"SOME DATA"})
            t1 = e.tpsuspend();

            e.tpbegin(60, 0)
            qctl = e.tpenqueue("SAMPLESPACE", "TESTQ", e.TPQCTL(), {"data":"SOME DATA2"})
            e.tpcommit()

            e.tpbegin(60, 0)
            qctl, retbuf = e.tpdequeue("SAMPLESPACE", "TESTQ", e.TPQCTL())
            e.tpcommit(0);
            self.assertEqual(retbuf["data"], "SOME DATA2")

            e.tpresume(t1)
            e.tpcommit();

            qctl, retbuf = e.tpdequeue("SAMPLESPACE", "TESTQ", e.TPQCTL())
            self.assertEqual(retbuf["data"], "SOME DATA")

        e.tpclose()

    def test_tpenqueue_nomsg(self):
        e.tpopen()
        log = u.NdrxLogConfig()
        log.set_lev(e.log_always)
        w = u.NdrxStopwatch()
        while w.get_delta_sec() < u.test_duratation():
            try:
                qctl, retbuf = e.tpdequeue("SAMPLESPACE", "TESTQ", e.TPQCTL())
            except e.QmException as ex:
                self.assertEqual(ex.code,e.QMENOMSG)
            else:
                self.assertEqual(True,False)
        log.restore()

    # enq/deq by corrid
    def test_tpenqueue_corrid(self):
        e.tpopen()
        w = u.NdrxStopwatch()
        while w.get_delta_sec() < u.test_duratation():

            qctl = e.TPQCTL()

            qctl.corrid=b'\x01\x02'
            qctl.flags=e.TPQCORRID
            qctl1 = e.tpenqueue("SAMPLESPACE", "TESTQ", qctl, {"data":"SOME DATA 1"})

            qctl = e.TPQCTL()
            qctl.flags=e.TPQCORRID
            qctl.corrid=b'\x01\x02'
            qctl2 = e.tpenqueue("SAMPLESPACE", "TESTQ", qctl, {"data":"SOME DATA 2"})

            qctl = e.TPQCTL()
            qctl.flags=e.TPQCORRID
            qctl.corrid=b'\x01\x03'
            qctl.msgid=b'\x00\x03\x55'
            qctl.failurequeue='failure'
            qctl.replyqueue='reply'
            qctl3 = e.tpenqueue("SAMPLESPACE", "TESTQ", qctl, {"data":"SOME DATA 22"})

            # get by corrid.
            qctl = e.TPQCTL()
            qctl.flags=e.TPQGETBYCORRID
            qctl.corrid=b'\x01\x03'
            qctl, retbuf = e.tpdequeue("SAMPLESPACE", "TESTQ", qctl)
            self.assertEqual(retbuf["data"], "SOME DATA 22")
            self.assertEqual(qctl.failurequeue, "failure")
            self.assertEqual(qctl.replyqueue, "reply")
            self.assertNotEqual(qctl.msgid, b'\x00\x03\x55')

            # get by msgid
            qctl2.flags=e.TPQGETBYMSGID
            qctl, retbuf = e.tpdequeue("SAMPLESPACE", "TESTQ", qctl2)
            self.assertEqual(retbuf["data"], "SOME DATA 2")

            # again by corrid...
            qctl1.flags=e.TPQGETBYCORRID
            qctl, retbuf = e.tpdequeue("SAMPLESPACE", "TESTQ", qctl2)
            self.assertEqual(retbuf["data"], "SOME DATA 1")

            # queue must be empty...
            log = u.NdrxLogConfig()
            log.set_lev(e.log_always)
            try:
                qctl, retbuf = e.tpdequeue("SAMPLESPACE", "TESTQ", e.TPQCTL())
            except e.QmException as ex:
                self.assertEqual(ex.code,e.QMENOMSG)
            else:
                self.assertEqual(True,False)
            log.restore()

        e.tpclose()

    # enq/deq by corrid
    def test_tpenqueue_tpabort(self):
        e.tpopen()
        w = u.NdrxStopwatch()
        while w.get_delta_sec() < u.test_duratation():

            qctl = e.TPQCTL()
            qctl1 = e.tpenqueue("SAMPLESPACE", "TESTQ", qctl, { "buftype":"VIEW", "subtype":"UBTESTVIEW2", "data":{
                "tshort1": 100
                , "tlong1": 200000
                , "tchar1": "Y"
                , "tfloat1": 999.11
                , "tdouble1": 8888.99
                , "tstring1": "HELLO WORLD"
                , "tcarray1": [b'\x00\x03\x05\x07', b'\x00\x00\x05\x07'],
                }})

            e.tpbegin(60)
            qctl, retbuf = e.tpdequeue("SAMPLESPACE", "TESTQ", e.TPQCTL())
            self.assertEqual(retbuf["data"]["tstring1"][0], "HELLO WORLD")
            e.tpabort()

            qctl, retbuf = e.tpdequeue("SAMPLESPACE", "TESTQ", e.TPQCTL())
            self.assertEqual(retbuf["data"]["tstring1"][0], "HELLO WORLD")
        
    
if __name__ == '__main__':
    unittest.main()

