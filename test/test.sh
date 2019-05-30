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

try_with_test_file()
{
    expected="$1"
    input="$2"
    test_func="$3"

    ./hcc $input > tmp.s
    gcc -o tmp tmp.s $test_func
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

rm *_tmp.s

try -i 0 'int main() { return 0; }'
try -i 42 'int main() { return 42; }'
try -i 24 'int main() { return 12+14-2; }'
try -i 10 'int main() { return 5 + 4 + 3 - 2; }'
try -i 20 'int main() { return (1 + 4) * 4; }'
try -i 3 'int main() { return 3 * 3 / 3; }'
try -i 0 'int main() { return 0 / 1; }'
try -i 100 'int main() { return 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1; }'
try -i 12 'int main() { int a; a=6+6; return a; }'
try -i 10 'int main() { int a; int b; a = 5; b = a + 5; return b; }'
try -i 20 'int main() { int hoge; int fuga; hoge = 10; fuga = hoge + 10; return fuga; }'
try -i 20 'int main() { int a; int b; int c; a = 1; b = 1; c = a + b; c = c * 10; return c; }'
try -i 5 'int main() { return 15 % 10; }'
try -i 5 'int main() { return 5 % 10; }'
try -i 10 'int main() { return -15 + (+25); }'
try -i 0 'int main() { int a; int b; a = -5; b = 5; return a + b; }'
try -i 1 'int main() { return 5 == 5; }'
try -i 0 'int main() { return 4 == 3; }'
try -i 1 'int main() { return 10 != 0; }'
try -i 0 'int main() { return 20 != 20; }'
try -i 1 'int main() { return 5 >= 4; }'
try -i 0 'int main() { return 5 >= 6; }'
try -i 1 'int main() { return 4 <= 5; }'
try -i 0 'int main() { return 5 <= 4; }'
try -i 1 'int main() { return 5 > 4; }'
try -i 0 'int main() { return 5 > 5; }'
try -i 1 'int main() { return 4 < 5; }'
try -i 0 'int main() { return 5 < 4; }'
try -i 1 'int main() { int a; int b; a = 10; b = 10; return a == b; }'
try -i 1 'int main() { int a; int b; a = 10; b = 20; return a != b; }'
try -i 1 'int main() { int a; int b; a = 30; b = 20; return a >= b; }'
try -i 1 'int main() { int a; int b; a = 10; b = 20; return a <= b; }'
try -i 42 'int main() { if (0) return 1; return 42; }'
try -i 42 'int main() { if (0) return 0; else return 42; }'
try -i 42 'int main() { if (0) return 0; else return 42; return 1919; }'
try -i 42 'int main() { int hoge; int fuga; hoge = 10; fuga = 20; if (hoge > fuga) return 0; else return 42; }'
try -i 42 'int main() { int n; n = 0; while (n < 42) n = n + 1; return n; }'
try -i 0 'int main() { int n; n = 42; while (n > 0) n = n - 1; return n; }'
try -i 45 'int main() { int result; int n; result = 0; for (n = 0; n < 10; n = n + 1) result = result + n; return result; }'
try -i 120 'int main() { (((((((((((((((1 + 2) + 3) + 4) + 5) + 6) + 7) + 8) + 9) + 10) + 11) + 12) + 13) + 14) + 15)); }'
try -i 10 'int main() { int super_long_var_name_wwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwww; super_long_var_name_wwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwww = 10; return super_long_var_name_wwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwww; }'
try -i 10 'int main() { int x; int y; x = 0; if(x == 0) { y = x + 10; return y; } else { return x; } }'
try -i 20 'int main() { int x; int y; x = 1; y = 0; if (x == 0) { y = 10; } else { y = 20; } return y; }'
try -i 10 'int main() { int x; int y; x = 0; y = 1; if (x == 0) { y = 10; } else { y = 20; } return y; }'
try -i 6 'int main() { int x; int y; int z; int a; x = 1; y = 2; z = 3; a = x + y + z; return a; }'
try -i 1 'int main() { int x; x = 1; if (x == 1) { return 1; } return 0; }'
try -i 10 'int main() { int x; int y; x = y = 5; return x + y; }'
try -i 50 'int main() { int x; int y; x = 0; y = 0; while(y < 5) { x = x + 10; y = y + 1; } return x; }'
try -i 10 'int f(int x) { return x + 5; } int main(){ int n; n = f(5); return n; }'
try -i 120 'int f(int x) { if(x == 0) return 1; return x * f(x-1); } int main(){ return f(5); }'
try -i 1 'int f(int x) { if(x == 0) return 1; return x * f(x-1); } int main(){ return f(1); }'
try -i 10 'int f(int x,int y,int z) { return x + y + z; } int main() { int n; n = f(1,2,3); return n + 4; }'
try -i 7 'int f(int n1,int n2,int n3,int n4,int n5,int n6,int n7) { return n1 + n2 + n3 + n4 + n5 + n6 + n7; } int main() { return f(1,1,1,1,1,1,1); } '
try -i 1 'int f(int x) { return x * 10; } int main() { int n; n = f(10); return n == 100; }'
try -i 0 'int main() { int* n; n = 0; return 0; }'
try -i 1 'int main() { int *x; int y; x = &y; *x = 1; return y; }'
try -i 1 'int main() { int x; return sizeof(x) == 4; }'
try -i 1 'int main() { int* x; return sizeof(x) == 8; }'
try -f 42 ./test/test_file0.c
try -f 25 ./test/test_file1.c
try_with_test_file 0 ./test/test_file2.c ./test/test_funcs.c
try_with_test_file 6 ./test/test_file3.c ./test/test_funcs.c
try_with_test_file 15 ./test/test_file4.c ./test/test_funcs.c
try_with_test_file 0 ./test/pointer_test0.c ./test/test_funcs.c
try_with_test_file 0 ./test/pointer_test1.c ./test/test_funcs.c

echo ---------------------------------------------------------------------

if [ "$failed_count" -lt 1 ]; then
    echo -e "$(green '[SUCCESS]') [$test_count/$test_count] tests are passed."
    exit 0
else
    echo -e "$(red '[FAILURE]') [$failed_count/$test_count] tests are failed."
    exit 1
fi
