#!/bin/bash

LDM_PROGRAM="./thread_hash"
RJC_PROGRAM="./Master/thread_hash"

LDM_STDOUT="ldm.output"
RJC_STDOUT="rjc.output"

LDM_STDERR="ldm.error"
RJC_STDERR="rjc.error"

TEN_LIST=("./Master/passwords10.txt" "./Master/plain10.txt")
TEN_K_LIST=("./Master/passwords10000.txt" "./Master/plain10000.txt")

echo "Starting Tests"

# Run diff and then say pass or fail. takes 2 args
run_diff() {
    diff "$1" "$2"
    if [ $? -eq 0 ]; then
        echo "Passed"
    else
        echo "Failed"
    fi
}

# Test 1 No Arguments
echo "Test 1: No Arguments"
$LDM_PROGRAM > $LDM_STDOUT 2>$LDM_STDERR
$RJC_PROGRAM > $RJC_STDOUT 2>$RJC_STDERR
run_diff $LDM_STDOUT $RJC_STDOUT
run_diff $LDM_STDERR $RJC_STDERR
rm -f $LDM_STDOUT $RJC_STDOUT $LDM_STDERR $RJC_STDERR

# Test 2 Just Load Shit
echo "Test 2: Load Files"
$LDM_PROGRAM -v -i ${TEN_LIST[0]} -d ${TEN_LIST[1]} -n >/dev/null 2>/dev/null
if [ $? -eq 0 ]; then
    echo "Passed"
else
    echo "Failed"
fi


# Test 3 Load Biggest File
echo "Test 3: Load Biggest Files"
$LDM_PROGRAM -v -i ${TEN_K_LIST[0]} -d ${TEN_K_LIST[1]} -n >/dev/null 2>/dev/null
if [ $? -eq 0 ]; then
    echo "Passed"
else
    echo "Failed"
fi

# Test Valgrind 1
echo "Valgrind Test"
$LDM_PROGRAM -v -i ${TEN_K_LIST[0]} -d ${TEN_K_LIST[1]} -n >/dev/null 2>/dev/null
