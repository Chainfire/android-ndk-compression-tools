@echo // Copyright (C) 1996-2010 Markus F.X.J. Oberhumer
@echo //
@echo //   OS/2 16-bit
@echo //   Symantec C/C++
@echo //
@call b\prepare.cmd
@if "%BECHO%"=="n" echo off


sc -c -mlo -2 -o -w- %CFI% @b\src.rsp
@if errorlevel 1 goto error
link @b\win32\sc.rsp,%BEXE%,,lzo/noi/map
@if errorlevel 1 goto error


@call b\done.cmd
@goto end
:error
@echo ERROR during build!
:end
@call b\unset.cmd
