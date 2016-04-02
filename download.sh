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

# pigz, use Android-ified sources from OmniROM

if [ -d "pigz" ]; then rm -rf pigz; fi
git init pigz
cd pigz
git remote add origin https://github.com/omnirom/android_bootable_recovery
git config core.sparsecheckout true
echo "pigz/*" >> .git/info/sparse-checkout
git pull --depth=1 origin android-6.0
rm -rf .git
mv pigz/* .
rmdir pigz
cd ..

# patch Makefile

sed -i -e 's/$(CC) -o pigz pigz.o yarn.o -lpthread -lz/$(CC) $(CFLAGS) $(LDFLAGS) -o pigz pigz.o yarn.o -lz/' pigz/Makefile
sed -i -e '/ln /d' pigz/Makefile

# bzip2

if [ -d "bzip2" ]; then rm -rf bzip2; fi
wget http://www.bzip.org/1.0.6/bzip2-1.0.6.tar.gz
tar xvf bzip2-1.0.6.tar.gz
mv bzip2-1.0.6 bzip2
rm bzip2-1.0.6.tar.gz

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
