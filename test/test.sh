#!/bin/bash

color_reset="\x1b[0m"
color_green="\x1b[32m"
color_red="\x1b[31m"

test_count=0
failed_count=0

green() {
    echo -e "${color_green}$1${color_reset}"
}

red() {
    echo -e "${color_red}$1${color_reset}"
}

try() {
    mode="$1"
    expected="$2"
    input="$3"

    if [ $mode = '-i' ]; then
        ./hcc -i "$input" > tmp.s
    else
        ./hcc "$input" > tmp.s
    fi
    gcc -o tmp tmp.s

    ./tmp
    actual="$?"

    test_count=$((test_count + 1))

    echo -n "[$test_count] "
    if [ "$actual" = "$expected" ]; then
        echo -e "$(green "[OK]") $input => $actual"

    else
        echo -e "$(red "[NG]") $input => $actual (expected $expected)"
        mv ./tmp.s ./${test_count}_tmp.s
        failed_count=$((failed_count + 1))
        return 1
    fi
}

try -i 0 '0;'
try -i 42 '42;'
try -i 24 '12+14-2;'
try -i 10 '5 + 4 + 3 - 2;'
try -i 20 '(1 + 4) * 4;'
try -i 3 '3 * 3 / 3;'
try -i 0 '0 / 1;'
try -i 100 '1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1;'
try -i 12 'a=6+6; return a;'
try -i 10 'a = 5; b = a + 5; return b;'
try -i 20 'hoge = 10; fuga = hoge + 10; return fuga;'
try -i 20 'a = 1; b = 1; c = a + b; c = c * 10; return c;'
try -i 5 'return 15 % 10;'
try -i 5 'return 5 % 10;'
try -i 10 'return -15 + (+25);'
try -i 0 'a = -5; b = 5; return a + b;'
try -i 1 'return 5 == 5;'
try -i 0 'return 4 == 3;'
try -i 1 'return 10 != 0;'
try -i 0 'return 20 != 20;'
try -i 1 'return 5 >= 4;'
try -i 0 'return 5 >= 6;'
try -i 1 'return 4 <= 5;'
try -i 0 'return 5 <= 4;'
try -i 1 'return 5 > 4;'
try -i 0 'return 5 > 5;'
try -i 1 'return 4 < 5;'
try -i 0 'return 5 < 4;'
try -i 1 'a = 10; b = 10; return a == b;'
try -i 1 'a = 10; b = 20; return a != b;'
try -i 1 'a = 30; b = 20; return a >= b;'
try -i 1 'a = 10; b = 20; return a <= b;'
try -i 42 'if (0) return 1; return 42;'
try -i 42 'if (0) return 0; else return 42;'
try -i 42 'if (0) return 0; else return 42; return 1919;'
try -i 42 'hoge = 10; fuga = 20; if (hoge > fuga) return 0; else return 42;'
try -i 42 'n = 0; while (n < 42) n = n + 1; return n;'
try -i 0 'n = 42; while (n > 0) n = n - 1; return n;'
try -i 45 'result = 0; for (n = 0; n < 10; n = n + 1) result = result + n; return result;'
try -i 120 '(((((((((((((((1 + 2) + 3) + 4) + 5) + 6) + 7) + 8) + 9) + 10) + 11) + 12) + 13) + 14) + 15));'
try -i 10 'super_long_var_name_wwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwww = 10; return super_long_var_name_wwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwww;'
try -i 10 'x = 0; if(x == 0) { y = x + 10; return y; } else { return x; }'
try -i 20 'x = 1; y = 0; if (x == 0) { y = 10; } else { y = 20; } return y;'
try -f 42 ./test/test_file.c

echo ---------------------------------------------------------------------

if [ "$failed_count" -lt 1 ]; then
    echo -e "$(green '[SUCCESS]') [$test_count/$test_count] tests are passed."
    exit 0
else
    echo -e "$(red '[FAILURE]') [$failed_count/$test_count] tests are failed."
    exit 1
fi
