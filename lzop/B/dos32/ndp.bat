@echo // Copyright (C) 1996-2010 Markus F.X.J. Oberhumer
@echo //
@echo //   DOS 32-bit
@echo //   Microway NDP C/C++
@echo //
@call b\prepare.bat
@if "%BECHO%"=="n" echo off


mx486 -ansi -on %CFI% -Dconst= -Isrc -o %BEXE% src\*.c %BLDLIBS% -bind -map
@if errorlevel 1 goto error


@call b\done.bat
@goto end
:error
@echo ERROR during build!
:end
@call b\unset.bat
