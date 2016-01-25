/* filter.c --

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

#if (ACC_CC_MSC && (_MSC_VER >= 1000))
   /* avoid '-W4' warnings */
#  pragma warning(disable: 4244)
#endif


/*************************************************************************
//
**************************************************************************/

void t_sub1(lzo_bytep p, lzo_uint l)
{
    unsigned char b = 0;

    if (l > 0) do {
        *p -= b;
        b += *p++;
    } while (--l > 0);
}


void t_add1(lzo_bytep p, lzo_uint l)
{
    unsigned char b = 0;

    if (l > 0) do {
        b += *p;
        *p++ = b;
    } while (--l > 0);
}


/*************************************************************************
//
**************************************************************************/

void t_sub(lzo_bytep p, lzo_uint l, int n)
{
    unsigned char b[16];
    int i;

    assert(n > 0 && n <= (int)sizeof(b));
    if (l <= (lzo_uint)n)
        return;

    n--;
    i = n; do b[i] = 0; while (--i >= 0);

    i = n;
    do {
        *p -= b[i];
        b[i] += *p++;
        if (--i < 0)
            i = n;
    } while (--l > 0);
}


void t_add(lzo_bytep p, lzo_uint l, int n)
{
    unsigned char b[16];
    int i;

    assert(n > 0 && n <= (int)sizeof(b));
    if (l <= (lzo_uint)n)
        return;

    n--;
    i = n; do b[i] = 0; while (--i >= 0);

    i = n;
    do {
        b[i] += *p;
        *p++ = b[i];
        if (--i < 0)
            i = n;
    } while (--l > 0);
}


/*************************************************************************
//
**************************************************************************/

void t_mtf(lzo_bytep p, lzo_uint l)
{
    unsigned char b[256];
    unsigned char c;
    unsigned i;

    if (l <= 1)
        return;

    i = 256; do { --i; b[i] = (unsigned char) i; } while (i != 0);

    do {
        c = *p;
        for (i = 0; c != b[i]; )
            i++;
        *p++ = (unsigned char) i;
        if (i > 0)
        {
            do b[i] = b[i-1]; while (--i > 0);
            b[0] = c;
        }
    } while (--l > 0);
}


void t_unmtf(lzo_bytep p, lzo_uint l)
{
    unsigned char b[256];
    unsigned char c;
    unsigned i;

    if (l <= 1)
        return;

    i = 256; do { --i; b[i] = (unsigned char) i; } while (i != 0);

    do {
        i = *p;
        c = b[i];
        *p++ = c;
        if (i > 0)
        {
            do b[i] = b[i-1]; while (--i > 0);
            b[0] = c;
        }
    } while (--l > 0);
}


/*
vi:ts=4:et
*/

