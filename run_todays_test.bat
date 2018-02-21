@echo off
echo Setting compilation variables...
set scriptdir=%cd%
set incdir="%scriptdir%\inc"
set cpldirX=%scriptdir%\cpl\%date%
set cpldir=%cpldirX:/=.%
set libdir="%scriptdir%\lib"
set lnkdir="%cpldir%\lnk"
echo Executing test executable... Will crash if you haven't moved the most recent 'libtopaz.dll' into the lnk directory.
if not exist "%lnkdir%\topaz_test.exe" echo "Caution: Couldn't find today's test executable, get ready for some batch errors."
cd %lnkdir%
topaz_test.exe