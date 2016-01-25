/* s_djgpp2.c --

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

#if defined(USE_SCREEN) && defined(__DJGPP__)

#include "screen.h"

#define this local_this

#define mask_fg 0x0f
#define mask_bg 0xf0


/*************************************************************************
// direct screen access
**************************************************************************/

#include <dos.h>
#if 0
#include <conio.h>
#endif
#include <dpmi.h>
#include <go32.h>
#include <sys/exceptn.h>
#include <sys/farptr.h>
#include <sys/movedata.h>
#define dossel  _go32_info_block.selector_for_linear_memory
#define co80    _go32_info_block.linear_address_of_primary_screen
#undef kbhit


struct screen_data_t
{
    int cols;
    int rows;
    int cursor_x;
    int cursor_y;
    unsigned char attr;
    unsigned char init_attr;
    unsigned char empty_attr;
    unsigned short empty_cell;
};



static void refresh(screen_t *this)
{
    UNUSED(this);
}


static __inline__
unsigned short make_cell(screen_t *this, int ch, int attr)
{
    UNUSED(this);
    return ((attr & 0xff) << 8) | (ch & 0xff);
}


static int getMode(const screen_t *this)
{
    UNUSED(this);
    return ScreenMode();
}


static int getPage(const screen_t *this)
{
    UNUSED(this);
    return _farpeekb(dossel, 0x462);
}


static int getRows(const screen_t *this)
{
    return this->data->rows;
}


static int getCols(const screen_t *this)
{
    return this->data->cols;
}


static int getFg(const screen_t *this)
{
    return this->data->attr & mask_fg;
}


static int getBg(const screen_t *this)
{
    return this->data->attr & mask_bg;
}


static void setFg(screen_t *this, int fg)
{
    this->data->attr = (this->data->attr & mask_bg) | (fg & mask_fg);
}


static void setBg(screen_t *this, int bg)
{
    this->data->attr = (this->data->attr & mask_fg) | (bg & mask_bg);
}


static void setCursor(screen_t *this, int x, int y)
{
    if (x >= 0 && y >= 0 && x < this->data->cols && y < this->data->rows)
    {
        ScreenSetCursor(y,x);
        this->data->cursor_x = x;
        this->data->cursor_y = y;
    }
}


static void getCursor(const screen_t *this, int *x, int *y)
{
    if (x)
        *x = this->data->cursor_x;
    if (y)
        *y = this->data->cursor_y;
}


static void putCharAttr(screen_t *this, int ch, int attr, int x, int y)
{
    UNUSED(this);
    ScreenPutChar(ch,attr,x,y);
}


static void putChar(screen_t *this, int ch, int x, int y)
{
    ScreenPutChar(ch,this->data->attr,x,y);
}


static void putStringAttr(screen_t *this, const char *s, int attr, int x, int y)
{
    UNUSED(this);
    ScreenPutString(s,attr,x,y);
}


static void putString(screen_t *this, const char *s, int x, int y)
{
    ScreenPutString(s,this->data->attr,x,y);
}


/* private */
static void getChar(screen_t *this, int *ch, int *attr, int x, int y)
{
    UNUSED(this);
    ScreenGetChar(ch,attr,x,y);
}


static int init(screen_t *this, int fd)
{
    int mode;
    int attr;

#if 0
    /* force linkage of conio.o */
    (void) _conio_kbhit();
#endif

    if (!this || !this->data)
        return -1;
    if (fd < 0 || !acc_isatty(fd))
        return -1;

    mode = getMode(this);
    if (mode != 2 && mode != 3 && mode != 7)
        return -1;
    if (getPage(this) != 0)
        return -1;
    this->data->rows = ScreenRows();
    this->data->cols = ScreenCols();
    ScreenGetCursor(&this->data->cursor_y,&this->data->cursor_x);
    getChar(this,NULL,&attr,this->data->cursor_x,this->data->cursor_y);
    this->data->init_attr = attr;
    if (mode == 2 || mode == 3)
    {
        /* Does it normally blink when bg has its 3rd bit set?  */
        int b_mask = (_farpeekb(dossel, 0x465) & 0x20) ? 0x70 : 0xf0;
        attr = attr & (0x0f | b_mask);
    }
    this->data->attr = attr;
    this->data->empty_attr = attr;
    this->data->empty_cell = make_cell(this,' ',attr);

    return 0;
}


static void updateLineN(screen_t *this, const void *line, int y, int len)
{
    if (y >= 0 && y < this->data->rows && len > 0 && len <= 2*this->data->cols)
        movedata(_my_ds(),(unsigned)line,dossel,co80+y*this->data->cols*2,len);
}


static void clearLine(screen_t *this, int y)
{
    if (y >= 0 && y < this->data->rows)
    {
        unsigned sp = co80 + y * this->data->cols * 2;
        unsigned short a = this->data->empty_cell;
        int i = this->data->cols;

        _farsetsel(dossel);
        do {
            _farnspokew(sp, a);
            sp += 2;
        } while (--i);
    }
}


static void clear(screen_t *this)
{
    unsigned char attr = ScreenAttrib;
    ScreenAttrib = this->data->empty_attr;
    ScreenClear();
    ScreenAttrib = attr;
}


static int scrollUp(screen_t *this, int lines)
{
    int sr = this->data->rows;
    int sc = this->data->cols;
    int y;

    if (lines <= 0)
        return 0;
    if (lines >= sr)
        clear(this);
    else
    {
        movedata(dossel,co80+lines*sc*2,dossel,co80,(sr-lines)*sc*2);
        for (y = sr - lines; y < sr; y++)
            clearLine(this,y);
    }
    return lines;
}


static int getCursorShape(const screen_t *this)
{
    UNUSED(this);
    return _farpeekw(dossel, 0x460);
}


static void setCursorShape(screen_t *this, int shape)
{
    __dpmi_regs r;

    r.h.ah = 0x01;
    r.h.al = getMode(this);
    r.x.cx = shape & 0x7f1f;
    __dpmi_int(0x10, &r);
}


static int s_kbhit(screen_t *this)
{
    UNUSED(this);
    return kbhit();
}


static int intro(screen_t *this, void (*show_frames)(screen_t *) )
{
    int shape;
    unsigned short old_flags = __djgpp_hwint_flags;

    if ((this->data->init_attr & mask_bg) != BG_BLACK)
        return 0;

    __djgpp_hwint_flags |= 3;
    while (kbhit())
        (void) getkey();

    shape = getCursorShape(this);
    setCursorShape(this,0x2000);
    show_frames(this);
    setCursorShape(this,shape);

    while (kbhit())
        (void) getkey();
    __djgpp_hwint_flags = old_flags;

    return 1;
}


static const screen_t driver =
{
    sobject_destroy,
    0,                  /* finalize, */
    init,
    refresh,
    getMode,
    getPage,
    getRows,
    getCols,
    getFg,
    getBg,
    getCursor,
    getCursorShape,
    setFg,
    setBg,
    setCursor,
    setCursorShape,
    putChar,
    putCharAttr,
    putString,
    putStringAttr,
    clear,
    clearLine,
    updateLineN,
    scrollUp,
    s_kbhit,
    intro,
    (struct screen_data_t *) 0
};


/* public constructor */
screen_t *screen_djgpp2_construct(void)
{
    return sobject_construct(&driver,sizeof(*driver.data));
}


#endif /* defined(USE_SCREEN) && defined(__DJGPP__) */


/*
vi:ts=4:et
*/

