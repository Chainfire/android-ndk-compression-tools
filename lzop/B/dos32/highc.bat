@echo // Copyright (C) 1996-2010 Markus F.X.J. Oberhumer
@echo //
@echo //   DOS 32-bit
@echo //   MetaWare High C/C++ (using Pharlap DOS extender)
@echo //
@call b\prepare.bat
@if "%BECHO%"=="n" echo off


hc386 -O3 -w4 -Dconst= %CFI% -o %BEXE% src\*.c %BLDLIBS%
@if errorlevel 1 goto error


@call b\done.bat
@goto end
:error
@echo ERROR during build!
:end
@call b\unset.bat
