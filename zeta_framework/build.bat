@echo off

if not exist bin mkdir bin

if not exist obj mkdir obj

echo Building the "zeta_framework" static library...

gcc -g -c src/glad.c -o obj/glad.o -Iinclude -DGLFW_INCLUDE_NONE
gcc -g -c src/zfw.c -o obj/zfw.o -Iinclude -I../zfw_common/include -DGLFW_INCLUDE_NONE
gcc -g -c src/zfw_utils.c -o obj/zfw_utils.o -Iinclude -I../zfw_common/include -DGLFW_INCLUDE_NONE

ar rcs bin/libzeta_framework.a obj/glad.o obj/zfw.o obj/zfw_utils.o
