import endurox as e

tperrno, tpurcode, retbuf = e.tpcall("ECHO", {})

print(retbuf)

