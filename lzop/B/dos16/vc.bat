@echo // Copyright (C) 1996-2010 Markus F.X.J. Oberhumer
@echo //
@echo //   DOS 16-bit
@echo //   Microsoft Visual C/C++
@echo //
@call b\prepare.bat
@if "%BECHO%"=="n" echo off


set CC=cl -nologo -AL
%CC% -O -W3 %CFI% -c @b\src.rsp
@if errorlevel 1 goto error
%CC% -F 2000 -Fm -Fe%BEXE% *.obj %BLDLIBS% /link /noe
@if errorlevel 1 goto error


@call b\done.bat
@goto end
:error
@echo ERROR during build!
:end
@call b\unset.bat
