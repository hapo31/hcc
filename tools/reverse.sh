gcc -o reversed $@
objdump -M intel -d ./reversed > reversed.s
