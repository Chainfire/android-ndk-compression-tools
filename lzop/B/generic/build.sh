#! /bin/sh
# vi:ts=4:et
set -e
echo "// Copyright (C) 1996-2010 Markus F.X.J. Oberhumer"
echo "//"
echo "//   Generic Posix/Unix system"
echo "//   Generic C compiler"

test "X${top_srcdir}" = X && top_srcdir=`echo "$0" | sed 's,[^/]*$,,'`../..

test "X${CC}" = X && CC="cc"
test "X${CFLAGS+set}" = Xset || CFLAGS="-O"
test "X${LIBS+set}" = Xset || LIBS="-llzo2"
# CPPFLAGS, LDFLAGS
# LZOP_EXTRA_CPPFLAGS, LZOP_EXTRA_CFLAGS, LZOP_EXTRA_LDFLAGS
# LZOP_EXTRA_SOURCES, LZOP_EXTRA_LIBS

CFI="-I${top_srcdir}"

CF="$CPPFLAGS $CFI $CFLAGS"
LF="$LDFLAGS $LZOP_EXTRA_LDFLAGS"
LL="$LIBS $LZOP_EXRA_LIBS"

. $top_srcdir/B/generic/clean.sh

for f in $top_srcdir/src/*.c $LZOP_EXTRA_SOURCES; do
    echo $CC $CF $LZOP_EXTRA_CPPFLAGS $LZOP_EXTRA_CFLAGS -c $f
         $CC $CF $LZOP_EXTRA_CPPFLAGS $LZOP_EXTRA_CFLAGS -c $f
done

echo $CC $CF $LF -o lzop.out *.o $LL
     $CC $CF $LF -o lzop.out *.o $LL

echo "//"
echo "// Building lzop was successful. All done."

true
