@echo off
set scriptdir=%cd%
set incdir="%scriptdir%\inc"
set cpldirX=%scriptdir%\cpl\%date%
set cpldir=%cpldirX:/=.%
set libdir="%scriptdir%\lib"
set lnkdir="%cpldir%\lnk"
goto compile_and_setup
:run_test
echo Running...
cd %lnkdir%
for %%f in (*.exe) do (
    if NOT "%%f" == "topaz_test.exe" call %%f
)
goto eof
:compile_and_setup
call cpl_unit_tests.bat
cd "%scriptdir%"
cd cmake-build-debug
echo Compilation complete, moving DLL over.
xcopy /Y libtopaz.dll "%lnkdir%"
goto run_test
:eof
echo Done