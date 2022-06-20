#!/bin/bash

#
# @(#) Test 001 - Python tpcall tests
#

export XMEMCK_LOG=`pwd`/xmemck.log
export XMEMCK_OUT=`pwd`/xmemck.out
export TEST_OUT=`pwd`/test.out
(
#
# Load system settings...
#
source ~/ndrx_home
export PYTHONPATH=`pwd`/../libs

TIMES=200
pushd .
rm -rf runtime/log
rm -rf runtime/ULOG*
mkdir runtime/log

cd runtime

export NDRX_SILENT=Y
MACHINE_TYPE=`uname -m`
OS=`uname -s`

runbigmsg=0
msgsizemax=56000

echo "OS=[$OS] matchine=[$MACHINE_TYPE]"
# Added - in front of freebsd, as currently not possible to use large messages with golang...
if [[ ( "X$OS" == "XLinux" || "X$OS" == "X-FreeBSD" ) && ( "X$MACHINE_TYPE" == "Xx86_64" || "X$MACHINE_TYPE" == "Xamd64" ) ]]; then
        echo "Running on linux => Using 1M message buffer"
        # set to 1M + 1024
        msgsizemax=1049600
	ulimit -s 30751
        runbigmsg=1
fi

echo "Message size: $msgsizemax bytes"

xadmin provision -d \
        -vaddubf=test.fd \
        -vtimeout=15 \
        -vinstallQ=n \
        -vmsgsizemax=$msgsizemax

cd conf

. settest1
xadmin down -y


# monitor our test instance, 0myWI5nu -> this is const by xadmin provision
xmemck -m "0myWI5nu|unittest" 2>$XMEMCK_LOG 1>$XMEMCK_OUT &
MEMCK_PID=$!
echo "Memck pid = $MEMCK_PID"

# So we are in runtime directory
cd ../bin
# Be on safe side...
unset NDRX_CCTAG 
xadmin start -y
xadmin psc
#
# Generic exit function
#
function go_out {
    echo "Test exiting with: $1"
    xadmin stop -y
    xadmin down -y
    kill -9 $MEMCK_PID

    popd 2>/dev/null
    exit $1
}

################################################################################
echo "Running UBF buffer test"
################################################################################

python3 -m unittest client-ubf-buffer.py

RET=$?

if [ $RET != 0 ]; then
    echo "client-ubf-buffer.py failed"
    go_out -1
fi

################################################################################
echo "Running UBF callinfo test"
################################################################################

python3 -m unittest client-ubf-callinfo.py

RET=$?

if [ $RET != 0 ]; then
    echo "client-ubf-callinfo.py failed"
    go_out -1
fi

################################################################################
echo "Running UBF boolev test"
################################################################################

python3 -m unittest client-ubf-boolev.py 

RET=$?

if [ $RET != 0 ]; then
    echo "client-ubf-boolev.py failed"
    go_out -1
fi

################################################################################
echo "Running UBF floatev test"
################################################################################

python3 -m unittest client-ubf-floatev.py 

RET=$?

if [ $RET != 0 ]; then
    echo "client-ubf-floatev.py failed"
    go_out -1
fi

################################################################################
echo "Running NULL test"
################################################################################

python3 -m unittest client-null-buffer.py 

RET=$?

if [ $RET != 0 ]; then
    echo "client-null-buffer.py failed"
    go_out -1
fi

################################################################################
echo "Running callinfo test"
################################################################################

# prints error...
NDRX_CCTAG=low python3 -m unittest client-null-callinfo.py

RET=$?

if [ $RET != 0 ]; then
    echo "client-null-callinfo.py failed"
    go_out -1
fi

################################################################################
echo "Running JSON test"
################################################################################

# prints error...
python3 -m unittest client-json-buffer.py

RET=$?

if [ $RET != 0 ]; then
    echo "client-json-buffer.py failed"
    go_out -1
fi

################################################################################
echo "Running VIEW test"
################################################################################

# prints error...
python3 -m unittest client-view-buffer.py

RET=$?

if [ $RET != 0 ]; then
    echo "client-view-buffer.py failed"
    go_out -1
fi

################################################################################
echo "Running VIEW bad len"
################################################################################

# prints error...
NDRX_CCTAG=low python3 -m unittest client-view-badbuf.py

RET=$?

if [ $RET != 0 ]; then
    echo "client-view-badbuf.py failed"
    go_out -1
fi

################################################################################
echo "Running VIEW bad occ"
################################################################################

# prints error...
NDRX_CCTAG=low python3 -m unittest client-view-badocc.py 

RET=$?

if [ $RET != 0 ]; then
    echo "client-view-badocc.py failed"
    go_out -1
fi

################################################################################
echo "UBF API checks"
################################################################################

# prints error...
NDRX_CCTAG=low python3 -m unittest client-ubf-api.py

RET=$?

if [ $RET != 0 ]; then
    echo "client-ubf-api.py failed"
    go_out -1
fi

###############################################################################
echo "Check leaks"
###############################################################################

echo "---- Leak info ----" >> $TEST_OUT
cat $XMEMCK_OUT >> $TEST_OUT
echo "-------------------" >> $TEST_OUT
if [ "X`xadmin pmode | grep '#define NDRX_SANITIZE'`" != "X" ]; then
    echo "Sanitizer mode, ignore memck output"
else
    echo "Catch memory leaks..."
    if [ "X`grep '>>> LEAK' $XMEMCK_OUT`" != "X" ]; then
        echo "Memory leak detected!"
        RET=-2
    fi
fi

###############################################################################
echo "Done"
###############################################################################

go_out 0

) > $TEST_OUT 2>&1

