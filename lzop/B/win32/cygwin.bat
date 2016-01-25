@echo // Copyright (C) 1996-2010 Markus F.X.J. Oberhumer
@echo //
@echo //   Windows 32-bit
@echo //   cygwin + gcc
@echo //
@call b\prepare.bat
@if "%BECHO%"=="n" echo off


gcc -x c -O2 -Wall %CFI% -s -o %BEXE% src/*.[cC] -L. -llzo2 -Wl,-Map,%BNAME%.map
@if errorlevel 1 goto error


@call b\done.bat
@goto end
:error
@echo ERROR during build!
:end
@call b\unset.bat
