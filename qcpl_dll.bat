@echo off
set scriptdir=%cd%
set incdir="%scriptdir%\inc"
set cpldirX=%scriptdir%\cpl\%date%
set cpldir=%cpldirX:/=.%

color e
cd src
ren test.cpp test.saved
g++ -std=c++14 -Wall -Wextra -pedantic-errors -O3 -c -DTOPAZDLLBUILD *.cpp -I %incdir%
gcc -c -DTOPAZDLLBUILD *.c -w -I %incdir%
ren test.saved test.cpp

if not exist "%cpldir%" mkdir "%cpldir%"

for %%o in (.o) do move "*%%o" "%cpldir%"

color c
echo Compilation Completed, ".o" files are in "%cpldir%"

cd "%cpldir%"
set libdir="%scriptdir%\lib"
set lnkdir="%cpldir%\lnk"

if not exist %lnkdir% mkdir %lnkdir%

g++ -std=c++17 -Wall -pedantic-errors -O3 -shared -Wl,-no-undefined,--enable-runtime-pseudo-reloc,--out-implib,libtopazdll.a *.o -L%libdir% -lOpenGL32 -lSDL2 -lSDL2_mixer -lSDL2main -lmdl -o "topaz.dll"

move "topaz.dll" %lnkdir%
move "libtopazdll.a" %lnkdir%
echo Linking Completed, "topaz.dll" is in %lnkdir%
cd %scriptdir%
cd src
g++ -std=c++17 -Wall -Wextra -pedantic-errors -O3 -c test.cpp -I %incdir%
move test.o %lnkdir%
color d
echo Ensuring that dependencies are present...
cd "%scriptdir%\res\dep"
xcopy /s "%cd%" %lnkdir%
echo Dependencies copied...
cd %lnkdir%
g++ -O3 -o topaz_test_dependent.exe test.o "%scriptdir%\res\exe\topaz_test.res" -L. -L%libdir% -lOpenGL32 -lSDL2 -lSDL2_mixer -lSDL2main -lmdl -ltopaz
move test.o "%cpldir%"
move libtopazdll.a "%cpldir%"

explorer %lnkdir%
color b
pause