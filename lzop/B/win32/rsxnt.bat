@echo // Copyright (C) 1996-2010 Markus F.X.J. Oberhumer
@echo //
@echo //   Windows 32-bit
@echo //   rsxnt + gcc
@echo //
@call b\prepare.bat
@if "%BECHO%"=="n" echo off


gcc -Zwin32 -Zsys -mprobe -Wall -O2 %CFI% -s -o %BEXE% @b\src.rsp -L. -llzo2
@if errorlevel 1 goto error


@call b\done.bat
@goto end
:error
@echo ERROR during build!
:end
@call b\unset.bat
