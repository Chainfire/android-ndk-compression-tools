@echo // Copyright (C) 1996-2010 Markus F.X.J. Oberhumer
@echo //
@echo //   DOS 32-bit
@echo //   Digital Mars C/C++
@echo //
@call b\prepare.bat
@if "%BECHO%"=="n" echo off


dmc -mx -o -w- %CFI% -o%BEXE% @b\src.rsp %BLDLIBS% x32.lib -L/map
@if errorlevel 1 goto error


@call b\done.bat
@goto end
:error
@echo ERROR during build!
:end
@call b\unset.bat
