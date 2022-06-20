import endurox as e

tperrno, tpurcode, retbuf = e.tpcall("ECHO", { "buftype":"VIEW", "subtype":"UBTESTVIEW2", "data":{
    "tshort1":5
    , "tlong1":100000
    , "tchar1":"J"
    , "tfloat1":9999.9
    , "tdouble1":11119999.9
    , "tstring1":"HELLO VIEW"
    , "tcarray1":[b'\x00\x00', b'\x01\x01']
}})

print(retbuf)

