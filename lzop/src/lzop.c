/* lzop.c --

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


#include "conf.h"
#include "version.h"


/*************************************************************************
// options
**************************************************************************/

int opt_cmd = CMD_NONE;
int opt_method = 0;
int opt_level = 0;
int opt_filter = 0;

int opt_checksum = -1;
int opt_console = CON_INIT;
lzo_bool opt_crc32 = 0;
lzo_bool opt_decompress_safe = 1;
lzo_bool opt_file = 0;
int opt_force = 0;
lzo_bool opt_ignorewarn = 0;
lzo_bool opt_keep = 0;
int opt_num_threads = 1; /* NOT YET IMPLEMENTED */
#ifdef MAINT
int opt_noheader = 0;
#endif
lzo_bool opt_nowarn = 0;
const char *opt_ls_flags = "";
int opt_name = -1;
const char *opt_output_name = NULL;
const char *opt_output_path = NULL;
lzo_bool opt_optimize = 0;
lzo_bool opt_path = 0;
lzo_bool opt_restore_mode = 1;
lzo_bool opt_restore_time = 1;
lzo_bool opt_shortname = 0;
int opt_stdin = 0;
lzo_bool opt_stdout = 0;
char opt_suffix[1+SUFFIX_MAX+1] = { 0 };
lzo_bool opt_unlink = 0;
int opt_verbose = 1;

static int done_output_name = 0;
static int done_output_path = 0;
static int done_suffix = 0;

/* invocation options */
enum {
    PGM_LZOP,
    PGM_UNLZOP,
    PGM_OCAT
};

int opt_pgm = PGM_LZOP;


const char *argv0 = "";
const char *progname = "";
MODE_T u_mask = 0700;
time_t current_time;

FILE *con_term = NULL;


static int num_files = -1;
static int exit_code = EXIT_OK;

static file_t fi;
static file_t fo;

static unsigned long total_d_files = 0;
static unsigned long total_c_files = 0;
static lzop_ulong_t total_d_len = 0;
static lzop_ulong_t total_c_len = 0;

/* some statistics */
static lzop_ulong_t total_bytes_written = 0;
static lzop_ulong_t total_bytes_read = 0;


/*************************************************************************
// exit handlers
**************************************************************************/

static void do_exit(void)
{
    static lzo_bool in_exit = 0;

    if (in_exit)
        exit(exit_code);
    in_exit = 1;

    fflush(con_term);
    fflush(stderr);
    exit(exit_code);
}


#define EXIT_FATAL  3

static lzo_bool set_eec(int ec, int *eec)
{
    if (ec == EXIT_FATAL)
    {
        *eec = EXIT_ERROR;
        return 1;
    }
    else if (ec < 0 || ec == EXIT_ERROR)
    {
        *eec = EXIT_ERROR;
    }
    else if (ec == EXIT_WARN)
    {
        if (!opt_ignorewarn)
            if (*eec == EXIT_OK)
                *eec = ec;
    }
    else if (ec == EXIT_OK)
    {
        /* do nothing */
    }
    else
    {
        assert(0);
    }
    return 0;
}

static lzo_bool set_ec(int ec)
{
    return set_eec(ec,&exit_code);
}


void e_exit(int ec)
{
    (void) set_ec(ec);
    do_exit();
}


void e_usage(void)
{
    usage();
    e_exit(EXIT_USAGE);
}


void e_memory(void)
{
    head();
    fflush(con_term);
    fprintf(stderr,"%s: out of memory\n", argv0);
    e_exit(EXIT_MEMORY);
}


void e_method(int m)
{
    fflush(con_term);
#if (UINT_MAX < LZO_0xffffffffL)
    /* 16-bit DOS/Windows */
    fprintf(stderr,"%s: 16-bit versions are not officially supported\n", argv0);
#endif
    fprintf(stderr,"%s: illegal method option -- %c\n", argv0, m & 255);
    e_usage();
}


#if defined(OPTIONS_VAR)
void e_envopt(const char *n)
{
    fflush(con_term);
    if (n)
        fprintf(stderr,"%s: invalid string '%s' in environment variable '%s'\n",
                        argv0, n, OPTIONS_VAR);
    else
        fprintf(stderr,"%s: illegal option in environment variable '%s'\n",
                        argv0, OPTIONS_VAR);
    e_exit(EXIT_USAGE);
}
#endif


RETSIGTYPE __acc_cdecl_sighandler e_sighandler(acc_signo_t signo)
{
    UNUSED(signo);
    e_exit(EXIT_FATAL);
}


/*************************************************************************
// error handlers
**************************************************************************/

static void do_error(file_t *ft, const char *n, const char *msg, int ec, int err)
{
    const char *fn;
    const char *errmsg;

    fflush(con_term);
    if (!(ec == EXIT_WARN && (opt_nowarn || opt_ignorewarn || opt_verbose == 0)))
    {
        fn = ft && ft->name[0] ? ft->name : UNKNOWN_NAME;
        fprintf(stderr, "%s%s: %s: ", n, progname, fn);
        if (ec == EXIT_WARN)
            fprintf(stderr, "warning: ");
        if (err != 0)
        {
            errmsg = strerror(err);
            if (msg && msg[0])
                fprintf(stderr, "%s: %s\n", msg, errmsg);
            else
                fprintf(stderr, "%s\n", errmsg);
        }
        else
            fprintf(stderr, "%s\n", msg);
        fflush(stderr);
    }
    if (set_ec(ec))
        do_exit();
}


static const char *err_nl = "";
void set_err_nl(lzo_bool x)
{
    err_nl = x ? "\n" : "";
}


void fatal(file_t *ft, const char *msg)
{
    do_error(ft,err_nl,msg,EXIT_FATAL,0);
}

void error(file_t *ft, const char *msg)
{
    do_error(ft,err_nl,msg,EXIT_ERROR,0);
}

void warn(file_t *ft, const char *msg)
{
    do_error(ft,err_nl,msg,EXIT_WARN,0);
}

void info(file_t *ft, const char *msg)
{
    do_error(ft,err_nl,msg,EXIT_OK,0);
}

void p_fatal(file_t *ft, const char *msg)
{
    do_error(ft,err_nl,msg,EXIT_FATAL,errno);
}

void p_error(file_t *ft, const char *msg)
{
    do_error(ft,err_nl,msg,EXIT_ERROR,errno);
}

void p_warn(file_t *ft, const char *msg)
{
    do_error(ft,err_nl,msg,EXIT_WARN,errno);
}


void read_error(file_t *ft)
{
    const char *fn = ft && ft->name[0] ? ft->name : UNKNOWN_NAME;
    const char *errmsg = "unexpected end of file";
    if (errno != 0)
        errmsg = strerror(errno);
    fflush(con_term);
    fprintf(stderr, "%s%s: %s: %s\n", err_nl, progname, errmsg, fn);
    e_exit(EXIT_FATAL);
}


void write_error(file_t *ft)
{
    const char *fn = ft && ft->name[0] ? ft->name : UNKNOWN_NAME;
    const char *errmsg = "write error";
    if (errno != 0)
        errmsg = strerror(errno);
    fflush(con_term);
    fprintf(stderr, "%s%s: %s: %s\n", err_nl, progname, errmsg, fn);
    e_exit(EXIT_FATAL);
}


/*************************************************************************
// file_t
**************************************************************************/

void f_reset(file_t *ft)
{
    ft->opt_name = opt_name;
    ft->part = 0;
    ft->bytes_read = 0;
    ft->bytes_written = 0;
    ft->warn_multipart = 0;
    ft->warn_unknown_suffix = 0;
}


void f_init(void)
{
    memset(&fi,0,sizeof(file_t));
    memset(&fo,0,sizeof(file_t));
    fi.fd = -1;
    fo.fd = -1;
#if defined(USE_FOPEN)
    fi.file = NULL;
    fo.file = NULL;
#endif
    f_reset(&fi);
    f_reset(&fo);
}


int f_open(file_t *ft, lzo_bool r)
{
    assert(ft->name[0]);
    ft->fd = -1;
#if defined(O_BINARY)
    ft->open_flags |= O_BINARY;
#endif
#if (ACC_OS_WIN32 || ACC_OS_WIN64) && defined(_O_SEQUENTIAL)
    ft->open_flags |= _O_SEQUENTIAL;
#endif
#if defined(USE_FOPEN)
    ft->file = NULL;
    if (r)
        ft->file = fopen(ft->name,"rb");
    else if (ft->open_flags & O_EXCL)
    {
        if (file_exists(ft->name))
            errno = EEXIST;
        else
            ft->file = fopen(ft->name,"wb");
    }
    else
        ft->file = fopen(ft->name,"wb");
    if (ft->file != NULL)
    {
        ft->fd = fileno(ft->file);
        assert(ft->fd >= 0);
    }
#else
    if (r)
        ft->fd = open(ft->name, ft->open_flags, 0);
    else
    {
#if defined(O_EXCL_BROKEN)
        if ((ft->open_flags & O_EXCL) && file_exists(ft->name))
            errno = EEXIST;
        else
#endif
            ft->fd = open(ft->name, ft->open_flags, ft->st.st_mode);
    }
#endif
    if (ft->fd >= 0 && (ft->fd == STDIN_FILENO || ft->fd == STDOUT_FILENO || ft->fd == STDERR_FILENO))
    {
        fatal(ft,"sanity check failed: f_open()");
        ft->fd = -1;
    }
    return ft->fd;
}


int f_close(file_t *ft)
{
    int r;

    if (ft->fd < 0)
        return 0;
    if (ft->fd == STDIN_FILENO || ft->fd == STDOUT_FILENO || ft->fd == STDERR_FILENO)
        return 0;
#if defined(USE_FOPEN)
    assert(ft->file != NULL);
    r = fclose(ft->file);
    ft->file = NULL;
#else
    r = close(ft->fd);
#endif
    ft->fd = -1;
    return r;
}


/*************************************************************************
// read and write
// handles partial pipe writes and interrupted system calls
**************************************************************************/

lzo_int read_buf(file_t *ft, lzo_voidp buffer, lzo_int cnt)
{
    lzo_int n;
    long l;

    assert(cnt >= 0 && cnt < LONG_MAX);
    l = acc_safe_hread(ft->fd, buffer, (long) cnt);
    n = (lzo_int) l;
    assert(n >= 0); assert(n == l);

    ft->bytes_read += n;
    total_bytes_read += n;
    return n;
}


void write_buf(file_t *ft, const lzo_voidp buffer, lzo_int cnt)
{
    lzo_int n;
    long l;

    assert(cnt >= 0 && cnt < LONG_MAX);
    if (ft->fd < 0)
        return;
    l = acc_safe_hwrite(ft->fd, buffer, (long) cnt);
    n = (lzo_int) l;
    assert(n >= 0); assert(n == l);

    ft->bytes_written += n;
    total_bytes_written += n;
    if (n != cnt)
        write_error(ft);
}


/*************************************************************************
// misc IO
**************************************************************************/

static unsigned get_be16(const unsigned char *b)
{
    unsigned v;

    v  = (unsigned) b[1] <<  0;
    v |= (unsigned) b[0] <<  8;
    return v;
}

static void set_be16(unsigned char *b, unsigned v)
{
    b[1] = (unsigned char) (v >>  0);
    b[0] = (unsigned char) (v >>  8);
}


static lzo_uint32 get_be32(const unsigned char *b)
{
    lzo_uint32 v;

    v  = (lzo_uint32) b[3] <<  0;
    v |= (lzo_uint32) b[2] <<  8;
    v |= (lzo_uint32) b[1] << 16;
    v |= (lzo_uint32) b[0] << 24;
    return v;
}

static void set_be32(unsigned char *b, lzo_uint32 v)
{
    b[3] = (unsigned char) (v >>  0);
    b[2] = (unsigned char) (v >>  8);
    b[1] = (unsigned char) (v >> 16);
    b[0] = (unsigned char) (v >> 24);
}


#if 0 /* NOT USED */
static void write8(file_t *ft, int v)
{
    unsigned char b = (unsigned char) v;
    write_buf(ft,&b,1);
}
#endif


void read32(file_t *ft, lzo_uint32 *v)
{
    unsigned char b[4];
    if (read_buf(ft,b,4) != 4)
        read_error(ft);
    *v = get_be32(b);
}

void write32(file_t *ft, lzo_uint32 v)
{
    unsigned char b[4];
    set_be32(b,v);
    write_buf(ft,b,4);
}


static int f_read8(file_t *ft, unsigned char *b)
{
    unsigned char bb;
    if (read_buf(ft,&bb,1) != 1)
        read_error(ft);
    ft->f_adler32 = lzo_adler32(ft->f_adler32,&bb,1);
    ft->f_crc32 = lzo_crc32(ft->f_crc32,&bb,1);
    if (b)
        *b = bb;
    return bb;
}

static void f_write8(file_t *ft, int v)
{
    unsigned char b = (unsigned char) v;
    write_buf(ft,&b,1);
    ft->f_adler32 = lzo_adler32(ft->f_adler32,&b,1);
    ft->f_crc32 = lzo_crc32(ft->f_crc32,&b,1);
}


static void f_read16(file_t *ft, unsigned *v)
{
    unsigned char b[2];
    if (read_buf(ft,b,2) != 2)
        read_error(ft);
    ft->f_adler32 = lzo_adler32(ft->f_adler32,b,2);
    ft->f_crc32 = lzo_crc32(ft->f_crc32,b,2);
    *v = get_be16(b);
}

static void f_write16(file_t *ft, unsigned v)
{
    unsigned char b[2];
    set_be16(b,v);
    write_buf(ft,b,2);
    ft->f_adler32 = lzo_adler32(ft->f_adler32,b,2);
    ft->f_crc32 = lzo_crc32(ft->f_crc32,b,2);
}


static void f_read32(file_t *ft, lzo_uint32 *v)
{
    unsigned char b[4];
    if (read_buf(ft,b,4) != 4)
        read_error(ft);
    ft->f_adler32 = lzo_adler32(ft->f_adler32,b,4);
    ft->f_crc32 = lzo_crc32(ft->f_crc32,b,4);
    *v = get_be32(b);
}

static void f_write32(file_t *ft, lzo_uint32 v)
{
    unsigned char b[4];
    set_be32(b,v);
    write_buf(ft,b,4);
    ft->f_adler32 = lzo_adler32(ft->f_adler32,b,4);
    ft->f_crc32 = lzo_crc32(ft->f_crc32,b,4);
}


static void f_write(file_t *ft, const lzo_voidp buf, lzo_int cnt)
{
    if (cnt > 0)
    {
        write_buf(ft,buf,cnt);
        ft->f_adler32 = lzo_adler32(ft->f_adler32,(const lzo_bytep)buf,cnt);
        ft->f_crc32 = lzo_crc32(ft->f_crc32,(const lzo_bytep)buf,cnt);
    }
}

static lzo_int f_read(file_t *ft, lzo_voidp buf, lzo_int cnt)
{
    cnt = read_buf(ft,buf,cnt);
    if (cnt > 0)
    {
        ft->f_adler32 = lzo_adler32(ft->f_adler32,(const lzo_bytep)buf,cnt);
        ft->f_crc32 = lzo_crc32(ft->f_crc32,(const lzo_bytep)buf,cnt);
    }
    return cnt;
}


/***********************************************************************
// lzop file signature
************************************************************************/

/*
 * The first nine bytes of a lzop file always contain the following values:
 *
 *                             0   1   2   3   4   5   6   7   8
 *                           --- --- --- --- --- --- --- --- ---
 * (hex)                      89  4c  5a  4f  00  0d  0a  1a  0a
 * (decimal)                 137  76  90  79   0  13  10  26  10
 * (C notation - ASCII)     \211   L   Z   O  \0  \r  \n \032 \n
 */

static const unsigned char lzop_magic[9] =
    { 0x89, 0x4c, 0x5a, 0x4f, 0x00, 0x0d, 0x0a, 0x1a, 0x0a };

static const char * const header_error[] = {
    "[0]",
    "not a " PACKAGE " file",                                           /*  1 */
    "header corrupted (checksum error)",                                /*  2 */
    "header corrupted",                                                 /*  3 */
    "header corrupted (DOS -> UNIX conversion ?)",                      /*  4 */
    "header corrupted (DOS -> Mac conversion ?)",                       /*  5 */
    "header corrupted (UNIX -> Mac conversion ?)",                      /*  6 */
    "header corrupted (Mac -> UNIX conversion ?)",                      /*  7 */
    "header corrupted (UNIX -> DOS conversion ?)",                      /*  8 */
    "header corrupted (end of line conversion ?)",                      /*  9 */
    "header corrupted (DOS EOF conversion ?)",                          /* 10 */
    "header corrupted (transmitted through a 7-bit channel ?)",         /* 11 */
    "header corrupted (transmitted in text mode ?)",                    /* 12 */
    "unknown header flags -- get a newer version of " PACKAGE,          /* 13 */
    "unknown compression method -- get a newer version of " PACKAGE,    /* 14 */
    "unknown compression level -- get a newer version of " PACKAGE,     /* 15 */
    "you need a newer version of " PACKAGE,                             /* 16 */
    "compression method not supported -- recompile " PACKAGE,           /* 17 */
    "decompression method not supported -- recompile " PACKAGE,         /* 18 */
    NULL
};


static int check_magic(const unsigned char *magic)
{
    const unsigned char *m;

    if (memcmp(magic,lzop_magic,sizeof(lzop_magic)) == 0)
        return 0;

    /* We have a bad magic signature. Try to figure what possibly
     * could have gone wrong. */

    /* look at bytes 1-3: "LZO" in hex and local text format */
    if (memcmp(&magic[1],&lzop_magic[1],3) != 0 &&
        memcmp(&magic[1],"LZO",3) != 0)
        return 1;

    /* look at byte 4 */
    if (magic[4] != lzop_magic[4])
        return 1;

    /* look at bytes 5-8 */
    m = &magic[5];
    if (memcmp(m,"\012\012\032",3) == 0)
        return 7;
    if (memcmp(m,"\012\012",2) == 0)
        return 4;
    if (memcmp(m,"\012\032",2) == 0)
        return 4;
    if (memcmp(m,"\015\012\012",3) == 0)
        return 10;
    if (memcmp(m,"\015\012\032\012",4) == 0)
        return 9;
    if (memcmp(m,"\015\012\032\015",4) == 0)
        return 8;
    if (memcmp(m,"\015\015\012\032",4) == 0)
        return 8;
    if (memcmp(m,"\015\015\032",3) == 0)
        return 6;
    if (memcmp(m,"\015\032",2) == 0)
        return 5;
    if (memcmp(m,&lzop_magic[5],4) != 0)
        return 12;

    /* look at byte 0 */
    if (magic[0] == (unsigned char) (lzop_magic[0] & 0x7f))
        return 11;
    if (magic[0] != lzop_magic[0])
        return 12;

    return 3;
}


/*************************************************************************
// lzop file header
**************************************************************************/

void init_compress_header(header_t *h, const file_t *fip, const file_t *fop)
{
    assert(opt_method > 0);
    assert(opt_level > 0);
    assert(fip->st.st_mode == 0 || S_ISREG(fip->st.st_mode));

    memset(h,0,sizeof(header_t));

    h->version = LZOP_VERSION & 0xffff;
    h->version_needed_to_extract = opt_filter ? 0x0950: 0x0940;
    h->lib_version = lzo_version() & 0xffff;
    h->method = (unsigned char) opt_method;
    h->level = (unsigned char) opt_level;
    h->filter = opt_filter;

    h->flags = 0;
    h->flags |= F_OS & F_OS_MASK;
    h->flags |= F_CS & F_CS_MASK;
    if (opt_filter)
        h->flags |= F_H_FILTER;
    if (fip->fd == STDIN_FILENO)
        h->flags |= F_STDIN;
    if (fop->fd == STDOUT_FILENO)
        h->flags |= F_STDOUT;
    if (!opt_file && num_files > 1)
        h->flags |= F_MULTIPART;
#ifdef OPT_NAME_DEFAULT
    h->flags |= F_NAME_DEFAULT;
#endif
#ifdef DOSISH
    h->flags |= F_DOSISH;
#endif
    if (opt_crc32)
    {
        h->flags |= F_H_CRC32;
        if (h->version_needed_to_extract < 0x1001)
            h->version_needed_to_extract = 0x1001;
    }

    h->mode = fix_mode_for_header(fip->st.st_mode);

    if (fip->st.st_mtime)
    {
        h->mtime_low = (lzo_uint32) (fip->st.st_mtime);
        h->mtime_high = (lzo_uint32) (fip->st.st_mtime >> 16 >> 16);
        if ((lzo_int32) h->mtime_high < 0)
            h->mtime_high = 0;
    }

    if (fip->name[0] && fip->fd != STDIN_FILENO)
    {
        int r = 0;
        if (opt_path)
        {
            char newname[255+1];
            r = fn_cleanpath(fip->name, newname, sizeof(newname), 0);
            if (r > 0 && newname[0] && strlen(newname) <= 255)
            {
                strcpy(h->name, newname);
                h->flags |= F_H_PATH;
                if (h->version_needed_to_extract < 0x1001)
                    h->version_needed_to_extract = 0x1001;
            }
            else
                r = 0;
        }
        if (r == 0)
        {
            const char *n = fn_basename(fip->name);
            if (n[0] && strlen(n) <= 255)
                strcpy(h->name, n);
        }
    }
}


void write_header(file_t *ft, const header_t *h)
{
    size_t l;

#ifdef MAINT
    /* undocumented option '--no-header'. just for testing. */
    if (opt_noheader > 0)
    {
        switch (opt_noheader)
        {
        case 1:
            write32(ft,h->flags);
            break;
        case 2:
            write32(ft,h->flags);
            write8(ft,h->method);
            write8(ft,h->level);
            break;
        default:
            /* write no header at all */
            break;
        }
        return;
    }
#endif

    write_buf(ft,lzop_magic,sizeof(lzop_magic));

    ft->f_adler32 = ADLER32_INIT_VALUE;
    ft->f_crc32 = CRC32_INIT_VALUE;

    f_write16(ft,h->version);
    f_write16(ft,h->lib_version);
    f_write16(ft,h->version_needed_to_extract);
    f_write8(ft,h->method);
    f_write8(ft,h->level);
    f_write32(ft,h->flags);
    if (h->flags & F_H_FILTER)
        f_write32(ft,h->filter);
    f_write32(ft,h->mode);
    f_write32(ft,h->mtime_low);
    f_write32(ft,h->mtime_high);

    l = strlen(h->name);
    assert(l <= 255);
    f_write8(ft,(int)l);
    if (l > 0)
        f_write(ft,h->name,(int)l);

    if (h->flags & F_H_CRC32)
        f_write32(ft,ft->f_crc32);
    else
        f_write32(ft,ft->f_adler32);
}


static int read_header(file_t *ft, header_t *h)
{
    int r;
    int l;
    lzo_uint32 checksum;

    memset(h,0,sizeof(header_t));
    h->version_needed_to_extract = 0x0900;  /* first public lzop version */
    h->level = 0;
    h->method_name = "unknown";

    ft->f_adler32 = ADLER32_INIT_VALUE;
    ft->f_crc32 = CRC32_INIT_VALUE;

    f_read16(ft,&h->version);
    if (h->version < 0x0900)
        return 3;
    f_read16(ft,&h->lib_version);
    if (h->version >= 0x0940)
    {
        f_read16(ft,&h->version_needed_to_extract);
        if (h->version_needed_to_extract > LZOP_VERSION)
            return 16;
        if (h->version_needed_to_extract < 0x0900)
            return 3;
    }
    f_read8(ft,&h->method);
    if (h->version >= 0x0940)
        f_read8(ft,&h->level);
    f_read32(ft,&h->flags);
    if (h->flags & F_H_FILTER)
        f_read32(ft,&h->filter);
    f_read32(ft,&h->mode);
#if 1
    if (h->flags & F_STDIN) /* do not use mode from stdin compression */
        h->mode = 0;
#endif
    f_read32(ft,&h->mtime_low);
    if (h->version >= 0x0940)
        f_read32(ft,&h->mtime_high);
    if (h->version < 0x0120) {
        if (h->mtime_low == 0xffffffffUL)
            h->mtime_low = 0;
        h->mtime_high = 0;
    }

    l = f_read8(ft,NULL);
    if (l > 0) {
        char name[255+1];
        if (f_read(ft,name,l) != l)
            read_error(ft);
        name[l] = 0;
        if (fn_cleanpath(name, h->name, 255+1, 1|2) < 0)
            h->name[0] = 0;
    }

    checksum = (h->flags & F_H_CRC32) ? ft->f_crc32 : ft->f_adler32;
    f_read32(ft,&h->header_checksum);
    if (h->header_checksum != checksum)
        return 2;

    if (h->method <= 0)
        return 14;
    r = x_get_method(h);
    if (r != 0)
        return r;

/* check reserved flags */
    if (h->flags & F_RESERVED)
        return (opt_force >= 2) ? -13 : 13;

/* skip extra field [not used yet] */
    if (h->flags & F_H_EXTRA_FIELD)
    {
        lzo_uint32 k;

        /* note: the checksum also covers the length */
        ft->f_adler32 = ADLER32_INIT_VALUE;
        ft->f_crc32 = CRC32_INIT_VALUE;
        f_read32(ft,&h->extra_field_len);
        for (k = 0; k < h->extra_field_len; k++)
            (void) f_read8(ft,NULL);
        checksum = (h->flags & F_H_CRC32) ? ft->f_crc32 : ft->f_adler32;
        f_read32(ft,&h->extra_field_checksum);
        if (h->extra_field_checksum != checksum)
            return 3;
        if (opt_verbose >= 2)
            info(ft,"ignoring extra field");
    }

    return 0;
}


/* return 0 for valid magic, -1 for EOF, or positive value for error */
static int p_magic(file_t *ft)
{
    int r;
    lzo_int l;
    unsigned char magic[sizeof(lzop_magic)];

    l = read_buf(ft,magic,sizeof(magic));
    if (ft->part > 0 && l <= 0)
        return -1;
    if (l == (lzo_int) sizeof(magic))
        r = check_magic(magic);
    else
        r = 1;
    assert(r >= 0);
    if (ft->part > 0 && r == 1)
    {
#if 1
        /* gzip: check for trailing zero bytes */
        unsigned char b;
        while (--l >= 0)
            if (magic[(int)l] != '\0')
                goto garbage;
        while (read_buf(ft,&b,1) == 1)
            if (b != '\0')
                goto garbage;
        if (opt_verbose >= 2)
            warn(ft,"ignoring trailing zero bytes in " PACKAGE " file");
        return -1;
garbage:
#endif
        warn(ft,"ignoring trailing garbage in " PACKAGE " file");
        return -1;
    }
    if (r != 0)
    {
        assert(r > 0 && r <= 18);
        error(ft,header_error[r]);
    }
    return r;
}


static lzo_bool p_header(file_t *ft, header_t *h)
{
    int r;

    r = read_header(ft,h);
    if (r == 0)
        return 1;
    if (r < 0)
    {
        r = -r;
        assert(r > 0 && r <= 18);
        error(ft,header_error[r]);
        return 1;
    }
    else
    {
        assert(r > 0 && r <= 18);
        error(ft,header_error[r]);
        return 0;
    }
}


/*************************************************************************
// test
**************************************************************************/

void do_test(const header_t *h, lzop_ulong_t d_len, lzop_ulong_t c_len)
{
    total_d_files++;
    total_c_len += c_len;
    total_d_len += d_len;
    UNUSED(h);
}


void do_test_total(void)
{
    FILE *f;

    if ((total_c_files < 2 && total_d_files < 2) || opt_verbose < 2)
        return;

    f = stderr;
    fprintf(f,"%lu file%s successfully tested", total_c_files,
        total_c_files == 1 ? " was" : "s were");
    if (total_c_files != total_d_files)
        fprintf(f," [containing %lu files]", total_d_files);
    fprintf(f,"\n");
    fflush(f);
}


/*************************************************************************
// list a file
**************************************************************************/

static unsigned long get_ratio(lzop_ulong_t d_len, lzop_ulong_t c_len)
{
    unsigned long n1 = 1000L * 1000L;
    unsigned long n2 = 1;
    const lzop_ulong_t umax = ~((lzop_ulong_t)0);

    if (d_len <= 0)
        return c_len <= 0 ? 0ul : n1;
    while (n1 > 1 && c_len > (umax / n1))
    {
        n1 /= 10;
        n2 *= 10;
    }
    return (unsigned long) ((c_len * n1) / (d_len / n2));
}


static void pr_size(FILE *f, lzop_ulong_t a, lzop_ulong_t b, int flags)
{
    unsigned long ratio, r1, r2, al, bl;

    ratio = (flags & 1) ? get_ratio(a, b) : get_ratio(b, a);
    ratio += 500;   /* for rounding */
    r1 = ratio / 10000;
    r2 = (ratio % 10000) / 1000;

#if (SIZEOF_LONG >= 8) || !defined(acc_int64l_t)
    al = (unsigned long) a;
    bl = (unsigned long) b;
    fprintf(f,"%9lu %9lu %3lu.%01lu%%", al, bl, r1, r2);
#else
    al = (unsigned long) (a % 1000000000ul);
    bl = (unsigned long) (b % 1000000000ul);
    if (a == al && b == bl)
    {
        fprintf(f,"%9lu %9lu %3lu.%01lu%%", al, bl, r1, r2);
    }
    else if (a == al)
    {
        unsigned long bh = (unsigned long) (b / 1000000000ul);
        fprintf(f,"%9lu %lu%09lu %3lu.%01lu%%", al, bh, bl, r1, r2);
    }
    else if (b == bl)
    {
        unsigned long ah = (unsigned long) (a / 1000000000ul);
        fprintf(f,"%lu%09lu %9lu %3lu.%01lu%%", ah, al, bl, r1, r2);
    }
    else
    {
        unsigned long ah = (unsigned long) (a / 1000000000ul);
        unsigned long bh = (unsigned long) (b / 1000000000ul);
        fprintf(f,"%lu%09lu %lu%09lu %3lu.%01lu%%", ah, al, bh, bl, r1, r2);
    }
#endif
}


static char *modestr(lzo_uint32 mode)
{
    static char s[10+1];

    mode_string(fix_mode_for_ls(mode),s);
    s[0] = '-';
    s[10] = 0;
    return s;
}


void do_list(const header_t *h, lzop_ulong_t d_len, lzop_ulong_t c_len)
{
    FILE *f;
    char s[40];
    time_t t;

    f = stdout;
    s[0] = 0;
    t = get_mtime(h);

    if (total_d_files == 0 && opt_verbose > 0)
    {
        if (opt_verbose >= 3)
            ((void)0);
        else if (opt_verbose >= 2)
        {
            fprintf(f,"Method         Length    Packed  Ratio ");
            fprintf(f,"    Date    Time   Name\n");
            fprintf(f,"------         ------    ------  ----- ");
            fprintf(f,"    ----    ----   ----\n");
        }
        else
        {
            fprintf(f,"%-11s ", "method");
            fprintf(f,"compressed  uncompr. ratio");
            fprintf(f," uncompressed_name\n");
        }
        fflush(f);
    }

    if (opt_verbose >= 3)
    {
        fprintf(f,"%-10s", modestr(h->mode));
        if (t)
            time2str(s,sizeof(s),&t);
        fprintf(f,"  %-19s",s);
        fprintf(f,"  %-20s",fi.name);
        if (fo.name[0])
            fprintf(f," %s", fo.name);
    }
    else if (opt_verbose >= 2)
    {
        fprintf(f,"%-11s ", h->method_name);
        pr_size(f, d_len, c_len, 1);
        if (t)
            time2str(s,sizeof(s),&t);
        s[16] = 0;  /* cut off seconds */
        fprintf(f,"  %-16s",s);
        if (fo.name[0])
            fprintf(f,"  %s", fo.name);
    }
    else
    {
        fprintf(f,"%-11s ", h->method_name);
        pr_size(f, c_len, d_len, 0);
        if (fo.name[0])
            fprintf(f," %s", fo.name);
    }

    fprintf(f,"\n");
    fflush(f);

    total_d_files++;
    total_c_len += c_len;
    total_d_len += d_len;
}


void do_list_total(void)
{
    FILE *f;

    if (total_d_files < 2 || opt_verbose == 0)
        return;
    if (opt_verbose >= 3)
        return;

    f = stdout;
    if (opt_verbose >= 2)
    {
        fprintf(f,"              -------   -------  ----- ");
        fprintf(f,"                   ----\n");
        fprintf(f,"%-11s ", "");
        pr_size(f, total_d_len, total_c_len, 1);
        fprintf(f,"  %-16s", "");
        fprintf(f,"  %lu files\n", total_d_files);
    }
    else
    {
        fprintf(f,"%-11s ", "");
        pr_size(f, total_c_len, total_d_len, 0);
        fprintf(f," (totals -- %lu files)\n", total_d_files);
    }
    fflush(f);
}


/*************************************************************************
// list a file similar to 'ls -ln'
**************************************************************************/

void do_ls(const header_t *h, lzop_ulong_t d_len, lzop_ulong_t c_len)
{
    FILE *f;
    char s[40];
    time_t t;
    const char *name = fo.name[0] ? fo.name : UNKNOWN_NAME;

    f = stdout;
    t = get_mtime(h);
    if (t == 0)
        time(&t);
    fprintf(f,"%-10s   1", modestr(h->mode));
    if (opt_stdin)
        fprintf(f," %-8s", "user");
    else
        fprintf(f," %-8ld", (long) fi.st.st_uid);
    if (!strchr(opt_ls_flags,'G'))
    {
        if (opt_stdin)
            fprintf(f," %-8s", "group");
        else
            fprintf(f," %-8ld", (long) fi.st.st_gid);
    }

#if (SIZEOF_LONG >= 8) || !defined(acc_int64l_t)
    fprintf(f," %8lu", (unsigned long) d_len);
#else
    {
        unsigned long d0, d1, d2;
        d0 = (unsigned long)  (d_len % 100000000ul);
        if (d0 == d_len)
            fprintf(f," %8lu", d0);
        else
        {
            d1 = (unsigned long) ((d_len / 100000000ul) % 100000000ul);
            d2 = (unsigned long) ((d_len / 100000000ul) / 100000000ul);
            if (d2 != 0)
                fprintf(f,"%lu%08lu%08lu", d2, d1, d0);
            else
                fprintf(f,"%lu%08lu", d1, d0);
        }
    }
#endif
    time2ls(s, sizeof(s), &t);
    fprintf(f," %-12s",s);
    if (strchr(opt_ls_flags,'Q'))
        fprintf(f," \"%s\"", name);
    else
        fprintf(f," %s", name);
    if (strchr(opt_ls_flags,'F'))
        if (h->mode & 0111)
            fprintf(f,"*");
    fprintf(f,"\n");
    fflush(f);

    UNUSED(c_len);
}


/*************************************************************************
// header info
**************************************************************************/

static void print_version(FILE *f, unsigned v)
{
    fprintf(f,"%1x.%03x", (v >> 12) & 0xf, v & 0xfff);
}


static void print_os(FILE *f, lzo_uint32 flags)
{
    flags = (flags & F_OS_MASK) >> F_OS_SHIFT;
    fprintf(f,"%2ld", (long) flags);
}


void do_info(const header_t *h, lzop_ulong_t d_len, lzop_ulong_t c_len)
{
    int v = opt_verbose;
    FILE *f;

    f = stdout;
    opt_verbose = 2;
    ++total_d_files;            /* do not print the list-header */
    do_list(h,d_len,c_len);
    --total_d_files;
    opt_verbose = v;

    fprintf(f,"   ");
    print_version(f,h->version);
    fprintf(f," ");
    print_version(f,h->lib_version);
    fprintf(f," ");
    print_version(f,h->version_needed_to_extract);
    fprintf(f,"  Fl: 0x%08lx", (long) h->flags);
    fprintf(f,"  Mo: 0%011lo", (long) h->mode);
    fprintf(f,"  Me: %d/%d", h->method, h->level);
    fprintf(f,"  OS: ");
    print_os(f,h->flags);
    if (h->filter)
        fprintf(f,"  Fi: %3ld", (long) h->filter);
    fprintf(f,"\n");
}


/*************************************************************************
// determine name of output file
**************************************************************************/

static lzo_bool can_restore_name(file_t *ft, const header_t *h)
{
    if (h->name[0] == 0 || ft->opt_name == 0)
        return 0;
    else if (ft->opt_name > 0)
        return 1;
#ifdef OPT_NAME_DEFAULT
#if 1
    else
        return 1;
#else
    /* restore the name by default only when created on such a system */
    else if ((h->flags & F_NAME_DEFAULT))
        return 1;
    else
        return 0;
#endif
#else
    /* do not restore the name by default */
    else
        return 0;
#endif /* OPT_NAME_DEFAULT */
}


static lzo_bool oname_error(void)
{
    if (opt_force >= 2 || opt_cmd == CMD_TEST)
    {
        warn(&fi,"can't determine name of output file -- using default");
        return 1;
    }
    if (opt_name != 1)
        error(&fi,"can't determine name of output file (try option '-N')");
    else
        error(&fi,"can't determine name of output file (use option '-o')");
    strcpy(fo.name,UNKNOWN_NAME);
    return 0;
}


static lzo_bool p_set_oname(const header_t *h)
{
    char *base;
    char *ext;
    int suff;
    size_t l;
    const char *err_name;
    const char *s;
    size_t sl;

    fo.name[0] = 0;
    if (opt_output_name)
    {
        /* name given on command line; perform no additional checks */
        strcpy(fo.name,opt_output_name);
        return 1;
    }

    assert(!opt_stdout);
    assert(opt_file);

#if defined(NRVP)
    err_name = (opt_cmd == CMD_COMPRESS) ? "nrvp.nrv" : "nrvp.raw";
    s = opt_suffix[0] ? opt_suffix : ".nrv";
#else
    err_name = (opt_cmd == CMD_COMPRESS) ? "lzop.lzo" : "lzop.raw";
    s = opt_suffix[0] ? opt_suffix : ".lzo";
#endif

    sl = strlen(s);

    if (opt_output_path)
    {
        strcpy(fo.name,opt_output_path);
        fn_addslash(fo.name,1);
        if (!opt_stdin)
            strcat(fo.name,fn_basename(fi.name));
    }
    else if (!opt_stdin)
        strcpy(fo.name,fi.name);
    l = strlen(fo.name);
    if (l >= PATH_MAX)
    {
        error(&fo,"name too long (use option '-o')");
        return 0;
    }
    base = fo.name + fn_baseindex(fo.name);
    suff = fn_has_suffix(base);
    ext = strchr(base,'.');

    if (opt_cmd == CMD_COMPRESS)
    {
        assert(!opt_stdin);
        assert(base[0]);
#if defined(DOSISH)
        if (suff == SUFF_TAR)
        {
#if defined(NRVP)
            strcpy(ext, opt_suffix[0] ? opt_suffix : ".tnv");
#else
            strcpy(ext, opt_suffix[0] ? opt_suffix : ".tzo");
#endif
        }
        else
#endif
        if (opt_shortname && ext)
            strcpy(ext,s);
        else
            strcat(fo.name,s);
    }
    else
    {
        lzo_bool u = 0;

        if (can_restore_name(&fi,h))
        {
            if (opt_path)
                strcpy(base,h->name);
            else
                strcpy(base,fn_basename(h->name));
        }
        else if (opt_stdin)
        {
            if (!oname_error())
                return 0;
            strcpy(base,err_name);
        }
        else if (suff == SUFF_LZO)
            fo.name[l-4] = 0;
        else if (suff == SUFF_LZOP)
            fo.name[l-5] = 0;
        else if (suff == SUFF_NRV)
            fo.name[l-4] = 0;
        else if (suff == SUFF_TZO)
            strcpy(&fo.name[l-4],".tar");
        else if (suff == SUFF_USER)
            fo.name[l-sl] = 0;
        else
        {
            u = 1;
            if (opt_shortname && ext)
                strcpy(ext,".raw");
            else
                strcat(fo.name,".raw");
        }

        if (u && opt_cmd == CMD_DECOMPRESS && !opt_stdin)
        {
#if 1
            if (!(opt_force >= 2))
            {
                if (fi.warn_unknown_suffix == 0)
                    error(&fi,"unknown suffix -- ignored");
                fi.warn_unknown_suffix = 1;
                return 0;
            }
#else
            /* gzip: '--force' doesn't override these checks */
            if (fi.warn_unknown_suffix == 0)
                error(&fi,"unknown suffix -- ignored");
            fi.warn_unknown_suffix = 1;
            return 0;
#endif
        }

    }

    if (strlen(fo.name) >= PATH_MAX)
    {
        error(&fo,"name too long (use option '-o')");
        return 0;
    }
#if defined(DOSISH)
    s = maybe_rename_file(fo.name);
    if (s == NULL)
    {
        if (!oname_error())
            return 0;
        strcpy(base,err_name);
    }
    else if (s != fo.name)
    {
        if (strcmp(s,fo.name) != 0)
        {
            warn(&fo,"renaming output file to match OS conventions");
            strcpy(fo.name,s);
        }
    }
#endif

    UNUSED(ext);
    return 1;
}


/*************************************************************************
// stdin/stdout
**************************************************************************/

static lzo_bool check_stdin(file_t *ft)
{
    if (!opt_force && acc_isatty(STDIN_FILENO))
    {
        strcpy(ft->name,STDIN_NAME);
        if (opt_cmd == CMD_COMPRESS)
            fatal(ft,"uncompressed data not read from a terminal");
        else
            fatal(ft,"compressed data not read from a terminal");
        return 0;
    }
    return 1;
}


static lzo_bool check_stdout(file_t *ft)
{
    if (!(opt_cmd == CMD_COMPRESS || opt_cmd == CMD_DECOMPRESS))
        return 1;
    if (!opt_force && acc_isatty(STDOUT_FILENO))
    {
        strcpy(ft->name,STDOUT_NAME);
        if (opt_cmd == CMD_COMPRESS)
            fatal(ft,"compressed data not written to a terminal");
        else
            fatal(ft,"uncompressed data not written to a terminal");
        return 0;
    }
    return 1;
}


static lzo_bool open_stdin(file_t *ft)
{
    static lzo_bool setmode_done = 0;

    assert(ft->fd == -1);
    f_reset(ft);

    strcpy(ft->name,STDIN_NAME);
    ft->fd = STDIN_FILENO;

#if !defined(NO_SETMODE)
    if (!setmode_done)
    {
        if (acc_set_binmode(ft->fd, 1) == -1)
        {
            p_fatal(ft,"acc_set_binmode(stdin) failed");
            return 0;
        }
    }
#endif
    setmode_done = 1;

    ft->st.st_mtime = time(NULL);
#if 1 && defined(HAVE_FSTAT)
    {
        struct stat st;
        if (fstat(ft->fd, &st) == 0 && S_ISREG(st.st_mode))
            ft->st = st;
    }
#endif
    ft->st.st_atime = fix_time(ft->st.st_atime);
    ft->st.st_mtime = fix_time(ft->st.st_mtime);
    return 1;
}


static lzo_bool open_stdout(file_t *ft)
{
    static lzo_bool setmode_done = 0;

    assert(ft->fd == -1);
    f_reset(ft);

    strcpy(ft->name,STDOUT_NAME);
    if (!(opt_cmd == CMD_COMPRESS || opt_cmd == CMD_DECOMPRESS))
    {
        ft->fd = -2;    /* special file-handle for dummy output */
        return 1;
    }
    ft->fd = STDOUT_FILENO;

#if !defined(NO_SETMODE)
    if (!setmode_done)
    {
        if (acc_set_binmode(ft->fd, 1) == -1)
        {
            p_fatal(ft,"acc_set_binmode(stdout) failed");
            return 0;
        }
    }
#endif
    setmode_done = 1;

    return 1;
}


/*************************************************************************
// open input file
**************************************************************************/

lzo_bool p_open_fi(const char *name)
{
    int r, saved_errno;
#if defined(HAVE_LSTAT) && defined(S_ISLNK)
    int r2;
#endif

    if (fi.fd != -1)
        return 1;

    f_reset(&fi);

/* prepare file name */
    assert(name != NULL);
    if (strlen(name) >= PATH_MAX)
    {
        if (strlen(name) >= sizeof(fi.name))
            strcpy(fi.name,UNKNOWN_NAME);
        else
            strcpy(fi.name,name);
        error(&fi,"name too long");
        return 0;
    }
    strcpy(fi.name,name);
    fn_strlwr(fi.name);
    if (opt_cmd == CMD_COMPRESS)
    {
        int suff = fn_has_suffix(fi.name);
#if 1
        if (opt_stdout || opt_output_name)
            suff = SUFF_NONE;   /* do not warn */
#endif
#if 1
        if (opt_force >= 2)
            suff = SUFF_NONE;   /* do not warn */
#else
        /* gzip: '--force' doesn't override these checks */
#endif
        if (suff == SUFF_LZO)
        {
            warn(&fi,"already has .lzo suffix -- unchanged");
            return 0;
        }
        else if (suff == SUFF_LZOP)
        {
            warn(&fi,"already has .lzop suffix -- unchanged");
            return 0;
        }
        else if (suff == SUFF_NRV)
        {
            warn(&fi,"already has .nrv suffix -- unchanged");
            return 0;
        }
        else if (suff == SUFF_TZO)
        {
            warn(&fi,"already has .tzo suffix -- unchanged");
            return 0;
        }
        else if (suff == SUFF_USER)
        {
            warn(&fi,"already has user suffix -- unchanged");
            return 0;
        }
    }

/* open file */
    errno = 0;
    r = stat(fi.name, &fi.st);
    saved_errno = errno;
    if (r != 0)
        memset(&fi.st, 0, sizeof(fi.st));
#if defined(HAVE_LSTAT) && defined(S_ISLNK)
    r2 = lstat(fi.name, &fi.lst);
    if (r2 != 0)
        memset(&fi.lst, 0, sizeof(fi.lst));
    if (r2 == 0 && S_ISLNK(fi.lst.st_mode))
    {
        if (r != 0)
        {
            errno = saved_errno;
#if 0
            p_error(&fi,"can't open input file -- dangling symlink");
#else
            do_error(&fi,err_nl,"can't open input file: Dangling symlink",EXIT_ERROR,0);
#endif
            return 0;
        }
    }
#endif
    if (r == 0 && !S_ISREG(fi.st.st_mode))
    {
        warn(&fi,"not a regular file -- skipped");
        return 0;
    }
    fi.open_flags = O_RDONLY;
    f_open(&fi,1);
#if 0 && defined(__DJGPP__)
    /* try again without LFN */
    if (fi.fd < 0 && errno == ENOENT && _USE_LFN)
    {
        if (!(_crt0_startup_flags & _CRT0_FLAG_NO_LFN))
        {
            int k = _crt0_startup_flags;
            _crt0_startup_flags |= _CRT0_FLAG_NO_LFN;
            r = stat(fi.name, &fi.st);
            saved_errno = errno;
            _crt0_startup_flags = k;
            if (r == 0 && !S_ISREG(fi.st.st_mode))
            {
                warn(&fi,"not a regular file -- skipped");
                return 0;
            }
            f_open(&fi,1);
        }
    }
#endif
    if (fi.fd < 0)
    {
        p_error(&fi,"can't open input file");
        return 0;
    }
    if (r != 0)
    {
        errno = saved_errno;
        p_error(&fi,"can't stat input file");
        (void) f_close(&fi);
        return 0;
    }

    fi.st.st_atime = fix_time(fi.st.st_atime);
    fi.st.st_mtime = fix_time(fi.st.st_mtime);
    return 1;
}


/*************************************************************************
// open output file
**************************************************************************/

lzo_bool p_open_fo(const header_t *h)
{
    if (fo.fd != -1)
        return 1;

    f_reset(&fo);

    if (!p_set_oname(h))
        return 0;
    fn_strlwr(fo.name);

    if (!(opt_cmd == CMD_COMPRESS || opt_cmd == CMD_DECOMPRESS))
    {
        fo.fd = opt_output_name ? -2 : -1;
        return 1;
    }

    if (fn_is_same_file(fi.name,fo.name))
    {
        if (opt_cmd == CMD_COMPRESS)
            error(&fi,"can't compress to same file");
        else
            error(&fi,"can't decompress to same file");
        return 0;
    }
    fo.open_flags = O_CREAT | O_WRONLY;
    if (opt_force)
        fo.open_flags |= O_TRUNC;
    else
        fo.open_flags |= O_EXCL;
#if defined(__MINT__)
    fo.open_flags |= O_TRUNC | O_DENYRW;
#endif
    fo.st.st_mode = fix_mode_for_open(fi.st.st_mode);
    if (opt_cmd == CMD_DECOMPRESS && opt_path && (h->flags & F_H_PATH))
    {
        /* create missing directories */
        char *name;
        int n = 0;
        name = fo.name;
        while (name[n])
        {
            while (name[n] && name[n] != '/')
                n++;
            if (name[n] == '/')
            {
                name[n] = 0;
                (void) acc_mkdir(name, 0777);
                name[n] = DIR_SEP[0];
                n++;
            }
        }
    }
    f_open(&fo,0);
    if (fo.fd < 0)
    {
        if ((fo.open_flags & O_EXCL) && errno == EEXIST)
            error(&fo,"already exists; not overwritten");
        else
            p_error(&fo,"can't open output file");
        return 0;
    }

    return 1;
}


/*************************************************************************
// close files
**************************************************************************/

static lzo_bool p_close(int i, int o)
{
    int r = 1;

    if (i && f_close(&fi) != 0)
    {
        p_error(&fi,"can't close input file");
        r = 0;
    }
    if (o && f_close(&fo) != 0)
    {
        p_error(&fo,"can't close output file");
        r = 0;
    }
    return r;
}


/*************************************************************************
// compress
**************************************************************************/

static void copy_perms(void)
{
#if defined(HAVE_UTIME)
    /* copy the time stamp */
    struct utimbuf u;
    u.actime = fi.st.st_atime;
    u.modtime = fi.st.st_mtime;
    if (utime(fo.name,&u) != 0)
        p_warn(&fo,"can't copy file time");
#endif
#if defined(HAVE_CHMOD)
    /* copy the protection mode */
    fo.st.st_mode = fi.st.st_mode;
    if (chmod(fo.name, fo.st.st_mode) != 0)
        p_warn(&fo,"can't copy file mode");
#endif
#if defined(HAVE_CHOWN)
    /* copy the ownership */
    if (chown(fo.name, fi.st.st_uid, fi.st.st_gid) != 0) {
        /* ignore */
    }
#endif
}


static lzo_bool do_compress(const char *name, lzo_bool handle_perms)
{
    lzo_bool ok = 1;
    header_t header;

    if (!p_open_fi(name))
        return 0;
    if (!p_open_fo(NULL))
    {
        if (opt_output_name || opt_stdout)
            e_exit(EXIT_ERROR);
        return 0;
    }

    ok = x_compress(&fi,&fo,&header);
    if (!ok)
        return 0;

    if (handle_perms)
    {
        if (!p_close(1,1))
            return 0;
        copy_perms();
    }

    return ok;
}


/*************************************************************************
// decompress
**************************************************************************/

static void restore_perms(const header_t *h)
{
#if defined(HAVE_UTIME)
    /* restore or copy the time stamp */
    struct utimbuf u;
    if (opt_restore_time && (h->mtime_low || h->mtime_high))
    {
        u.actime = u.modtime = get_mtime(h);
        if (u.actime)
            if (utime(fo.name,&u) != 0)
                p_warn(&fo,"can't restore file time");
    }
    else if (fi.st.st_atime && fi.st.st_mtime)
    {
        u.actime = fi.st.st_atime;
        u.modtime = fi.st.st_mtime;
        if (utime(fo.name,&u) != 0)
            p_warn(&fo,"can't copy file time");
    }
#endif
#if defined(HAVE_CHMOD)
    /* restore or copy the protection mode */
    if (opt_restore_mode && h->mode)
    {
        fo.st.st_mode = fix_mode_for_chmod(h->mode);
        if (chmod(fo.name, fo.st.st_mode) != 0)
            p_warn(&fo,"can't restore file mode");
    }
    else if (fi.st.st_mode > 0)
    {
        fo.st.st_mode = fi.st.st_mode;
        if (chmod(fo.name, fo.st.st_mode) != 0)
            p_warn(&fo,"can't copy file mode");
    }
#endif
#if defined(HAVE_CHOWN)
    /* copy the ownership */
    if (!opt_stdin)
        if (chown(fo.name, fi.st.st_uid, fi.st.st_gid) != 0) {
            /* ignore */
        }
#endif
    UNUSED(h);
}


static lzo_bool warn_multipart(file_t *ft, const header_t *h)
{
    if (!((ft->part > 0) || (h->flags & F_MULTIPART)))
        return 1;

    if (opt_stdin && opt_stdout && opt_cmd == CMD_TEST && can_restore_name(ft,h))
        return 1;
    if (opt_stdout || opt_output_name)
    {
        if (!ft->warn_multipart)
            warn(&fi,"this is a multipart archive (try option '-N')");
        ft->warn_multipart = 1;
    }
    else if (opt_file && !can_restore_name(ft,h))
    {
        ft->opt_name = 1;
        if (opt_cmd == CMD_TEST)
        {
            if (!ft->warn_multipart)
                warn(&fi,"this is a multipart archive (try option '-N')");
            ft->warn_multipart = 1;
        }
        else if (can_restore_name(ft,h))
        {
            if (!ft->warn_multipart)
                warn(&fi,"multipart archive -- restoring file names");
            ft->warn_multipart = 1;
        }
        else
        {
            error(&fi,"multipart archive, but no filename stored (use option '-o')");
            return 0;
        }
    }
    return 1;
}


static lzo_bool do_decompress(const char *name, lzo_bool handle_perms)
{
    lzo_bool ok = 1;
    lzo_bool unlink_ok = 1;
    lzo_bool skip = 0;
    header_t header;
    int r;

    if (!p_open_fi(name))
        return 0;

    for ( ; ok; fi.part++)
    {
        r = p_magic(&fi);
        if (r > 0)
            return 0;
        if (fi.part == 0)
            total_c_files++;
        if (fi.part > 0 && (opt_file || (r < 0 && handle_perms)))
        {
            if (!p_close(0,1))
                return 0;
            if (!skip && handle_perms && (opt_file || (r < 0 && fi.part == 1)))
                restore_perms(&header);
        }
        if (r < 0)
        {
            assert(fi.part > 0);
            break;
        }

        if (!p_header(&fi,&header))
        {
            /* use '--info -f -f' to try to list a corrupted header */
            if (opt_cmd == CMD_INFO && opt_force >= 2)
            {
                (void) x_get_method(&header);
                do_info(&header,0,0);
            }
            return 0;
        }

#if 0
        /* debug */
        do_info(&header,0,0);
#endif

        if (!warn_multipart(&fi,&header))
            return 0;

        skip = 0;
        ok = p_open_fo(&header);
        if (!ok)
        {
            unlink_ok = 0;
            if (opt_output_name || opt_stdout)
                e_exit(EXIT_ERROR);
            if (opt_cmd != CMD_TEST)
                skip = 1;
        }

        ok = x_decompress(&fi,&fo,&header,skip);
    }

    return ok && unlink_ok;
}


/*************************************************************************
// process files
**************************************************************************/

static lzo_bool do_one_file(const char *name, lzo_bool handle_perms)
{
    lzo_bool ok;

    if (opt_cmd == CMD_COMPRESS)
        ok = do_compress(name,handle_perms);
    else
        ok = do_decompress(name,handle_perms);

    if (!p_close(1,0))
        ok = 0;
    if (opt_file && !p_close(0,1))
        ok = 0;

    if (ok && opt_unlink)
    {
#if defined(HAVE_CHMOD)
        (void) chmod(fi.name, 0777);
#endif
        if (unlink(fi.name) != 0)
            p_warn(&fi,"can't unlink file");
    }

    if (fi.fd == -1)
        fi.name[0] = 0;
    if (fo.fd == -1)
        fo.name[0] = 0;

    return ok;
}


static void do_files(int i, int argc, char *argv[])
{
    lzo_bool handle_perms;

    if (opt_cmd == CMD_COMPRESS)
        handle_perms = !opt_stdin;
    else if (opt_cmd == CMD_DECOMPRESS)
        handle_perms = 1;
    else
        handle_perms = 0;

    if (opt_stdin)
    {
        assert(opt_stdout || opt_output_name || opt_output_path);
        assert(i == argc);
        assert(num_files == 0);
        if (!check_stdin(&fi) || !open_stdin(&fi))
            return;
    }
    if (opt_stdout)
    {
        assert(!opt_output_name);
        if (!check_stdout(&fo) || !open_stdout(&fo))
            return;
        handle_perms = 0;
    }
    if (opt_output_name)
    {
        assert(!opt_stdout);
        handle_perms &= (num_files == 1);
    }

    if (opt_stdin)
        do_one_file(NULL,handle_perms);
    else
    {
        for ( ; i < argc; i++)
            do_one_file(argv[i],handle_perms);
    }

    (void) p_close(1,1);

    if (opt_cmd == CMD_LIST)
        do_list_total();
    if (opt_cmd == CMD_TEST)
        do_test_total();
}


/*************************************************************************
// check options
**************************************************************************/

static void check_not_both(lzo_bool e1, lzo_bool e2, int c1, int c2)
{
    if (e1 && e2)
    {
        fprintf(stderr,"%s: ",argv0);
        fprintf(stderr,"cannot use both '-%c' and '-%c'\n", c1, c2);
        e_usage();
    }
}


void check_options(int i, int argc)
{
    assert(i <= argc);

    if (opt_keep)
        opt_unlink = 0;
    if (!(opt_cmd == CMD_COMPRESS || opt_cmd == CMD_DECOMPRESS))
        opt_unlink = 0;

    if (opt_stdin == OPT_STDIN_GUESSED && i != argc)
        opt_stdin = 0;
    if (opt_stdin)
    {
        opt_unlink = 0;
#if 0
        /* gzip: always use stdout */
        opt_stdout = 1;
#else
        if (!opt_output_name && !opt_output_path)
            opt_stdout = 1;
#endif
    }
    if (opt_stdout)
    {
        check_not_both(1, opt_output_name != NULL, 'c', 'o');
        check_not_both(1, opt_output_path != NULL, 'c', 'p');
        check_not_both(1, opt_suffix[0] != 0, 'c', 'S');
        opt_output_name = NULL;
        opt_output_path = NULL;
        opt_suffix[0] = 0;
        if (opt_unlink && !opt_force)
        {
            fprintf(stderr,"%s: both '-c' and '-U' given (use '-f' to force)\n",argv0);
            e_usage();
        }
    }
    if (opt_output_name)
    {
        check_not_both(1, opt_output_path != NULL, 'o', 'p');
        check_not_both(1, opt_suffix[0] != 0, 'o', 'S');
        opt_output_path = NULL;
        opt_suffix[0] = 0;
    }

    /* check number of remaining args */
    if (opt_stdin)
    {
        if (opt_cmd == CMD_COMPRESS && opt_output_path)
        {
            fprintf(stderr,"%s: cannot use '-p' when compressing stdin\n",argv0);
            e_usage();
        }

        /* No more args allowed */
        if (i != argc)
        {
            fprintf(stderr,"%s: no filename allowed when reading from stdin\n",argv0);
            e_usage();
        }
    }
    else
    {
        if (i == argc)
        {
            fprintf(stderr,"%s: nothing to do !\n",argv0);
            e_usage();
        }

        if (opt_stdout || opt_output_name)
        {
#if 1
            /* Allow multiple files */
            if (i + 1 != argc)
            {
                opt_name = 1;
            }
#else
            /* Exactly one input file */
            if (i + 1 != argc)
            {
                fprintf(stderr,"%s: only one file allowed\n",argv0);
                e_usage();
            }
#endif
        }
    }

    opt_file = !opt_stdout && !opt_output_name;
}


/*************************************************************************
// misc
**************************************************************************/

void e_help(void)
{
    if (opt_pgm == PGM_LZOP)
        help();
    else if (opt_pgm == PGM_UNLZOP)
        help();
    else if (opt_pgm == PGM_OCAT)
    {
        if (opt_stdin)
            check_stdin(&fi);
        if (opt_stdout)
            check_stdout(&fo);
        usage();
    }
    else
        help();
    e_exit(EXIT_USAGE);
}


void set_term(FILE *f)
{
    if (f)
        con_term = f;
    else
        con_term = acc_isatty(STDOUT_FILENO) ? stdout : stderr;
}


void set_cmd(int cmd)
{
    if (cmd == CMD_COMPRESS && (opt_pgm == PGM_UNLZOP || opt_pgm == PGM_OCAT))
        return;
#if 0
    if (opt_cmd != CMD_NONE && cmd != opt_cmd)
    {
        fprintf(stderr,"%s: multiple commands given\n",argv0);
        e_usage();
    }
    opt_cmd = cmd;
#else
    /* gzip: commands have a certain priority */
    if (cmd > opt_cmd)
        opt_cmd = cmd;
#endif
}


lzo_bool set_method(int m, int l)
{
    if (x_set_method(m,l) != 0)
        return 0;
    set_cmd(CMD_COMPRESS);
    return 1;
}


void set_output_name(const char *n, lzo_bool allow_m)
{
#if 1
    if (done_output_name > 0)
    {
        fprintf(stderr,"%s: option '-o' more than once given\n",argv0);
        e_usage();
    }
#endif
    if (!n || !n[0] || (!allow_m && n[0] == '-'))
    {
        fprintf(stderr,"%s: missing output name\n",argv0);
        e_usage();
    }
    if (strlen(n) >= PATH_MAX)
    {
        fprintf(stderr,"%s: output name too long\n",argv0);
        e_usage();
    }
    opt_output_name = n;
    done_output_name++;
}


void set_output_path(const char *n, lzo_bool allow_m)
{
    int r;
    struct stat st;
    file_t f;

#if 1
    if (done_output_path > 0)
    {
        fprintf(stderr,"%s: option '-p' more than once given\n",argv0);
        e_usage();
    }
#endif
    if (!n || (!allow_m && n[0] == '-'))
    {
        fprintf(stderr,"%s: missing path\n",argv0);
        e_usage();
    }
    if (strlen(n) >= PATH_MAX)
    {
        fprintf(stderr,"%s: path too long\n",argv0);
        e_usage();
    }
    if (n[0])
    {
        r = stat(n, &st);
        if (r != 0)
        {
            strcpy(f.name,n);
            p_fatal(&f,"invalid path");
        }
#if defined(S_ISDIR)
        if (!S_ISDIR(st.st_mode))
        {
            strcpy(f.name,n);
            fatal(&f,"invalid path - must be a directory");
        }
#endif
    }
#if defined(HAVE_ACCESS) && defined(W_OK)
    {
    const char *p = n[0] ? n : ".";
    if (access(p,W_OK) != 0)
    {
        strcpy(f.name,p);
        p_fatal(&f,"can't write to path");
    }
    }
#endif
    opt_output_path = n;
    done_output_path++;
}


lzo_bool set_suffix(const char *n)
{
    size_t l;
    const char *p;
    static const char * const invalid_suffixes[] =
        { "ace", "arc", "arj", "bz", "bz2", "gz", "lha", "lzh",
#if !defined(NRVP)
          "nrv", "tnv",
#endif
          "rar", "raw", "sz", "tar", "taz", "tbz", "tgz", "tsz",
          "upx", "Z", "zip", "zoo", NULL };
    const char * const *is = invalid_suffixes;

#if 1
    if (done_suffix > 0)
    {
        fprintf(stderr,"%s: option '-S' more than once given\n",argv0);
        e_usage();
        return 0;
    }
#endif
    while (n && *n == '.')
        n++;

    if (!n || *n == 0 || *n == '-')
        return 0;
#if 1 || defined(DOSISH)
    if (strchr(n,'.'))
        return 0;
#endif
    for (p = n; *p; p++)
        if (strchr("+*?=/\\ \t\n\r\a", *p))
            return 0;
    for ( ; *is; is++)
        if (strcasecmp(n,*is) == 0)
            return 0;

    l = strlen(n);
    if (l + 1 > SUFFIX_MAX || (opt_shortname && l > 3))
    {
        fprintf(stderr,"%s: suffix '%s' is too long\n",argv0,n);
        e_usage();
        return 0;
    }

    opt_suffix[0] = '.';
    strcpy(opt_suffix + 1, n);
    done_suffix++;
    return 1;
}


/*************************************************************************
// get options
**************************************************************************/

static
char* prepare_shortopts(char *buf, const char *n,
                        const struct acc_getopt_longopt_t *longopts)
{
    char *o = buf;

    for ( ; n && *n; n++)
        if (*n != ' ')
            *o++ = *n;
    *o = 0;
    for ( ; longopts && longopts->name; longopts++)
    {
        int v = longopts->val;
        if (v > 0 && v < 256 && strchr(buf,v) == NULL)
        {
            *o++ = (char) v;
            if (longopts->has_arg >= 1)
                *o++ = ':';
            if (longopts->has_arg >= 2)
                *o++ = ':';
            *o = 0;
        }
    }
    return buf;
}


static int do_option(acc_getopt_p g, int optc)
{
#define mfx_optarg      g->optarg
    int i = 0;
    int m = -1;

    switch (optc)
    {
    case 'c':
        opt_stdout = 1;
        break;
    case 'C':
        opt_checksum = (opt_checksum >= 1) ? opt_checksum + 1 : 1;
        opt_decompress_safe = 1;
        break;
    case 'd':
        set_cmd(CMD_DECOMPRESS);
        break;
    case 'f':
        opt_force++;
        break;
    case 'F':
        opt_checksum = 0;
        opt_decompress_safe = 0;
        break;
    case 'h':
    case 'H':
    case '?':
        set_cmd(CMD_HELP);
        break;
    case 'h'+256:
        /* according to GNU standards */
        set_term(stdout);
        opt_console = CON_NONE;
        help();
        e_exit(EXIT_OK);
        break;
    case 'i':
    case 'i'+256:
        set_cmd(CMD_INFO);
        break;
    case 'I':
        set_cmd(CMD_SYSINFO);
        break;
    case 'k':
        opt_keep = 1;
        break;
    case 'l':
        set_cmd(CMD_LIST);
        break;
    case 'L':
        set_cmd(CMD_LICENSE);
        break;
    case 'n':
        opt_name = 0;
        opt_path = 0;
        break;
    case 'N':
        opt_name = 1;
        break;
    case 'o':
        set_output_name(mfx_optarg,1);
        break;
    case 'p':
    case 'p'+256:
        if (mfx_optarg && mfx_optarg[0])
            set_output_path(mfx_optarg,0);
        else if (optc == 'p')
            set_output_path("",0);
        else
            set_output_path(NULL,0);
        break;
    case 'P':
        opt_path = 1;
        opt_name = 1;
        break;
    case 'q':
        opt_verbose = 0;
        break;
    case 'S':
        if (!set_suffix(mfx_optarg))
        {
            fprintf(stderr,"%s: invalid suffix '%s'\n",argv0,mfx_optarg);
            e_usage();
        }
        break;
    case 't':
        set_cmd(CMD_TEST);
        break;
    case 'T':
        if (!(mfx_optarg && isdigit(mfx_optarg[0])))
        {
            fprintf(stderr,"%s: invalid '--threads=' args: '%s'\n",argv0,mfx_optarg);
            e_usage();
        }
        opt_num_threads = atoi(mfx_optarg);
        if (opt_num_threads < 1 || opt_num_threads > MAX_NUM_THREADS)
        {
            fprintf(stderr,"%s: invalid number of threads: %d\n",argv0,opt_num_threads);
            e_usage();
        }
#if !defined(WITH_THREADS)
        opt_num_threads = 1;
#endif
        break;
    case 'U':
        opt_unlink = 1;
        break;
    case 'v':
        opt_verbose = (opt_verbose < 2) ? 2 : opt_verbose + 1;
        break;
    case 'V':
        set_cmd(CMD_VERSION);
        break;
    case 'V'+256:
        /* according to GNU standards */
        set_term(stdout);
        opt_console = CON_NONE;
        fprintf(stdout,"lzop %s\n",LZOP_VERSION_STRING);
        fprintf(stdout,"LZO library %s\n",lzo_version_string());
        fprintf(stdout,"Copyright (C) 1996-2010 Markus Franz Xaver Johannes Oberhumer\n");
        e_exit(EXIT_OK);
        break;
    case 'x':
        set_cmd(CMD_DECOMPRESS);
        opt_name = 1;
        opt_path = 1;
        opt_restore_mode = 1;
        opt_restore_time = 1;
        if (!opt_output_name && !opt_output_path)
        {
            set_output_path("",0);
            --done_output_path;
        }
        opt_unlink = 0;
        break;
    case 'Z'+256:
        set_cmd(CMD_LS);
        if (mfx_optarg && mfx_optarg[0])
        {
            opt_ls_flags = mfx_optarg;
            for (i = 0; opt_ls_flags[i]; i++)
                if (!strchr("FGQ",opt_ls_flags[i]))
                {
                    fprintf(stderr,"%s: invalid '--ls' flags: '%s'\n",argv0,mfx_optarg);
                    e_usage();
                }
        }
        break;
#ifdef MAINT
    case 520:
        opt_noheader++;
        break;
#endif
    case 522:
        opt_stdin = 0;
        break;
    case 523:
        opt_restore_mode = 0;
        break;
    case 524:
        opt_restore_time = 0;
        break;
    case 525:
        opt_nowarn = 1;
        break;
    case 526:
        opt_ignorewarn = 1;
        break;
    case 527:
        opt_crc32 = 1;
        break;

    case 512:
        opt_console = CON_NONE;
        break;
    case 513:
        opt_console = CON_ANSI_MONO;
        break;
    case 514:
        opt_console = CON_ANSI_COLOR;
        break;
    case 515:
        set_cmd(CMD_INTRO);
        break;


    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
        if (!set_method(0,optc - '0'))
            e_method(optc);
        break;

#if defined(WITH_NRV)
    case 811:
        if (m < 0) m = M_NRV1A;
        /* fallthrough */
    case 812:
        if (m < 0) m = M_NRV1B;
        /* fallthrough */
    case 813:
        if (m < 0) m = M_NRV2A;
        /* fallthrough */
    case 814:
        if (m < 0) m = M_NRV2B;
        i = 1;
        if (mfx_optarg && isdigit(mfx_optarg[0]) && !mfx_optarg[1])
            i = mfx_optarg[0] - '0';
        if (!set_method(m,i))
            e_usage();
        break;
#endif
#if defined(WITH_ZLIB)
    case 801:
        i = 6;
        if (mfx_optarg && mfx_optarg[0] && !mfx_optarg[1])
            i = mfx_optarg[0] - '0';
        if (!set_method(M_ZLIB,i))
            e_usage();
        break;
#endif

    case 521:
        if (!(mfx_optarg && isdigit(mfx_optarg[0])))
        {
            fprintf(stderr,"%s: invalid '--filter=' args: '%s'\n",argv0,mfx_optarg);
            e_usage();
        }
        if (strcmp(mfx_optarg,"0") == 0)
        {
            opt_filter = 0;
            break;
        }
        opt_filter = atoi(mfx_optarg);
        if (opt_filter < 1 || opt_filter > 16)
        {
            fprintf(stderr,"%s: invalid filter: %d\n",argv0,opt_filter);
            e_usage();
        }
        break;

    case '\0':
        return -1;
    case ':':
        return -2;
    default:
        fprintf(stderr,"%s: internal error in getopt (%d)\n",argv0,optc);
        return -3;
    }

    UNUSED(i);
    UNUSED(m);
    return 0;
#undef mfx_optarg
}


static void handle_opterr(acc_getopt_p g, const char *f, void *v)
{
    struct A { va_list ap; };
    struct A *a = (struct A *) v;
    fprintf( stderr, "%s: ", g->progname);
    if (a)
        vfprintf(stderr, f, a->ap);
    else
        fprintf( stderr, "UNKNOWN GETOPT ERROR");
    fprintf( stderr, "\n");
}


static int get_options(int argc, char **argv)
{

static const struct acc_getopt_longopt_t longopts[] =
{
    {"best",       0, 0, '9'},      /* compress better */
    {"decompress", 0, 0, 'd'},      /* decompress */
    {"fast",       0, 0, '1'},      /* compress faster */
    {"help",       0, 0, 'h'+256},  /* give help */
    {"info",       0, 0, 'i'+256},
    {"license",    0, 0, 'L'},      /* display software license */
    {"list",       0, 0, 'l'},      /* list .lzo file contents */
    {"ls",         2, 0, 'Z'+256},  /* list .lzo file contents */
    {"sysinfo",    0, 0, 'I'},
    {"test",       0, 0, 't'},      /* test compressed file integrity */
    {"uncompress", 0, 0, 'd'},      /* decompress */
    {"version",    0, 0, 'V'+256},  /* display version number */
#if defined(WITH_NRV)
    {"nrv1a",   0x22, 0, 811},
    {"nrv2a",   0x22, 0, 813},
    {"nrv2b",   0x22, 0, 814},
#endif
#if defined(WITH_ZLIB)
    {"zlib",    0x22, 0, 801},
#endif

    {"checksum",   0, 0, 'C'},
    {"crc32",   0x10, 0, 527},      /* use a crc32 checksum instead of adler32 */
    {"delete",     0, 0, 'U'},
    {"extract",    0, 0, 'x'},
    {"filter",     1, 0, 521},
    {"force",      0, 0, 'f'},      /* force overwrite of output file */
    {"ignore-warn",0, 0, 526},      /* ignore any warnings */
    {"keep",       0, 0, 'k'},
    {"name",       0, 0, 'N'},      /* restore original name */
    {"no-checksum",0, 0, 'F'},
#ifdef MAINT
    {"no-header",  0, 0, 520},
#endif
    {"no-mode",    0, 0, 523},      /* don't restore original mode */
    {"no-name",    0, 0, 'n'},      /* don't restore original name */
    {"no-stdin",   0, 0, 522},
    {"no-time",    0, 0, 524},      /* don't restore original time */
    {"no-warn",    0, 0, 525},      /* do not display any warnings */
    {"output",     1, 0, 'o'},
    {"path",       1, 0, 'p'+256},
    {"quiet",      0, 0, 'q'},      /* quiet mode */
    {"silent",     0, 0, 'q'},      /* quiet mode */
    {"stdout",     0, 0, 'c'},      /* write output on standard output */
    {"suffix",     1, 0, 'S'},      /* use given suffix instead of .lzo */
    {"threads", 0x21, 0, 'T'},      /* number of threads */
    {"to-stdout",  0, 0, 'c'},      /* write output on standard output */
    {"unlink",     0, 0, 'U'},
    {"verbose",    0, 0, 'v'},      /* verbose mode */

    {"no-color",   0, 0, 512},
    {"mono",       0, 0, 513},
    {"color",      0, 0, 514},
    {"intro",      0, 0, 515},

    { 0, 0, 0, 0 }
};

    acc_getopt_t mfx_getopt;
    int optc;
    int i;
    char shortopts[128];

    prepare_shortopts(shortopts, "123456789hH?PVp::", longopts),
    acc_getopt_init(&mfx_getopt, 1, argc, argv);
    mfx_getopt.progname = argv0;
    mfx_getopt.opterr = handle_opterr;
    while ((optc = acc_getopt(&mfx_getopt, shortopts, longopts, NULL)) >= 0)
    {
        if (do_option(&mfx_getopt, optc) != 0)
            e_usage();
    }

    /* accept "-" as synonym for stdin */
    for (i = mfx_getopt.optind; i < argc; i++)
        if (strcmp(argv[i], "-") == 0)
            opt_stdin = OPT_STDIN_REQUESTED;
    for (i = mfx_getopt.optind; i < argc; i++)
        if (strcmp(argv[i], "-") != 0)
            break;
    return i;
}


#if defined(OPTIONS_VAR)
static void get_envoptions(int argc, char **argv)
{

/* only some options are allowed in the environment variable */

static const struct acc_getopt_longopt_t longopts[] =
{
    {"best",       0, 0, '9'},      /* compress better */
    {"checksum",   0, 0, 'C'},
    {"crc32",   0x10, 0, 527},      /* use a crc32 checksum instead of adler32 */
    {"delete",     0, 0, 'U'},
    {"fast",       0, 0, '1'},      /* compress faster */
    {"ignore-warn",0, 0, 526},      /* ignore any warnings */
    {"keep",       0, 0, 'k'},
    {"name",       0, 0, 'N'},      /* restore original name */
    {"no-checksum",0, 0, 'F'},
    {"no-mode",    0, 0, 523},      /* don't restore original mode */
    {"no-name",    0, 0, 'n'},      /* don't restore original name */
    {"no-time",    0, 0, 524},      /* don't restore original time */
    {"no-stdin",   0, 0, 522},
    {"no-warn",    0, 0, 525},      /* do not display any warnings */
    {"quiet",      0, 0, 'q'},      /* quiet mode */
    {"silent",     0, 0, 'q'},      /* quiet mode */
    {"threads", 0x21, 0, 'T'},      /* number of threads */
    {"unlink",     0, 0, 'U'},
    {"verbose",    0, 0, 'v'},      /* verbose mode */

    {"no-color",   0, 0, 512},
    {"mono",       0, 0, 513},
    {"color",      0, 0, 514},

    { 0, 0, 0, 0 }
};

    char *env, *p;
    int i, optc;
    int nargc;
    char **nargv = NULL;
    static const char sep[] = " \t";
    acc_getopt_t mfx_getopt;
    char shortopts[128];

    env = (char *) getenv(OPTIONS_VAR);
    if (env == NULL || !env[0])
        return;
    p = (char *) malloc(strlen(env)+1);
    if (p == NULL)
        return;
    strcpy(p,env);
    env = p;

    nargc = 1;
    for (;;)
    {
        while (*p && strchr(sep,*p))
            p++;
        if (*p == '\0')
            break;
        nargc++;
        while (*p && !strchr(sep,*p))
            p++;
        if (*p == '\0')
            break;
        p++;
    }

    if (nargc > 1)
        nargv = (char **) calloc(nargc+1,sizeof(char *));
    if (nargv == NULL)
    {
        free(env);
        return;
    }

    nargv[0] = argv[0];
    p = env;
    nargc = 1;
    for (;;)
    {
        while (*p && strchr(sep,*p))
            p++;
        if (*p == '\0')
            break;
        nargv[nargc++] = p;
        while (*p && !strchr(sep,*p))
            p++;
        if (*p == '\0')
            break;
        *p++ = '\0';
    }
    nargv[nargc] = NULL;

#if 0
    /* debug */
    fprintf(stderr,"%3d\n",nargc);
    for (i = 0; i <= nargc; i++)
        fprintf(stderr,"%3d '%s'\n",i,nargv[i]);
#endif

    for (i = 1; i < nargc; i++)
        if (nargv[i][0] != '-' || !nargv[i][1] || strcmp(nargv[i],"--") == 0)
            e_envopt(nargv[i]);

    prepare_shortopts(shortopts, "123456789P", longopts);
    acc_getopt_init(&mfx_getopt, 1, nargc, nargv);
    mfx_getopt.progname = argv0;
    mfx_getopt.opterr = handle_opterr;
    while ((optc = acc_getopt(&mfx_getopt, shortopts, longopts, NULL)) >= 0)
    {
        if (do_option(&mfx_getopt, optc) != 0)
            e_envopt(NULL);
    }

    if (mfx_getopt.optind < nargc)
        e_envopt(nargv[mfx_getopt.optind]);

    free(nargv);
    free(env);
    UNUSED(argc);

    if (opt_checksum)
        opt_checksum = -1;          /* reset to default */
}
#endif /* defined(OPTIONS_VAR) */


#define ACC_WANT_ACC_CHK_CH 1
#undef ACCCHK_ASSERT
#include "miniacc.h"
#undef ACCCHK_ASSERT

static void sanity_check(void)
{
#if (ACC_CC_MSC && ((_MSC_VER) < 700))
#else
#define ACCCHK_ASSERT(expr)     ACC_COMPILE_TIME_ASSERT(expr)
#include "miniacc.h"
#endif
#undef ACCCHK_ASSERT
#undef ACC_WANT_ACC_CHK_CH
}


/*************************************************************************
// main entry point
**************************************************************************/

int __acc_cdecl_main main(int argc, char *argv[])
{
    int i;
    lzo_bool foreground = 0;
    static char default_argv0[] = "lzop";
    int cmdline_cmd = CMD_NONE;

    sanity_check();

#if defined(__MINT__)
    __binmode(1);
    __set_binmode(stdout, 0);
    __set_binmode(stderr, 0);
#endif
    acc_wildargv(&argc, &argv);


#if defined(__DJGPP__)
    opt_shortname = !_USE_LFN;
#elif (ACC_OS_DOS16 || ACC_OS_WIN16 || ACC_OS_DOS32)
    opt_shortname = 1;
#endif

    current_time = fix_time(time(NULL));

    if (!argv[0] || !argv[0][0])
        argv[0] = default_argv0;
    argv0 = argv[0];
    progname = fn_basename(argv0);
#if defined(DOSISH)
    if (strcasecmp(progname,"lzop.exe") == 0)
        progname = default_argv0;
    else if (strcasecmp(progname,"lzop.ttp") == 0)
        progname = default_argv0;
#endif

    /* For compatibility with gzip, use program name as an option. */
    if (strncasecmp(progname, "un", 2) == 0)                /* unlzop */
        opt_pgm = PGM_UNLZOP;
#if 0
    if (progname[0] && strcasecmp(progname+1, "cat") == 0)  /* ocat */
        opt_pgm = PGM_OCAT;
#endif

    set_term(stderr);
    opt_stdin = acc_isatty(STDIN_FILENO) ? 0 : OPT_STDIN_GUESSED;

    UNUSED(foreground);
#ifdef SIGINT
    foreground = signal(SIGINT, SIG_IGN) != SIG_IGN;
    if (foreground)
        (void) signal(SIGINT, e_sighandler);
#endif
#ifdef SIGBREAK
    if (foreground)
        (void) signal(SIGBREAK, e_sighandler);
#endif
#ifdef SIGTERM
    if (signal(SIGTERM, SIG_IGN) != SIG_IGN)
        (void) signal(SIGTERM, e_sighandler);
#endif
#ifdef SIGHUP
    if (signal(SIGHUP, SIG_IGN) != SIG_IGN)
        (void) signal(SIGHUP,  e_sighandler);
#endif

#if defined(HAVE_UMASK)
    u_mask = (MODE_T) umask(0700);
    (void) umask(u_mask);
#endif
    u_mask &= 0777;

#if defined(WITH_LZO)
    if (lzo_init() != LZO_E_OK)
    {
        head();
        fprintf(stderr,"lzo_init() failed - check your LZO installation !\n");
        if (LZO_VERSION != lzo_version())
            fprintf(stderr,"library version conflict (%x, %x) - check your LZO installation !\n", LZO_VERSION, lzo_version());
        e_exit(EXIT_LZO_INIT);
    }
#if 0
    if (lzo_version() < LZO_VERSION)
    {
        head();
        fprintf(stderr,"library version conflict (%x, %x) - check your LZO installation !\n", LZO_VERSION, lzo_version());
        e_exit(EXIT_LZO_INIT);
    }
#endif
#endif
#if defined(WITH_NRV)
    if (nrv_init() != NRV_E_OK)
    {
        e_exit(EXIT_LZO_INIT);
    }
#endif

    ACC_COMPILE_TIME_ASSERT(sizeof(lzo_uint32) >= 4)
#if defined(SIZEOF_SIZE_T)
    ACC_COMPILE_TIME_ASSERT(sizeof(size_t) == SIZEOF_SIZE_T)
#endif
    ACC_COMPILE_TIME_ASSERT(sizeof(fi.name) >= 2*(PATH_MAX))
    assert(STDIN_FILENO >= 0);
    assert(STDOUT_FILENO >= 0);
    assert(STDERR_FILENO >= 0);

    f_init();
#if defined(OPTIONS_VAR)
    get_envoptions(argc,argv);
#endif
    assert(cmdline_cmd == CMD_NONE);
    i = get_options(argc,argv);
    assert(i <= argc);

    set_term(NULL);
    cmdline_cmd = opt_cmd;
    switch (opt_cmd)
    {
    case CMD_NONE:
        /* For compatibility with gzip, use program name as an option. */
        if (opt_pgm == PGM_UNLZOP)
        {
            set_cmd(CMD_DECOMPRESS);
            break;
        }
#if 0
        if (opt_pgm == PGM_OCAT)
        {
            set_cmd(CMD_DECOMPRESS);
            if (i == argc)
                opt_stdin = OPT_STDIN_REQUESTED;
            if (!opt_output_name)
                opt_stdout = 1;
            break;
        }
#endif
        /* default - compress */
        if (!set_method(0,3))
            e_method('3');
        break;
    case CMD_COMPRESS:
        break;
    case CMD_DECOMPRESS:
        break;
    case CMD_TEST:
        opt_checksum = 1;
        opt_decompress_safe = 1;
        break;
    case CMD_LIST:
        break;
    case CMD_LS:
        break;
    case CMD_INFO:
        break;
    case CMD_SYSINFO:
        sysinfo();
        e_exit(EXIT_OK);
        break;
    case CMD_LICENSE:
        license();
        e_exit(EXIT_OK);
        break;
    case CMD_HELP:
        help();
        e_exit(EXIT_OK);
        break;
    case CMD_INTRO:
        opt_console = CON_SCREEN;
        (void) ((con_intro(con_term) || (help(), 0)));
        e_exit(EXIT_OK);
    case CMD_VERSION:
        version();
        e_exit(EXIT_OK);
        break;
    default:
        /* ??? */
        break;
    }

    if (opt_cmd != CMD_COMPRESS)
    {
        opt_method = 0;
        opt_level = 0;
        opt_filter = 0;
    }

    if (!opt_stdin && !opt_stdout && opt_verbose > 0)
    {
        if (argc == 1)
        {
            /* no arguments */
            (void) (opt_pgm == PGM_LZOP && con_intro(con_term));
            e_help();
        }
#if 0
        else if (cmdline_cmd == CMD_NONE && i == argc)
        {
            /* no command and no file */
            e_help();
        }
#endif
    }

    set_term(stderr);
    check_options(i,argc);
    num_files = argc - i;

    if (!x_enter(NULL))
        e_memory();

    do_files(i,argc,argv);

    x_leave(NULL);
    do_exit();
    return exit_code;
}


/*
vi:ts=4:et
*/

