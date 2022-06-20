import endurox as e

tperrno, tpurcode, retbuf = e.tpcall("ECHO", { "data":"HELLO STRING", "callinfo":{"T_SHORT_FLD":55, "T_STRING_FLD":"HELLO"}})
print(retbuf)

