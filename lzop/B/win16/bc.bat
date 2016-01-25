@echo // Copyright (C) 1996-2010 Markus F.X.J. Oberhumer
@echo //
@echo //   Windows 16-bit
@echo //   Borland C/C++
@echo //
@call b\prepare.bat
@if "%BECHO%"=="n" echo off


bcc -ml -2 -tW -O1 -w -Isrc -Iacc %CFI% -e%BEXE% @b\src.rsp %BLDLIBS%
@if errorlevel 1 goto error


@call b\done.bat
@goto end
:error
@echo ERROR during build!
:end
@call b\unset.bat
