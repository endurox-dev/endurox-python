#!/bin/bash

#
# @(#) Test 006 - Python tpcall tests
#

export TEST_OUT=`pwd`/test.out
(
#
# Load system settings...
#
source ~/ndrx_home
export PYTHONPATH=`pwd`/../libs
export PATH=`pwd`/../../scripts:$PATH

function go_out {
    echo "Test exiting with: $1"

    if [ $1 != 0 ]; then
        popd 2>/dev/null
    fi

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

PYTHONPATH=../src/test007_submod/libs  expyld -m ../src/test007_submod/main.py -o test007_1 -i test_mod.other_mod -i test_mod.some_mod -i test_mod.x
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

expected='test_mod.other_mod
other
func1 ok
x_func'

if [ "$OUT" != "$expected" ]; then
    echo "test007_1 failed: expected [$expected] got [$OUT]"
    go_out 1
fi

popd


################################################################################
echo ">>> Compiler test007_submod, recursive, pkg precendence"
################################################################################
cleanup
pushd .

cd tmp

PYTHONPATH=../src/test007_submod/libs  expyld -m ../src/test007_submod/main.py -o test007_2 -i test_mod
RET=$?
if [ $RET != 0 ]; then
    echo "test007_2 failed to compile $RET"
    go_out -1
fi

OUT=`./test007_2`

RET=$?

if [ $RET != 0 ]; then
    echo "test007_2 failed to exec $RET"
    go_out -1
fi

expected='test_mod.other_mod
other
func1 ok
x_func'

if [ "$OUT" != "$expected" ]; then
    echo "test007_2 failed: expected [$expected] got [$OUT]"
    go_out 1
fi

popd


################################################################################
echo ">>> Compiler test008_pkgprec, package precendence over the main dir"
################################################################################
cleanup
pushd .

cd tmp

PYTHONPATH=../src/test008_pkgprec  expyld -m ../src/test008_pkgprec/main.py -o test008 -i some_mod
RET=$?
if [ $RET != 0 ]; then
    echo "test008 failed to compile $RET"
    go_out -1
fi

OUT=`./test008`

RET=$?

if [ $RET != 0 ]; then
    echo "test008 failed to exec: $RET"
    go_out -1
fi

expected='PKG-HELLO
MOD-OTHER-HELLO'

if [ "$OUT" != "$expected" ]; then
    echo "test007_2 failed: expected [$expected] got [$OUT]"
    go_out 1
fi

popd

################################################################################
echo ">>> Compiler test009_linkorder, link order"
################################################################################
cleanup
pushd .

cd tmp
rm ../src/test009_linkorder/* 2>/dev/null
rm -rf ../src/test009_linkorder/__pycache__ 2>/dev/null


pushd .
cd ../src/test009_linkorder

# attempt to link from pyc in the pkg dir
echo "print('hello_pyc')" > main.py
/usr/bin/env python3 -m compileall .
mv ./__pycache__/* main.pyc

# attempt to use cache
echo "print('hello_pyc_cache')" > main.py
/usr/bin/env python3 -m compileall .

# build from sources.
echo "print('hello_py')" > main.py

popd

# first comes from sources
PYTHONPATH=../src/test009_linkorder  expyld -m ../src/test009_linkorder/main -o test009_1
RET=$?
if [ $RET != 0 ]; then
    echo "test009_1 failed to compile $RET"
    go_out -1
fi

OUT=`./test009_1`

RET=$?

if [ $RET != 0 ]; then
    echo "test009_1 failed to exec: $RET"
    go_out -1
fi

expected='hello_py'

if [ "$OUT" != "$expected" ]; then
    echo "hello_py failed: expected [$expected] got [$OUT]"
    go_out 1
fi

# remove sources version
rm ../src/test009_linkorder/main.py
PYTHONPATH=../src/test009_linkorder  expyld -m ../src/test009_linkorder/main -o test009_2
RET=$?
if [ $RET != 0 ]; then
    echo "test009_2 failed to compile $RET"
    go_out -1
fi

OUT=`./test009_2`

RET=$?

if [ $RET != 0 ]; then
    echo "test009_2 failed to exec: $RET"
    go_out -1
fi

expected='hello_pyc'

if [ "$OUT" != "$expected" ]; then
    echo "test009_2 failed: expected [$expected] got [$OUT]"
    go_out 1
fi

# remove pyc version, shall come from cache
rm ../src/test009_linkorder/main.pyc
PYTHONPATH=../src/test009_linkorder  expyld -m ../src/test009_linkorder/main -o test009_3
RET=$?
if [ $RET != 0 ]; then
    echo "test009_3 failed to compile $RET"
    go_out -1
fi

OUT=`./test009_3`

RET=$?

if [ $RET != 0 ]; then
    echo "test009_3 failed to exec: $RET"
    go_out -1
fi

expected='hello_pyc_cache'

if [ "$OUT" != "$expected" ]; then
    echo "test009_3 failed: expected [$expected] got [$OUT]"
    go_out 1
fi

popd

###############################################################################
echo ">>> Done"
###############################################################################

go_out 0

) > $TEST_OUT 2>&1

