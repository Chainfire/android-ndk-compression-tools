/* s_object.c --

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

#if defined(USE_SCREEN)

#define this local_this

#include "screen.h"


/*************************************************************************
//
**************************************************************************/

void sobject_destroy(screen_t *this)
{
    if (!this)
        return;
    if (this->data)
    {
        if (this->finalize)
            this->finalize(this);
        free(this->data);
        this->data = NULL;
    }
    free(this);
}


screen_t *sobject_construct(const screen_t *c, size_t data_size)
{
    screen_t *this;

    /* allocate object */
    this = (screen_t *) malloc(sizeof(*this));
    if (!this)
        return NULL;

    /* copy function table */
    *this = *c;

    /* initialize instance variables */
    this->data = (struct screen_data_t *) malloc(data_size);
    if (!this->data)
    {
        free(this);
        return NULL;
    }
    memset(this->data,0,data_size);

    return this;
}

#endif /* defined(USE_SCREEN) */


/*
vi:ts=4:et
*/

