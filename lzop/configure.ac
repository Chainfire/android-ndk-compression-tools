#
#  configure.ac -- autoconf configuration for the lzop package
#
#  This file is part of the lzop file compressor.
#
#  Copyright (C) 1996-2010 Markus Franz Xaver Johannes Oberhumer
#  All Rights Reserved.
#
#  lzop and the LZO library are free software; you can redistribute them
#  and/or modify them under the terms of the GNU General Public License as
#  published by the Free Software Foundation; either version 2 of
#  the License, or (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program; see the file COPYING.
#  If not, write to the Free Software Foundation, Inc.,
#  59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
#
#  Markus F.X.J. Oberhumer
#  <markus@oberhumer.com>
#  http://www.oberhumer.com/opensource/lzop/
#


# /***********************************************************************
# // Init
# ************************************************************************/

AC_COPYRIGHT([Copyright (C) 1996-2010 Markus Franz Xaver Johannes Oberhumer.
All Rights Reserved.
This configure script may be copied, distributed and modified under the
terms of the GNU General Public License; see COPYING for more details.])

AC_PREREQ(2.67)
AC_INIT([lzop],[1.03],[lzop-bugs@oberhumer.com],[lzop],[http://www.oberhumer.com/opensource/lzop/])
AC_MSG_NOTICE([Configuring $PACKAGE_NAME $PACKAGE_VERSION])
AC_CONFIG_SRCDIR(src/lzop.c)
AC_CONFIG_AUX_DIR(autoconf)
AC_PREFIX_DEFAULT(/usr/local)
AC_CANONICAL_BUILD
AC_CANONICAL_HOST
AC_CANONICAL_TARGET
AM_MAINTAINER_MODE

if test -z "$ac_abs_top_srcdir"; then
    _AC_SRCDIRS([.])
fi
if test -z "$ac_abs_top_srcdir"; then
    as_fn_error 1 "internal error: ac_abs_top_srcdir is not set"
fi
if test -r .Conf.settings1; then
    . ./.Conf.settings1
fi

AC_ARG_ENABLE(asm, AS_HELP_STRING(--disable-asm,disable LZO assembly versions))
AC_ARG_ENABLE(ansi, AS_HELP_STRING(--disable-ansi,disable ansi console))
if test "X$enable_ansi" != Xno; then
    AC_DEFINE(LZOP_ENABLE_ANSI,1,[Define to 1 if should be enabled.])
fi

AC_PROG_CC
AC_PROG_CPP
mfx_PROG_CPPFLAGS
AC_C_CONST
mfx_ACC_CHECK_ENDIAN
AC_SYS_LARGEFILE

AM_INIT_AUTOMAKE([1.11.1 foreign])
AM_SILENT_RULES([no])
AC_CONFIG_HEADERS([config.h:config.hin])


# /***********************************************************************
# // Checks for header files 1)
# ************************************************************************/

mfx_ACC_CHECK_HEADERS
if test "X$ac_cv_header_limits_h" != Xyes; then
    AC_MSG_ERROR([<limits.h> header not found])
fi
mfx_CHECK_HEADER_SANE_LIMITS_H
if test "X$mfx_cv_header_sane_limits_h" != Xyes; then
    AC_MSG_ERROR([your <limits.h> header is broken - for details see config.log])
fi


# /***********************************************************************
# // Checks for LZO library
# ************************************************************************/

lzoconf_h=
if test "X$lzoconf_h" = X; then
    AC_CHECK_HEADERS([lzo/lzoconf.h lzo/lzo1x.h])
    if test "X$ac_cv_header_lzo_lzoconf_h$ac_cv_header_lzo_lzo1x_h" = Xyesyes; then
        lzoconf_h='lzo/lzoconf.h'
    fi
fi
if test "X$lzoconf_h" = X; then
    AC_CHECK_HEADERS([lzoconf.h lzo1x.h])
    if test "X$ac_cv_header_lzoconf_h$ac_cv_header_lzo1x_h" = Xyesyes; then
        lzoconf_h='lzoconf.h'
    fi
fi
if test "X$lzoconf_h" = X; then
  AC_MSG_ERROR([LZO header files not found. Please check your installation or set the environment variable \`CPPFLAGS'.])
fi


AC_MSG_CHECKING([for version information in $lzoconf_h...])
lzoconf_h_version=
if test "X$lzoconf_h_version" = X; then
    AC_COMPILE_IFELSE([AC_LANG_SOURCE([#include <$lzoconf_h>
#if defined(LZO_VERSION) && (LZO_VERSION >= 0x2000)
#else
#  error "LZO_VERSION v2"
#endif
])], [lzoconf_h_version=2], [])
fi
if test "X$lzoconf_h_version" = X; then
    AC_COMPILE_IFELSE([AC_LANG_SOURCE([#include <$lzoconf_h>
#if defined(LZO_VERSION) && (LZO_VERSION >= 0x1040)
#else
#  error "LZO_VERSION v1"
#endif
])], [lzoconf_h_version=1], [])
fi
AC_MSG_RESULT([v$lzoconf_h_version])
if test "X$lzoconf_h_version" = X; then
    AC_MSG_ERROR([your LZO headers are broken - for details see config.log])
fi


if test "X$lzoconf_h_version" = X1; then
    AC_CHECK_LIB(lzo,__lzo_init2,,AC_MSG_ERROR([LZO library v1 not found. Please check your installation or set the environment variable \`LDFLAGS'.]))
    if test "X$enable_asm" != Xno; then
        AC_CHECK_LIB(lzo,lzo1x_decompress_asm_fast,AC_DEFINE(LZO_USE_ASM_1,1,[Use asm.]))
        AC_CHECK_LIB(lzo,_lzo1x_decompress_asm_fast,AC_DEFINE(LZO_USE_ASM_2,1,[Use asm.]))
    fi
fi
if test "X$lzoconf_h_version" = X2; then
    AC_CHECK_LIB(lzo2,__lzo_init_v2,,AC_MSG_ERROR([LZO library v2 not found. Please check your installation or set the environment variable \`LDFLAGS'.]))
    if test "X$enable_asm" != Xno; then
        AC_CHECK_LIB(lzo2,lzo1x_decompress_asm_fast,AC_DEFINE(LZO_USE_ASM_1,1,[Use asm.]))
        AC_CHECK_LIB(lzo2,_lzo1x_decompress_asm_fast,AC_DEFINE(LZO_USE_ASM_2,1,[Use asm.]))
    fi
fi


# /***********************************************************************
# // Checks for header files 2)
# ************************************************************************/

AC_CHECK_HEADERS([conio.h direct.h dos.h io.h share.h sys/utime.h])
dnl AC_CHECK_HEADERS([linux/kd.h linux/kdev_t.h linux/major.h])
dnl AC_CHECK_HEADERS([curses.h ncurses.h])


# /***********************************************************************
# // Checks for typedefs and structures
# ************************************************************************/

AC_TYPE_OFF_T
AC_CHECK_TYPE(ptrdiff_t,long)
AC_TYPE_SIZE_T
AC_TYPE_SIGNAL
AC_STRUCT_ST_MTIM_NSEC

mfx_ACC_CHECK_SIZEOF
mfx_CHECK_SIZEOF


# /***********************************************************************
# // Checks for library functions
# ************************************************************************/

mfx_ACC_CHECK_FUNCS
mfx_CHECK_LIB_WINMM


# /***********************************************************************
# // Write output files
# ************************************************************************/

mfx_MINIACC_ACCCHK(["-I$srcdir"],["src/miniacc.h"])

if test -r .Conf.settings2; then
    . ./.Conf.settings2
fi

test "X$CPPFLAGS" != "X" && CPPFLAGS="$CPPFLAGS "
CPPFLAGS="${CPPFLAGS}-DLZOP_HAVE_CONFIG_H"

configure_CPPFLAGS=$CPPFLAGS
configure_CFLAGS=$CFLAGS
AC_SUBST(configure_CPPFLAGS)
AC_SUBST(configure_CFLAGS)

AC_CONFIG_FILES([Makefile doc/Makefile src/Makefile])
AC_OUTPUT


cat <<EOF

   lzop configuration summary
   --------------------------
   lzop version               : ${PACKAGE_VERSION}
   configured for host        : ${host_cpu}-${host_vendor}-${host_os}
   source code location       : ${srcdir}
   compiler                   : ${CC}
   preprocessor flags         : ${CPPFLAGS}
   compiler flags             : ${CFLAGS}
   linker flags               : ${LDFLAGS}
   link libraries             : ${LIBS}


   ${PACKAGE_NAME} ${PACKAGE_VERSION} configured.

   Copyright (C) 1996-2010 Markus Franz Xaver Johannes Oberhumer
   All Rights Reserved.

   lzop and the LZO library are free software; you can redistribute them
   and/or modify them under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of
   the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   Markus F.X.J. Oberhumer
   <markus@oberhumer.com>
   http://www.oberhumer.com/opensource/lzop/


Type \`make' to build ${PACKAGE_NAME}. Type \`make install' to install ${PACKAGE_NAME}.
After installing ${PACKAGE_NAME}, please read the accompanied documentation.

EOF

# vi:ts=4:et
