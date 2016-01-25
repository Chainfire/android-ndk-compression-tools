@echo // Copyright (C) 1996-2010 Markus F.X.J. Oberhumer
@echo //
@echo //   Windows 16-bit
@echo //   Symantec C/C++ (using WINIO)
@echo //
@call b\prepare.bat
@if "%BECHO%"=="n" echo off


sc -c -ml -2 -W -o -w- %CFI% @b\src.rsp
@if errorlevel 1 goto error
link @b\win32\sc.rsp,%BEXE%,,lzo+lwindos+libw+commdlg/noi/map/stack:16384
@if errorlevel 1 goto error


@call b\done.bat
@goto end
:error
@echo ERROR during build!
:end
@call b\unset.bat
