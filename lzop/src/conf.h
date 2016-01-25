/* conf.h --

   This file is part of the lzop file compressor.

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

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.
   If not, write to the Free Software Foundation, Inc.,
   59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

   Markus F.X.J. Oberhumer
   <markus@oberhumer.com>
   http://www.oberhumer.com/opensource/lzop/
 */


#ifndef __LZOP_CONF_H
#define __LZOP_CONF_H 1

#if defined(LZOP_HAVE_CONFIG_H)
#  include <config.h>
#endif
#if !defined(PACKAGE)
#  define PACKAGE "lzop"
#endif

#define WITH_LZO 1
#if defined(WITH_LZO)
#if defined(HAVE_LZO_LZOCONF_H) && defined(HAVE_LZO_LZO1X_H)
#  include <lzo/lzoconf.h>
#  include <lzo/lzo1x.h>
#elif defined(HAVE_LZOCONF_H) && defined(HAVE_LZO1X_H)
#  include <lzoconf.h>
#  include <lzo1x.h>
#elif !defined(LZOP_HAVE_CONFIG_H)
#  include <lzo/lzoconf.h>
#  include <lzo/lzo1x.h>
#else
#  include <lzoconf.h>
#  include <lzo1x.h>
#endif
#if !defined(LZO_VERSION)
#  error "you need the LZO library"
#elif (LZO_VERSION < 0x1040)
#  error "please upgrade your LZO package"
#endif
#endif


/*************************************************************************
//
**************************************************************************/

#if defined(LZOP_HAVE_CONFIG_H)
#  define ACC_CONFIG_NO_HEADER 1
#endif
#define ACC_WANT_ACC_INCD_H 1
#define ACC_WANT_ACC_INCE_H 1
#define ACC_WANT_ACC_LIB_H 1
#include "miniacc.h"
#undef ACC_WANT_ACC_INCD_H
#undef ACC_WANT_ACC_INCE_H
#undef ACC_WANT_ACC_LIB_H

/* we _always_ compile lzop with assertions on */
#undef NDEBUG
#include <assert.h>

#if defined(acc_int64l_t)
#  define lzop_long_t       acc_int64l_t
#  define lzop_ulong_t      acc_uint64l_t
#else
#  define lzop_long_t       long int
#  define lzop_ulong_t      unsigned long int
#endif

#if (ACC_CC_MSC && (_MSC_VER >= 1400))
   /* avoid warnings when using "deprecated" POSIX functions */
#  pragma warning(disable: 4996)
#endif


/*************************************************************************
//
**************************************************************************/

#if defined(ACC_OS_EMX)
#  define DOSISH 1
#  define F_OS          (_osmode == 0 ? F_OS_FAT : F_OS_OS2)
#  define F_CS          (_osmode == 0 ? F_CS_DOS : F_CS_NATIVE)
#elif (ACC_OS_DOS16 || ACC_OS_DOS32)
#  define DOSISH 1
#  define F_OS          F_OS_FAT
#  define F_CS          F_CS_DOS
#elif (ACC_OS_OS216 || ACC_OS_OS2)
#  define DOSISH 1
#  define F_OS          F_OS_OS2
#elif (ACC_OS_TOS)
#  define DOSISH 1
#  define F_OS          F_OS_ATARI
#elif (ACC_OS_WIN16)
#  define DOSISH 1
#  define F_OS          F_OS_FAT
#  define F_CS          F_CS_WIN16
#elif (ACC_OS_WIN32 || ACC_OS_WIN64 || ACC_OS_CYGWIN)
#  define DOSISH 1
#  define F_OS          F_OS_VFAT
#  define F_CS          F_CS_WIN32
#endif


#if defined(DOSISH)
#  define OPT_NAME_DEFAULT 1
#  if !defined(DIR_SEP)
#    define DIR_SEP         "/\\"
#  endif
#  if !defined(fn_tolower)
#    define fn_tolower(x)   tolower(((unsigned char)(x)))
#  endif
#endif

#if defined(__BORLANDC__)
#  define SIGTYPEENTRY      __cdecl
#  define MODE_T            unsigned short
#elif defined(__DMC__)
#  define SIGTYPEENTRY      __cdecl
#elif defined(_MSC_VER)
#  define SIGTYPEENTRY      __cdecl
#  define MODE_T            unsigned short
#elif defined(__WATCOMC__)
#  define MODE_T            unsigned short
#endif


/*************************************************************************
//
**************************************************************************/

#ifndef F_OS
#  define F_OS          F_OS_UNIX
#endif
#ifndef F_CS
#  define F_CS          F_CS_NATIVE
#endif

#ifndef DIR_SEP
#  define DIR_SEP       "/"
#endif

#ifndef fn_tolower
#  define fn_tolower(x) (x)
#endif

#ifndef OPTIONS_VAR
#  define OPTIONS_VAR   "LZOP"
#endif


/*************************************************************************
//
**************************************************************************/

#if defined(__DJGPP__)
#  define NO_SETMODE 1
#  if ((__DJGPP__ * 100 + __DJGPP_MINOR__) < 203)
#    error "need djgpp 2.03 or above"
#  endif
#  include <crt0.h>
#  include <dpmi.h>
#  include <sys/exceptn.h>
#  undef kbhit  /* want to be able to call kbhit from libc */
#endif

#if defined(ACC_OS_TOS)
#  if defined(__MINT__)
#  elif (ACC_CC_PUREC || ACC_CC_TURBOC)
#    include <ext.h>
#    define O_EXCL_BROKEN 1
#    if !defined(S_IFMT)
#      if defined(S_IFREG) && defined(S_IFDIR) && defined(S_IFCHR)
#        define S_IFMT      (S_IFREG | S_IFDIR | S_IFCHR)
#      endif
#    endif
#  else
#    error "FIXME"
#  endif
#endif

#define ADLER32_INIT_VALUE  1
#define CRC32_INIT_VALUE    0


/*************************************************************************
//
**************************************************************************/

#if !defined(PATH_MAX)
#  define PATH_MAX          512
#elif (PATH_MAX < 512)
#  undef PATH_MAX
#  define PATH_MAX          512
#endif


#ifndef RETSIGTYPE
#  define RETSIGTYPE        void
#endif
#ifndef SIGTYPEENTRY
#  define SIGTYPEENTRY      /*empty*/
#endif

#if !defined(MODE_T)
#if defined(SIZEOF_MODE_T) && (SIZEOF_MODE_T > 0)
#  define MODE_T            mode_t
#else
#  define MODE_T            int
#endif
#endif


#if defined(NO_MEMCMP)
#  undef HAVE_MEMCMP
#endif
#if !defined(HAVE_MEMCMP)
#  undef memcmp
#  define memcmp            lzo_memcmp
#endif
#if !defined(HAVE_MEMCPY)
#  undef memcpy
#  define memcpy            lzo_memcpy
#endif
#if !defined(HAVE_MEMSET)
#  undef memset
#  define memset            lzo_memset
#endif


#if !defined(HAVE_STRCASECMP) && defined(HAVE_STRICMP)
#  define strcasecmp        stricmp
#endif
#if !defined(HAVE_STRNCASECMP) && defined(HAVE_STRNICMP)
#  define strncasecmp       strnicmp
#endif


#ifndef STDIN_FILENO
#  define STDIN_FILENO      (fileno(stdin))
#endif
#ifndef STDOUT_FILENO
#  define STDOUT_FILENO     (fileno(stdout))
#endif
#ifndef STDERR_FILENO
#  define STDERR_FILENO     (fileno(stderr))
#endif

#if !defined(S_IFMT) && defined(_S_IFMT)
#  define S_IFMT        _S_IFMT
#endif
#if !defined(S_IFREG) && defined(_S_IFREG)
#  define S_IFREG       _S_IFREG
#endif
#if !defined(S_IFDIR) && defined(_S_IFDIR)
#  define S_IFDIR       _S_IFDIR
#endif
#if !defined(S_IFCHR) && defined(_S_IFCHR)
#  define S_IFCHR       _S_IFCHR
#endif

#if !defined(S_ISREG)
#  if defined(S_IFMT) && defined(S_IFREG)
#    define S_ISREG(m)  (((m) & S_IFMT) == S_IFREG)
#  else
#    error "S_ISREG"
#  endif
#endif
#if !defined(S_ISDIR)
#  if defined(S_IFMT) && defined(S_IFDIR)
#    define S_ISDIR(m)  (((m) & S_IFMT) == S_IFDIR)
#  else
#    error "S_ISDIR"
#  endif
#endif
#if !defined(S_ISCHR)
#  if defined(S_IFMT) && defined(S_IFCHR)
#    define S_ISCHR(m)  (((m) & S_IFMT) == S_IFCHR)
#  endif
#endif


/*************************************************************************
//
**************************************************************************/

#ifndef SUFFIX_MAX
#define SUFFIX_MAX      32
#endif

#define STDIN_NAME      "<stdin>"
#define STDOUT_NAME     "<stdout>"
#define STDERR_NAME     "<stderr>"
#define UNKNOWN_NAME    "<unknown>"


#define ALIGN_DOWN(a,b) (((a) / (b)) * (b))
#define ALIGN_UP(a,b)   ALIGN_DOWN((a) + ((b) - 1), b)

#undef UNUSED
#define UNUSED(var)     ACC_UNUSED(var)


/*************************************************************************
//
**************************************************************************/

/* exit codes of this program: 0 ok, 1 error, 2 warning */
#define EXIT_OK         0
#define EXIT_ERROR      1
#define EXIT_WARN       2

#define EXIT_USAGE      1
#define EXIT_FILE_READ  1
#define EXIT_FILE_WRITE 1
#define EXIT_MEMORY     1
#define EXIT_CHECKSUM   1
#define EXIT_LZO_ERROR  1
#define EXIT_LZO_INIT   1
#define EXIT_INTERNAL   1


/*************************************************************************
// options
**************************************************************************/

enum {
    CMD_NONE,
    CMD_COMPRESS,
    CMD_DECOMPRESS, CMD_TEST, CMD_LIST, CMD_LS, CMD_INFO,
    CMD_SYSINFO, CMD_LICENSE, CMD_HELP, CMD_INTRO, CMD_VERSION
};


enum {
    M_LZO1X_1     =     1,
    M_LZO1X_1_15  =     2,
    M_LZO1X_999   =     3,
    M_NRV1A       =  0x1a,
    M_NRV1B       =  0x1b,
    M_NRV2A       =  0x2a,
    M_NRV2B       =  0x2b,
    M_NRV2D       =  0x2d,
    M_ZLIB        =   128,

    M_UNUSED
};

extern int opt_cmd;
extern int opt_method;
extern int opt_level;

extern int opt_checksum;
extern int opt_console;
extern int opt_crc32;
extern lzo_bool opt_decompress_safe;
extern int opt_force;
extern int opt_name;
#define MAX_NUM_THREADS 64
extern int opt_num_threads;
extern const char *opt_output_name;
extern lzo_bool opt_optimize;
extern lzo_bool opt_path;
extern lzo_bool opt_shortname;
extern int opt_stdin;
extern lzo_bool opt_stdout;
extern char opt_suffix[1+SUFFIX_MAX+1];
extern lzo_bool opt_unlink;
extern int opt_verbose;

#define OPT_STDIN_GUESSED   1
#define OPT_STDIN_REQUESTED 2


/*************************************************************************
// input and output files
**************************************************************************/

typedef struct
{
    int fd;
#if defined(USE_FOPEN)
    FILE *file;
#endif
    int open_flags;
    struct stat st;
#if defined(HAVE_LSTAT)
    struct stat lst;
#endif
    lzo_uint32 f_adler32;
    lzo_uint32 f_crc32;

    int opt_name;
    unsigned long part;
    lzop_ulong_t bytes_read;
    lzop_ulong_t bytes_written;
    lzo_bool warn_multipart;
    lzo_bool warn_unknown_suffix;

    lzop_ulong_t bytes_processed;

#define FILE_T_NAME_LEN     ( 2*((PATH_MAX)+1) + SUFFIX_MAX + 1 )
    char name[ FILE_T_NAME_LEN ];
}
file_t;


/*************************************************************************
// lzop file header
**************************************************************************/

/* header flags */
#define F_ADLER32_D     0x00000001L
#define F_ADLER32_C     0x00000002L
#define F_STDIN         0x00000004L
#define F_STDOUT        0x00000008L
#define F_NAME_DEFAULT  0x00000010L
#define F_DOSISH        0x00000020L
#define F_H_EXTRA_FIELD 0x00000040L
#define F_H_GMTDIFF     0x00000080L
#define F_CRC32_D       0x00000100L
#define F_CRC32_C       0x00000200L
#define F_MULTIPART     0x00000400L
#define F_H_FILTER      0x00000800L
#define F_H_CRC32       0x00001000L
#define F_H_PATH        0x00002000L
#define F_MASK          0x00003FFFL

/* operating system & file system that created the file [mostly unused] */
#define F_OS_FAT        0x00000000L         /* DOS, OS2, Win95 */
#define F_OS_AMIGA      0x01000000L
#define F_OS_VMS        0x02000000L
#define F_OS_UNIX       0x03000000L
#define F_OS_VM_CMS     0x04000000L
#define F_OS_ATARI      0x05000000L
#define F_OS_OS2        0x06000000L         /* OS2 */
#define F_OS_MAC9       0x07000000L
#define F_OS_Z_SYSTEM   0x08000000L
#define F_OS_CPM        0x09000000L
#define F_OS_TOPS20     0x0a000000L
#define F_OS_NTFS       0x0b000000L         /* Win NT/2000/XP */
#define F_OS_QDOS       0x0c000000L
#define F_OS_ACORN      0x0d000000L
#define F_OS_VFAT       0x0e000000L         /* Win32 */
#define F_OS_MFS        0x0f000000L
#define F_OS_BEOS       0x10000000L
#define F_OS_TANDEM     0x11000000L
#define F_OS_SHIFT      24
#define F_OS_MASK       0xff000000L

/* character set for file name encoding [mostly unused] */
#define F_CS_NATIVE     0x00000000L
#define F_CS_LATIN1     0x00100000L
#define F_CS_DOS        0x00200000L
#define F_CS_WIN32      0x00300000L
#define F_CS_WIN16      0x00400000L
#define F_CS_UTF8       0x00500000L         /* filename is UTF-8 encoded */
#define F_CS_SHIFT      20
#define F_CS_MASK       0x00f00000L

/* these bits must be zero */
#define F_RESERVED      ((F_MASK | F_OS_MASK | F_CS_MASK) ^ 0xffffffffL)

typedef struct
{
    unsigned version;
    unsigned lib_version;
    unsigned version_needed_to_extract;
    unsigned char method;
    unsigned char level;
    lzo_uint32 flags;
    lzo_uint32 filter;
    lzo_uint32 mode;
    lzo_uint32 mtime_low;
    lzo_uint32 mtime_high;
    lzo_uint32 header_checksum;

    lzo_uint32 extra_field_len;
    lzo_uint32 extra_field_checksum;

/* info */
    const char *method_name;

    char name[255+1];
}
header_t;


/*************************************************************************
// lzop.c
**************************************************************************/

void set_err_nl(lzo_bool x);
void warn(file_t *ft, const char *m);
void error(file_t *ft, const char *m);
void fatal(file_t *ft, const char *m);
void read_error(file_t *ft);
void write_error(file_t *ft);
void e_memory(void);

lzo_int read_buf(file_t *ft, lzo_voidp buffer, lzo_int cnt);
void write_buf(file_t *ft, const lzo_voidp buffer, lzo_int cnt);
void read32(file_t *ft, lzo_uint32 *v);
void write32(file_t *ft, lzo_uint32 v);

void init_compress_header(header_t *h, const file_t *fip, const file_t *fop);
void write_header(file_t *ft, const header_t *h);

void do_test(const header_t *h, lzop_ulong_t d_len, lzop_ulong_t c_len);
void do_test_total(void);
void do_info(const header_t *h, lzop_ulong_t d_len, lzop_ulong_t c_len);
void do_ls(const header_t *h, lzop_ulong_t d_len, lzop_ulong_t c_len);
void do_list(const header_t *h, lzop_ulong_t d_len, lzop_ulong_t c_len);
void do_list_total(void);


/*************************************************************************
// compress.c
**************************************************************************/

lzo_bool x_enter(const header_t *h);
void x_leave(const header_t *h);

lzo_bool x_compress(file_t *fip, file_t *fop, header_t *h);
lzo_bool x_decompress(file_t *fip, file_t *fop, const header_t *h, lzo_bool skip);

int x_get_method(header_t *h);
int x_set_method(int m, int l);

void x_filter(lzo_bytep p, lzo_uint l, const header_t *h);


/*************************************************************************
// p_lzo.c
**************************************************************************/

#if defined(WITH_LZO)

lzo_bool lzo_enter(const header_t *h);
void lzo_leave(const header_t *h);

lzo_bool lzo_compress(file_t *fip, file_t *fop, const header_t *h);
lzo_bool lzo_decompress(file_t *fip, file_t *fop, const header_t *h, lzo_bool skip);

int lzo_get_method(header_t *h);
int lzo_set_method(int m, int l);
void lzo_init_compress_header(header_t *h);

#endif


/*************************************************************************
// p_lzo_mt.c
**************************************************************************/

#if defined(WITH_LZO) && defined(WITH_THREADS)

lzo_bool lzo_threaded_enter(const header_t *h);
void lzo_threaded_leave(const header_t *h);

lzo_bool lzo_threaded_compress(file_t *fip, file_t *fop, const header_t *h);
lzo_bool lzo_threaded_decompress(file_t *fip, file_t *fop, const header_t *h, lzo_bool skip);

#endif


/*************************************************************************
// filter.c
**************************************************************************/

void t_sub1(lzo_bytep p, lzo_uint l);
void t_add1(lzo_bytep p, lzo_uint l);

void t_sub(lzo_bytep p, lzo_uint l, int n);
void t_add(lzo_bytep p, lzo_uint l, int n);

void t_mtf(lzo_bytep p, lzo_uint l);
void t_unmtf(lzo_bytep p, lzo_uint l);


/*************************************************************************
// mblock.c
**************************************************************************/

typedef struct
{
    /* public */
    lzo_bytep   mb_mem;
    lzo_uint32  mb_size;
    /* private */
    lzo_bytep   mb_mem_alloc;
    lzo_uint32  mb_size_alloc;
    lzo_uint32  mb_align;
    /* the following fields are not yet used but may prove useful for
     * adding new algorithms */
    lzo_uint32  mb_flags;
    lzo_uint32  mb_id;
    lzo_uint32  mb_len;
    lzo_uint32  mb_adler32;
    lzo_uint32  mb_crc32;
}
mblock_t;
#define mblock_p      mblock_t *

lzo_bool mb_alloc(mblock_p m, lzo_uint32 size, lzo_uint align);
void mb_free(mblock_p m);

lzo_bool mb_init(mblock_p m, lzo_uint32 size, lzo_uint align,
                 lzo_voidp heap, lzo_uint32 heap_size);


/*************************************************************************
// util.c
**************************************************************************/

enum {
    SUFF_NONE,
    SUFF_LZO,
    SUFF_LZOP,
    SUFF_NRV,
    SUFF_TAR,
    SUFF_TNV,
    SUFF_TZO,
    SUFF_USER
};

unsigned fn_baseindex(const char *name);
const char *fn_basename(const char *name);
void fn_addslash(char *n, lzo_bool slash);
char *fn_strlwr(char *n);
int fn_strcmp(const char *n1, const char *n2);
lzo_bool fn_is_same_file(const char *n1, const char *n2);
int fn_has_suffix(const char *name);
int fn_cleanpath(const char *name, char *newname, size_t size, int flags);

time_t fix_time(time_t t);
time_t get_mtime(const header_t *h);
#if defined(HAVE_LOCALTIME)
void tm2str(char *s, size_t size, const struct tm *tmp);
#endif
void time2str(char *s, size_t size, const time_t *t);
void time2ls(char *s, size_t size, const time_t *t);

lzo_bool file_exists(const char *name);

lzo_uint32 fix_mode_for_header(lzo_uint32 mode);
MODE_T fix_mode_for_chmod(lzo_uint32 mode);
MODE_T fix_mode_for_ls(lzo_uint32 mode);
MODE_T fix_mode_for_open(MODE_T mode);

void mode_string(MODE_T mode, char *str);
char *maybe_rename_file(const char *original_name);


/*************************************************************************
// other globals
**************************************************************************/

extern const char *progname;
extern MODE_T u_mask;
extern time_t current_time;

void sysinfo(void);
void license(void);
void head(void);
void help(void);
void usage(void);
void version(void);

void sysinfo_djgpp(void);


/*************************************************************************
// LZO section
**************************************************************************/

#if defined(WITH_LZO)

#define USE_LZO1X_1 1
#if 1
#  define USE_LZO1X_999 1
#endif
#if !defined(ACC_OS_DOS16)
#  define USE_LZO1X_1_15 1
#endif

#if defined(LZO_USE_ASM_1)
#if !defined(LZO_EXTERN_CDECL)
#define LZO_EXTERN_CDECL(r) LZO_EXTERN(r)
#endif
LZO_EXTERN_CDECL(int)
lzo1x_decompress_asm_fast
                        ( const lzo_bytep src, lzo_uint  src_len,
                                lzo_bytep dst, lzo_uintp dst_len,
                                lzo_voidp wrkmem /* NOT USED */ );
LZO_EXTERN_CDECL(int)
lzo1x_decompress_asm_fast_safe
                        ( const lzo_bytep src, lzo_uint  src_len,
                                lzo_bytep dst, lzo_uintp dst_len,
                                lzo_voidp wrkmem /* NOT USED */ );
#  define lzo1x_decompress          lzo1x_decompress_asm_fast
#  define lzo1x_decompress_safe     lzo1x_decompress_asm_fast_safe
#elif defined(LZO_USE_ASM_2)
LZO_EXTERN_CDECL(int)
_lzo1x_decompress_asm_fast
                        ( const lzo_bytep src, lzo_uint  src_len,
                                lzo_bytep dst, lzo_uintp dst_len,
                                lzo_voidp wrkmem /* NOT USED */ );
LZO_EXTERN_CDECL(int)
_lzo1x_decompress_asm_fast_safe
                        ( const lzo_bytep src, lzo_uint  src_len,
                                lzo_bytep dst, lzo_uintp dst_len,
                                lzo_voidp wrkmem /* NOT USED */ );
#  define lzo1x_decompress          _lzo1x_decompress_asm_fast
#  define lzo1x_decompress_safe     _lzo1x_decompress_asm_fast_safe
#endif

#endif /* WITH_LZO */


/*************************************************************************
//
**************************************************************************/

#include "console.h"


#endif /* already included */


/*
vi:ts=4:et
*/

