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
xmemck -d5 -m "0myWI5nu|unittest" 2>$XMEMCK_LOG 1>$XMEMCK_OUT &
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
echo "Running tpcall test"
################################################################################

python3 -m unittest tpcall.py

RET=$?

if [ $RET != 0 ]; then
    echo "tpcall.py failed"
    go_out -1
fi

################################################################################
echo "Running tpforward test"
################################################################################

python3 -m unittest tpforward.py

RET=$?

if [ $RET != 0 ]; then
    echo "tpforward.py failed"
    go_out -1
fi


################################################################################
echo "Running tppost test"
################################################################################

python3 -m unittest tppost.py

RET=$?

if [ $RET != 0 ]; then
    echo "tppost.py failed"
    go_out -1
fi

################################################################################
echo "Running tpacall test"
################################################################################

python3 -m unittest tpacall.py

RET=$?

if [ $RET != 0 ]; then
    echo "tpacall.py failed"
    go_out -1
fi

################################################################################
echo "Running tpconnect test"
################################################################################

python3 -m unittest tpconnect.py

RET=$?

if [ $RET != 0 ]; then
    echo "tpconnect.py failed"
    go_out -1
fi

################################################################################
echo "Running tpnotify test"
################################################################################

python3 -m unittest tpnotify.py

RET=$?

if [ $RET != 0 ]; then
    echo "tpnotify.py failed"
    go_out -1
fi

################################################################################
echo "Running tpbroadcast test"
################################################################################

python3 -m unittest tpbroadcast.py

RET=$?

if [ $RET != 0 ]; then
    echo "tpbroadcast.py failed"
    go_out -1
fi


################################################################################
echo "Running tpchkunsol test"
################################################################################

python3 -m unittest tpchkunsol.py

RET=$?

if [ $RET != 0 ]; then
    echo "tpchkunsol.py failed"
    go_out -1
fi

################################################################################
echo "Running tpcancel test"
################################################################################

python3 -m unittest tpcancel.py

RET=$?

if [ $RET != 0 ]; then
    echo "tpcancel.py failed"
    go_out -1
fi

################################################################################
echo "Running server contexting test"
################################################################################

python3 -m unittest tpcall_srvctx.py

RET=$?

if [ $RET != 0 ]; then
    echo "tpcall_srvctx.py failed"
    go_out -1
fi

################################################################################
echo "Running ATMI contexting"
################################################################################

python3 -m unittest ctxt.py

RET=$?

if [ $RET != 0 ]; then
    echo "ctxt.py failed"
    go_out -1
fi

################################################################################
echo "Running ATMI time-out"
################################################################################

python3 -m unittest tout.py

RET=$?

if [ $RET != 0 ]; then
    echo "tout.py failed"
    go_out -1
fi

################################################################################
echo "Running priority settings"
################################################################################

python3 -m unittest prio.py

RET=$?

if [ $RET != 0 ]; then
    echo "prio.py failed"
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

