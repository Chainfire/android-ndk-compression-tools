@echo // Copyright (C) 1996-2010 Markus F.X.J. Oberhumer
@echo //
@echo //   DOS 16-bit
@echo //   Borland C/C++ + Pharlap 286DOS-Extender
@echo //
@call b\prepare.bat
@if "%BECHO%"=="n" echo off


bcc -ml -2 -f- -O1 -d -w -Isrc -Iacc %CFI% -e%BEXE% @b\src.rsp %BLDLIBS%
@if errorlevel 1 goto error


@call b\done.bat
@goto end
:error
@echo ERROR during build!
:end
@call b\unset.bat
