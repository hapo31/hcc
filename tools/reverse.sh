target=$1

gcc -o reversed $target
objdump -M intel -d ./reversed > reversed.s
