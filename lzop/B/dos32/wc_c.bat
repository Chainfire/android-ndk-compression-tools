@echo // Copyright (C) 1996-2010 Markus F.X.J. Oberhumer
@echo //
@echo //   DOS 32-bit
@echo //   Watcom C/C++ (using CauseWay extender)
@echo //
@call b\prepare.bat
@if "%BECHO%"=="n" echo off


set WCL386=-zq -bt#dos -l#CauseWay %CFI% %WCL_W%
wcl386 -mf -5r -ox -fm -Fe#%BEXE% src\*.c %BLDLIBS%
@if errorlevel 1 goto error


@call b\done.bat
@goto end
:error
@echo ERROR during build!
:end
@call b\unset.bat
