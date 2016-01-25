@echo // Copyright (C) 1996-2010 Markus F.X.J. Oberhumer
@echo //
@echo //   DOS 16-bit
@echo //   Turbo C/C++
@echo //
@call b\prepare.bat
@if "%BECHO%"=="n" echo off


tcc -ml -f- -O -G -w -Isrc -Iacc %CFI% -e%BEXE% src\*.c %BLDLIBS%
@if errorlevel 1 goto error


@call b\done.bat
@goto end
:error
@echo ERROR during build!
:end
@call b\unset.bat
