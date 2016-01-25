@call b\unset.cmd
@call b\clean.cmd

@if "%LZODIR%"=="" if exist b\all\prepare.cmd call b\all\prepare.cmd
@if "%LZODIR%"=="" set LZODIR=..\lzo-2.01
@set CFI=-I. -I%LZODIR%\include

@set BNAME=lzop
@set BEXE=lzop.exe
@set BLDLIBS=lzo2.lib

@echo Compiling, please be patient...
