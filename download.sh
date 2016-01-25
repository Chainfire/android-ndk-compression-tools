# gzip

if [ -d "gzip" ]; then rm -rf gzip; fi
wget http://ftp.gnu.org/gnu/gzip/gzip-1.6.tar.gz
tar xvf gzip-1.6.tar.gz
mv gzip-1.6 gzip
rm gzip-1.6.tar.gz

# gnulib

if [ -d "gnulib" ]; then rm -rf gnulib; fi
git clone git://git.savannah.gnu.org/gnulib.git

# patch gnulib in gzip with newer version that supports Android, yikes

if [ -d "gnulib/m4" ]; then
  cd gnulib/m4
  for i in `ls`; do
    if [ -f "../../gzip/m4/$i" ]; then
      cp -f $i ../../gzip/m4/$i
    fi
  done
  cd ../..
fi

if [ -d "gnulib" ]; then
  if [ -d "gzip" ]; then
    cp -f gnulib/build-aux/ar-lib gzip/build-aux/ar-lib
    cp -f gnulib/m4/absolute-header.m4 gzip/m4/absolute-header.m4
    cp -f gnulib/lib/stdio-impl.h gzip/lib/stdio-impl.h
    cd gzip
    autoreconf -fi
    cd lib
    cat Makefile.in | grep "fpending.\$(OBJEXT)" >/dev/null
    if [ "$?" -ne "0" ]; then
      sed -i 's/yesno.\$(OBJEXT)/yesno.\$(OBJEXT) fpending.\$(OBJEXT)/g' Makefile.in
    fi
    cd ../..
  fi
fi

# bzip2

if [ -d "bzip2" ]; then rm -rf bzip2; fi
wget http://www.bzip.org/1.0.6/bzip2-1.0.6.tar.gz
tar xvf bzip2-1.0.6.tar.gz
mv bzip2-1.0.6 bzip2
rm bzip2-1.0.6.tar.gz

# crosscompile variables are hardcoded in the bzip2 makefile, breaking the builds

if [ -f "bzip2/Makefile" ]; then
  sed -i '/CC=/d' bzip2/Makefile
  sed -i '/AR=/d' bzip2/Makefile
  sed -i '/RANLIB=/d' bzip2/Makefile
fi

# xz

if [ -d "xz" ]; then rm -rf xz; fi
git clone http://git.tukaani.org/xz.git xz
rm -rf xz/.git

# lz4

if [ -d "lz4" ]; then rm -rf lz4; fi
git clone https://github.com/Cyan4973/lz4.git lz4
rm -rf lz4/.git

# lzo library

if [ -d "lzo" ]; then rm -rf lzo; fi
wget http://www.oberhumer.com/opensource/lzo/download/lzo-2.09.tar.gz
tar xvf lzo-2.09.tar.gz
mv lzo-2.09 lzo
rm lzo-2.09.tar.gz

# lzop (uses lzo)

if [ -d "lzop" ]; then rm -rf lzop; fi
wget http://www.lzop.org/download/lzop-1.03.tar.gz
tar xvf lzop-1.03.tar.gz
mv lzop-1.03 lzop
rm lzop-1.03.tar.gz

# fix lzop compilation for arm64, configure script does not support aarch64, and the tools used to build it are older than latest, so autoreconf barfs

if [ -f "lzop/autoconf/config.sub" ]; then
  cat lzop/autoconf/config.sub | grep "aarch64" >/dev/null
  if [ "$?" -ne "0" ]; then
    sed -i '/a29k \\/a \ \ \ \ \ \ \ \ | aarch64 | aarch64_be \\' lzop/autoconf/config.sub
    sed -i '/a29k-* \\/a \ \ \ \ \ \ \ \ | aarch64-* | aarch64_be-* \\' lzop/autoconf/config.sub
  fi
fi

# clear git

rm -rf */.git
