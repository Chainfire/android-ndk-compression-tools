#!/bin/sh

if [ -z "$NDK" ]; then
  if [ ! -z "$ANDROID_NDK_ROOT" ]; then
    NDK=$ANDROID_NDK_ROOT
  elif [ ! -z "$NDK_ROOT" ]; then
    NDK=$NDK_ROOT
  else
    NDK=$((find ~/. -maxdepth 1 -type d; find ~/. -maxdepth 1 -type l) | grep "android-ndk" | sort --reverse | grep -m 1 "android-ndk")
    if [ ! -z "$NDK" ]; then
      NDK=$(readlink -f $NDK)
    fi
    if [ -z "$NDK" ]; then
      echo Please set ANDROID_NDK_ROOT
      exit 1
    fi
  fi
fi

if [ -z "$LIBSPATHS" ]; then
  LIBSPATHS=false
fi

if ($LIBSPATHS); then
  # Use paths compatible with Android packaging
  PATH_ARM=armeabi
  PATH_ARMV7=armeabi-v7a
  PATH_ARM64=arm64-v8a
  PATH_X86=x86
  PATH_X64=x86_64
  PATH_MIPS=mips
  PATH_MIPS64=mips64
else
  # Use paths as used by SuperSU ZIP
  PATH_ARM=arm
  PATH_ARMV7=armv7
  PATH_ARM64=arm64
  PATH_X86=x86
  PATH_X64=x64
  PATH_MIPS=mips
  PATH_MIPS64=mips64
fi

# build_conf platform sysroot srcpath copyfunc outpath
build_core() {
  local _STATIC=true
  if [ ! -z "$STATIC" ]; then
    _STATIC=$STATIC
  fi

  local _PIE=false
  if [ ! -z "$PIE" ]; then
    _PIE=$PIE
  fi

  if ($_STATIC); then
    _PIE=false
  fi

  local _CONFIGURE=true
  if [ ! -z "$CONFIGURE" ]; then
    _CONFIGURE=$CONFIGURE
  fi

  local COMPILER=$(find $NDK/. | grep -e "$1-gcc$" | sort --reverse | grep -m 1 "linux-x86_64")
  local COMPILER_FIXED=$(readlink -f $COMPILER)
  local COMPILER_PATH=$(dirname $COMPILER_FIXED)
  local _VISIBILITY=hidden
  if [ ! -z "$VISIBILITY" ]; then
    _VISIBILITY=$VISIBILITY
  fi
  local COMPILER_FLAGS="-mandroid -mbionic -Os -fdata-sections -ffunction-sections -s -fvisibility=$_VISIBILITY --sysroot=$2 -I$2/usr/include $EXTRA_CFLAGS"

  local LIB=$2/usr/lib
  if (echo $1 | grep 64 >/dev/null); then
    LIB=$2/usr/lib64
  fi
  local LINKER_FLAGS="-Wl,--gc-sections -L$PWD/$5 -L$LIB $EXTRA_LDFLAGS"

  if ($_PIE); then
    COMPILER_FLAGS="$COMPILER_FLAGS -fPIE"
    LINKER_FLAGS="$LINKER_FLAGS -rdynamic -pie"
  else
    COMPILER_FLAGS="$COMPILER_FLAGS -fPIC"
  fi

  local LIBTOOL=false
  if [ -f "libtool" ]; then
    LIBTOOL=true
  fi

  echo ------------------------------------------------------------------
  echo "Building [$1] [$COMPILER_PATH] [$COMPILER_FLAGS] [$LINKER_FLAGS]"
  echo ------------------------------------------------------------------

  rm config.log 2>/dev/null
  rm config.status 2>/dev/null

  if ($_CONFIGURE); then
    if ($_STATIC); then
      PATH=$COMPILER_PATH:$PATH ./configure --host=$1 --disable-shared --enable-static LDFLAGS="-static $LINKER_FLAGS" CFLAGS="$COMPILER_FLAGS" CPPFLAGS="$COMPILER_FLAGS" CXXFLAGS="$COMPILER_FLAGS" $EXTRA_CONFIG
    else
      PATH=$COMPILER_PATH:$PATH ./configure --host=$1 --enable-shared --enable-static LDFLAGS="$LINKER_FLAGS" CFLAGS="$COMPILER_FLAGS" CPPFLAGS="$COMPILER_FLAGS" CXXFLAGS="$COMPILER_FLAGS" $EXTRA_CONFIG
    fi
    unset CC
    unset AR
    unset RANLIB
  else
    export CC="$COMPILER_FIXED"
    export AR=`echo $CC | sed 's/-gcc/\-ar/'`
    export RANLIB=`echo $CC | sed 's/-gcc/\-ranlib/'`
  fi

  PATH=$COMPILER_PATH:$PATH make -C "$3" clean

  if ($_CONFIGURE); then
    if ($_STATIC && $LIBTOOL); then
      PATH=$COMPILER_PATH:$PATH make -C "$3" -j4 all LDFLAGS="-all-static $LINKER_FLAGS"
    else
      PATH=$COMPILER_PATH:$PATH make -C "$3" -j4 all LDFLAGS="$LINKER_FLAGS"
    fi
  else
    if ($_STATIC); then
      PATH=$COMPILER_PATH:$PATH make -C "$3" -j4 all LDFLAGS="-static $LINKER_FLAGS" CFLAGS="$COMPILER_FLAGS" CPPFLAGS="$COMPILER_FLAGS" CXXFLAGS="$COMPILER_FLAGS"
    else
      PATH=$COMPILER_PATH:$PATH make -C "$3" -j4 all LDFLAGS="$LINKER_FLAGS" CFLAGS="$COMPILER_FLAGS" CPPFLAGS="$COMPILER_FLAGS" CXXFLAGS="$COMPILER_FLAGS"
    fi
  fi

  if [ ! -z "$4" ]; then $4 $5 $_STATIC $_PIE; fi

  PATH=$COMPILER_PATH:$PATH make -C "$3" clean
  rm config.log 2>/dev/null
  rm config.status 2>/dev/null

  unset CC
  unset AR
  unset RANLIB
}

# build_arch platform api arch-id out-id srcpath extra_config extra_cflags extra_ldflags copyfunc
build_arch() {
  local PLATFORM=$2
  if [ "$2" -lt "21" ]; then
    PLATFORM=21
  fi
  STATIC=true PIE=false EXTRA_CONFIG="$6" EXTRA_CFLAGS="$7 $STATIC_CFLAGS" EXTRA_LDFLAGS="$8 $STATIC_LDFLAGS" build_core $1 $NDK/platforms/android-$PLATFORM/arch-$3 "$5" "$9" "../out/static/$4"

  if (! echo $1 | grep 64 >/dev/null); then
    # 64 bit is always PIE, so only generate non-PIE on 32 bit
    STATIC=false PIE=false EXTRA_CONFIG="$6" EXTRA_CFLAGS="$7 $DYNAMIC_CFLAGS" EXTRA_LDFLAGS="$8 $DYNAMIC_LDFLAGS" build_core $1 $NDK/platforms/android-$2/arch-$3 "$5" "$9" "../out/dynamic/$4"
  fi

  local PLATFORM=$2
  if [ "$2" -lt "17" ]; then
    PLATFORM=17;
  fi
  STATIC=false PIE=true EXTRA_CONFIG="$6" EXTRA_CFLAGS="$7 $DYNAMIC_PIE_CFLAGS" EXTRA_LDFLAGS="$8 $DYNAMIC_PIE_LDFLAGS" build_core $1 $NDK/platforms/android-$PLATFORM/arch-$3 "$5" "$9" "../out/dynamic.pie/$4"

  unset STATIC
  unset PIE
  unset EXTRA_CONFIG
  unset EXTRA_CFLAGS
  unset EXTRA_LDFLAGS
}

# build_archs srcpath extra_config extra_cflags extra_ldflags copyfunc
build_archs() {
  build_arch arm-linux-androideabi 5 arm $PATH_ARM "$1" "$2" "-mthumb $3 $ARM_CFLAGS" "$4 $ARM_LDFLAGS" $5
  build_arch arm-linux-androideabi 9 arm $PATH_ARMV7 "$1" "$2" "-mthumb -march=armv7-a -mfloat-abi=softfp -mfpu=vfpv3-d16 $3 $ARMV7_CFLAGS" "-march=armv7-a -Wl,--fix-cortex-a8 $4 $ARMV7_LDFLAGS" $5
  build_arch i686-linux-android 9 x86 $PATH_X86 "$1" "$2" "$3 $X86_CFLAGS" "$4 $X86_LDFLAGS" $5
  build_arch mipsel-linux-android 12 mips $PATH_MIPS "$1" "$2" "$3 $MIPS_CFLAGS" "$4 $MIPS_LDFLAGS" $5
  build_arch aarch64-linux-android 21 arm64 $PATH_ARM64 "$1" "$2" "$3 $ARM64_CFLAGS" "$4 $ARM64_LDFLAGS" $5
  build_arch x86_64-linux-android 21 x86_64 $PATH_X64 "$1" "$2" "$3 $X64_CFLAGS" "$4 $X64_LDFLAGS" $5
  build_arch mips64el-linux-android 21 mips64 $PATH_MIPS64 "$1" "$2" "$3 $MIPS64_CFLAGS" "$4 $MIPS64_LDFLAGS" $5
}

# confirm_output name
confirm_output() {
  local IS_SO=false
  echo $1 | grep "\.so" >/dev/null
  if [ "$?" -eq "0" ]; then
    IS_SO=true
  fi

  for i in $PATH_ARM $PATH_ARMV7 $PATH_ARM64 $PATH_X86 $PATH_X64 $PATH_MIPS $PATH_MIPS64; do
    if [ ! -f "out/static/$i/$1" ]; then
      if (! $IS_SO); then
        echo MISSING [static/$i/$1]
      fi
    fi
  done
  for i in $PATH_ARM $PATH_ARMV7 $PATH_X86 $PATH_MIPS; do
    if [ ! -f "out/dynamic/$i/$1" ]; then
      echo MISSING [dynamic/$i/$1]
    fi
  done
  for i in $PATH_ARM $PATH_ARMV7 $PATH_ARM64 $PATH_X86 $PATH_X64 $PATH_MIPS $PATH_MIPS64; do
    if [ ! -f "out/dynamic.pie/$i/$1" ]; then
      echo MISSING [dynamic.pie/$i/$1]
    fi
  done
}

# build_gzip_cp outpath static pie
build_gzip_cp() {
  mkdir -p $1
  cp -f lib/config.h $1/config.gzip.h
  cp -f lib/libgzip.a $1/libgzip.a
  cp -f gzip $1/gzip
}

# build_gzip
build_gzip() {
  cd gzip

  ARM64_CFLAGS="-DMAXSEG_64K"
  build_archs . "" "-DPENDING_OUTPUT_N_BYTES=0 -D__sferror" "" build_gzip_cp
  unset ARM64_CFLAGS

  # x86 and x86_64 static build fails, build again
  build_arch i686-linux-android 9 x86 $PATH_X86 . "" "-DPENDING_OUTPUT_N_BYTES=0 -D__sferror -DGNULIB_defined_struct_option" "" build_gzip_cp
  build_arch x86_64-linux-android 21 x86_64 $PATH_X64 . "" "-DPENDING_OUTPUT_N_BYTES=0 -D__sferror -DGNULIB_defined_struct_option" "" build_gzip_cp

  cd ..
}

# build_bzip2_cp outpath static pie
build_bzip2_cp() {
  mkdir -p $1
  cp libbz2.a $1/libbz2.a
  cp bzip2 $1/bzip2
}

# build_bzip2
build_bzip2() {
  cd bzip2
  local CONFIGURE=false 
  build_archs . "" "" "" build_bzip2_cp
  cd ..
}

# build_xz_cp outpath static pie
build_xz_cp() {
  mkdir -p $1
  cp -f config.h $1/config.xz.h
  cp -f src/liblzma/.libs/liblzma.a $1/liblzma.a
  if [ ! -f src/xz/.libs/xz ]; then
    cp -f src/xz/xz $1/xz
  else
    cp -f src/xz/.libs/xz $1/xz
    cp -fd src/liblzma/.libs/liblzma.so* $1/.
  fi
}

# build_xz
build_xz() {
  cd xz
  ./autogen.sh
  build_archs src "--disable-xzdec --disable-lzmadec --disable-lzmainfo --disable-lzma-links --disable-scripts" "" "" build_xz_cp
  cd ..
}

# build_lz4_cp outpath static pie
build_lz4_cp() {
  mkdir -p $1
  cp -f lib/liblz4.a $1/liblz4.a
  if (! $2); then
    cp -f lib/liblz4.so* $1/.
  fi
  cp -f programs/lz4 $1/lz4
}

# build_lz4
build_lz4() {
  cd lz4
  local CONFIGURE=false
  build_archs . "" "" "" build_lz4_cp
  cd ..
}

# build_lzo_cp outpath static pie
build_lzo_cp() {
  mkdir -p $1
  cp -f config.h $1/config.lzo.h 
  cp -f src/.libs/liblzo2.a $1/liblzo2.a
  if (! $2); then
    cp -f src/.libs/liblzo2.so* $1/.
  fi
}

# build_lzo
build_lzo() {
  cd lzo
  local VISIBILITY=default
  build_archs . "" "" "" build_lzo_cp
  cd ..
}

# build_lzop_cp outpath static pie
build_lzop_cp() {
  mkdir -p $1
  cp -f config.h $1/config.lzop.h
  cp -f src/lzop $1/lzop
}

# build_lzop
build_lzop() {
  cd lzop
  build_archs . "" "-I$PWD/../lzo/include/lzo -I$PWD/../lzo/include" "" build_lzop_cp
  cd ..
}

rm -rf out

# ---

build_gzip

build_bzip2

build_xz

build_lz4

build_lzo
build_lzop

# ---

confirm_output libgzip.a
confirm_output gzip

confirm_output libbz2.a
confirm_output bzip2

confirm_output liblzma.a
confirm_output liblzma.so
confirm_output xz

confirm_output liblz4.a
confirm_output liblz4.so
confirm_output lz4

confirm_output liblzo2.a
confirm_output liblzo2.so
confirm_output lzop
