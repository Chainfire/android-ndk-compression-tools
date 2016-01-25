@echo // Copyright (C) 1996-2010 Markus F.X.J. Oberhumer
@echo //
@echo //   Windows 32-bit
@echo //   lcc-win32
@echo //
@echo // NOTE: some lcc-win32 versions are buggy, so we disable optimizations
@echo //
@call b\prepare.bat
@if "%BECHO%"=="n" echo off


for %%f in (src\*.c) do lcc -A %CFI% -Iacc -c %%f
@if errorlevel 1 goto error
lc *.obj %BLDLIBS% -o %BEXE% -s
@if errorlevel 1 goto error


@call b\done.bat
@goto end
:error
@echo ERROR during build!
:end
@call b\unset.bat
