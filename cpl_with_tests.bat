@echo off
echo Setting compilation variables...
set scriptdir=%cd%
set incdir="%scriptdir%\inc"
set cpldirX=%scriptdir%\cpl\%date%
set cpldir=%cpldirX:/=.%
echo Hiding test.cpp...
cd src
ren test.cpp test.saved
echo Compiling Topaz source...
color c
g++ -std=c++17 -Wall -Wextra -pedantic-errors -O3 -c *.cpp -I %incdir%
gcc -c *.c -w -I %incdir%
color e
echo Revealing test.cpp...
ren test.saved test.cpp
echo Ensuring compilation directory is available and then moving compiled source inside...
if not exist "%cpldir%" mkdir "%cpldir%"
for %%o in (.o) do move "*%%o" "%cpldir%"
echo Primary compilation complete.
echo Setting linking variables...
cd "%cpldir%"
set libdir="%scriptdir%\lib"
set lnkdir="%cpldir%\lnk"
echo Ensuring link directory is available...
if not exist %lnkdir% mkdir %lnkdir%
echo Linking and building Topaz dynamic link library...
g++ -std=c++17 -Wall -pedantic-errors -O3 -shared -Wl,-no-undefined,--enable-runtime-pseudo-reloc,--out-implib,libtopazdll.a *.o -L%libdir% -lOpenGL32 -lSDL2 -lSDL2_mixer -lSDL2_ttf -lSDL2main -lmdl -o "topaz.dll"
color a
echo Primary linking complete, moving output files to link directory...
move "topaz.dll" %lnkdir%
move "libtopazdll.a" %lnkdir% 
echo Moving back to source directory and compiling test.cpp...
cd %scriptdir%
cd src
g++ -std=c++17 -Wall -Wextra -pedantic-errors -O3 -c test.cpp -I %incdir%
color 5
echo Secondary compilation complete.
move test.o %lnkdir%
echo Copying over all dependencies...
cd "%scriptdir%\res\dep"
xcopy /s "%cd%" %lnkdir%
color 4
echo Dependencies copied. Building final executable...
cd %lnkdir%
g++ -O3 -o topaz_test_dependent.exe test.o "%scriptdir%\res\exe\topaz_test.res" -static-libstdc++ -L. -L%libdir% -lOpenGL32 -lSDL2 -lSDL2_mixer -lSDL2_ttf -lSDL2main -lmdl -ltopaz
move test.o "%cpldir%"
move libtopazdll.a "%cpldir%"
color b
echo Topaz building complete, opening link directory...
explorer %lnkdir%
pause