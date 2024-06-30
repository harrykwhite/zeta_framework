@echo off

cd ../zfw_common
call build.bat

cd ../zfw_asset_packer

if not exist bin mkdir bin

if not exist obj mkdir obj

echo Building the "zfw_asset_packer" executable...

gcc -c src/main.c -o obj/main.o -I../zfw_common/include
gcc -c src/stb_image.c -o obj/stb_image.o

gcc -o bin/zfw_asset_packer obj/stb_image.o obj/main.o -L../zfw_common/bin -lzfw_common -lcjson
