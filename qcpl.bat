@echo off
set scriptdir=%cd%
set incdir="%scriptdir%\inc"
set cpldirX=%scriptdir%\cpl\%date%
set cpldir=%cpldirX:/=.%

cd src
g++ -std=c++14 -c *.cpp -I %incdir%
gcc -c *.c -w -I %incdir%

if not exist "%cpldir%" mkdir "%cpldir%"

for %%o in (.o) do move "*%%o" "%cpldir%"

color a
echo Compilation Completed, ".o" files are in "%cpldir%"

cd "%cpldir%"
set libdir="%scriptdir%\lib"
set lnkdir="%cpldir%\lnk"

if not exist %lnkdir% mkdir %lnkdir%

"C:\MinGW\bin\g++" -std=c++1y -static-libgcc -static-libstdc++ -Wall -pedantic -O3 *.o -L%libdir% -lOpenGL32 -lSDL2 -lSDL2main -lmdl "%scriptdir%\res\exe\ocular.res" -o "oculargame-test.exe"

move "oculargame-test.exe" %lnkdir%
echo Linking Completed, "oculargame-test.exe" is in %lnkdir%
echo Opening %lnkdir%...

explorer %lnkdir%
color b
pause