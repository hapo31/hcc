target=$1

gcc -o tmp.d $target
objdump -M intel -d ./reversed > reversed.s
