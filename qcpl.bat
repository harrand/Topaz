@echo off
set scriptdir=%cd%
set incdir="%scriptdir%\inc"
set cpldirX=%scriptdir%\cpl\%date%
set cpldir=%cpldirX:/=.%
cd src
g++ -std=c++11 -c *.cpp -I %incdir% -I "%scriptdir%\Lua\5.3.2\inc"
gcc -c *.c -w -I %incdir% -I "%scriptdir%\Lua\5.3.2\inc"
if not exist "%cpldir%" mkdir "%cpldir%"
for %%o in (.o) do move "*%%o" "%cpldir%"
color a
echo Compilation Completed, ".o" files are in "%cpldir%"
cd "%cpldir%"
set libdir="%scriptdir%\lib"
set lnkdir="%cpldir%\lnk"
if not exist %lnkdir% mkdir %lnkdir%
"C:\MinGW\bin\g++" -std=c++11 -static-libgcc -static-libstdc++ -Wall -pedantic -O3 *.o -L%libdir% -L"%scriptdir%\Lua\5.3.2\lib" -lOpenGL32 -lSDL2 -lSDL2main "%scriptdir%\res\exe\ocular.res" -o "oculargame-test.exe"
move "oculargame-test.exe" %lnkdir%
echo Linking Completed, "oculargame-test.exe" is in %lnkdir%
echo Opening %lnkdir%...
explorer %lnkdir%
color b
pause