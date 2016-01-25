@echo // Copyright (C) 1996-2010 Markus F.X.J. Oberhumer
@echo //
@echo //   DOS 16-bit
@echo //   Watcom C/C++
@echo //
@call b\prepare.bat
@if "%BECHO%"=="n" echo off


set WCL=-zq -bt#dos -l#dos %CFI% %WCL_W%
wcl -ml -ox -k0x2000 -fm -Fe#%BEXE% src\*.c %BLDLIBS%
@if errorlevel 1 goto error


@call b\done.bat
@goto end
:error
@echo ERROR during build!
:end
@call b\unset.bat
