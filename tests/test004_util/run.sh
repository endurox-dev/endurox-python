#!/bin/bash

#
# @(#) Test 004 - Enduro/X utility apis
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
echo "Running tpencrypt+tpdecrypt test"
################################################################################

python3 -m unittest tpencrypt.py

RET=$?

if [ $RET != 0 ]; then
    echo "tpencrypt.py failed"
    go_out -1
fi

################################################################################
echo "Running tpexport+tpimport test"
################################################################################

python3 -m unittest tpexport.py

RET=$?

if [ $RET != 0 ]; then
    echo "tpexport.py failed"
    go_out -1
fi

################################################################################
echo "Running tpexit test"
################################################################################

python3 -m unittest tpexit.py

RET=$?

if [ $RET != 0 ]; then
    echo "tpexit.py failed"
    go_out -1
fi

################################################################################
echo "Running tpext_addb4pollcb test"
################################################################################

python3 -m unittest b4pollcl.py

RET=$?

if [ $RET != 0 ]; then
    echo "b4pollcl.py failed"
    go_out -1
fi

################################################################################
echo "Running tpext_addperiodcb test"
################################################################################

python3 -m unittest periodcbcl.py

RET=$?

if [ $RET != 0 ]; then
    echo "periodcbcl.py failed"
    go_out -1
fi

################################################################################
echo "Running fdpoller test"
################################################################################

python3 -m unittest pollerfdcl.py

RET=$?

if [ $RET != 0 ]; then
    echo "pollerfdcl.py failed"
    go_out -1
fi

################################################################################
echo "Running tplog.py test"
################################################################################

python3 -m unittest tplog.py

RET=$?

if [ $RET != 0 ]; then
    echo "tplog.py failed"
    go_out -1
fi

################################################################################
echo "Running tpgetnodeid.py test"
################################################################################

python3 -m unittest tpgetnodeid.py

RET=$?

if [ $RET != 0 ]; then
    echo "tpgetnodeid.py failed"
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

