#!/bin/bash

#
# @(#) Test 006 - Python tpcall tests
#

export TEST_OUT=`pwd`/test.out
export PATH=$PATH:`pwd`/../../scripts
(
#
# Load system settings...
#
source ~/ndrx_home
export PYTHONPATH=`pwd`/../libs

function go_out {
    echo "Test exiting with: $1"
    popd 2>/dev/null
    exit $1
}

function cleanup {
    rm -rf ./tmp 2>/dev/null
    mkdir ./tmp
}
################################################################################
echo "Compiler test001"
################################################################################

cleanup
pushd . 

cd tmp

expyld -m ../src/test001/hello.py -o test1
RET=$?
if [ $RET != 0 ]; then
    echo "test1 failed to compile $RET"
    go_out -1
fi

OUT=`./test1`

RET=$?
if [ $RET != 0 ]; then
    echo "test1 failed to exec $RET"
    go_out -1
fi

expected='Hello main
call 2'

if [ "$OUT" != "$expected" ]; then
    echo "test1 failed: expected [$expected] got [$OUT]"
    go_out 1
fi

popd
################################################################################
echo "Compiler test002 - include packages"
################################################################################
cleanup
pushd . 

cd tmp

# PYTHONPATH is relative to the test directory
PYTHONPATH=../src/test002/libs expyld -m ../src/test002/main.py -o test2 -i lib1 -i lib2
RET=$?
if [ $RET != 0 ]; then
    echo "test2 failed to compile $RET"
    go_out -1
fi

OUT=`./test2`

RET=$?
if [ $RET != 0 ]; then
    echo "test2 failed to exec $RET"
    go_out -1
fi

expected='lib1
lib1.ok
lib2'

if [ "$OUT" != "$expected" ]; then
    echo "test1 failed: expected [$expected] got [$OUT]"
    go_out 1
fi

popd
###############################################################################
echo "Done"
###############################################################################

go_out 0

) > $TEST_OUT 2>&1

