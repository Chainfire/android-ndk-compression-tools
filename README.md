#About#

This is a collection of compression tools, and a script to download/build them, using the Android NDK.

- gzip 1.6 - GPLv3
- pigz (omni git) - BSD-ish
- bzip2 1.0.6 - BSD-ish
- xz utils (git) - public domain core / (L)GPLv2/v3 portions
- lz4 (git) - BSD 2-clause core / GPLv2  portions
- lzo 2.09 - GPLv2
- lzop 1.03 - GPLv2

I make no claims as to how well the output programs in all their variants actually work. I only made sure they built. I need this for a different project, but thought I might share it. If anything breaks, you get to keep all the pieces.

#Building#

At least the following packages are needed:

- automake
- autoconf
- autopoint
- autotools-dev
- m4
- perl

Versions needed may vary.

I built all of this on an x86-64 Linux Mint box.

To download fresh copies, run ``./download.sh``. Note that this script makes some minor modifications to the downloads. Because upstream may change, I included all files as they were at the time of this writing.

To build, run ``./build.sh``, and wait approximately one forever.

