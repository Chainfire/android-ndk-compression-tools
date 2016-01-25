/* help.c --

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
//
**************************************************************************/

static lzo_bool head_done = 0;

void head(void)
{
    FILE *f = con_term;
    int fg;

    if (head_done)
        return;
    head_done = 1;

    fg = con_fg(f,FG_GREEN);
    con_fprintf(f,
                "                          Lempel-Ziv-Oberhumer Packer\n"
                "                           Copyright (C) 1996 - 2010\n"
                "lzop v%-11s  Markus Franz Xaver Johannes Oberhumer  %20s\n"
                "\n",
                LZOP_VERSION_STRING, LZOP_VERSION_DATE);
    fg = con_fg(f,fg);
    UNUSED(fg);
}


/*************************************************************************
//
**************************************************************************/

void usage(void)
{
    FILE *f = con_term;

    con_fprintf(f,"Usage: %s [-dxlthIVL%s] [-qvcfFnNPkUp] [-o file] [-S suffix] [%sfile..]\n", progname,
#if defined(USE_LZO1X_1_15) && defined(USE_LZO1X_999)
                "19",
#elif defined(USE_LZO1X_1_15)
                "1",
#else
                "",
#endif
#if defined(__DJGPP__) || defined(__EMX__)
                "[@]");
#else
                "");
#endif
}


/*************************************************************************
//
**************************************************************************/

void help(void)
{
    FILE *f = con_term;
    int fg;

#ifdef OPT_NAME_DEFAULT
    const char *dn = "";
    const char *dN = " (default)";
#else
    const char *dn = " (default)";
    const char *dN = "";
#endif

    head();
    usage();

    con_fprintf(f,"\n");
    fg = con_fg(f,FG_YELLOW);
    con_fprintf(f,"Commands:\n");
    fg = con_fg(f,fg);
    con_fprintf(f,
#if defined(USE_LZO1X_1_15) && defined(USE_LZO1X_999)
"  -1     compress faster                   -9    compress better\n"
#elif defined(USE_LZO1X_1_15)
"  -1     compress faster\n"
#endif
"  -d     decompress                        -x    extract (same as -dPp)\n"
"  -l     list compressed file              -I    display system information\n"
"  -t     test compressed file              -V    display version number\n"
"  -h     give this help                    -L    display software license\n"
);

    fg = con_fg(f,FG_YELLOW);
    con_fprintf(f,"Options:\n");
    fg = con_fg(f,fg);

    con_fprintf(f,
"  -q     be quiet                          -v       be verbose\n"
"  -c     write on standard output          -oFILE   write output to 'FILE'\n"
"  -p     write output to current dir       -pDIR    write to path 'DIR'\n"
"  -f     force overwrite of output files\n"
"  -n     do not restore the original file name%s\n"
"  -N     restore the original file name%s\n"
"  -P     restore or save the original path and file name\n"
"  -S.suf use suffix .suf on compressed files\n"
#if 0
"  -F     do *not* store or verify checksum of files (a little bit faster)\n"
#endif
"  -U     delete input files after successful operation (like gzip and bzip2)\n"
"  file.. files to (de)compress. If none given, try standard input.\n"
, dn, dN);

    UNUSED(fg);
}


/*************************************************************************
//
**************************************************************************/

void license(void)
{
    FILE *f = con_term;

    head();

con_fprintf(f,
"   lzop and the LZO library are free software; you can redistribute them\n"
"   and/or modify them under the terms of the GNU General Public License as\n"
"   published by the Free Software Foundation; either version 2 of\n"
"   the License, or (at your option) any later version.\n"
"\n"
"   This program is distributed in the hope that it will be useful,\n"
"   but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
"   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
"   GNU General Public License for more details.\n"
"\n"
"   You should have received a copy of the GNU General Public License\n"
"   along with this program; see the file COPYING.\n"
"   If not, write to the Free Software Foundation, Inc.,\n"
"   59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.\n"
"\n"
"   Markus F.X.J. Oberhumer\n"
"   <markus@oberhumer.com>\n"
"   http://www.oberhumer.com/opensource/lzop/\n"
    );
}


/*************************************************************************
//
**************************************************************************/

#ifndef LZOP_BUILD_DATE_TIME
#define LZOP_BUILD_DATE_TIME __DATE__ " " __TIME__
#endif

const char lzop_rcsid[] =
    "\n$" "Id: lzop " LZOP_VERSION_STRING " built " LZOP_BUILD_DATE_TIME " $\n";

LZO_EXTERN(const lzo_bytep) lzo_copyright(void);

void version(void)
{
    FILE *f = con_term;
    char pp[2048];
    const lzo_bytep qq;
    char *p, *q, *s;
    size_t i;

    head();

    con_fprintf(f,
        "lzop version: v" LZOP_VERSION_STRING ", " LZOP_VERSION_DATE "\n"
        "lzop build date: " LZOP_BUILD_DATE_TIME "\n");

    for (i = 0, qq = lzo_copyright(); i < sizeof(pp)-1 && *qq; i++, qq++)
        pp[i] = (char) *qq;
    pp[i] = 0;
    p = strstr(pp,"LZO version");
    if (p == NULL)
        return;
    s = strchr(p,'$');
    if (s == NULL)
        return;
    for (q = s; q > p && q[-1] == '\n'; )
        *--q = 0;
    q = strchr(s+1,'$');
    if (q == NULL)
        return;
    q[1] = 0;
    con_fprintf(f,"\n%s\n",p);
    con_fprintf(f,"\n%s\n",s);
}


/*************************************************************************
//
**************************************************************************/

void sysinfo(void)
{
    FILE *f = con_term;
    int fg = 0;
    const char *env = NULL;

    head();

#if defined(HAVE_LOCALTIME) && defined(HAVE_GMTIME)
    {
        char s[40];
        time_t t;

        t = time(NULL);
        tm2str(s, sizeof(s), localtime(&t));
        con_fprintf(f,"Local time is:  %s\n",s);
        tm2str(s, sizeof(s), gmtime(&t));
        con_fprintf(f,"GMT time is:    %s\n\n",s);
    }
#endif

#if defined(OPTIONS_VAR)
    env = getenv(OPTIONS_VAR);
    if (env && env[0])
        con_fprintf(f,"Contents of environment variable %s: '%s'\n\n",
                       OPTIONS_VAR, env);
    else
        con_fprintf(f,"Environment variable '%s' is not set.\n\n",
                       OPTIONS_VAR);
#endif

#if defined(USE_FOPEN)
    con_fprintf(f,"This version uses stdio for opening files.\n\n");
#endif

#if defined(__DJGPP__)
    sysinfo_djgpp();
#endif

    UNUSED(env);
    UNUSED(fg);
}


/*
vi:ts=4:et
*/

