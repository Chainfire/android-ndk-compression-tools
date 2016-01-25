@echo // Copyright (C) 1996-2010 Markus F.X.J. Oberhumer
@echo //
@echo //   OS/2 32-bit
@echo //   Watcom C/C++
@echo //
@call b\prepare.bat
@if "%BECHO%"=="n" echo off


set WCL386=-zq -bt#os2 -l#os2v2 %CFI% %WCL_W%
wcl386 -mf -5r -ox -fm -Fe#%BEXE% src\*.c %BLDLIBS%
@if errorlevel 1 goto error


@call b\done.bat
@goto end
:error
@echo ERROR during build!
:end
@call b\unset.bat
