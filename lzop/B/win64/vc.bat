@echo // Copyright (C) 1996-2010 Markus F.X.J. Oberhumer
@echo //
@echo //   Windows 64-bit (AMD64 or Itanium)
@echo //   Microsoft Visual C/C++
@echo //
@call b\prepare.bat
@if "%BECHO%"=="n" echo off


cl -nologo -MD -O2 -GF -W3 %CFI% -Fe%BEXE% @b\src.rsp %BLDLIBS% /link /map:%BNAME%.map
@if errorlevel 1 goto error


@call b\done.bat
@goto end
:error
@echo ERROR during build!
:end
@call b\unset.bat
