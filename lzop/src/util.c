/* util.c --

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


/*************************************************************************
// filename util
**************************************************************************/

static const char dir_sep[] = DIR_SEP;

#define fn_is_sep(c)        (strchr(dir_sep,c) != NULL)

#if defined(DOSISH)
#define fn_is_drive(n)      (n[0] && n[1] == ':')
#define fn_skip_drive(n)    (fn_is_drive(n) ? (n) + 2 : (n))
#else
#define fn_is_drive(n)      (0)
#define fn_skip_drive(n)    (n)
#endif


unsigned fn_baseindex(const char *name)
{
    const char *n, *nn;

    n = fn_skip_drive(name);
    for (nn = n; *nn; nn++)
        if (fn_is_sep(*nn))
            n = nn + 1;
    return (unsigned) (n - name);
}


const char *fn_basename(const char *name)
{
    return name + fn_baseindex(name);
}


void fn_addslash(char *name, lzo_bool slash)
{
    char *p;

    name = fn_skip_drive(name);
    p = name + strlen(name);
    while (p > name && fn_is_sep(p[-1]))
        *p-- = 0;
    if (p > name)
    {
        if (slash)
            *p++ = dir_sep[0];
        *p = 0;
    }
}


char *fn_strlwr(char *n)
{
    char *p;
    for (p = n; *p; p++)
        *p = (char) fn_tolower(*p);
    return n;
}


int fn_strcmp(const char *n1, const char *n2)
{
    for (;;)
    {
        if (*n1 != *n2)
        {
            int c = fn_tolower(*n1) - fn_tolower(*n2);
            if (c)
                return c;
        }
        if (*n1 == 0)
            return 0;
        n1++; n2++;
    }
}


lzo_bool fn_is_same_file(const char *n1, const char *n2)
{
    /* very simple... */
    if (fn_strcmp(n1,n2) == 0)
        return 1;
    return 0;
}


int fn_has_suffix(const char *name)
{
    size_t l;
    size_t s;

    name = fn_skip_drive(name);
    l = strlen(name);
    if (l > 4 && name[l-4] == '.')
    {
        if (strcasecmp(&name[l-3],"lzo") == 0)
            return SUFF_LZO;
        if (strcasecmp(&name[l-3],"nrv") == 0)
            return SUFF_NRV;
        if (strcasecmp(&name[l-3],"tar") == 0)
            return SUFF_TAR;
        if (strcasecmp(&name[l-3],"tnv") == 0)
            return SUFF_TNV;
        if (strcasecmp(&name[l-3],"tzo") == 0)
            return SUFF_TZO;
    }

    if (l > 5 && name[l-5] == '.')
    {
        if (strcasecmp(&name[l-4],"lzop") == 0)
            return SUFF_LZOP;
    }

    s = strlen(opt_suffix);
    if (s > 0 && l > s)
    {
        if (strcasecmp(&name[l-s],opt_suffix) == 0)
            return SUFF_USER;
    }

    return SUFF_NONE;
}


int fn_cleanpath(const char *name, char *newname, size_t size, int flags)
{
    size_t l = 0;
    size_t n = 0;
    int slashes = 0;
#define add(x)  if (l >= size) return -1; else newname[l++] = (x)

    name = fn_skip_drive(name);
    while (*name && fn_is_sep(*name))
        name++;
    while (name[n])
    {
        size_t last_n;
        assert(!fn_is_sep(name[n]));
        last_n = n++;
        while (name[n] && !fn_is_sep(name[n]))
            n++;
        if (n - last_n == 2 && name[n-2] == '.' && name[n-1] == '.')
        {
            if (flags & 1)
                return -2;
            while (l > 0) {
                if (newname[--l] == '/')
                {
                    --slashes;
                    break;
                }
            }
            /* newname[l] = 0; printf("del %s\n", newname); */
        }
        else if (n - last_n == 1 && name[n-1] == '.')
        {
            if (flags & 2)
                return -3;
        }
        else
        {
            if (l > 0)
                { ++slashes; add('/'); }
            while (last_n < n)
                { add(name[last_n++]); }
            /* newname[l] = 0; printf("add %s\n", newname); */
        }
        while (name[n] && fn_is_sep(name[n]))
            n++;
    }
    add('\0');
    return slashes;
#undef add
}


/*************************************************************************
// time util
**************************************************************************/

time_t fix_time(time_t t)
{
    if (t == (time_t) -1)
        t = 0;
    return t;
}

time_t get_mtime(const header_t *h)
{
    lzop_ulong_t t;
    t = h->mtime_high;
    t <<= 16; t <<= 16;
    t |= h->mtime_low;
    return t == (lzop_ulong_t)(time_t)t ? (time_t)t : (time_t)0;
}


#if defined(HAVE_LOCALTIME)
void tm2str(char *s, size_t size, const struct tm *tmp)
{
    assert(size >= 18);
#if defined(HAVE_SNPRINTF)
    snprintf(s, size, "%04d-%02d-%02d %02d:%02d:%02d",
#else
    sprintf(s, "%04d-%02d-%02d %02d:%02d:%02d",
#endif
            (int) tmp->tm_year + 1900, (int) tmp->tm_mon + 1,
            (int) tmp->tm_mday,
            (int) tmp->tm_hour, (int) tmp->tm_min, (int) tmp->tm_sec);
}
#endif


void time2str(char *s, size_t size, const time_t *t)
{
#if defined(HAVE_LOCALTIME)
    tm2str(s, size, localtime(t));
#elif defined(HAVE_CTIME)
    const char *p = ctime(t);
    assert(size >= 18);
    memset(s, ' ', 16);
    memcpy(s + 2, p + 4, 6);
    memcpy(s + 11, p + 11, 5);
    s[16] = 0;
#else
    s[0] = 0;
    UNUSED(size);
#endif
}


void time2ls(char *s, size_t size, const time_t *t)
{
#if defined(HAVE_LOCALTIME) && defined(HAVE_STRFTIME)
    const char *fmt = "%b %e  %Y";
#if defined(HAVE_DIFFTIME)
#  if (ACC_OS_DOS16)
    /* do not introduce any floating point dependencies */
    long d = (long)current_time - (long)*t;
#  else
    const double d = difftime(current_time, *t);
#  endif
    if (d <= 6 * 30 * 24 * 3600L && d >= -3600L)
        fmt = "%b %e %H:%M";
#endif
    assert(size >= 13);
    if (strftime(s, 13, fmt, localtime(t)) == 13)
        s[0] = 0;
#else
    s[0] = 0;
    UNUSED(size);
    UNUSED(t);
#endif
}


/*************************************************************************
//
**************************************************************************/

lzo_bool file_exists(const char *name)
{
    int fd, r;
    struct stat st;

    /* return true if we can open it */
    fd = open(name, O_RDONLY, 0);
    if (fd >= 0)
    {
        (void) close(fd);
        return 1;
    }

    /* return true if we can stat it */
    r = stat(name, &st);
    if (r != -1)
        return 1;

    /* return true if we can lstat it */
#if defined(HAVE_LSTAT)
    r = lstat(name, &st);
    if (r != -1)
        return 1;
#endif

    return 0;
}


/*************************************************************************
// Some systems have very exotic mode values.
// Convert them to standard values for portable use in our header.
**************************************************************************/

lzo_uint32 fix_mode_for_header(lzo_uint32 mode)
{
    lzo_uint32 m = mode;

    if (mode == 0)
        return 0;

    /* This function can only deal with S_ISREG and S_ISDIR modes. */
    assert(S_ISREG(mode) || S_ISDIR(mode));

#if defined(ACC_OS_TOS) && defined(__PUREC__)
    m = 0444;
    if (mode & S_IWRITE)
        m |= 0200;
    if (mode & S_IEXEC)
        m |= 0111;
    if (S_ISREG(mode))
        m |= 0100000 | 0400;
    else if (S_ISDIR(mode))
        m |= 0040000 | 0700;
#elif defined(DOSISH)
    m &= 0777;
    if (S_ISREG(mode))
        m |= 0100000 | 0400;
    else if (S_ISDIR(mode))
        m |= 0040000 | 0700;
#else
    m &= 07777;
    if (S_ISREG(mode))
        m |= 0100000;
    else if (S_ISDIR(mode))
        m |= 0040000 | 0700;
#endif

    if ((m & 0777) == 0)
        m |= (0644 & ~u_mask);
    return m;
}


MODE_T fix_mode_for_chmod(lzo_uint32 mode)
{
    MODE_T m = (MODE_T) (mode & 07777);
    if ((m & 0777) == 0)
        m |= (MODE_T) (0644 & ~u_mask);
    return m;
}


MODE_T fix_mode_for_ls(lzo_uint32 mode)
{
    MODE_T m = (MODE_T) mode;
    if ((m & 0777) == 0)
        m |= (MODE_T) (0644 & ~u_mask);
    return m;
}


MODE_T fix_mode_for_open(MODE_T mode)
{
    MODE_T m = (MODE_T) (mode & 0666);
#if defined(ACC_OS_TOS) && defined(__PUREC__)
    m = S_IWRITE | S_IREAD;
#else
    if ((m & 0777) == 0)
        m |= (MODE_T) (0644 & ~u_mask);
    m |= 0600;
#endif
    return m;
}


/*************************************************************************
// ls util - adapted from GNU fileutils 3.16
**************************************************************************/

/* Return a character indicating the type of file described by
   file mode BITS:
   'd' for directories
   'b' for block special files
   'c' for character special files
   'm' for multiplexor files
   'l' for symbolic links
   's' for sockets
   'p' for fifos
   '-' for regular files
   '?' for any other file type.  */

static char ftypelet(unsigned mode)
{
    if (S_ISREG(mode)) return '-';
    if (S_ISDIR(mode)) return 'd';
#ifdef S_ISBLK
    if (S_ISBLK(mode)) return 'b';
#endif
#ifdef S_ISCHR
    if (S_ISCHR(mode)) return 'c';
#endif
#ifdef S_ISFIFO
    if (S_ISFIFO(mode)) return 'p';
#endif
#ifdef S_ISLNK
    if (S_ISLNK(mode)) return 'l';
#endif
#ifdef S_ISSOCK
    if (S_ISSOCK(mode)) return 's';
#endif
#ifdef S_ISMPC
    if (S_ISMPC(mode)) return 'm';
#endif
#ifdef S_ISNWK
    if (S_ISNWK(mode)) return 'n';
#endif
#ifdef S_ISOFD
    if (S_ISOFD(mode)) return 'M';     /* Cray migrated dmf file.  */
#endif
#ifdef S_ISOFL
    if (S_ISOFL(mode)) return 'M';     /* Cray migrated dmf file.  */
#endif
    return '?';
}


/* Set the read, write, and execute flags. */
static void set_rwx(unsigned mode, char *str)
{
    str[0] = (char) ((mode & 4) ? 'r' : '-');
    str[1] = (char) ((mode & 2) ? 'w' : '-');
    str[2] = (char) ((mode & 1) ? 'x' : '-');
}


/* Set the 's' and 't' flags in file attributes string. */
static void set_st(unsigned mode, char *str)
{
#ifdef S_ISUID
    if (mode & S_ISUID)
        str[3] = (char) (str[3] == 'x' ? 's' : 'S');
#endif
#ifdef S_ISGID
    if (mode & S_ISGID)
        str[6] = (char) (str[6] == 'x' ? 's' : 'S');
#endif
#ifdef S_ISVTX
    if (mode & S_ISVTX)
        str[9] = (char) (str[9] == 'x' ? 't' : 'T');
#endif
    UNUSED(mode);
    UNUSED(str);
}


void mode_string(MODE_T m, char *str)
{
    unsigned mode = (unsigned) m;
    str[0] = ftypelet(mode);
    set_rwx((mode & 0700) >> 6, &str[1]);
    set_rwx((mode & 0070) >> 3, &str[4]);
    set_rwx((mode & 0007) >> 0, &str[7]);
    set_st(mode, str);
}


/*************************************************************************
// adapted from the djgpp port of cpio 2.4.2 by Eli Zaretskii
**************************************************************************/

#if defined(DOSISH)

/* If the original file name includes characters illegal for MS-DOS and
   MS-Windows, massage it to make it suitable and return a pointer to
   static storage with a new name.  If the original name is legit,
   return it instead.  Return NULL if the rename failed (shouldn't happen).

   The file name changes are: (1) any name that is reserved by a DOS
   device driver (such as 'prn.txt' or 'aux.c') is prepended with a '_';
   and (2) illegal characters are replaced with '_' or '-' (well, almost;
   look at the code below for details).  */

#define is_slash(x)     fn_is_sep(x)

char *maybe_rename_file(const char *original_name)
{
    static char dosified_name[PATH_MAX+1];
    static const char illegal_chars_dos[] = ".+, ;=[]|<>\":?*";
    const char *illegal_chars = illegal_chars_dos;
    int idx, dot_idx;
    const char *s = original_name;
    char *d = dosified_name;

    /* Support for Win32 VFAT systems, when available.  */
#if defined(__DJGPP__)
    if (_use_lfn(original_name))
        illegal_chars = illegal_chars_dos + 8;
#elif (ACC_OS_WIN32 || ACC_OS_WIN64 || ACC_OS_CYGWIN)
    illegal_chars = illegal_chars_dos + 8;
#endif

    /* Get past the drive letter, if any. */
    if (fn_is_drive(s))
    {
        *d++ = *s++;
        *d++ = *s++;
    }

    for (idx = 0, dot_idx = -1; *s; s++, d++)
    {
        if (strchr(illegal_chars, *s))
        {
            /* Dots are special on DOS: it doesn't allow them as the leading
             character, and a file name cannot have more than a single dot.
             We leave the first non-leading dot alone, unless it comes too
             close to the beginning of the name: we want sh.lex.c to become
             sh_lex.c, not sh.lex-c.  */
            if (*s == '.')
            {
                if (idx == 0
                    && (is_slash(s[1]) || s[1] == '\0'
                        || (s[1] == '.' && (is_slash(s[2]) || s[2] == '\0'))))
                {
                    /* Copy "./" and "../" verbatim.  */
                    *d++ = *s++;
                    if (*s == '.')
                        *d++ = *s++;
                    *d = *s;
                }
                else if (idx == 0)
                    *d = '_';
                else if (dot_idx >= 0)
                {
                    if (dot_idx < 5) /* 5 is merely a heuristic ad-hoc'ery */
                    {
                        d[dot_idx - idx] = '_'; /* replace previous dot */
                        *d = '.';
                    }
                    else
                        *d = '-';
                }
                else
                    *d = '.';

                if (*s == '.')
                    dot_idx = idx;
            }
            else if (*s == '+' && s[1] == '+')
            {
                if (idx - 2 == dot_idx) /* .c++, .h++ etc. */
                {
                    *d++ = 'x';
                    *d   = 'x';
                }
                else
                {
                    /* libg++ etc.  */
                    memcpy (d, "plus", 4);
                    d += 3;
                }
                s++;
                idx++;
            }
            else
                *d = '_';
        }
        else
            *d = *s;
        if (is_slash(*s))
        {
            idx = 0;
            dot_idx = -1;
        }
        else
            idx++;
    }

    *d = '\0';

#if defined(S_ISCHR)
    /* We could have a file in an archive whose name is reserved
     on MS-DOS by a device driver.  Trying to extract such a
     file would fail at best and wedge us at worst.  We need to
     rename such files.  */

    if (idx > 0)
    {
        struct stat st_buf;
        char *base = d - idx;
        int i = 0;

        /* The list of character devices is not constant: it depends on
         what device drivers did they install in their CONFIG.SYS.
         'stat' will tell us if the basename of the file name is a
         characer device.  */
        while (stat(base, &st_buf) == 0 && S_ISCHR(st_buf.st_mode))
        {
            size_t blen = strlen(base);

            /* I don't believe any DOS character device names begin with a
             '_'.  But in case they invent such a device, let us try twice.  */
            if (++i > 2)
                return (char *)0;

            /* Prepend a '_'.  */
            memmove(base + 1, base, blen + 1);
            base[0] = '_';
        }
    }
#endif

    return dosified_name;
}

#endif /* DOSISH */


/*************************************************************************
//
**************************************************************************/

#if (ACC_CC_MSC && (_MSC_VER >= 1000 && _MSC_VER < 1200))
   /* avoid '-W4' warnings in <windows.h> */
#  pragma warning(disable: 4201 4214 4514)
#endif
#if (ACC_CC_MSC && (_MSC_VER >= 1300))
   /* avoid '-Wall' warnings in <windows.h> */
#  pragma warning(disable: 4255)
#endif

#define ACC_WANT_ACC_INCI_H 1
#define ACC_WANT_ACCLIB_GETOPT 1
#define ACC_WANT_ACCLIB_HALLOC 1
#define ACC_WANT_ACCLIB_HMEMCPY 1
#define ACC_WANT_ACCLIB_HSREAD 1
#define ACC_WANT_ACCLIB_MISC 1
#define ACC_WANT_ACCLIB_WILDARGV 1
#if (ACC_HAVE_MM_HUGE_PTR)
#  define ACC_WANT_ACCLIB_HREAD 1
#endif
#include "miniacc.h"
#undef ACC_WANT_ACC_INCI_H
#undef ACC_WANT_ACCLIB_HALLOC
#undef ACC_WANT_ACCLIB_MISC
#undef ACC_WANT_ACCLIB_WILDARGV
#undef ACC_WANT_ACCLIB_HREAD

#if (__ACCLIB_REQUIRE_HMEMCPY_CH) && !defined(__ACCLIB_HMEMCPY_CH_INCLUDED)
#  define ACC_WANT_ACCLIB_HMEMCPY 1
#  include "acc/acclib/hmemcpy.ch"
#  undef ACC_WANT_ACCLIB_HMEMCPY
#endif


/*
vi:ts=4:et
*/

