#!/bin/bash

#
# @(#) Test 006 - Python tpcall tests
#

export TEST_OUT=`pwd`/test.out
export PATH=`pwd`/../../scripts:$PATH
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
echo ">>> Compiler test001"
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
echo ">>> Compiler test002 - include packages"
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
    echo "test2 failed: expected [$expected] got [$OUT]"
    go_out 1
fi

popd

################################################################################
echo ">>> Compiler test003_resfiles - resfile warning"
################################################################################
cleanup
pushd .

cd tmp

# PYTHONPATH is relative to the test directory
PYTHONPATH=../src/test003_resfiles/libs expyld -m ../src/test003_resfiles/main.py -o test3 -i lib1
RET=$?
if [ $RET == 0 ]; then
    echo "test3 comp must fail but did not"
    go_out -1
fi

# Ignore error...
PYTHONPATH=../src/test003_resfiles/libs expyld -m ../src/test003_resfiles/main.py -o test3 -i lib1 -n

OUT=`./test3`

RET=$?
if [ $RET != 0 ]; then
    echo "./test3 failed to exec $RET"
    go_out -1
fi

expected='lib1'

if [ "$OUT" != "$expected" ]; then
    echo "./test3 failed: expected [$expected] got [$OUT]"
    go_out 1
fi

popd

################################################################################
echo ">>> Compiler test004_syntaxerr - syntax error"
################################################################################
cleanup
pushd .

cd tmp

expyld -m ../src/test004_syntaxerr/main.py -o test4
RET=$?
if [ $RET == 0 ]; then
    echo "test4 must fail but did not"
    go_out -1
fi

popd


################################################################################
echo ">>> Compiler test5_nonzero (exit code 1)"
################################################################################
cleanup
pushd .

cd tmp

expyld -m ../src/test005_exitcode/nonzero.py -o test5_nonzero
RET=$?
if [ $RET != 0 ]; then
    echo "test5_nonzero to compile $RET"
    go_out -1
fi

OUT=`./test5_nonzero`

RET=$?
if [ $RET !=1 ]; then
    echo "test5_nonzero failed to exec $RET"
    go_out -1
fi

echo ">>> Compiler test5_except (exception test) "

expyld -m ../src/test005_exitcode/except.py -o test5_except
RET=$?
if [ $RET != 0 ]; then
    echo "test5_nonzero to compile $RET"
    go_out -1
fi

OUT=`./test5_except 2>&1`

RET=$?
if [ $RET == 0  ]; then
    echo "test5_except must fail, but did not"
    go_out -1
fi

echo "*** Trace back ***"
echo $OUT
echo "*** Trace back, END ***"

if [[ "$OUT" != *"line 3"* ]] ;then
    echo "Stacktrace expected, but output did not contain it.."
    go_out -1
fi

popd

################################################################################
echo ">>> Compiler test006_modinc"
################################################################################
cleanup
pushd .

cd tmp

PYTHONPATH=../src/test006_modinc/libs  expyld -m ../src/test006_modinc/main.py -o test006 -i some_mod -i some_mod
RET=$?
if [ $RET != 0 ]; then
    echo "test006 failed to compile $RET"
    go_out -1
fi

OUT=`./test006`

RET=$?

if [ $RET != 0 ]; then
    echo "test006 failed to exec $RET"
    go_out -1
fi

expected='func1 ok'

if [ "$OUT" != "$expected" ]; then
    echo "test006 failed: expected [$expected] got [$OUT]"
    go_out 1
fi

popd


################################################################################
echo ">>> Compiler test007_submod"
################################################################################
cleanup
pushd .

cd tmp

PYTHONPATH=../src/test007_submod/libs  expyld -m ../src/test007_submod/main.py -o test007_1 -i test_mod.other_mod -i test_mod.some_mod -k
RET=$?
if [ $RET != 0 ]; then
    echo "test007_1 failed to compile $RET"
    go_out -1
fi

OUT=`./test007_1`

RET=$?

if [ $RET != 0 ]; then
    echo "test007_1 failed to exec $RET"
    go_out -1
fi

expected='other
func1 ok'

if [ "$OUT" != "$expected" ]; then
    echo "test007_1 failed: expected [$expected] got [$OUT]"
    go_out 1
fi

popd

###############################################################################
echo ">>> Done"
###############################################################################

go_out 0

) > $TEST_OUT 2>&1

