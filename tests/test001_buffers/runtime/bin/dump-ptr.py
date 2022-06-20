
import endurox as e
import json

#
# Support for VIEWS
# Support for PTR, when sending out -> allocate bunch of PTR buffers
# PTR leak? On receiving rsp same buffer?
# When parsed rsp in -> free all ptr. Also free all ptr original buffers.
#
while 1==1:
    tperrno, tpurcode, retbuf = e.tpcall("ECHO", { "data":{
        "T_CHAR_2_FLD": ["X", "Y"],
        "T_CHAR_FLD": [0],
        "T_SHORT_FLD": 1,
        "T_LONG_FLD": 5,
        "T_FLOAT_FLD": 1000.99,
        "T_STRING_FLD": "HELLO INPUT",
        "T_STRING_2_FLD": "HELLO INPUT 2ĀČ",
        "T_UBF_FLD": {"T_SHORT_FLD":99, "T_UBF_FLD":{"T_LONG_2_FLD":1000091}},
# empty occ -> {}
        "T_VIEW_FLD": [{}, {"vname":"UBTESTVIEW2", "data":{"tshort1":5} }],
        "T_PTR_FLD": {"buftype":"UBF", "data":{"T_CHAR_2_FLD":"A"}}
        , "T_PTR_2_FLD": {"buftype":"YOPT"}
    }})

#tperrno, tpurcode, retbuf = e.tpcall("ECHO", { "buftype":"CARRAY", "data":"HELLO WORLD"})
#tperrno, tpurcode, retbuf = e.tpcall("ECHO", { "data":{}})
print(retbuf)

