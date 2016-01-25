@echo // Copyright (C) 1996-2010 Markus F.X.J. Oberhumer
@echo //
@echo //   Windows 16-bit
@echo //   Microsoft Visual C/C++ (using QuickWin)
@echo //
@call b\prepare.bat
@if "%BECHO%"=="n" echo off


set CC=cl -nologo -AL -G2 -Mq
%CC% -O -W3 %CFI% -c @b\src.rsp
@if errorlevel 1 goto error
%CC% -Fe%BEXE% -Fm *.obj %BLDLIBS% /link /noe
@if errorlevel 1 goto error


@call b\done.bat
@goto end
:error
@echo ERROR during build!
:end
@call b\unset.bat
