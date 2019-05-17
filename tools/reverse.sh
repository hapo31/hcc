target=$1

gcc -o tmp.d $target
objdump -M intel -d ./tmp.d > reversed.s
