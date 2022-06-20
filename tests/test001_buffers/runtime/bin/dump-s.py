
import endurox as e
import json

tperrno, tpurcode, retbuf = e.tpcall("ECHO", { "data":"HELLO WORLD" })

print(retbuf)

