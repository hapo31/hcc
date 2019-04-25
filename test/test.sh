#!/bin/bash

try() {
    expected="$1"
    input="$2"
    ./hcc "$input" > tmp.s
    gcc -o tmp tmp.s

    ./tmp
    actual="$?"

    if [ "$actual" = "$expected" ]; then
        echo "$input => $actual"

    else
        echo "$expected expected, but got $actual"
        exit 1
    fi
}

try 0 0
try 42 42
try 24 '12+14-2'
try 10 '5 + 4 + 3 - 2'
try 20 '(1 + 4) * 4'
try 3 '3 * 3 / 3'

echo OK
