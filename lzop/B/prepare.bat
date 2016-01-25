@call b\unset.bat
@call b\clean.bat

@if "%LZODIR%"=="" if exist b\all\prepare.bat call b\all\prepare.bat
@if "%LZODIR%"=="" set LZODIR=..\lzo-2.01
@set CFI=-I. -I%LZODIR%\include

@set BNAME=lzop
@set BEXE=lzop.exe
@set BLDLIBS=lzo2.lib

@echo Compiling, please be patient...
