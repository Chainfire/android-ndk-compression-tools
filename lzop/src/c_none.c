/* c_none.c --

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

#if defined(USE_CONSOLE)

/*************************************************************************
//
**************************************************************************/

static int init(FILE *f, int o, int now)
{
    UNUSED(f);
    UNUSED(o);
    UNUSED(now);
    return CON_NONE;
}


static int set_fg(FILE *f, int fg)
{
    UNUSED(f);
    UNUSED(fg);
    return -1;
}


static void print0(FILE *f, const char *s)
{
#if 1
    fputs(s,f);
#else
    /* filter out all ANSI sequences */
    int c;
    while ((c = *s++) != 0)
    {
        if (c == '\033' && *s == ']')
        {
            while (*s && *s != 'm')
                s++;
        }
        else
            fputc(c,f);
    }
#endif
}


static lzo_bool intro(FILE *f)
{
    UNUSED(f);
    return 0;
}


console_t console_none =
{
    init,
    set_fg,
    print0,
    intro
};

#endif /* USE_CONSOLE */


/*
vi:ts=4:et
*/

