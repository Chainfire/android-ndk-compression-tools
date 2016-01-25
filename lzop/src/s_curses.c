/* s_curses.c --

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

#if defined(USE_SCREEN) && defined(USE_SCREEN_CURSES)

#include "screen.h"

#define this local_this

#define mask_fg 0x0f
#define mask_bg 0xf0


/*************************************************************************
// direct screen access
**************************************************************************/

#include <termios.h>
#include <ncurses.h>
#undef refresh
#undef clear


struct screen_data_t
{
    WINDOW *w;
    int mode;
    int page;
    int cols;
    int rows;
    int cursor_x;
    int cursor_y;
    unsigned char attr;
    unsigned char init_attr;
    chtype empty_cell;
};



static void s_refresh(screen_t *this)
{
    wrefresh(this->data->w);
}


static __inline__
chtype make_cell(screen_t *this, int ch, int attr)
{
    int fg = attr & mask_fg;
    chtype c = ch & 0xff;
    if (fg < 8)
        c |= COLOR_PAIR((fg + 0));
    else
        c |= COLOR_PAIR(((fg & 7) + 0)) | A_BOLD;
    return c;
}


static int getMode(const screen_t *this)
{
    return this->data->mode;
}


static int getPage(const screen_t *this)
{
    return this->data->page;
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


/* private */
static __inline__ int gotoxy(screen_t *this, int x, int y)
{
    if (wmove(this->data->w, this->data->w->_begy+y,
                             this->data->w->_begx+x) != ERR)
        return 0;
    return -1;
}


static void setCursor(screen_t *this, int x, int y)
{
    if (gotoxy(this,x,y) == 0)
    {
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
    if (gotoxy(this,x,y) == 0)
        waddch(this->data->w,make_cell(this,ch,attr));
}


static void putChar(screen_t *this, int ch, int x, int y)
{
    putCharAttr(this,ch,this->data->attr,x,y);
}


static void putStringAttr(screen_t *this, const char *s, int attr, int x, int y)
{
    while (*s)
        putCharAttr(this,*s++,attr,x++,y);
}


static void putString(screen_t *this, const char *s, int x, int y)
{
    putStringAttr(this,s,this->data->attr,x,y);
}


static int init(screen_t *this, int fd)
{
    int fg, bg;

    if (!this || !this->data)
        return -1;

    this->data->mode = -1;
    this->data->page = 0;

    if (fd < 0 || !acc_isatty(fd))
        return -1;

    this->data->w = initscr();
    if (!this->data->w)
        return -1;
    (void) noecho();

    this->data->cols = this->data->w->_maxx - this->data->w->_begx + 1;
    this->data->rows = this->data->w->_maxy - this->data->w->_begy + 1;
    this->data->cursor_x = this->data->w->_curx;
    this->data->cursor_y = this->data->w->_cury;

    this->data->empty_cell = ' ';

    if (has_colors())
        start_color();
    fg = (this->data->w->_attrs);
    bg = (this->data->w->_bkgd);
    fg = PAIR_NUMBER(this->data->w->_attrs);
    bg = PAIR_NUMBER(this->data->w->_bkgd);
    if (has_colors() && COLORS >= 8 && COLOR_PAIRS >= 8)
        this->data->mode = 3;
    else
        this->data->mode = 7;

    if (this->data->mode == 3)
    {
        int i = 0;
        init_pair(i++, COLOR_BLACK,   bg);
        init_pair(i++, COLOR_BLUE,    bg);
        init_pair(i++, COLOR_GREEN,   bg);
        init_pair(i++, COLOR_CYAN,    bg);
        init_pair(i++, COLOR_RED,     bg);
        init_pair(i++, COLOR_MAGENTA, bg);
        init_pair(i++, COLOR_YELLOW,  bg);
        init_pair(i++, COLOR_WHITE,   bg);
    }

    this->data->attr = (bg << 4) | fg;
    this->data->attr = 0x07;

    return 0;
}


static void finalize(screen_t *this)
{
#if 0
    if (this->data->w)
        (void) endwin();
#endif
    UNUSED(this);
}


static void updateLineN(screen_t *this, const void *line, int y, int len)
{
    if (len > 0 && len <= 2*this->data->cols)
    {
        int x;
        const unsigned char *l = line;

        for (x = 0; x < len / 2; x++, l += 2)
        {
#if 1
            if ((l[1] & mask_bg) != BG_BLACK)
                putCharAttr(this,'#',l[1] >> 4,x,y);
            else
                putCharAttr(this,l[0],l[1],x,y);
#else
            putCharAttr(this,l[0],l[1],x,y);
#endif
        }
    }
}


static void clearLine(screen_t *this, int y)
{
    int x;

    for (x = 0; x < this->data->cols; x++)
        if (gotoxy(this,x,y) == 0)
            waddch(this->data->w,this->data->empty_cell);
}


static void s_clear(screen_t *this)
{
    int y;

    for (y = 0; y < this->data->rows; y++)
        clearLine(this,y);
}


static int scrollUp(screen_t *this, int lines)
{
    if (lines <= 0)
        return 0;
    if (lines >= this->data->rows)
        s_clear(this);
    else
        wscrl(this->data->w,lines);
    return lines;
}


static int getCursorShape(const screen_t *this)
{
    UNUSED(this);
    return 0;
}


static void setCursorShape(screen_t *this, int shape)
{
    UNUSED(this);
    UNUSED(shape);
}


static int kbhit(screen_t *this)
{
    const int fd = STDIN_FILENO;
    const unsigned long usec = 0;
    struct timeval tv;
    fd_set fds;

    UNUSED(this);
    FD_ZERO(&fds);
    FD_SET(fd, &fds);
    tv.tv_sec  = usec / 1000000;
    tv.tv_usec = usec % 1000000;
    return (select(fd + 1, &fds, NULL, NULL, &tv) > 0);
}


static int intro(screen_t *this, void (*show_frames)(screen_t *) )
{
    int shape;
    struct termios term_old, term_new;
    int term_r;

    term_r = tcgetattr(STDIN_FILENO, &term_old);
    if (term_r == 0)
    {
        term_new = term_old;
        term_new.c_lflag &= ~(ISIG | ICANON | ECHO);
        tcsetattr(STDIN_FILENO, TCSANOW, &term_new);
    }

    shape = getCursorShape(this);
    setCursorShape(this,0x2000);
    show_frames(this);
    if (this->data->rows > 24)
        setCursor(this,this->data->cursor_x,this->data->cursor_y+1);
    setCursorShape(this,shape);
    s_refresh(this);

    if (term_r == 0)
        tcsetattr(STDIN_FILENO, TCSANOW, &term_old);

    return 1;
}


static const screen_t driver =
{
    sobject_destroy,
    finalize,
    init,
    s_refresh,
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
    s_clear,
    clearLine,
    updateLineN,
    scrollUp,
    kbhit,
    intro,
    (struct screen_data_t *) 0
};


/* public constructor */
screen_t *screen_curses_construct(void)
{
    return sobject_construct(&driver,sizeof(*driver.data));
}


#endif /* defined(USE_SCREEN) && defined(USE_SCREEN_CURSES) */


/*
vi:ts=4:et
*/

