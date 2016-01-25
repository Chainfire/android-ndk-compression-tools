@echo // Copyright (C) 1996-2010 Markus F.X.J. Oberhumer
@echo //
@echo //   OS/2 32-bit
@echo //   IBM Visual Age C/C++
@echo //
@call b\prepare.cmd
@if "%BECHO%"=="n" echo off


icc -Q+ -O -W3 %CFI% -Fm -Fe%BEXE% src\*.c %BLDLIBS%
@if errorlevel 1 goto error


@call b\done.cmd
@goto end
:error
@echo ERROR during build!
:end
@call b\unset.cmd
