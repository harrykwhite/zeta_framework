@echo off

if not exist bin mkdir bin

if not exist obj mkdir obj

echo Building the "zfw_common" static library...

gcc -g -c src/zfw_common_debugging.c -o obj/zfw_common_debugging.o -Iinclude
gcc -g -c src/zfw_common_math.c -o obj/zfw_common_math.o -Iinclude

ar rcs bin/libzfw_common.a obj/zfw_common_debugging.o obj/zfw_common_math.o
