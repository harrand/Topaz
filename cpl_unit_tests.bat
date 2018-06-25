@echo off
echo Setting compilation variables...
set scriptdir=%cd%
set incdir="%scriptdir%\inc"
set cpldirX=%scriptdir%\cpl\%date%
set cpldir=%cpldirX:/=.%
set libdir="%scriptdir%\lib"
set lnkdir="%cpldir%\lnk"
cd %scriptdir%
cd src
cd test
cd lib
g++ -std=c++17 -Wall -Wextra -pedantic-errors -O3 -c *.cpp -I%incdir% -I%scriptdir%\src -I%scriptdir%\src\graphics -I%scriptdir%\src\physics -I%scriptdir%\src\audio
ar crf libunit_test.a *.o
move libunit_test.a %libdir%
cd ..
g++ -std=c++17 -Wall -Wextra -pedantic-errors -O3 -c *.cpp -I%incdir% -I%scriptdir%\src -I%scriptdir%\src\graphics -I%scriptdir%\src\physics -I%scriptdir%\src\audio
if not exist %lnkdir% mkdir %lnkdir%
color 5
echo Compiled test.cpp...
move *.o %lnkdir%
echo Copying over all dependencies...
cd "%scriptdir%\res\dep"
xcopy /s /I /Y "%cd%" %lnkdir%
color 4
echo Dependencies copied. Building final executable...
cd %lnkdir%
for %%f in (*.o) do (
    g++ -O3 -o %%~nf.exe %%~nf.o -static-libstdc++ -L. -L%libdir% -L%scriptdir%\cmake-build-debug -lOpenGL32 -lSDL2 -lSDL2_mixer -lSDL2_ttf -lSDL2main -ltopaz.dll -lunit_test
    move %%~nf.o "%cpldir%"
)
color b
echo Unit test builds complete, opening link directory...