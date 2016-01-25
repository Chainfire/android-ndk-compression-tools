@echo // Copyright (C) 1996-2010 Markus F.X.J. Oberhumer
@echo //
@echo //   Windows 32-bit
@echo //   Pelles C
@echo //
@call b\prepare.bat
@if "%BECHO%"=="n" echo off


cc -Ze -Go -O2 -W2 %CFI% src\*.c %BLDLIBS% /out:%BEXE% /map:%BNAME%.map
@if errorlevel 1 goto error


@call b\done.bat
@goto end
:error
@echo ERROR during build!
:end
@call b\unset.bat
