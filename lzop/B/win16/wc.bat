@echo // Copyright (C) 1996-2010 Markus F.X.J. Oberhumer
@echo //
@echo //   Windows 16-bit
@echo //   Watcom C/C++
@echo //
@call b\prepare.bat
@if "%BECHO%"=="n" echo off


set WCL=-zq -bt#windows -l#windows %CFI% %WCL_W%
wcl -ml -2 -bw -ox -fm -Fe#%BEXE% src\*.c %BLDLIBS%
@if errorlevel 1 goto error


@call b\done.bat
@goto end
:error
@echo ERROR during build!
:end
@call b\unset.bat
