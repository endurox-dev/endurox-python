import endurox as e
import json

tperrno, tpurcode, retbuf = e.tpcall("ECHO", { "data":{
    # 3x occs:
    "T_CHAR_FLD": ["X", "Y", 0]
    , "T_SHORT_FLD": 3200
    , "T_LONG_FLD": 99999111
    , "T_FLOAT_FLD": 1000.99
    , "T_DOUBLE_FLD": 1000111.99
    , "T_STRING_FLD": "HELLO INPUT"
    # contains sub-ubf buffer, which againt contains sub-buffer
    , "T_UBF_FLD": {"T_SHORT_FLD":99, "T_UBF_FLD":{"T_LONG_2_FLD":1000091}}
    # at occ 0 EMPTY view is used
    , "T_VIEW_FLD": [ {}, {"vname":"UBTESTVIEW2", "data":{
                    "tshort1":5
                    , "tlong1":100000
                    , "tchar1":"J"
                    , "tfloat1":9999.9
                    , "tdouble1":11119999.9
                    , "tstring1":"HELLO VIEW"
                    , "tcarray1":[b'\x00\x00', b'\x01\x01'] 
                    }}]
    # contains pointer to STRING buffer:
    , "T_PTR_FLD":{"data":"HELLO WORLD"}
}})

print(retbuf)

