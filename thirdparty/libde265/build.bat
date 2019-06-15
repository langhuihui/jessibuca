@echo off
if "%1" == "" goto all
if /i %1 == x86       goto x86
if /i %1 == x64       goto x64
if /i %1 == amd64     goto x64
goto all

:all
echo "Compiling libde265"
cmd /c "%0" x86
cmd /c "%0" x64
pause
goto :eof

:x86
echo "Compiling libde265 (x86)"
call "%VS110COMNTOOLS%\..\..\VC\vcvarsall.bat" x86
nmake -f Makefile.vc7 /nologo /s /c
if not exist bin_x86\lib (
    mkdir bin_x86\lib
)
copy /y dec265\dec265.exe bin_x86\
copy /y enc265\enc265.exe bin_x86\
copy /y libde265\libde265.dll bin_x86\
copy /y libde265\libde265.lib bin_x86\lib\
copy /y libde265\libde265.exp bin_x86\lib\
goto exit

:x64
echo "Compiling libde265 (x64)"
call "%VS110COMNTOOLS%\..\..\VC\vcvarsall.bat" x64
nmake -f Makefile.vc7 /nologo /s /c
if not exist bin_x64\lib (
    mkdir bin_x64\lib
)
copy /y dec265\dec265.exe bin_x64\
copy /y enc265\enc265.exe bin_x64\
copy /y libde265\libde265.dll bin_x64\
copy /y libde265\libde265.lib bin_x64\lib\
copy /y libde265\libde265.exp bin_x64\lib\
goto exit

:exit
