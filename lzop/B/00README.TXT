Simple make drivers for DOS, Windows, OS/2 and other systems
============================================================

This directory contains a bunch of simple build scripts - I've tried
to make them as foolproof as possible.

To build lzop for your system type `b\OS\COMPILER' in the base directory,
e.g. `b\win32\vc.bat' will build the Win32 Visual C/C++ version.

All scripts expect that the LZO library has been installed and built
in the directory ..\lzo-2.01. You can also set the environment variable
`LZODIR' to point to your LZO directory, e.g. `set LZODIR=c:\src\lzo-2.01',
or you can modify `prepare.bat' to suit your needs.

Please send me your additional/improved versions.


Overview:
---------

b\dos16\bc.bat              Borland C/C++                         (1)
b\dos16\dm.bat              Digital Mars C/C++                    (1)
b\dos16\mc.bat              Microsoft C 7.0                       (1)
b\dos16\sc.bat              Symantec C/C++                        (1)
b\dos16\tc.bat              Turbo C/C++                           (1)
b\dos16\vc.bat              Visual C/C++                          (1)
b\dos16\wc.bat              Watcom C/C++                          (1)

b\dos32\dj2.bat             djgpp v2 + gcc
b\dos32\dm.bat              Digital Mars C/C++
b\dos32\emx.bat             emx + gcc
b\dos32\sc.bat              Symantec C/C++
b\dos32\wc.bat              Watcom C/C++

b\os2\emx.cmd               emx + gcc
b\os2\wc.cmd                Watcom C/C++

b\win16\bc.bat              Borland C/C++                         (1)
b\win16\sc.bat              Symantec C/C++                        (1)
b\win16\vc.bat              Visual C/C++                          (1)
b\win16\wc.bat              Watcom C/C++                          (1)

b\win32\bc.bat              Borland C/C++
b\win32\cygwin.bat          Cygwin + gcc
b\win32\dm.bat              Digital Mars C/C++
b\win32\ic.bat              Intel C/C++
b\win32\lccwin32.bat        lcc-win32
b\win32\mingw.bat           MinGW + gcc
b\win32\rsxnt.bat           rsxnt + gcc
b\win32\sc.bat              Symantec C/C++
b\win32\vc.bat              Visual C/C++
b\win32\wc.bat              Watcom C/C++

b\win64\ic.bat              Intel C/C++ (AMD64 or Itanium)
b\win64\vc.bat              Visual C/C++ (AMD64 or Itanium)


Notes:
  (1) 16-bit versions should work but are not officially supported


