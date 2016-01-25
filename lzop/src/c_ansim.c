/* c_ansim.c --

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

#if defined(USE_ANSI)

/*************************************************************************
//
**************************************************************************/

static int cur_fg = -1;


static int init(FILE *f, int o, int now)
{
    UNUSED(f);
    UNUSED(o);
    UNUSED(now);
    return CON_ANSI_MONO;
}


static int set_fg(FILE *f, int fg)
{
    int last_fg = cur_fg;

    if (fg < 0)
        fputs("\033[0m",f);
    else
        /* just treat any color as intense */
        fputs("\033[0;1m",f);

    cur_fg = fg;
    return last_fg;
}


console_t console_ansi_mono =
{
    init,
    set_fg,
    0,
    0
};

#endif /* USE_ANSI */


/*
vi:ts=4:et
*/

